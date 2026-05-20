/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/NonNullPtr.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltWidgets/Property/PropertyEditor.h>
#include <SkyboltWidgets/Property/PropertyEditorWidgetFactory.h>
#include <SkyboltWidgets/Util/QtTimerUtil.h>

#include <QWidget>
#include <QComboBox>
#include <QPointer>
#include <QVBoxLayout>
#include <functional>
#include <map>
#include <memory>
#include <string>

class QComboBox;
class QVBoxLayout;

namespace skybolt {

//! A widget that allows the user to select a derived type T from a combo box and edit
//! the properties of the selected instance using a PropertyEditor.
template <typename T>
class PolymorphicTypeEditor : public QWidget
{
public:
	using FactoryT = std::function<std::shared_ptr<T>()>;
	using FactoryRegistryT = std::map<std::string, FactoryT>;
	using FactoryRegistryTPtr = std::shared_ptr<FactoryRegistryT>;

	PolymorphicTypeEditor(
		QPointer<QtValue> value,
		const FactoryRegistryTPtr& valueFactoryRegistry,
		NonNullPtr<refl::TypeRegistry> typeRegistry,
		const ReflValueTranslatorMapPtr& valueTranslatorFactories,
		const PropertyEditorWidgetFactoryMapPtr& editorWidgetFactoryMap,
		QWidget* parent = nullptr)
		: QWidget(parent)
		, mValue(value)
		, mValueFactoryRegistry(valueFactoryRegistry)
		, mTypeRegistry(typeRegistry)
		, mTypePropertyFactories(valueTranslatorFactories)
		, mEditorWidgetFactoryMap(editorWidgetFactoryMap)
	{
		assert(mProperty);
		assert(mTypePropertyFactories);
		assert(mEditorWidgetFactoryMap);

		mLayout = new QVBoxLayout(this);
		mLayout->setContentsMargins(0, 0, 0, 0);

		mComboBox = new QComboBox(this);

		// Populate combobox
		for (const auto& [name, factory] : *mValueFactoryRegistry)
		{
			mComboBox->addItem(QString::fromStdString(name), QString::fromStdString(name));
		}

		mComboBox->setCurrentIndex(-1); // Qt selects first item by default. We want to start with no selection.

		mLayout->addWidget(mComboBox);

		connect(mComboBox, &QComboBox::currentTextChanged, this, &PolymorphicTypeEditor<T>::onTypeSelected);

		connect(mValue, &QtValue::valueChanged, this, [this]() {
			populateEditorFromPropertyValue();
			});

		populateEditorFromPropertyValue();
	}

private:
	void onTypeSelected(const QString& typeName)
	{
		if (!mValue)
		{
			return;
		}

		// Create a new value via the factory
		auto it = mValueFactoryRegistry->find(typeName.toStdString());
		if (it == mValueFactoryRegistry->end())
		{
			// Failed to create new value, set property to null and clear editor
			clearEditorWidget();
			mValue->setValue(QVariant::fromValue(std::shared_ptr<T>()));
			return;
		}

		// Set the property to the new value
		mValue->setValue(QVariant::fromValue(it->second()));
	}

	void populateEditorFromPropertyValue()
	{
		// Clear previous editor
		mInstanceValue.reset();
		clearEditorWidget();

		// Get the current value
		std::shared_ptr<T> value = mValue->value().value<std::shared_ptr<T>>();

		// Populate the property editor for the current value
		if (value)
		{
			// Create a QtProperty holding a ReflValueInstanceVariant for the value
			ReflInstanceVariant instanceVariant;
			instanceVariant.instance = refl::makeRefInstance(*mTypeRegistry, value.get());
			mInstanceValue = createQtValue(QVariant::fromValue(instanceVariant));

			// Use the editor widget factory map to create an editor widget for the property
			mEditorWidget = createReflValueInstanceEditor(mInstanceValue.get(), this, mTypeRegistry.get(), *mTypePropertyFactories, mEditorWidgetFactoryMap);
			if (mEditorWidget)
			{
				mLayout->addWidget(mEditorWidget);
			}

			// Select the corresponding type in the combo box
			std::string currentTypeName = findTypeNameForValue(*value);
			mComboBox->blockSignals(true);
			mComboBox->setCurrentText(QString::fromStdString(currentTypeName));
			mComboBox->blockSignals(false);
		}
		else
		{
			// Select the first type in the combo box if one is available
			if (!mValueFactoryRegistry->empty())
			{
				mComboBox->setCurrentText(QString::fromStdString(mValueFactoryRegistry->begin()->first));
			}
		}
	}

	void clearEditorWidget()
	{
		if (mEditorWidget)
		{
			mLayout->removeWidget(mEditorWidget);
			delete mEditorWidget;
			mEditorWidget = nullptr;
		}
	}

	std::string findTypeNameForValue(const T& value) const
	{
		// TODO: find a more optimal solution. This is inefficient since it creates a new instance of each type just to compare its typeid.
		for (const auto& [name, factory] : *mValueFactoryRegistry)
		{
			std::shared_ptr<T> instance = factory();
			if (instance && typeid(*instance) == typeid(value))
			{
				return name;
			}
		}
		return {};
	}

private:
	QPointer<QtValue> mValue;
	FactoryRegistryTPtr mValueFactoryRegistry;
	NonNullPtr<refl::TypeRegistry> mTypeRegistry;
	ReflValueTranslatorMapPtr mTypePropertyFactories;
	PropertyEditorWidgetFactoryMapPtr mEditorWidgetFactoryMap;

	QVBoxLayout* mLayout = nullptr;
	QComboBox* mComboBox = nullptr;
	QtValuePtr mInstanceValue;
	QWidget* mEditorWidget = nullptr;
};

} // namespace skybolt
