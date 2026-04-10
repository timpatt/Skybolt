/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "CameraController.h"
#include "Pitchable.h"
#include "Yawable.h"

namespace skybolt {
namespace sim {

class FreeCameraController : public CameraController, public Pitchable, public Yawable
{
public:
	FreeCameraController(Entity* camera);

	void update(SecondsD dt) override;
	void setInput(const Input& input) override { mInput = input; }

	double getZoom() const;
	void setZoom(double zoom);

	double minFovY = math::degToRadD() * 10.0;
	double maxFovY = math::degToRadD() * 120.0;

private:
	Input mInput = Input::zero();
};

SKYBOLT_REFLECT_EXTERN(FreeCameraController);

} // namespace sim
} // namespace skybolt