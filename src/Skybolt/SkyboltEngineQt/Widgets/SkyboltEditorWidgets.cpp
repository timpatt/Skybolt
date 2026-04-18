/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SkyboltEditorWidgets.h"
#include "Widgets/PositionEditor.h"
#include "PolymorphicTypeEditor.h"

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltWidgets/Property/EditorWidgets.h>
#include <SkyboltWidgets/Property/PropertyEditor.h>
#include <SkyboltWidgets/Property/QtProperty.h>
#include <SkyboltWidgets/Property/QtPropertyMetadata.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>
#include <SkyboltSim/CameraController/CameraControllerSelector.h>
#include <SkyboltSim/CameraController/CameraModifierStack.h>
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/PropertyMetadata.h>
#include <SkyboltSim/Spatial/LatLon.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltWidgets/Util/QtTimerUtil.h>

#include <QComboBox>
#include <QVBoxLayout>

using namespace skybolt;

class LatLonPropertyEditor : public DoubleVectorEditor
{
public:
	LatLonPropertyEditor(QtProperty* property, QWidget* parent = nullptr) :
		mProperty(property),
		DoubleVectorEditor({"Latitude", "Longitude"}, parent)
	{
		setValue(mProperty->value().value<sim::LatLon>());

		connect(property, &QtProperty::valueChanged, this, [=]() {
			setValue(mProperty->value().value<sim::LatLon>());
		});
	}

protected:
	void setValue(const sim::LatLon& value)
	{
		for (int i = 0; i < 2; ++i)
		{
			DoubleVectorEditor::setValue(i, value[i]);
		}
	}

	void componentEdited(int index, double value) override
	{
		sim::LatLon v = mProperty->value().value<sim::LatLon>();
		v[index] = value;
		mProperty->setValue(QVariant::fromValue(v));
	}

private:
	QtProperty* mProperty;
};

static sim::Vector3 toSimVector3(const QVector3D& v)
{
	return sim::Vector3(v.x(), v.y(), v.z());
}

static QVector3D toQVector3D(const sim::Vector3& v)
{
	return QVector3D(v.x, v.y, v.z);
}

static PositionEditor* createWorldPositionEditor(QtProperty* property, QWidget* parent)
{
	auto widget = new PositionEditor(parent);
	widget->setPosition(sim::GeocentricPosition(toSimVector3(property->value().value<QVector3D>())));

	QObject::connect(property, &QtProperty::valueChanged, widget, [widget, property]() {
		widget->blockSignals(true);
		widget->setPosition(sim::GeocentricPosition(toSimVector3(property->value().value<QVector3D>())));
		widget->blockSignals(false);
	});

	QObject::connect(widget, &PositionEditor::valueChanged, property, [=] (const sim::Position& position) {
		property->setValue(toQVector3D(sim::toGeocentric(position).position));
	});
	return widget;
}

static QWidget* createSkyboltVector3DEditor(QtProperty* property, QWidget* parent)
{
	if (auto value = property->property(QtPropertyMetadataKeys::representation); value.isValid())
	{
		if (value.toString() == sim::PropertyRepresentations::worldPosition)
		{
			return createWorldPositionEditor(property, parent);
		}
	}
	return new QVector3PropertyEditor(property, { "x", "y", "z" }, parent);
}

static QWidget* createLatLonEditor(QtProperty* property, QWidget* parent)
{
	return new LatLonPropertyEditor(property, parent);
}

template <typename ValueT>
class NameToValueMapEditor : public QWidget
{
public:
	using MapTypeT = typename std::map<std::string, ValueT>;

	NameToValueMapEditor(QtProperty* property,
		NonNullPtr<refl::TypeRegistry> typeRegistry,
		const ReflTypePropertyFactoryMapPtr& typePropertyFactories,
		const PropertyEditorWidgetFactoryMapPtr& factoryMap,
		QWidget* parent = nullptr)
		: QWidget(parent)
		, mProperty(property)
		, mTypeRegistry(typeRegistry)
		, mTypePropertyFactories(typePropertyFactories)
		, mFactoryMap(factoryMap)
	{
		assert(mProperty);
		assert(mTypePropertyFactories);
		assert(mFactoryMap);

		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		mCombo = new QComboBox(this);
		layout->addWidget(mCombo);

		mPropertyEditor = new PropertyEditor(mFactoryMap, this);
		layout->addWidget(mPropertyEditor);

		populateCombo();
		updatePropertyEditor();

		QObject::connect(property, &QtProperty::valueChanged, this, [this]() {
			populateCombo();
			updatePropertyEditor();
		});

		QObject::connect(mCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
			updatePropertyEditor();
		});

		createAndStartIntervalTimer(/* intervalMilliseconds*/ 500, this, [this]() {
			if (mModel)
			{
				mModel->update();
			}
		});
	}

private:
	void populateCombo()
	{
		QSignalBlocker blocker(mCombo);
		mCombo->clear();
		const auto& container = mProperty->value().value<MapTypeT>();
		for (const auto& [name, value] : container)
		{
			mCombo->addItem(QString::fromStdString(name));
		}
	}

	void updatePropertyEditor()
	{
		const auto& container = mProperty->value().value<MapTypeT>();
		const std::string selectedName = mCombo->currentText().toStdString();

		auto it = container.find(selectedName);
		if (it == container.end())
		{
			mPropertyEditor->setModel(nullptr);
			return;
		}

		auto value = it->second.get();

		mModel = std::make_shared<PropertiesModel>();
		refl::Instance instance = refl::makeRefInstance(*mTypeRegistry, value);
		addReflPropertiesToModel(*mTypeRegistry, *mModel, toValuesVector(refl::getProperties(instance)),
			[this, name = selectedName]() -> std::optional<refl::Instance> {
				const auto& container = mProperty->value().value<MapTypeT>();
				auto it = container.find(name);
				if (it != container.end())
				{
					return refl::makeRefInstance(*mTypeRegistry, it->second.get());
				}
				return std::nullopt;
			},
			*mTypePropertyFactories);

		mPropertyEditor->setModel(mModel);
	}

private:
	QtProperty* mProperty;
	NonNullPtr<refl::TypeRegistry> mTypeRegistry;
	ReflTypePropertyFactoryMapPtr mTypePropertyFactories;
	PropertyEditorWidgetFactoryMapPtr mFactoryMap;
	QComboBox* mCombo;
	PropertyEditor* mPropertyEditor;
	std::shared_ptr<PropertiesModel> mModel;
};

skybolt::PropertyEditorWidgetFactoryMapPtr createSkyboltEditorWidgetFactoryMap(
	const DefaultEditorWidgetFactoryMapConfig& config,
	NonNullPtr<refl::TypeRegistry> typeRegistry,
	const ReflTypePropertyFactoryMapPtr& typePropertyFactories,
	NonNullPtr<FactoryRegistries> factoryRegistries)
{
	assert(typePropertyFactories);

	PropertyEditorWidgetFactoryMapPtr factoryMap = createDefaultEditorWidgetFactoryMap(config);

	(*factoryMap)[QMetaType::Type::QVector3D] = &createSkyboltVector3DEditor;
	(*factoryMap)[qMetaTypeId<sim::LatLon>()] = &createLatLonEditor;

	(*factoryMap)[qMetaTypeId<skybolt::sim::CameraControllerSelector::ControllersMap>()] =
		[typeRegistry, typePropertyFactories, factoryMap](QtProperty* property, QWidget* parent) -> QWidget* {
			return new NameToValueMapEditor<sim::CameraControllerPtr>(property, typeRegistry, typePropertyFactories, factoryMap, parent);
		};

	if (auto cameraModifierRegistry = factoryRegistries->getFirstItemOfType<sim::CameraModifierFactoryRegistry>(); cameraModifierRegistry)
	{
		(*factoryMap)[qMetaTypeId<sim::CameraModifierPtr>()] =
			[typeRegistry, typePropertyFactories, factoryMap, cameraModifierRegistry](QtProperty* property, QWidget* parent) -> QWidget* {
			auto cameraModifierFactories = std::make_shared<PolymorphicTypeEditor<sim::CameraModifier>::FactoryRegistryT>();
			for (const auto& [name, factory] : *cameraModifierRegistry)
			{
				cameraModifierFactories->emplace(name, [factory] {
					return factory({});
					});
			}
			if (cameraModifierFactories->empty())
			{
				// If there are no factories, return null
				return nullptr;
			}
			return new PolymorphicTypeEditor<sim::CameraModifier>(property, cameraModifierFactories, typeRegistry, typePropertyFactories, factoryMap, parent);
			};
	}

	return factoryMap;
}