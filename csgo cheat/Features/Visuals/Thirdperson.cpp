#include "Thirdperson.hpp"
#include "../Settings.hpp"
#include "../Tools/Tools.hpp"
#include "../SDK/Math/Math.hpp"

void C_ThirdPerson::Init(const bool fakeducking, const float progress)
{
	if (!g_sdk.local)
		return;

	/* our current fraction. */
	static float current_fraction = 0.0f;

	const auto distance = static_cast<float>(g_cfg->m_iThirdPersonDistance) * progress;
	QAngle angles, inverse_angles;

	// get camera angles.
	g_interfaces.engine->GetViewAngles(&angles);
	g_interfaces.engine->GetViewAngles(&inverse_angles);

	// cam_idealdist convar.
	inverse_angles.roll = distance;

	// set camera direction.
	Vector forward, right, up;
	Math::angle_vectors(inverse_angles, &forward, &right, &up);

	// various fixes to camera when fakeducking.
	const auto eye_pos = fakeducking
		? g_sdk.local->GetAbsOrigin() + g_interfaces.game_movement->
		GetPlayerViewOffset(false)
		: g_sdk.local->GetAbsOrigin() + g_sdk.local->m_vecViewOffset();
	const auto offset = eye_pos + forward * -distance + right + up;

	// setup trace filter and trace.
	CTraceFilterWorldOnly filter;
	trace_t tr;
	Ray_t ray;
	ray.Init(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f));

	// tracing to camera angles.
	g_interfaces.trace->TraceRay(ray, 131083, &filter, &tr);

	// interpolate camera speed if something behind our camera.
	if (current_fraction > tr.fraction)
	{
		current_fraction = tr.fraction;
	}
	else if (current_fraction > 0.9999f)
	{
		current_fraction = 1.0f;
	}

	// adapt distance to travel time.
	current_fraction = Math::Interpolate(current_fraction,
		tr.fraction,
		g_interfaces.globals->m_frametime * 4.0f);
	angles.roll = distance * current_fraction;

	// override camera angles.
	g_interfaces.input->m_vecCameraOffset = Vector(angles.pitch, angles.yaw, angles.roll);
}

void C_ThirdPerson::Thirdperson(const bool fakeducking)
{
	/* thirdperson code. */
	{
		static float progress;
		static bool in_transition;
		static auto in_thirdperson = false;

		if (!in_thirdperson && g_Tools->IsBindActive(g_cfg->m_aThirdPerson))
		{
			in_thirdperson = true;
		}
		else if (in_thirdperson && !g_Tools->IsBindActive(g_cfg->m_aThirdPerson))
		{
			in_thirdperson = false;
		}

		if (g_sdk.local->IsAlive() && in_thirdperson)
		{
			in_transition = false;

			if (!g_interfaces.input->m_bCameraInThirdPerson)
			{
				g_interfaces.input->m_bCameraInThirdPerson = true;
			}
		}
		else
		{
			progress -= g_interfaces.globals->m_frametime * 4.f + progress / 100;
			progress = std::clamp(progress, 0.f, 1.f);

			if (!progress)
			{
				g_interfaces.input->m_bCameraInThirdPerson = false;
			}
			else
			{
				in_transition = true;
				g_interfaces.input->m_bCameraInThirdPerson = true;
			}
		}

		if (g_interfaces.input->m_bCameraInThirdPerson && !in_transition)
		{
			progress += g_interfaces.globals->m_frametime * 4.f + progress / 100;
			progress = std::clamp(progress, 0.f, 1.f);
		}

		Init(fakeducking, progress);
	}

	{
		static auto require_reset = false;

		if (g_sdk.local->IsAlive())
		{
			require_reset = false;
			return;
		}

		if (require_reset)
			g_sdk.local->m_iObserverMode() = OBS_MODE_CHASE;

		if (g_sdk.local->m_iObserverMode() == OBS_MODE_IN_EYE)
			require_reset = true;
	}
}
