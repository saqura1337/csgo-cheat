// fully working legitbot

//#include "LegitBot.h"
//
//void LegitBot::normalize_angles(QAngle& angles)
//{
//	if (!angles.IsValid() || angles.IsZero())
//	{
//		return;
//	}
//
//	while (angles.pitch > 89.0f)
//	{
//		angles.pitch -= 180.0f;
//	}
//
//	while (angles.pitch < -89.0f)
//	{
//		angles.pitch += 180.0f;
//	}
//
//	while (angles.yaw < -180.0f)
//	{
//		angles.yaw += 360.0f;
//	}
//
//	while (angles.yaw > 180.0f)
//	{
//		angles.yaw -= 360.0f;
//	}
//
//	angles.roll = 0.0f;
//}
//
//void LegitBot::FindTarget(float& best_fov, Vector& best_target, QAngle aim_punch)
//{
//	C_BaseCombatWeapon* pCombatWeapon = g_sdk.local->m_hActiveWeapon().Get();
//	if (!pCombatWeapon)
//		return;
//
//	const auto local_player_eye_position = g_sdk.local->GetEyePosition();
//
//	const auto config_bone = m_LegitSettings.m_Hitboxes;
//
//	for (auto i = 1; i <= g_interfaces.m_EngineClient->GetMaxClients(); i++)
//	{
//		auto* entity = static_cast<C_BasePlayer*>(g_interfaces.m_EntityList->GetClientEntity(i));
//		if (!entity
//			|| entity == g_sdk.local
//			|| entity->IsDormant()
//			|| !entity->IsAlive()
//			|| entity->m_iTeamNum() == g_sdk.local->m_iTeamNum()
//			|| entity->m_bGunGameImmunity())
//		{
//			continue;
//		}
//
//		for (const auto hitbox : {
//				 HITBOX_HEAD,
//				 HITBOX_NECK,
//				 HITBOX_PELVIS,
//				 HITBOX_STOMACH,
//				 HITBOX_LOWER_CHEST,
//				 HITBOX_CHEST,
//				 HITBOX_UPPER_CHEST,
//				 HITBOX_RIGHT_THIGH,
//				 HITBOX_LEFT_THIGH,
//				 HITBOX_RIGHT_CALF,
//				 HITBOX_LEFT_CALF,
//			})
//		{
//			auto bone_position = entity->GetHitboxPosition(config_bone > 1 ? 10 - config_bone : hitbox);
//			const auto angle =
//				this->CalculateRelativeAngle(local_player_eye_position, bone_position, g_PacketManager->GetModifableCommand()->m_angViewAngles + aim_punch);
//
//			const auto fov = std::hypot(angle.pitch, angle.yaw);
//
//			if (fov > best_fov)
//			{
//				continue;
//			}
//
//			if (!this->IsVisible(entity, local_player_eye_position, bone_position))
//				continue;
//
//			best_fov = fov;
//			best_target = bone_position;
//
//			if (config_bone)
//			{
//				break;
//			}
//		}
//	}
//}
//
//void LegitBot::RunTriggerBot()
//{
//	if (!g_sdk.local || g_sdk.local->IsAlive())
//		return;
//
//	C_BaseCombatWeapon* pCombatWeapon = g_sdk.local->m_hActiveWeapon().Get(); // this->m_hActiveWeapon().Get()
//	if (!pCombatWeapon)
//		return;
//
//	C_CSWeaponData* pWeaponData = pCombatWeapon->GetWeaponData();
//	if (!pWeaponData)
//		return;
//
//	if (!m_LegitSettings.m_bTriggerEnable)
//		return;
//
//	// get crosshair id; https://github.com/frk1/hazedumper/blob/master/csgo.hpp#L71
//	uint32_t CrosshairId = *(uint32_t*)(g_sdk.local + 0x11838);
//
//	if (CrosshairId > 0 && CrosshairId < 32)
//	{
//		for (auto i = 1; i <= g_interfaces.m_EngineClient->GetMaxClients(); i++)
//		{
//			// run if entity
//			auto* entity = static_cast<C_BasePlayer*>(g_interfaces.m_EntityList->GetClientEntity(i));
//			if (!entity
//				|| entity == g_sdk.local
//				|| entity->IsDormant()
//				|| !entity->IsAlive()
//				|| entity->m_iTeamNum() == g_sdk.local->m_iTeamNum() && !m_LegitSettings.m_bFriendlyTrigger
//				|| entity->m_bGunGameImmunity())
//			{
//				continue;
//			}
//
//			// check if alive
//			if (entity->m_iHealth() > 0 && entity->m_iHealth() <= 100)
//			{
//				mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
//				mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
//				Sleep(20);
//			}
//			Sleep(1);
//		}
//	}
//}
//
//void LegitBot::Rcs(QAngle& angle, const bool is_player, const bool shots_fired)
//{
//	const auto& aim_punch = g_sdk.local->m_aimPunchAngle();
//	static auto last_punch = QAngle(0, 0, 0);
//
//	if (is_player)
//	{
//		angle.pitch -= aim_punch.pitch * m_LegitSettings.m_flRcs_x;
//		angle.yaw -= aim_punch.yaw * m_LegitSettings.m_flRcs_y;
//	}
//	else if (shots_fired)
//	{
//		const auto new_punch = QAngle(aim_punch.pitch - last_punch.pitch, aim_punch.yaw - last_punch.yaw, 0);
//		angle.pitch -= new_punch.pitch * m_LegitSettings.m_flRcs_x;
//		angle.yaw -= new_punch.yaw * m_LegitSettings.m_flRcs_y;
//	}
//
//	last_punch = aim_punch;
//}
//
//void LegitBot::Run()
//{
//	C_BaseCombatWeapon* pCombatWeapon = g_sdk.local->m_hActiveWeapon().Get(); // this->m_hActiveWeapon().Get()
//	if (!pCombatWeapon)
//		return;
//
//	C_CSWeaponData* pWeaponData = pCombatWeapon->GetWeaponData();
//	if (!pWeaponData)
//		return;
//
//	int32_t iCurrentWeapon = -1;
//	switch (pCombatWeapon->m_iItemDefinitionIndex())
//	{
//	case WEAPON_AK47:
//	case WEAPON_M4A1:
//	case WEAPON_M4A1_SILENCER:
//	case WEAPON_FAMAS:
//	case WEAPON_SG553:
//	case WEAPON_GALILAR:
//		iCurrentWeapon = RAGE_WEAPON::RIFLE; break;
//	case WEAPON_MAG7:
//	case WEAPON_NOVA:
//	case WEAPON_XM1014:
//	case WEAPON_SAWEDOFF:
//		iCurrentWeapon = RAGE_WEAPON::SHOTGUN; break;
//	case WEAPON_MP7:
//	case WEAPON_MP9:
//	case WEAPON_P90:
//	case WEAPON_M249:
//	case WEAPON_NEGEV:
//	case WEAPON_UMP45:
//		iCurrentWeapon = RAGE_WEAPON::SMG; break;
//	case WEAPON_SCAR20:
//	case WEAPON_G3SG1:
//		iCurrentWeapon = RAGE_WEAPON::AUTO; break;
//	case WEAPON_GLOCK:
//	case WEAPON_HKP2000:
//	case WEAPON_USP_SILENCER:
//	case WEAPON_CZ75A:
//	case WEAPON_TEC9:
//	case WEAPON_ELITE:
//	case WEAPON_FIVESEVEN:
//	case WEAPON_P250:
//		iCurrentWeapon = RAGE_WEAPON::PISTOL; break;
//	case WEAPON_SSG08:
//		iCurrentWeapon = RAGE_WEAPON::SCOUT; break;
//	case WEAPON_AWP:
//		iCurrentWeapon = RAGE_WEAPON::AWP; break;
//	case WEAPON_DEAGLE:
//		iCurrentWeapon = RAGE_WEAPON::DEAGLE; break;
//	case WEAPON_REVOLVER:
//		iCurrentWeapon = RAGE_WEAPON::REVOLVER; break;
//	default: iCurrentWeapon = -1;
//	}
//
//	if (iCurrentWeapon < 0)
//		return;
//
//	// get config data
//	m_LegitSettings = g_Settings->m_aLegitSettings[iCurrentWeapon];
//
//	// get target data
//	auto best_fov = m_LegitSettings.m_flSilentFov + m_LegitSettings.m_flFov;
//	auto best_target = Vector(0, 0, 0);
//	const auto aim_punch = m_LegitSettings.m_bRcsEnable
//		? g_sdk.local->m_aimPunchAngle() * QAngle(m_LegitSettings.m_flRcs_x, m_LegitSettings.m_flRcs_y, 0)
//		: QAngle(0, 0, 0);
//
//	// check legit data
//	if (!m_LegitSettings.m_bEnabled)
//		return;
//
//	if (!(m_LegitSettings.m_bEnabled && (g_PacketManager->GetModifableCommand()->m_nButtons & IN_ATTACK
//		|| m_LegitSettings.m_bAutoFire)))
//	{
//		return;
//	}
//
//	auto weapon_recoil = g_interfaces.m_CVar->FindVar("weapon_recoil_scale");
//
//	// remove recoil
//	m_LegitSettings.m_bRemoveRecoil ? weapon_recoil->SetValue(0) : weapon_recoil->SetValue(2);
//
//	// run triggerbot
//	//RunTriggerBot();
//
//	// find our entity
//	FindTarget(best_fov, best_target, aim_punch);
//
//	const auto can_use_silent = m_LegitSettings.m_bSilent && best_fov <= m_LegitSettings.m_flSilentFov && g_sdk.local->m_iShotsFired() < 1;
//
//	// more rcs sings
//	if (!best_target.IsValid() || best_target.IsZero() || !can_use_silent && best_fov > m_LegitSettings.m_flFov)
//	{
//		if (m_LegitSettings.m_bRcsEnable)
//		{
//			Rcs(g_PacketManager->GetModifableCommand()->m_angViewAngles, false, g_sdk.local->m_iShotsFired() > 1);
//
//			g_interfaces.m_EngineClient->SetViewAngles(&g_PacketManager->GetModifableCommand()->m_angViewAngles);
//		}
//
//		return;
//	}
//
//	if (g_sdk.local->m_iShotsFired() < 1 && CrosshairOnEnemy(g_PacketManager->GetModifableCommand()->m_angViewAngles))
//	{
//		return;
//	}
//
//	static auto last_angles = g_PacketManager->GetModifableCommand()->m_angViewAngles;
//	static auto last_command = 0;
//
//	const auto local_player_eye_position = g_sdk.local->GetEyePosition();
//
//	auto angle = CalculateRelativeAngle(local_player_eye_position, best_target, g_PacketManager->GetModifableCommand()->m_angViewAngles + aim_punch);
//	normalize_angles(angle);
//	Math::ClampAngles(angle);
//
//	if (!can_use_silent || last_command > g_PacketManager->GetModifableCommand()->m_nCommand || last_angles.IsZero())
//	{
//		angle /= g_sdk.local->m_iShotsFired() > 1 ? m_LegitSettings.m_flRcsSmooth : m_LegitSettings.m_flSmooth;
//	}
//
//	g_PacketManager->GetModifableCommand()->m_angViewAngles += angle;
//
//	normalize_angles(g_PacketManager->GetModifableCommand()->m_angViewAngles);
//
//	auto clamped = false;
//	if (std::abs(g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch) > 255.0f || std::abs(g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw) > 255.0f)
//	{
//		g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch = std::clamp(g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch, -255.0f, 255.0f);
//		g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw = std::clamp(g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw, -255.0f, 255.0f);
//		clamped = true;
//	}
//
//	// do auto fire
//	if (m_LegitSettings.m_bAutoFire && !clamped)
//		g_PacketManager->GetModifableCommand()->m_nButtons |= IN_ATTACK;
//
//	// do scope
//	if (m_LegitSettings.m_bAutoScope && pCombatWeapon->IsSniper() && g_PacketManager->GetModifableCommand()->m_nButtons & IN_ATTACK2)
//		g_PacketManager->GetModifableCommand()->m_nButtons |= IN_ATTACK2;
//
//	if (clamped)
//	{
//		g_PacketManager->GetModifableCommand()->m_nButtons &= ~IN_ATTACK;
//	}
//
//	// smooth
//	if (clamped || m_LegitSettings.m_flSmooth >= 1.0f)
//	{
//		last_angles = g_PacketManager->GetModifableCommand()->m_angViewAngles;
//	}
//	else
//	{
//		last_angles = QAngle(0, 0, 0);
//	}
//
//	last_command = g_PacketManager->GetModifableCommand()->m_nCommand;
//
//	// if local didnt use silent, break angle data to def.
//	if (!can_use_silent)
//	{
//		g_interfaces.m_EngineClient->SetViewAngles(&g_PacketManager->GetModifableCommand()->m_angViewAngles);
//	}
//}