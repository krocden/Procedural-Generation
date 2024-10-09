#include "WaterGen.h"

#include "../Shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

static inline float randf(float min, float max, int precision = 1000) {
    if (min > max) std::swap(min, max);
    float delta = max - min;
    auto i = int(delta * precision);
    return ((float)(rand() % i) / (float)precision) + min;
}
static inline int randi(int min, int max) {
    if (min > max) std::swap(min, max);
    return (rand() % (max - min)) + min;
}

WaterGen::WaterGen() {};
WaterGen::WaterGen(int waterSize, Shader* waterShader) {
    this->waterSize = waterSize;
    vertexCount = 3 * pow(waterSize, 2);
    indexCount = 6 * pow(waterSize, 2);
    waterVertices = new float[vertexCount];
    waterIndices = new int[indexCount];
    this->waterShader = waterShader;
}

void WaterGen::generateMap() {

    std::cout << "\n\n Generating Water Map ...";

    genWaterVertexBuffer(waterSize, waterVertices, waterIndices);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount, waterVertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, waterIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glm::vec2 windDirection = glm::vec2(0.85f, 0.30f);
    GerstnerWave waves[30];
    setGersterWaveData(*waterShader, windDirection, waveCount, waves);
    for (int i = 0; i < waveMapCount; ++i) {
        waveMaps[i] = genGersterWaveTexture(windDirection);
        waterShader->use();
        waterShader->setInt("waveMaps[" + std::to_string(i) + "]", i);
    }

    std::cout << "\n\n Generating Water Map Complete ";
}

void WaterGen::genWaterVertexBuffer(int width, float *vertices, int *indices) {
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < width; ++j) {
            vertices[3 * (i * width + j)    ] = (i - width / 2.0f) / 10.0f;
            vertices[3 * (i * width + j) + 1] = 0;
            vertices[3 * (i * width + j) + 2] = (j - width / 2.0f) / 10.0f;
        }
    }
    for (int i = 0; i < width - 1; ++i) {
        for (int j = 0; j < width - 1; ++j) {
            indices[6 * (i * width + j)    ] = (i * width + j);
            indices[6 * (i * width + j) + 1] = (i * width + j + 1);
            indices[6 * (i * width + j) + 2] = ((i + 1) * width + j);
            indices[6 * (i * width + j) + 3] = (i * width + j + 1);
            indices[6 * (i * width + j) + 4] = ((i + 1) * width + j);
            indices[6 * (i * width + j) + 5] = ((i + 1) * width + j + 1);
        }
    }
}

void WaterGen::setGersterWaveData(Shader &shader, glm::vec2 windDirection, int waveCount, GerstnerWave *waves) {

    int r = rand() % 100;
    windDirection = glm::vec2(cosf(r), sinf(r));
    
    shader.use();
    shader.setInt("waveCount", waveCount);
    for (int i = 0; i < waveCount; ++i) {
        waves[i].A = randf(0.01f, 0.05f);
        shader.setFloat("waves[" + std::to_string(i) + "].A", waves[i].A);

        waves[i].Q = randf(0.3f, 0.4f);
        shader.setFloat("waves[" + std::to_string(i) + "].Q", waves[i].Q);

        float windAngle = acosf((windDirection.x/sqrtf(windDirection.x*windDirection.x + windDirection.y*windDirection.y)));
        if (windDirection.y < 0) windAngle = - windAngle;
        float waveAngle = randf(windAngle - glm::radians(60.0f),
                                windAngle + glm::radians(60.0f));
        waves[i].D.x = cos(waveAngle);
        waves[i].D.y = sin(waveAngle);
        shader.setVec2("waves[" + std::to_string(i) + "].D", waves[i].D);

        waves[i].s = randf(0.5f, 1.0f);
        shader.setFloat("waves[" + std::to_string(i) + "].s", waves[i].s);

        waves[i].l = waves[i].A * randf(30.0f, 60.0f);
        shader.setFloat("waves[" + std::to_string(i) + "].l", waves[i].l);
    }
}

int WaterGen::genGersterWaveTexture(glm::vec2 windDirection) {

    GerstnerWave wave;
    wave.A = randf(0.01f, 0.02f);
    wave.Q = randf(0.3f, 0.4f);
    float windAngle = acos((windDirection.x/sqrt(windDirection.x*windDirection.x + windDirection.y*windDirection.y)));
    if (windDirection.y < 0) windAngle = -windAngle;
    float waveAngle = randf(windAngle - glm::radians(45.0f), windAngle + glm::radians(45.0f));
    wave.D.x = cos(waveAngle);
    wave.D.y = sin(waveAngle);
    wave.s = randf(0.5f, 1.0f);
    wave.l = ((float)randi(4, 32) / n);

    auto *data = new float[3 * n * n];
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
            float time = (float) glfwGetTime();
            float u = (float) j / n, v = (float) i / n;
            float w = 2 * 3.14f / wave.l;
            float fi = 2 * 3.14f * wave.s / wave.l;

            normal.x += (-wave.D.x) * w * wave.A * cos(glm::dot(wave.D, glm::vec2(u, v)) * w + fi * time);
            normal.y -= wave.Q * w * wave.A * sin(dot(wave.D, glm::vec2(u, v)) * w + fi * time);
            normal.z += (-wave.D.y) * w * wave.A * cos(glm::dot(wave.D, glm::vec2(u, v)) * w + fi * time);

            data[3 * (i * n + j) + 0] = normal.x;
            data[3 * (i * n + j) + 1] = normal.y;
            data[3 * (i * n + j) + 2] = normal.z;
        }
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, n, n, 0, GL_RGB, GL_FLOAT, data);
    delete[] data;
    return texture;
}