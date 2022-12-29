/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <osgDB/Registry>
#include <osgViewer/GraphicsWindow>

// If this file is built, the OSG plugins have been built statically.
// We statically reference the plugins we need here to link them into this library.
USE_OSGPLUGIN(bmp)
USE_OSGPLUGIN(curl)
USE_OSGPLUGIN(dds)
USE_OSGPLUGIN(freetype)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(png)
USE_OSGPLUGIN(tga)

USE_OSGPLUGIN(osg2)
USE_SERIALIZER_WRAPPER_LIBRARY(osg)

// include the platform specific GraphicsWindow implementation
USE_GRAPHICSWINDOW()
