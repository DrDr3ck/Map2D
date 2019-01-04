#ifndef perlin_noise_h
#define perlin_noise_h

#include <vector>

/********************************************************************/

class PerlinNoise {
public:
 	// Initialize with the reference values for the permutation vector
 	PerlinNoise();
 	// Generate a new permutation vector based on the value of seed
 	PerlinNoise(unsigned int seed);
 	// Get a noise value, for 2D images z can have any value
 	double noise(double x, double y, double z);
protected:
 	double fade(double t);
 	double lerp(double t, double a, double b);
 	double grad(int hash, double x, double y, double z);

private:
 	std::vector<int> permutation_; // The permutation vector
};

class Noise {
public:
    // code in https://www.youtube.com/watch?v=MRNFcywkUSA
    static float** generateNoiseMap(int maxWidth, int maxHeight, unsigned int seed, float scale, int octaves=4, float persistance=0.5, float lacunarity=2);
};

/********************************************************************/

#endif // perlin_noise_h
