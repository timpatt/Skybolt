/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <FftOcean/FftOceanGenerator.h>
#include <SkyboltVis/Image/SimpleImageFactory.h>


using namespace skybolt::vis;

//! Calculates the maximum wave height based on trough to crest
static float calcMaxWaveHeight(const std::vector<glm::vec3>& displacementMap)
{
	float vMin = std::numeric_limits<float>::infinity();
	float vMax = -std::numeric_limits<float>::infinity();

	for (const glm::vec3& v : displacementMap)
	{
		vMin = std::min(vMin, v.z);
		vMax = std::max(vMax, v.z);
	}
	return vMax - vMin;
}

TEST_CASE("Generate FFT ocean texture")
{
	FftOceanGeneratorConfig config;
	config.seed = 0;
	config.textureSizePixels = 256;
	config.textureWorldSize = 1000;
	config.significantWaveHeight = 5;
	config.gravity = 9.8;
	FftOceanGenerator generator(config);

	const float maxHeight = config.significantWaveHeight * 2; // Approx guess for max save height

	std::vector<glm::vec3> result(generator.getTextureSizePixels().x * generator.getTextureSizePixels().y);
	generator.calculate(0, std::span<glm::vec3>{result.data(), result.size()});

	float measuredMaxHeight = calcMaxWaveHeight(result);
	CHECK(measuredMaxHeight > config.significantWaveHeight);
	CHECK(measuredMaxHeight < maxHeight);

	constexpr bool saveImage = false;
	if (saveImage)
	{
		size_t elementCount = result.size();
		std::vector<unsigned char> resultChar(result.size() * 3);
		for (size_t i = 0; i < elementCount * 3; ++i)
		{
			float v = reinterpret_cast<float*>(result.data())[i];
			resultChar[i] = 128.f + 128.f * v / maxHeight;
		}

		SimpleImageFactory factory;
		ImagePtr image = valueOrThrowException(factory.createImage(config.textureSizePixels, config.textureSizePixels, Image::Format::RGB8, Image::ColorSpace::Linear));
		memcpy(image->getRawData(), resultChar.data(), elementCount * 3);
		factory.writeImage(*image, "C:/Users/Public/test.png");
	}

}

static float calcElevationStandardDeviation(const std::vector<glm::vec3>& displacementMap)
{
	if (displacementMap.empty()) return 0.0;

	// Calculate mean
	double mean = 0;
	for (const glm::vec3& v : displacementMap)
	{
		mean += v.z;
	}
	mean /= displacementMap.size();

	// Calculate variance
	double variance = 0.0;
	for (const glm::vec3& v : displacementMap)
	{
		variance += (v.z - mean) * (v.z - mean);
	}
	variance /= displacementMap.size();

	// Return the standard deviation (square root of variance)
	return std::sqrt(variance);
}

//! Significant wave height (trough to crest) is average height of highest 1/3 of waves.
//! Also defined as 4 times the standard deviation of the surface elevation.
//! See https://en.wikipedia.org/wiki/Significant_wave_height
static float calcSignificantWaveHeight(const std::vector<glm::vec3>& displacementMap)
{
	return 4.f * calcElevationStandardDeviation(displacementMap);
}

static float measureSignificantWaveHeightFromModelForConfiguredHeight(float significantWaveHeight)
{
	FftOceanGeneratorConfig config;
	config.seed = 0;
	config.textureSizePixels = 512;
	config.textureWorldSize = 2000;
	config.significantWaveHeight = significantWaveHeight;
	config.gravity = 9.8;
	FftOceanGenerator generator(config);

	std::vector<glm::vec3> result(generator.getTextureSizePixels().x * generator.getTextureSizePixels().y);
	generator.calculate(0, std::span<glm::vec3>{result.data(), result.size()});

	return calcSignificantWaveHeight(result);
}


TEST_CASE("FFT ocean wave height is as expected")
{
	constexpr float espilon = 0.1f;

	for (float height = 0; height < 20.f; height += 2.f)
	{
		float measuredHeight = measureSignificantWaveHeightFromModelForConfiguredHeight(height);
		CHECK(measuredHeight == Approx(height).epsilon(espilon));
	}
}
