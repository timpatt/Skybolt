/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CLOUDS_H
#define CLOUDS_H

#pragma import_defines ( USE_CLOUD_COVERAGE_MAP )

#include "GlobalDefines.h"
#include "Remap.h"

uniform vec2 cloudDisplacementMeters;
uniform vec2 cloudUvScaleRelTerrainUv;
uniform vec2 cloudUvOffsetRelTerrainUv;
uniform float cloudCoverageFraction; // only used if USE_CLOUD_COVERAGE_MAP is not defined

vec2 offsetCloudUvByWorldDistance(vec2 startUv, vec2 worldDistance)
{
	return startUv + vec2(worldDistance.y / 40070000.0, 0.5 * worldDistance.x / 40070000.0);
}

vec2 cloudUvFromTerrainUv(vec2 geoTexCoord)
{
	vec2 uv = geoTexCoord.xy * cloudUvScaleRelTerrainUv + cloudUvOffsetRelTerrainUv;
	return offsetCloudUvByWorldDistance(uv, cloudDisplacementMeters);
}

vec2 cloudUvFromPosRelPlanet(vec3 positionRelPlanetPlanetAxes)
{
	vec2 pos2d = normalize(positionRelPlanetPlanetAxes.yx);
	float psi = atan(pos2d.x, pos2d.y);
	float theta = asin(normalize(positionRelPlanetPlanetAxes).z);
	vec2 uv = vec2(psi * M_RCP_2PI + 0.5, theta * M_RCP_PI + 0.5);
	return offsetCloudUvByWorldDistance(uv, cloudDisplacementMeters);
}

float sampleCoverageDetail(sampler2D coverageDetailSampler, vec2 uv, float lod, out float cloudType)
{
	vec4 c = textureLod(coverageDetailSampler, uv.xy * vec2(300.0, 150.0), lod);
	float coverageDetail = c.r;
	cloudType = c.b*c.b;
	return coverageDetail;
}

float sampleBaseCloudCoverage(sampler2D cloudSampler, vec2 uv)
{
#ifdef USE_CLOUD_COVERAGE_MAP
	float c = clamp(textureLod(cloudSampler, uv, 0).r * 1.3 - 0.001, 0.0, 1.0);
	return pow(c, 0.45); // apply fudge factor so that high detail cloud coverage matches map
#else
	return cloudCoverageFraction;
#endif
}

// Modulation a detail map by a coverage map. The modulation has the following behavior:
//   * Input coverage of 0 gives an output of 0.
//   * Input coverage of 1 gives an output of 1.
//   * Input coverage between 0 and 1 remaps the color range of the detail map
//     such that the mean output brightness equals coverage amount.
// @param filterWidth [0, 1] controls the smoothness of the output,
//    where 0 produces harsh edges and 1 smooths out the detail so much that 
//    the input coverage signal is passed through unchanged.
float coverageModulation(float coverage, float detail, float filterWidth)
{
	float f = 1.0 - coverage;
	return clamp(remapNormalized(detail*(1.0-filterWidth)+filterWidth, f, f+filterWidth), 0.0, 1.0);
}

vec2 coverageModulation(vec2 coverage, vec2 detail, vec2 filterWidth)
{
	vec2 f = vec2(1.0) - coverage;
	return clamp(remapNormalized(detail*(vec2(1.0)-filterWidth)+filterWidth, f, f+filterWidth), vec2(0.0), vec2(1.0));
}

float cloudCoverageModulationFilterWidth = 0.6;

float calcCloudDensityHull(float coverageBase, float coverageDetail, float heightMultiplier)
{
	float coverage = coverageBase * heightMultiplier;
	return coverageModulation(coverage, coverageDetail, cloudCoverageModulationFilterWidth);
}

//! @return x component is the low-res density with equivelent average density to the expected post-erosion high res density.
//!   This value is useful as a low res proxy for the high res density.
//! @return y component is the low-res density pre-erosion by high res noise.
//!   This value be useful for calculating the high res density by applying erosion.
//! the final high res density by applying erosion to it.
vec2 calcCloudDensityLowRes(float coverageBase, float coverageDetail, float heightMultiplier)
{
	vec2 coverage = vec2(coverageBase * heightMultiplier);

	// Eroding the low detail density by high detail textures results in reduction of cloud coverage.
	// We need to subtract this amount from the low detail coverage to match the high detail result.
	float erosionCompensation = 0.14;
	coverage.x = max(0.0, coverage.x-erosionCompensation);

	return coverageModulation(coverage, vec2(coverageDetail), vec2(cloudCoverageModulationFilterWidth));
}

#endif // CLOUDS_H