/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltEngine/ComponentFactory.h>
#include <SkyboltVis/VisFactory.h>
#include "SkyboltVisOsg/SkyboltVisOsgFwd.h"
#include "SkyboltVis/DisplaySettings.h"
#include "SkyboltVisOsg/RenderContext.h"

#include <set>

namespace osgViewer {
class CompositeViewer;
class ViewerBase;
}

namespace skybolt {
namespace vis {

class VisObjectFactory;

class VisRoot
{
public:
	VisRoot(const DisplaySettings& config = DisplaySettings());
	~VisRoot();

	//! @returns false if window has been closed
	bool render();

	osgViewer::ViewerBase& getViewer() const;

	void addWindow(const WindowPtr& window);
	void removeWindow(const WindowPtr& window);

	const std::vector<WindowPtr>& getWindows() const { return mWindows; }

	//! Default is LoadTimingPolicy::LoadAcrossMultipleFrames
	void setLoadTimingPolicy(LoadTimingPolicy loadTimingPolicy) { mLoadTimingPolicy = loadTimingPolicy; }

	const DisplaySettings& getDisplaySettings() const { return mDisplaySettings; }

	vis::VisFactoryRegistry& getVisFactoryRegistry() { return *mVisFactoryRegistry; }
	const vis::ShaderPrograms& getShaderPrograms() const { return *mPrograms; }
	const vis::ScenePtr& getScene() const { return mScene; }

protected:
	std::shared_ptr<osgViewer::CompositeViewer> mViewer;
	std::unique_ptr<vis::VisFactoryRegistry> mVisFactoryRegistry;
	std::unique_ptr<VisObjectFactory> mVisObjectFactory;
	std::unique_ptr<vis::ShaderPrograms> mPrograms; //!< Never null
	vis::ScenePtr mScene;

private:
	const DisplaySettings mDisplaySettings;
	std::vector<WindowPtr> mWindows;
	LoadTimingPolicy mLoadTimingPolicy;
};

} // namespace vis
} // namespace skybolt
