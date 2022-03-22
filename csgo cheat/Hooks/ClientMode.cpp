#include "../Hooks.hpp"
#include "../Menu.hpp"
#include "../Settings.hpp"
#include "../Features/Visuals/Players.hpp"
#include "../Features/Visuals/Thirdperson.hpp"
#include "../Features/Visuals/ShotChams.hpp"
#include "../SDK/Math/Math.hpp"

int __fastcall C_Hooks::hkDoPostScreenEffects(LPVOID pEcx, uint32_t)
{
	if (g_sdk.local)
	{
		g_ShotChams->OnDrawModel();
		g_PlayerESP->RenderGlow();
	}

	return g_sdk.hooks.originals.m_DoPostScreenEffects(pEcx);
}

void __fastcall C_Hooks::hkOverrideView(LPVOID pEcx, uint32_t, C_ViewSetup* pSetupView)
{
	if (!g_sdk.local)
		return g_sdk.hooks.originals.m_OverrideView(pEcx, pSetupView);

	if (!g_sdk.local->IsAlive())
	{
		g_interfaces.input->m_bCameraInThirdPerson = false;
		return g_sdk.hooks.originals.m_OverrideView(pEcx, pSetupView);
	}

	if (g_sdk.packet.m_bVisualFakeDuck)
		pSetupView->vecOrigin = g_sdk.local->GetAbsOrigin() + g_interfaces.game_movement->GetPlayerViewOffset(false);

	if (!g_sdk.local->m_bIsScoped() || g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_ZOOM])
		pSetupView->flFOV = g_cfg->m_iCameraDistance;

	if (g_sdk.local->m_hViewModel().Get())
	{
		QAngle angViewSetup = pSetupView->angView;
		if (g_cfg->m_iViewmodelRoll)
			g_sdk.local->m_hViewModel()->SetAbsoluteAngles(QAngle(angViewSetup.pitch, angViewSetup.yaw, g_cfg->m_iViewmodelRoll));
	}

	if (!g_sdk.local->m_bIsScoped() || g_cfg->m_bOverrideFOVWhileScoped)
		pSetupView->flFOV = g_cfg->m_iCameraDistance;

	g_ThirdPerson->Thirdperson(g_sdk.packet.m_bFakeDuck);
	//g_sdk.hooks.originals.m_OverrideView(pEcx, pSetupView);

	//if (g_sdk.packet.m_bFakeDuck)
	//{
	//	pSetupView->vecOrigin = g_sdk.local->GetAbsOrigin() + Vector(0.0f, 0.0f, g_interfaces.game_movement->GetPlayerViewOffset(false).z + 0.064f);

	//	if (g_interfaces.input->m_bCameraInThirdPerson)
	//	{
	//		auto camera_angles = Vector(
	//			g_interfaces.input->m_vecCameraOffset.x,
	//			g_interfaces.input->m_vecCameraOffset.y,
	//			0.0f
	//		);

	//		auto camera_forward = Vector(0,0,0);

	//		Math::angle_vectors(camera_angles, camera_forward);
	//		Math::VectorMA(pSetupView->vecOrigin, -g_interfaces.input->m_vecCameraOffset.z, camera_forward, pSetupView->vecOrigin);
	//	}
	//}

	return g_sdk.hooks.originals.m_OverrideView(pEcx, pSetupView);
}

float_t __fastcall C_Hooks::hkGetViewmodelFOV(LPVOID pEcx, uint32_t)
{
	return g_cfg->m_iViewmodelDistance;
}