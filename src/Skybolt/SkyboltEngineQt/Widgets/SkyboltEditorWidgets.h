/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltEngine/FactoryRegistries.h>
#include <SkyboltWidgets/Property/DefaultEditorWidgets.h>
#include <SkyboltWidgets/Property/PropertyEditorWidgetFactory.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>

namespace skybolt::refl { class TypeRegistry; }

skybolt::PropertyEditorWidgetFactoryMapPtr createSkyboltEditorWidgetFactoryMap(
	const skybolt::DefaultEditorWidgetFactoryMapConfig& config,
	skybolt::NonNullPtr<skybolt::refl::TypeRegistry> typeRegistry,
	const skybolt::ReflValueTranslatorMapPtr& valueTranslatorFactories,
	skybolt::NonNullPtr<skybolt::FactoryRegistries> factoryRegistries);