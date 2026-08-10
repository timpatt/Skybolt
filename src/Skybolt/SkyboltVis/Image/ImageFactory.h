#pragma once

#include <SkyboltCommon/Expected.h>
#include "SkyboltVis/SkyboltVisFwd.h"
#include "SkyboltVis/VisFactory.h"
#include "SkyboltVis/Image/Image.h"

#include <string>

namespace skybolt::vis {

class ImageFactory : public VisFactoryT<ImageFactory>
{
public:
	~ImageFactory() override = default;

	//! @returns non-null ImagePtr on success, or error message
	virtual Expected<ImagePtr> createImage(int width, int height, Image::Format format, Image::ColorSpace colorSpace) const = 0;

	//! @returns non-null ImagePtr on success, or error message
	virtual Expected<ImagePtr> readImage(std::istream& s, const std::string& formatExtension) const = 0; //!< @return nullptr on fail

	//! @returns true on success, or error message. MTOOD: change to std::monostate
	virtual Expected<bool> writeImage(const Image& image, std::ostream& s, const std::string& formatExtension) const = 0; //!< @return true on success

	//! @returns non-null ImagePtr on success, or error message
	virtual Expected<ImagePtr> readImage(const std::string& filename) const; //!< @return nullptr on fail

	//! @returns true on success, or error message
	virtual Expected<bool> writeImage(const Image& image, const std::string& filename) const; //!< @return true on success
};

} // namespace skybolt::vis