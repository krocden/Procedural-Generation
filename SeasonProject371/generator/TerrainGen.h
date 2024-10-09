#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

using namespace std;

class TerrainGen {
	public:
		int mapSize;
		float mapAmplitude;
		vector<GLuint> map;

		TerrainGen(int, float);
		void generateMap();

		vector<int> generateIndices();
		vector<float> generateNoiseMap(int, int);
		vector<float> generateVertices(vector<float>&);
		vector<float> generateNormals(vector<float>&, vector<int>&);
		vector<float> generateColors(vector<float>&, int, int);
};

class Color {
	public:
		float height;
		glm::vec3 color;
		Color(float height, glm::vec3 color) {
			this->height = height;
			this->color = color;
		};
};
