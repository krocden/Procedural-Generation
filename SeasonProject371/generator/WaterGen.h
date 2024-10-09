#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Shader.h"

struct GerstnerWave
{
    float Q;
    float A;
    float l;
    float s;
    glm::vec2 D;
};

class WaterGen {
    public:
        int waterSize;
        int vertexCount;
        int indexCount;
        float *waterVertices;
        int *waterIndices;

        int waveMapCount = 0;
        int waveCount = 10;

        unsigned int VBO;
        unsigned int EBO;
        unsigned int VAO;

        unsigned int waveMaps[5];

        Shader *waterShader;

        int n = 16;

        WaterGen();
        WaterGen(int, Shader*);
        void generateMap();
        void genWaterVertexBuffer(int, float*, int*);
        void setGersterWaveData(Shader&, glm::vec2, int, GerstnerWave*);
        int genGersterWaveTexture(glm::vec2);
};