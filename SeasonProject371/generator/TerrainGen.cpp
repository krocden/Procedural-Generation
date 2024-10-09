#include "TerrainGen.h"
#include "PerlinNoise.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

using namespace std;

TerrainGen::TerrainGen(int mapSize, float mapAmplitude) {
    this->mapSize = mapSize;
    this->mapAmplitude = mapAmplitude;

    vector<GLuint> map(pow(mapSize, 2));
    this->map = map;
}

vector<int> TerrainGen::generateIndices() {

    vector<int> indices;
    for (int i = 0; i < mapSize; i++) {
        for (int j = 0; j < mapSize; j++) {

            int position = i + j * mapSize;
            int shared = mapSize - 1;

            if (i == shared || j == shared) {
                continue;
            } else {
                indices.push_back(position + mapSize);
                indices.push_back(position);
                indices.push_back((position + mapSize) + 1);
                indices.push_back(position + 1);
                indices.push_back((position + mapSize) + 1);
                indices.push_back(position);
            }
        }
    }

    return indices;
}
vector<float> TerrainGen::generateNoiseMap(int offsetX, int offsetY) {

    int octaves = 6;
    float amplitude = 24.0;
    float scale = 64.0;
    float persistence = 0.4;
    float lacunarity = 2.0;

    PerlinNoise noise(mapSize, octaves, amplitude, scale, persistence, lacunarity);
    vector<float> noiseMap = noise.generate(offsetX, offsetY);
    
    return noiseMap;
}
vector<float> TerrainGen::generateVertices(vector<float> &noiseMap) {

    vector<float> vertices;
    
    for (int i = 0; i < mapSize; i++) {
        for (int j = 0; j < mapSize; j++) {
            float easedNoise = std::pow(noiseMap[j + (i * mapSize)] * 1.15, 2);
            vertices.push_back(j);
            vertices.push_back(easedNoise * mapAmplitude);
            vertices.push_back(i);
        }
    }

    return vertices;
}
vector<float> TerrainGen::generateNormals(vector<float> &vertices, vector<int> &indices) {

    glm::vec3 currentNormal;
    vector<float> normals;
    vector<glm::vec3> vertice;
    int position;
    
    for (int i = 0; i < indices.size(); i += 3) {
        for (int j = 0; j < 3; j++) {
            position = indices[i + j] * 3;
            glm::vec3 currentVertice = glm::vec3(vertices[position], vertices[position + 1], vertices[position + 2]);
            vertice.push_back(currentVertice);
        }
        
        glm::vec3 v1 = vertice[i + 1] - vertice[i];
        glm::vec3 v2 = vertice[i + 2] - vertice[i];
        
        currentNormal = - (glm::normalize(glm::cross(v1, v2)));
        normals.push_back(currentNormal.x);
        normals.push_back(currentNormal.y);
        normals.push_back(currentNormal.z);
    }
    
    return normals;
}

vector<float> TerrainGen::generateColors(vector<float> &vertices, int offsetX, int offsetY) {
    
    glm::vec3 currentColor;
    vector<float> finalColor;
    vector<Color> colorList;
    
    colorList.push_back(Color(0.30, glm::vec3(0.54f, 0.45f, 0.30f)));
    colorList.push_back(Color(0.33, glm::vec3(0.78f, 0.70f, 0.57f)));
    colorList.push_back(Color(0.36, glm::vec3(0.88f, 0.82f, 0.56f)));
    colorList.push_back(Color(0.39, glm::vec3(0.91f, 0.87f, 0.61f)));
    colorList.push_back(Color(0.43, glm::vec3(0.94f, 0.89f, 0.49f))); 
    colorList.push_back(Color(0.50, glm::vec3(0.98f, 0.93f, 0.42f))); 
    colorList.push_back(Color(0.54, glm::vec3(0.69f, 0.75f, 0.32f))); 
    colorList.push_back(Color(0.61, glm::vec3(0.50f, 0.80f, 0.25f))); 
    colorList.push_back(Color(0.83, glm::vec3(0.28f, 0.49f, 0.10f))); 
    colorList.push_back(Color(0.87, glm::vec3(0.13f, 0.25f, 0.03f)));
    colorList.push_back(Color(0.89, glm::vec3(0.29f, 0.35f, 0.24f))); 
    colorList.push_back(Color(0.92, glm::vec3(0.33f, 0.35f, 0.24f))); 
    colorList.push_back(Color(1.15, glm::vec3(0.30f, 0.30f, 0.25f)));
    colorList.push_back(Color(1.25, glm::vec3(0.47f, 0.45f, 0.45f)));
    colorList.push_back(Color(1.30, glm::vec3(0.65f, 0.60f, 0.64f)));

    for (int i = 1; i < vertices.size(); i += 3) {
        for (int j = 0; j < colorList.size(); j++) {
            if (vertices[i] < colorList[j].height * mapAmplitude) {
                currentColor = colorList[j].color;
                break;
            }
        }
        finalColor.push_back(currentColor.r);
        finalColor.push_back(currentColor.g);
        finalColor.push_back(currentColor.b);
    }

    return finalColor;
}

void TerrainGen::generateMap() {

    std::cout << "\n Generating Terrain Map ...";

    GLuint &VAO = map[0];
    int offsetX = 0;
    int offsetY = 0;

    vector<int> indices = generateIndices();
    vector<float> noise = generateNoiseMap(offsetX, offsetY);
    vector<float> vertices = generateVertices(noise);
    vector<float> normals = generateNormals(vertices, indices);
    vector<float> colors = generateColors(vertices, offsetX, offsetY);

    GLuint VBO[3], EBO;
            
    glGenBuffers(3, VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
            
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
            
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
            
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
            
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
            
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW);
            
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    std::cout << "\n\n Generating Terrain Map Complete ";
}
