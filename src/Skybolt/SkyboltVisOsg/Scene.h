/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include "SkyboltEngine/SkyboltEngineFwd.h"
#include "SkyboltVisOsgFwd.h"
#include "VisObject.h"

#include <SkyboltSim/SimMath.h>
#include <osg/ClipNode>
#include <osg/Group>
#include <optional>

namespace skybolt {
namespace vis {

class Scene
{
public:
	Scene(const osg::ref_ptr<osg::StateSet>& stateSet);
	~Scene();

	enum class Bucket
	{
		Default,
		Clouds,
		Hud,
		BucketCount
	};

	// @param object can be added to multiple scenes.
	// Object cannot exist in multiple buckets.
	void addObject(const VisObjectPtr& object, Bucket bucket = Bucket::Default);
	void removeObject(const VisObjectPtr& object);

	//! @returns the direction the primary light is shining
	osg::Vec3f getPrimaryLightDirection() const;
	Light* getPrimaryLight() const { return mPrimaryLight; }

	void translateNoiseOrigin(const osg::Vec3f& offset);

	Planet* getPrimaryPlanet() const { return mPrimaryPlanet; }
	float calcAtmosphericDensity(const osg::Vec3f& position) const;

	osg::Vec3f getAmbientLightColor() const;
	void setAmbientLightColor(const osg::Vec3f& color);

	//! @returns the geocentric position of the scene origin, or std::nullopt if not set.
	std::optional<sim::Vector3> getSceneOriginGeocentric() const { return mSceneOriginGeocentric; }
	void setSceneOriginGeocentric(const sim::Vector3& sceneOriginGeocentric) { mSceneOriginGeocentric = sceneOriginGeocentric; }

	const osg::Vec3& getWrappedNoiseOrigin() const {return mWrappedNoiseOrigin;}

	osg::Group* getBucketGroup(Bucket bucket) const;

	void updatePreRender(const CameraRenderContext& context);

	osg::ref_ptr<osg::StateSet> getStateSet() const { return mStateSet; }

private:
	osg::ref_ptr<osg::StateSet> mStateSet;

	std::map<VisObjectPtr, Bucket> mObjects;
	std::vector<osg::ref_ptr<osg::Group>> mBucketGroups;
	Light* mPrimaryLight;
	Planet* mPrimaryPlanet;

	osg::Uniform* mLightDirectionUniform;
	osg::Uniform* mDirectLightColorUniform;
	osg::Uniform* mAmbientLightColorUniform;
	osg::Uniform* mWrappedNoiseOriginUniform;
	osg::Uniform* mGroundIrradianceMultiplierUniform;

	std::optional<sim::Vector3> mSceneOriginGeocentric;
	osg::Vec3f mWrappedNoiseOrigin;
	float mWrappedNoisePeriod;
};

} // namespace vis
} // namespace skybolt
