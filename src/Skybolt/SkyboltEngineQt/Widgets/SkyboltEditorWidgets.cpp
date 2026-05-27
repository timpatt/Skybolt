/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "SkyboltEditorWidgets.h"
#include "Widgets/PositionEditor.h"
#include "PolymorphicTypeEditor.h"

#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/Units.h>
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

template <typename T>
std::optional<T> getDisplayUnitsConversionFactor(const QtValue& qtValue)
{
	if (auto value = qtValue.property(sim::PropertyMetadataNames::units); value.isValid())
	{
		Units units = std::any_cast<Units>(value.value<std::any>());

		if (units == Units::Radians)
		{
			return (T)skybolt::math::radToDegD();
		}
	}
	return std::nullopt;
}

double getDoubleValueInDisplayUnits(const QtValue& value)
{
	double v = value.value().toDouble();
	if (auto conversionFactor = getDisplayUnitsConversionFactor<double>(value))
	{
		v *= *conversionFactor;
	}
	return v;
}

void setDoubleValueInDisplayUnits(QtValue& value, double v)
{
	if (auto conversionFactor = getDisplayUnitsConversionFactor<double>(value))
	{
		v /= *conversionFactor;
	}
	value.setValue(v);
}


QWidget* createDoubleEditorWithUnits(QtValue* value, QWidget* parent)
{
	constexpr int decimalCount = 9;

	QLineEdit* widget = createDoubleLineEdit(parent, decimalCount);

	auto widgetTextSetter = [widget](double value) {
		widget->setText(QString::number(value, 'g', decimalCount));
	};
	widgetTextSetter(getDoubleValueInDisplayUnits(*value));

	QObject::connect(value, &QtValue::valueChanged, widget, [widget, value, widgetTextSetter]() {
		widget->blockSignals(true);
		widgetTextSetter(getDoubleValueInDisplayUnits(*value));
		widget->blockSignals(false);
	});

	QObject::connect(widget, &QLineEdit::editingFinished, value, [=]() {
		setDoubleValueInDisplayUnits(*value, widget->text().toDouble());
	});

	return widget;
}

class LatLonPropertyEditor : public DoubleVectorEditor
{
public:
	LatLonPropertyEditor(QtValue* value, QWidget* parent = nullptr) :
		mValue(value),
		DoubleVectorEditor({"Latitude", "Longitude"}, parent)
	{
		setValue(mValue->value().value<sim::LatLon>());

		connect(mValue, &QtValue::valueChanged, this, [=]() {
			setValue(mValue->value().value<sim::LatLon>());
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
		sim::LatLon v = mValue->value().value<sim::LatLon>();
		v[index] = value;
		mValue->setValue(QVariant::fromValue(v));
	}

private:
	QtValue* mValue;
};

static sim::Vector3 toSimVector3(const QVector3D& v)
{
	return sim::Vector3(v.x(), v.y(), v.z());
}

static QVector3D toQVector3D(const sim::Vector3& v)
{
	return QVector3D(v.x, v.y, v.z);
}

static PositionEditor* createWorldPositionEditor(QtValue* value, QWidget* parent)
{
	auto widget = new PositionEditor(parent);
	widget->setPosition(sim::GeocentricPosition(toSimVector3(value->value().value<QVector3D>())));

	QObject::connect(value, &QtValue::valueChanged, widget, [widget, value]() {
		widget->blockSignals(true);
		widget->setPosition(sim::GeocentricPosition(toSimVector3(value->value().value<QVector3D>())));
		widget->blockSignals(false);
	});

	QObject::connect(widget, &PositionEditor::valueChanged, value, [=] (const sim::Position& position) {
		value->setValue(toQVector3D(sim::toGeocentric(position).position));
	});
	return widget;
}

static QWidget* createSkyboltVector3DEditor(QtValue* value, QWidget* parent)
{
	if (auto representation = value->property(QtPropertyMetadataKeys::representation); representation.isValid())
	{
		if (representation.toString() == sim::PropertyRepresentations::worldPosition)
		{
			return createWorldPositionEditor(value, parent);
		}
		else if (representation.toString() == sim::PropertyRepresentations::rollPitchYaw)
		{
			return new QVector3Editor(value, { "roll", "pitch", "yaw" }, parent);
		}
	}
	return new QVector3Editor(value, { "x", "y", "z" }, parent);
}

static QWidget* createLatLonEditor(QtValue* value, QWidget* parent)
{
	return new LatLonPropertyEditor(value, parent);
}

template <typename ValueT>
class NameToValueMapEditor : public QWidget
{
public:
	using MapTypeT = typename std::map<std::string, ValueT>;

	NameToValueMapEditor(QtValue* value,
		NonNullPtr<refl::TypeRegistry> typeRegistry,
		const ReflValueTranslatorMapPtr& valueTranslatorFactories,
		const PropertyEditorWidgetFactoryMapPtr& factoryMap,
		QWidget* parent = nullptr)
		: QWidget(parent)
		, mValue(value)
		, mTypeRegistry(typeRegistry)
		, mValueTranslators(valueTranslatorFactories)
		, mFactoryMap(factoryMap)
	{
		assert(mProperty);
		assert(mValueTranslators);
		assert(mFactoryMap);

		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);

		mCombo = new QComboBox(this);
		layout->addWidget(mCombo);

		mPropertyEditor = new PropertyEditor(mFactoryMap, this);
		layout->addWidget(mPropertyEditor);

		populateCombo();
		updatePropertyEditor();

		QObject::connect(value, &QtValue::valueChanged, this, [this]() {
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
		const auto& container = mValue->value().value<MapTypeT>();
		for (const auto& [name, value] : container)
		{
			mCombo->addItem(QString::fromStdString(name));
		}
	}

	void updatePropertyEditor()
	{
		const auto& container = mValue->value().value<MapTypeT>();
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
				const auto& container = mValue->value().value<MapTypeT>();
				auto it = container.find(name);
				if (it != container.end())
				{
					return refl::makeRefInstance(*mTypeRegistry, it->second.get());
				}
				return std::nullopt;
			},
			*mValueTranslators);

		mPropertyEditor->setModel(mModel);
	}

private:
	QtValue* mValue;
	NonNullPtr<refl::TypeRegistry> mTypeRegistry;
	ReflValueTranslatorMapPtr mValueTranslators;
	PropertyEditorWidgetFactoryMapPtr mFactoryMap;
	QComboBox* mCombo;
	PropertyEditor* mPropertyEditor;
	std::shared_ptr<PropertiesModel> mModel;
};

skybolt::PropertyEditorWidgetFactoryMapPtr createSkyboltEditorWidgetFactoryMap(
	const DefaultEditorWidgetFactoryMapConfig& config,
	NonNullPtr<refl::TypeRegistry> typeRegistry,
	const ReflValueTranslatorMapPtr& valueTranslatorFactories,
	NonNullPtr<FactoryRegistries> factoryRegistries)
{
	assert(valueTranslatorFactories);

	PropertyEditorWidgetFactoryMapPtr factoryMap = createDefaultEditorWidgetFactoryMap(config);
	(*factoryMap)[qMetaTypeId<ReflInstanceVariant>()] = [typeRegistry, valueTranslatorFactories, factoryMap] (QtValue* value, QWidget* parent) {
		return createReflValueInstanceEditor(value, parent, typeRegistry, *valueTranslatorFactories, factoryMap);
	};

	// Override float and double editors to use our custom editor that supports units
	(*factoryMap)[QMetaType::Type::Float] = &createDoubleEditorWithUnits;
	(*factoryMap)[QMetaType::Type::Double] = &createDoubleEditorWithUnits;

	(*factoryMap)[QMetaType::Type::QVector3D] = &createSkyboltVector3DEditor;
	(*factoryMap)[qMetaTypeId<sim::LatLon>()] = &createLatLonEditor;

	(*factoryMap)[qMetaTypeId<skybolt::sim::CameraControllerSelector::ControllersMap>()] =
		[typeRegistry, valueTranslatorFactories, factoryMap](QtValue* value, QWidget* parent) -> QWidget* {
			return new NameToValueMapEditor<sim::CameraControllerPtr>(value, typeRegistry, valueTranslatorFactories, factoryMap, parent);
		};

	if (auto cameraModifierRegistry = factoryRegistries->getFirstItemOfType<sim::CameraModifierFactoryRegistry>(); cameraModifierRegistry)
	{
		(*factoryMap)[qMetaTypeId<sim::CameraModifierPtr>()] =
			[typeRegistry, valueTranslatorFactories, factoryMap, cameraModifierRegistry](QtValue* value, QWidget* parent) -> QWidget* {
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
			return new PolymorphicTypeEditor<sim::CameraModifier>(value, cameraModifierFactories, typeRegistry, valueTranslatorFactories, factoryMap, parent);
			};
	}

	return factoryMap;
}