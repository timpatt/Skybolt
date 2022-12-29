/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "Window.h"
#include "DisplaySettings.h"
#include "SkyboltVis/Camera.h"
#include "SkyboltVis/OsgLogHandler.h"
#include "SkyboltVis/RenderContext.h"
#include "SkyboltVis/RenderOperation/RenderTarget.h"

#include <boost/foreach.hpp>

namespace skybolt {
namespace vis {

Window::Window(const DisplaySettings& settings) :
	mViewer(new osgViewer::Viewer),
	mRootGroup(new osg::Group),
	mRenderOperationSequence(std::make_unique<RenderOperationSequence>())
{
	mRootGroup->addChild(mRenderOperationSequence->getRootNode());

	forwardOsgLogToBoost();

	osg::DisplaySettings::instance()->setNumMultiSamples(settings.multiSampleCount);

	osg::setNotifyLevel(osg::WARN);
	mViewer->setKeyEventSetsDone(0); // disable default 'escape' key binding to quit the application
	mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded); // TODO: Use multi-threaded?

	osg::StateSet* stateSet = mRootGroup->getOrCreateStateSet();
	// We will write to the frame buffer in linear light space, and it will automatically convert to SRGB
	stateSet->setMode(GL_FRAMEBUFFER_SRGB, osg::StateAttribute::ON);

	stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

	mViewer->setSceneData(osg::ref_ptr<osg::Node>(mRootGroup));

	mScreenSizePixelsUniform = new osg::Uniform("screenSizePixels", osg::Vec2f(0, 0));
	stateSet->addUniform(mScreenSizePixelsUniform);
}

Window::~Window()
{
}

bool Window::render()
{
	mScreenSizePixelsUniform->set(osg::Vec2f(getWidth(), getHeight()));

	RenderContext context;
	context.targetDimensions = osg::Vec2i(getWidth(), getHeight());
	mRenderOperationSequence->updatePreRender(context);

	mViewer->frame();

	return !mViewer->done();
}

void Window::configureGraphicsState()
{
	// Set global graphics state
	osg::GraphicsContext* context = mViewer->getCamera()->getGraphicsContext();
	if (!context)
	{
		throw std::runtime_error("Could not initialize graphics context");
	}
	osg::State* state = context->getState();

	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);

	// Disable GL error checking because it is expensive (measured about 13% performance hit compared with osg::State::ONCE_PER_FRAME).
	// Enable to debug OpenGL.
	state->setCheckForGLErrors(osg::State::NEVER_CHECK_GL_ERRORS);
}

} // namespace vis
} // namespace skybolt