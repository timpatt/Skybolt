/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Texture2D>

namespace skybolt {
namespace vis {

struct Wake
{
	std::vector<osg::Vec3f> points; //!< x, y, width
};

struct WaterStateSetConfig
{
	struct WaveTextureSet
	{
		osg::ref_ptr<osg::Texture2D> waveHeight;
		osg::ref_ptr<osg::Texture2D> waveNormal;
		osg::ref_ptr<osg::Texture2D> waveFoamMask;
		float textureWorldSize;
	};

	std::vector<WaveTextureSet> waveTextureSets;
	float maxWaveHeight;
};

//! StateSet containing textures and uniforms for a generic water shader.
//! Does not contain the shader program.
class WaterStateSet : public osg::StateSet
{
public:
	WaterStateSet(const WaterStateSetConfig& config);

	void setWakes(const std::vector<Wake>& wakes);
	void setFoamTexture(int index, osg::ref_ptr<osg::Texture2D> texture);
	void setTextureOriginNorthEast(const osg::Vec2f& originNorthEast);
	void setTextureWorldSize(int i, const osg::Vec2f& textureWorldSize);
	void setMaxWaveHeight(float maxWaveHeight);

private:
	osg::ref_ptr<osg::Image> mWakeHashMapImage; //!< Maps a hashed wake spatial coordinate to {wake params first index, wake params last index + 1}
	osg::ref_ptr<osg::Image> mWakeParamsImage;
	int mFirstFoamMaskTextureIndex;

	osg::ref_ptr<osg::Uniform> mHeightMapTexCoordScales;
	osg::ref_ptr<osg::Uniform> mTextureOriginNorthEastUniform;
	osg::ref_ptr<osg::Uniform> mMaxWaveHeightUniform;
};

} // namespace vis
} // namespace skybolt
