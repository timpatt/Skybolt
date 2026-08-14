/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltEngine/SkyboltEngineFwd.h>
#include <SkyboltVis/SkyboltVisFwd.h>

#include <QWindow>
#include <osg/Camera>

class OsgWindow : public QWindow
{
	Q_OBJECT
public:
	OsgWindow(const skybolt::vis::VisRootPtr& visRoot);
	~OsgWindow() override;

	//! @returns null if the window has not initialized yet. Typically there is a delay from the constructor to when the window is fully initialized and ready to be used.
	//! The window can be destroyed at any time. Therefore the returned pointer should not be stored by the caller.
	skybolt::vis::Window* getWindow() const;

signals:
	void mousePressed(const QPointF& position, Qt::MouseButton button, const Qt::KeyboardModifiers& modifiers);
	void mouseReleased(const QPointF& position, Qt::MouseButton button);
	void mouseMoved(const QPointF& position, Qt::MouseButtons buttons);

	void windowCreated();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	bool event(QEvent* event);

private:
	skybolt::vis::VisRootPtr mVisRoot;
	std::shared_ptr<skybolt::vis::Window> mWindow;
	osg::ref_ptr<osg::Camera::DrawCallback> mDrawCallback;
};
