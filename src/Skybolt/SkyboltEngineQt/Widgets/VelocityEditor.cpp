/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "VelocityEditor.h"
#include <SkyboltSim/Spatial/Geocentric.h>
#include <SkyboltCommon/Math/MathUtility.h>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <cmath>

using namespace skybolt;
using namespace skybolt::math;
using namespace skybolt::sim;

constexpr int decimalCount = 9;

class VelocityEditorImpl : public QWidget
{
	Q_OBJECT
public:
	VelocityEditorImpl(QWidget* parent = nullptr) : QWidget(parent) {}

	virtual void setVelocity(const Vector3& geocentricVelocity) = 0;
	virtual Vector3 getVelocity() const = 0;

	void setGeocentricPosition(const Vector3& position) { mGeocentricPosition = position; }
	const Vector3& getGeocentricPosition() const { return mGeocentricPosition; }

	Q_SIGNAL void valueChanged(const Vector3& velocity);

protected:
	QLineEdit* addDoubleEditor(QGridLayout& layout, const QString& name)
	{
		int row = layout.rowCount();
		layout.addWidget(new QLabel(name, this), row, 0);

		QLineEdit* editor = new QLineEdit(this);
		layout.addWidget(editor, row, 1);

		QDoubleValidator* validator = new QDoubleValidator();
		validator->setNotation(QDoubleValidator::ScientificNotation);
		validator->setDecimals(decimalCount);
		editor->setValidator(validator);

		connect(editor, &QLineEdit::editingFinished, this, [this] {
			Vector3 velocity = getVelocity();
			emit valueChanged(velocity);
		});

		return editor;
	}

	Vector3 mGeocentricPosition = Vector3(0, 0, 0);
};

class GeocentricVelocityEditor : public VelocityEditorImpl
{
public:
	GeocentricVelocityEditor(QWidget* parent = nullptr) :
		VelocityEditorImpl(parent)
	{
		QGridLayout* layout = new QGridLayout(this);
		layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
		layout->setContentsMargins(0, 0, 0, 0);
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "x");
		mValues[1] = addDoubleEditor(*layout, "y");
		mValues[2] = addDoubleEditor(*layout, "z");
	}

	void setVelocity(const Vector3& velocity) override
	{
		for (int i = 0; i < 3; ++i)
		{
			mValues[i]->setText(QString::number(velocity[i], 'g', decimalCount));
		}
		emit valueChanged(velocity);
	}

	Vector3 getVelocity() const override
	{
		Vector3 v;
		for (int i = 0; i < 3; ++i)
		{
			v[i] = mValues[i]->text().toDouble();
		}
		return v;
	}

private:
	QLineEdit* mValues[3];
};

class NedVelocityEditor : public VelocityEditorImpl
{
public:
	NedVelocityEditor(QWidget* parent = nullptr) :
		VelocityEditorImpl(parent)
	{
		QGridLayout* layout = new QGridLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
		setLayout(layout);
		mValues[0] = addDoubleEditor(*layout, "North");
		mValues[1] = addDoubleEditor(*layout, "East");
		mValues[2] = addDoubleEditor(*layout, "Down");
	}

	void setVelocity(const Vector3& geocentricVelocity) override
	{
		Matrix3 ltpOrientation = geocentricToLtpOrientation(mGeocentricPosition);
		Vector3 ned = glm::transpose(ltpOrientation) * geocentricVelocity;
		for (int i = 0; i < 3; ++i)
		{
			mValues[i]->setText(QString::number(ned[i], 'g', decimalCount));
		}
		emit valueChanged(geocentricVelocity);
	}

	Vector3 getVelocity() const override
	{
		Vector3 ned;
		for (int i = 0; i < 3; ++i)
		{
			ned[i] = mValues[i]->text().toDouble();
		}
		Matrix3 ltpOrientation = geocentricToLtpOrientation(mGeocentricPosition);
		return ltpOrientation * ned;
	}

private:
	QLineEdit* mValues[3];
};

class SpeedBearingInclinationVelocityEditor : public VelocityEditorImpl
{
public:
	SpeedBearingInclinationVelocityEditor(QWidget* parent = nullptr) :
		VelocityEditorImpl(parent)
	{
		QGridLayout* layout = new QGridLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
		setLayout(layout);
		mSpeed = addDoubleEditor(*layout, "Speed");
		mBearing = addDoubleEditor(*layout, "Bearing");
		mInclination = addDoubleEditor(*layout, "Inclination");
	}

	void setVelocity(const Vector3& geocentricVelocity) override
	{
		Matrix3 ltpOrientation = geocentricToLtpOrientation(mGeocentricPosition);
		Vector3 ned = glm::transpose(ltpOrientation) * geocentricVelocity;

		double speed = glm::length(geocentricVelocity);
		double horizontalSpeed = std::sqrt(ned.x * ned.x + ned.y * ned.y);
		double bearing = (horizontalSpeed > 1e-12) ? std::atan2(ned.y, ned.x) * radToDegD() : 0.0;
		double inclination = (speed > 1e-12) ? std::asin(std::clamp(-ned.z / speed, -1.0, 1.0)) * radToDegD() : 0.0;

		mSpeed->setText(QString::number(speed, 'g', decimalCount));
		mBearing->setText(QString::number(bearing, 'g', decimalCount));
		mInclination->setText(QString::number(inclination, 'g', decimalCount));

		emit valueChanged(geocentricVelocity);
	}

	Vector3 getVelocity() const override
	{
		double speed = mSpeed->text().toDouble();
		double bearingRad = mBearing->text().toDouble() * degToRadD();
		double inclinationRad = mInclination->text().toDouble() * degToRadD();

		double horizontalSpeed = speed * std::cos(inclinationRad);
		Vector3 ned(
			horizontalSpeed * std::cos(bearingRad),
			horizontalSpeed * std::sin(bearingRad),
			-speed * std::sin(inclinationRad)
		);

		Matrix3 ltpOrientation = geocentricToLtpOrientation(mGeocentricPosition);
		return ltpOrientation * ned;
	}

private:
	QLineEdit* mSpeed;
	QLineEdit* mBearing;
	QLineEdit* mInclination;
};

VelocityEditor::VelocityEditor(QWidget* parent) :
	QWidget(parent)
{
	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	QComboBox* velocityTypeSelector = new QComboBox(this);
	velocityTypeSelector->addItems({ "Geocentric", "North-East-Down", "Speed-Bearing-Inclination" });
	layout->addWidget(velocityTypeSelector);

	mStackedWidget = new QStackedWidget(this);
	mStackedWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	addEditor(new GeocentricVelocityEditor(this));
	addEditor(new NedVelocityEditor(this));
	addEditor(new SpeedBearingInclinationVelocityEditor(this));
	layout->addWidget(mStackedWidget);

	connect(velocityTypeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), mStackedWidget, [=](int index) {
		Vector3 velocity = getCurrentEditor()->getVelocity();
		mStackedWidget->setCurrentIndex(index);
		getCurrentEditor()->setVelocity(velocity);
	});
}

void VelocityEditor::addEditor(VelocityEditorImpl* editor)
{
	mStackedWidget->addWidget(editor);
	editor->setMaximumHeight(50);
	editor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	connect(editor, &VelocityEditorImpl::valueChanged, this, [this](const Vector3& velocity) {
		emit valueChanged(velocity);
	});
}

void VelocityEditor::setGeocentricPosition(const Vector3& position)
{
	for (int i = 0; i < mStackedWidget->count(); ++i)
	{
		static_cast<VelocityEditorImpl*>(mStackedWidget->widget(i))->setGeocentricPosition(position);
	}
}

void VelocityEditor::setVelocity(const Vector3& velocity)
{
	getCurrentEditor()->setVelocity(velocity);
}

Vector3 VelocityEditor::getVelocity() const
{
	return getCurrentEditor()->getVelocity();
}

VelocityEditorImpl* VelocityEditor::getCurrentEditor() const
{
	return static_cast<VelocityEditorImpl*>(mStackedWidget->currentWidget());
}

#include "VelocityEditor.moc"