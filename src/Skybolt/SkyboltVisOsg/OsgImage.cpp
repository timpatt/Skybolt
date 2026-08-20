/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "OsgImage.h"
#include "OsgImageHelpers.h"

#include <SkyboltCommon/Exception.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>

namespace skybolt::vis {

namespace {

unsigned char toByte(float value)
{
	return static_cast<unsigned char>(std::round(glm::clamp(value, 0.0f, 1.0f) * 255.0f));
}

GLenum toPixelFormat(Image::Format format)
{
	switch (format)
	{
	case Image::Format::R8:
	case Image::Format::R16:
	case Image::Format::R32F:
		return GL_LUMINANCE;
	case Image::Format::RGB8:
		return GL_RGB;
	case Image::Format::RGBA8:
		return GL_RGBA;
	case Image::Format::Count:
		break;
	}
	throw skybolt::Exception("Unsupported image format");
}

GLenum toDataType(Image::Format format)
{
	switch (format)
	{
	case Image::Format::R8:
	case Image::Format::RGB8:
	case Image::Format::RGBA8:
		return GL_UNSIGNED_BYTE;
	case Image::Format::R16:
		return GL_UNSIGNED_SHORT;
	case Image::Format::R32F:
		return GL_FLOAT;
	case Image::Format::Count:
		break;
	}
	throw skybolt::Exception("Unsupported image format");
}

Image::Format toImageFormat(GLenum pixelFormat, GLenum dataType)
{
	if (pixelFormat == GL_LUMINANCE && dataType == GL_UNSIGNED_BYTE)
	{
		return Image::Format::R8;
	}
	if (pixelFormat == GL_LUMINANCE && dataType == GL_UNSIGNED_SHORT)
	{
		return Image::Format::R16;
	}
	if (pixelFormat == GL_LUMINANCE && dataType == GL_FLOAT)
	{
		return Image::Format::R32F;
	}
	if (pixelFormat == GL_RGB && dataType == GL_UNSIGNED_BYTE)
	{
		return Image::Format::RGB8;
	}
	if (pixelFormat == GL_RGBA && dataType == GL_UNSIGNED_BYTE)
	{
		return Image::Format::RGBA8;
	}
	throw skybolt::Exception("Unsupported osg::Image pixel format or data type");
}

} // namespace

OsgImage::OsgImage(osg::ref_ptr<osg::Image> image, ColorSpace colorSpace) :
	mImage(std::move(image)),
	mColorSpace(colorSpace)
{
	if (!mImage)
	{
		throw skybolt::Exception("OsgImage requires a non-null osg::Image");
	}

	(void)getFormat();
}

int OsgImage::getWidth() const
{
	return mImage->s();
}

int OsgImage::getHeight() const
{
	return mImage->t();
}

const unsigned char* OsgImage::getRawData() const
{
	return mImage->data();
}

unsigned char* OsgImage::getRawData()
{
	return mImage->data();
}

glm::vec4 OsgImage::getColor(int x, int y) const
{
	const osg::Vec4 color = mImage->getColor(x, y);
	return glm::vec4(color.r(), color.g(), color.b(), color.a());
}

void OsgImage::setColor(int x, int y, const glm::vec4& color)
{
	switch (getFormat())
	{
	case Format::R8:
	{
		unsigned char* pixel = mImage->data(x, y);
		pixel[0] = toByte(color.r);
		return;
	}
	case Format::R16:
	{
		auto* pixel = reinterpret_cast<unsigned short*>(mImage->data(x, y));
		pixel[0] = static_cast<unsigned short>(glm::clamp(color.r, 0.0f, 1.0f) * 65535.0f);
		return;
	}
	case Format::R32F:
	{
		auto* pixel = reinterpret_cast<float*>(mImage->data(x, y));
		pixel[0] = color.r;
		return;
	}
	case Format::RGB8:
	case Format::RGBA8:
		mImage->setColor(osg::Vec4(color.r, color.g, color.b, color.a), x, y);
		return;
	case Format::Count:
		break;
	}
	throw skybolt::Exception("Unsupported image format");
}

Image::Format OsgImage::getFormat() const
{
	return toImageFormat(mImage->getPixelFormat(), mImage->getDataType());
}

Image::ColorSpace OsgImage::getColorSpace() const
{
	return mColorSpace;
}

void OsgImage::setColorSpace(ColorSpace space)
{
	mColorSpace = space;
}

void OsgImage::addMetadata(const std::string& key, const ImageMetadataValue& value)
{
	mMetadata[key] = value;
}

void OsgImage::removeMetadata(const std::string& key)
{
	mMetadata.erase(key);
}

std::optional<ImageMetadataValue> OsgImage::getMetadata(const std::string& key) const
{
	auto it = mMetadata.find(key);
	if (it == mMetadata.end())
	{
		return std::nullopt;
	}
	return it->second;
}

Expected<ImagePtr> OsgImageFactory::createImage(int width, int height, Image::Format format, Image::ColorSpace colorSpace) const
{
	if (width <= 0 || height <= 0)
	{
		return UnexpectedMessage("Invalid image dimensions: " + std::to_string(width) + "x" + std::to_string(height));
	}

	if (std::size_t(format) >= std::size_t(Image::Format::Count))
	{
		return UnexpectedMessage("Invalid image format: " + std::to_string(static_cast<std::size_t>(format)));
	}

	osg::ref_ptr<osg::Image> image = new osg::Image();
	image->allocateImage(width, height, 1, toPixelFormat(format), toDataType(format));
	return std::make_shared<OsgImage>(image, colorSpace);
}

Expected<ImagePtr> OsgImageFactory::readImage(std::istream& s, const std::string& formatExtension) const
{
	osg::ref_ptr<osg::Image> image = readImageWithUserData(s, formatExtension);
	if (!image)
	{
		return UnexpectedMessage("Failed to read image from stream");
	}
	return std::make_shared<OsgImage>(image, Image::ColorSpace::Srgb);
}

Expected<bool> OsgImageFactory::writeImage(const Image& image, std::ostream& s, const std::string& formatExtension) const
{
	const OsgImage* osgImage = dynamic_cast<const OsgImage*>(&image);
	if (osgImage)
	{
		if (!writeImageWithUserData(*osgImage->getOsgImage(), s, formatExtension))
		{
			return UnexpectedMessage("Failed to write image to stream");
		}
		return true;
	}

	Expected<ImagePtr> converted = createImage(image.getWidth(), image.getHeight(), image.getFormat(), image.getColorSpace());
	if (!has_value(converted))
	{
		return std::get<UnexpectedMessage>(converted);
	}

	ImagePtr convertedImage = *value(converted);
	std::copy(image.getRawData(), image.getRawData() + image.getRawDataSizeBytes(), convertedImage->getRawData());

	osgImage = dynamic_cast<const OsgImage*>(convertedImage.get());
	assert(osgImage);
	if (!writeImageWithUserData(*osgImage->getOsgImage(), s, formatExtension))
	{
		return UnexpectedMessage("Failed to write image to stream");
	}
	return true;
}

} // namespace skybolt::vis
