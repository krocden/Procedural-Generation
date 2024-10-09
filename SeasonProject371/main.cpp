//#include <cmath>
//#include <string>
//#include <random>
#include <iostream>
#include <math.h>
//#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"

#include "generator/TerrainGen.h"
#include "generator/WaterGen.h"

GLint SCREEN_WIDTH = 1024;
GLint SCREEN_HEIGHT = 800;

void debug_gl(int place) {
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        cout << "\n Error found at line " << place << " . Error code: " << e;
    }
}

int initializeOpenGL();
void processInput(GLFWwindow *window, Shader &terrainShader, Shader &waterShader);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow *window, double positionX, double positionY);
void scrollCallback(GLFWwindow* window, double offsetX, double offsetY);
void renderTerrain(std::vector<GLuint> &map, Shader &shader, glm::mat4 &view, glm::mat4 &model, glm::mat4 &projection);
void renderWater(WaterGen &waterGen, Shader &shader, glm::mat4 &view, glm::mat4 &model, glm::mat4 &projection);

GLFWwindow *window;
Camera camera(glm::vec3(0.0f, 25.0f, 0.0f));
glm::vec3 origin(0.0f, 0.0f, 0.0f);

int mapSize = 414;

double previousTime = glfwGetTime();
int frameCount = 0;

float globalDeltaTime = 0.0f;
float globalLastFrame = 0.0f;
float currentFrame;

int main() {

    initializeOpenGL();
    
    Shader terrainShader("assets/tVertexShader.txt", "assets/tFragmentShader.txt");
    Shader waterShader("assets/wVertexShader.txt", "assets/wFragmentShader.txt");

    float amplitude = 30;

    TerrainGen terrainGen(mapSize, amplitude);
    terrainGen.generateMap();
    
    int waterSize = 4096;

    WaterGen waterGen(waterSize, &waterShader);
    waterGen.generateMap();

    cout << "\n\n";

    srand(time(NULL));

    while (!glfwWindowShouldClose(window)) {

        currentFrame = (float) glfwGetTime();
        globalDeltaTime = currentFrame - globalLastFrame;
        globalLastFrame = currentFrame;

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, terrainShader, waterShader);

        glm::mat4 viewMatrix = camera.GetViewMatrix();
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, (float)pow(mapSize, 3));
        glm::mat4 terrainModel = glm::mat4(1.0f);
        glm::mat4 waterModel = glm::mat4(1.0f);
        
        terrainShader.use();
        terrainShader.setVec3("ambient", 0.40, 0.40, 0.40);
        terrainShader.setVec3("diffuse", 0.60, 0.60, 0.60);
        terrainShader.setVec3("specular", 1.0, 1.0, 1.0);
        terrainShader.setVec3("incLightDirection", 0.83f, -1.0f, 0.17f);
        terrainShader.setMat4("projection", projectionMatrix);
        terrainShader.setMat4("view", viewMatrix);
        terrainShader.setVec3("viewPosition", camera.position);

        renderTerrain(terrainGen.map, terrainShader, viewMatrix, terrainModel, projectionMatrix);
        
        waterShader.use();
        waterShader.setMat4("projection", projectionMatrix);
        waterShader.setMat4("view", viewMatrix);
        waterShader.setMat4("model", waterModel);
        waterShader.setFloat("currentTime", (float)glfwGetTime());

        renderWater(waterGen, waterShader, viewMatrix, terrainModel, projectionMatrix);

        frameCount++;
        auto currentTime = glfwGetTime();
        if (currentTime - previousTime >= 1.0 ){
            cerr << "\r Current FPS : " << frameCount << ' ' <<  flush;
            frameCount = 0;
            previousTime = currentTime;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    
    return 0;
}

void renderWater(WaterGen &waterGen, Shader &waterShader, glm::mat4 &view, glm::mat4 &model, glm::mat4 &projection) {

    waterShader.setVec3("viewPosition", camera.position);
    waterShader.setVec3("waterColor", glm::vec3(0.10f, 0.35f, 0.60f));
    waterShader.setVec3("frozenWaterColor", glm::vec3(0.41f, 0.99f, 0.89f));
    waterShader.setVec3("lightDirection", glm::vec3(0.83f, -1.0f, 0.17f));
    waterShader.setInt("waveMapCount", waterGen.waveMapCount);

    for (int i = 0; i < waterGen.waveMapCount; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, waterGen.waveMaps[i]);
    }

    glBindVertexArray(waterGen.VAO);
    glDrawElements(GL_TRIANGLES, waterGen.indexCount, GL_UNSIGNED_INT, nullptr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void renderTerrain(std::vector<GLuint> &map, Shader &terrainShader, glm::mat4 &view, glm::mat4 &model, glm::mat4 &projection) {

    auto terrainNormalIndices = 6 * pow(mapSize, 2);
    
    float distanceFromOrigin = (float) mapSize / 2.0f;
    float halfCellSize = (float) mapSize / 2.0f;

    float x = origin.x - (distanceFromOrigin);
    float y = origin.y - 10.0f;
    float z = origin.z - (distanceFromOrigin);

    glm::vec3 translationMatrix(x, y, z);
    model = glm::mat4(1.0f);
    model = glm::translate(model, translationMatrix);
    terrainShader.setMat4("model", model);
    glBindVertexArray(map[0]);
    glDrawElements(GL_TRIANGLES, terrainNormalIndices, GL_UNSIGNED_INT, 0);
}

int initializeOpenGL() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Season 371", NULL, NULL);
    
    if (window == NULL) {
        std::cout << "  Failed to create GLFW window ..." << std::endl;
        glfwTerminate();
        return 0;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "  Failed to initialize GLAD ..." << std::endl;
        return 0;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_FRAMEBUFFER_SRGB);
    
    return 0;
}

void processInput(GLFWwindow *window, Shader &terrainShader, Shader &waterShader) {
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        terrainShader.use();
        terrainShader.setBool("isFlat", true);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        terrainShader.use();
        terrainShader.setBool("isFlat", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        waterShader.use();
        waterShader.setBool("isFrozen", true);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        waterShader.use();
        waterShader.setBool("isFrozen", false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeyboard(FORWARD, globalDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(BACKWARD, globalDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(LEFT, globalDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(RIGHT, globalDeltaTime);
    }
}

void mouseCallback(GLFWwindow *window, double positionX, double positionY) {
    
    static bool first = true;
    static float mouseX = 500.0f;
    static float mouseY = 500.0f;
    
    if (first) {
        mouseX = positionX;
        mouseY = positionY;
        first = false;
    }
    
    float offsetX = positionX - mouseX;
    float offsetY = -(positionY - mouseY);
    
    mouseX = positionX;
    mouseY = positionY;

    camera.processMouseMovement(offsetX, offsetY);
}

void scrollCallback(GLFWwindow* window, double offsetX, double offsetY) {
    camera.processMouseScroll(offsetY);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
