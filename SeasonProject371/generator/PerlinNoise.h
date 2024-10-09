#pragma once

#include <vector>

using namespace std;

class PerlinNoise {
	public:
		int mapSize;
		int octaves;
		float amplitude;
		float scale;
		float persistence;
		float lacunarity;

		static const vector<int> Permutation;

		PerlinNoise(int, int, float, float, float, float);
		vector<float> generate(int, int);
		vector<int> getPermutationTable();
		double fade(double);
		double lerp(double, double, double);
		double grad(int, double, double, double);
		double perlinNoise(float, float, vector<int> &);
};
