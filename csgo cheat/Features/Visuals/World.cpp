#include "World.hpp"
#include "../Render.hpp"
#include "../Settings.hpp"
#include "../Tools/Tools.hpp"
#include "../Grenades/Warning.hpp"
#include "../RageBot/Autowall.hpp"
#include "../Grenades/Trace.h"

void C_World::Instance(ClientFrameStage_t Stage)
{
	if (Stage != ClientFrameStage_t::FRAME_RENDER_START)
		return;

	if (g_cfg->m_bUnhideConvars)
	{
		if (!m_bDidUnlockConvars)
		{
			auto pIterator = **reinterpret_cast<ConCommandBase***>(reinterpret_cast<DWORD>(g_interfaces.cvar) + 0x34);
			for (auto c = pIterator->m_pNext; c != nullptr; c = c->m_pNext)
			{
				c->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
				c->m_nFlags &= ~FCVAR_HIDDEN;
			}

			m_bDidUnlockConvars = true;
		}
	}

	if (g_cfg->m_iViewmodelX != g_sdk.convars.m_ViewmodelX->GetInt())
		g_sdk.convars.m_ViewmodelX->SetValue(g_cfg->m_iViewmodelX);

	if (g_cfg->m_iViewmodelY != g_sdk.convars.m_ViewmodelY->GetInt())
		g_sdk.convars.m_ViewmodelY->SetValue(g_cfg->m_iViewmodelY);

	if (g_cfg->m_iViewmodelZ != g_sdk.convars.m_ViewmodelZ->GetInt())
		g_sdk.convars.m_ViewmodelZ->SetValue(g_cfg->m_iViewmodelZ);

	g_interfaces.cvar->FindVar(_S("con_filter_text"))->SetValue(_S("eqwoie2ue398129e8wuq12we9yw98d"));
	if (g_interfaces.cvar->FindVar(_S("con_filter_enable"))->GetBool() != g_cfg->m_bFilterConsole)
	{
		g_interfaces.cvar->FindVar(_S("con_filter_enable"))->SetValue(g_cfg->m_bFilterConsole);
		g_interfaces.engine->ExecuteClientCmd(_S("clear"));
	}

	this->RemoveHandShaking();
	this->RemoveShadows();
	this->RemoveSmokeAndPostProcess();
	this->SunsetMode();
	this->ViewmodelDistansePfix();
	this->DisablePanorama();

	static C_Material* pBlurOverlay = g_interfaces.material_system->FindMaterial(_S("dev/scope_bluroverlay"), TEXTURE_GROUP_OTHER);
	static C_Material* pScopeDirt = g_interfaces.material_system->FindMaterial(_S("models/weapons/shared/scope/scope_lens_dirt"), TEXTURE_GROUP_OTHER);

	pBlurOverlay->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
	pScopeDirt->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

	C_BaseCombatWeapon* pCombatWeapon = g_sdk.local->m_hActiveWeapon().Get();
	if (pCombatWeapon)
	{
		if (pCombatWeapon->IsSniper())
		{
			if (g_sdk.local->m_bIsScoped())
			{
				if (g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SCOPE])
				{
					pBlurOverlay->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
					pScopeDirt->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
			}
		}
	}

	this->Clantag();
	if (g_cfg->m_bForceCrosshair)
		g_sdk.convars.m_WeaponDebugShowSpread->SetValue(3);
	else
		g_sdk.convars.m_WeaponDebugShowSpread->SetValue(0);

	this->DrawClientImpacts();
	this->LeftHandKnife();
	return this->SkyboxChanger();
}

void C_World::OnBulletImpact(C_GameEvent* pEvent)
{
	Vector vecPosition = Vector(pEvent->GetInt(_S("x")), pEvent->GetInt(_S("y")), pEvent->GetInt(_S("z")));

	C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))));
	if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive())
		return;

	if (pPlayer == g_sdk.local)
	{
		if (g_cfg->m_bDrawServerImpacts)
			g_interfaces.debug_overlay->BoxOverlay
			(
				vecPosition,
				Vector(-2.0f, -2.0f, -2.0f),
				Vector(2.0f, 2.0f, 2.0f),
				QAngle(0.0f, 0.0f, 0.0f),
				g_cfg->m_ServerImpacts->r(),
				g_cfg->m_ServerImpacts->g(),
				g_cfg->m_ServerImpacts->b(),
				g_cfg->m_ServerImpacts->a(),
				4.0f
			);

		return;
	}

	if (!g_cfg->m_bDrawEnemyTracers || pPlayer->m_iTeamNum() == g_sdk.local->m_iTeamNum())
		return;

	C_BulletTrace BulletTrace;

	BulletTrace.m_bIsLocalTrace = false;
	BulletTrace.m_vecEndPosition = vecPosition;
	BulletTrace.m_vecStartPosition = pPlayer->GetAbsOrigin() + pPlayer->m_vecViewOffset();

	m_BulletTracers.emplace_back(BulletTrace);
}

void C_World::PreserveKillfeed()
{
	if (!g_interfaces.engine->IsInGame() || !g_interfaces.engine->IsConnected())
		return;

	static PDWORD pHudDeathNotice = NULL;
	if (!g_sdk.local || !g_sdk.local->IsAlive())
	{
		pHudDeathNotice = NULL;
		return;
	}

	if (g_sdk.local->m_fFlags() & FL_FROZEN || g_sdk.local->m_bGunGameImmunity() /*|| ( *g_interfaces.game_rules )->IsFreezePeriod( ) */)
	{
		pHudDeathNotice = NULL;
		return;
	}

	if (!pHudDeathNotice)
	{
		pHudDeathNotice = g_Tools->FindHudElement(_S("CCSGO_HudDeathNotice"));
		return;
	}

	PFLOAT pNoticeExpireTime = (PFLOAT)((DWORD)(pHudDeathNotice)+0x50);
	if (pNoticeExpireTime)
		*pNoticeExpireTime = g_cfg->m_bPreserveKillfeed ? FLT_MAX : 1.5f;

	if (g_sdk.round_info.m_bShouldClearDeathNotices)
		((void(__thiscall*)(DWORD))(g_sdk.address_list.m_ClearDeathList))((DWORD)(pHudDeathNotice)-20);

	g_sdk.round_info.m_bShouldClearDeathNotices = false;
}

void C_World::SunsetMode()
{
	if (g_cfg->bSunSetMode)
	{
		g_interfaces.cvar->FindVar(_S("cl_csm_rot_override"))->SetValue(1);
		g_interfaces.cvar->FindVar(_S("cl_csm_rot_x"))->SetValue(g_cfg->m_iSunSetX);
		g_interfaces.cvar->FindVar(_S("cl_csm_rot_y"))->SetValue(g_cfg->m_iSunSetY);
		g_interfaces.cvar->FindVar(_S("cl_csm_rot_z"))->SetValue(g_cfg->m_iSunSetZ);
	}

	else
	{
		g_interfaces.cvar->FindVar(_S("cl_csm_rot_override"))->SetValue(0);
	}
}

void C_World::Clantag()
{
	auto apply = [](const char* tag) -> void
	{
		using Fn = int(__fastcall*)(const char*, const char*);
		static auto fn = reinterpret_cast<Fn>(g_Tools->FindPattern(GetModuleHandleA(_S("engine.dll")), _S("53 56 57 8B DA 8B F9 FF 15")));

		fn(tag, tag);
	};

	static auto removed = false;

	if (!g_cfg->m_bTagChanger && !removed)
	{
		removed = true;
		apply(_S(""));
		return;
	}

	if (g_cfg->m_bTagChanger)
	{
		auto nci = g_interfaces.engine->GetNetChannelInfo();

		if (!nci)
			return;

		static auto time = -1;

		auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)g_interfaces.globals->m_tickcount;
		auto intervals = 0.5f / g_interfaces.globals->m_intervalpertick;

		auto main_time = (int)(ticks / intervals) % 21;

		if (main_time != time && !g_interfaces.client_state->m_nChokedCommands())
		{
			auto tag = "";
			switch (main_time)
			{
			case 0: tag = ""; break;
			case 1: tag = "r"; break;
			case 2: tag = "r3"; break;
			case 3: tag = "r3b"; break;
			case 4: tag = "r3b0"; break;
			case 5: tag = "r3b0r"; break;
			case 6: tag = "r3b0rn"; break;
			case 7: tag = "r3b0rn h"; break;
			case 8: tag = "r3b0rn h@"; break;
			case 9: tag = "r3b0rn h@c"; break;
			case 10: tag = "r3b0rn h@ck"; break;
			case 11: tag = "r3b0rn h@ck"; break;
			case 12: tag = "r3b0rn h@c"; break;
			case 13: tag = "r3b0rn h@"; break;
			case 14: tag = "r3b0rn h"; break;
			case 15: tag = "r3b0rn "; break;
			case 16: tag = "r3b0rn"; break;
			case 17: tag = "r3b0r"; break;
			case 18: tag = "r3b0"; break;
			case 19: tag = "r3b"; break;
			case 20: tag = "r3"; break;
			case 21: tag = "r"; break;
			}

			apply(tag);
			time = main_time;
		}

		removed = false;
	}
}

void C_World::PostFrame(ClientFrameStage_t Stage)
{
	if (Stage != ClientFrameStage_t::FRAME_START)
		return;

	return this->DrawBulletTracers();
}

void C_World::OnRageBotFire(Vector vecStartPosition, Vector vecEndPosition)
{
	if (!g_cfg->m_bDrawLocalTracers)
		return;

	C_BulletTrace BulletTrace;

	BulletTrace.m_bIsLocalTrace = true;
	BulletTrace.m_vecStartPosition = vecStartPosition;
	BulletTrace.m_vecEndPosition = vecEndPosition;

	m_BulletTracers.emplace_back(BulletTrace);
}

void C_World::DrawScopeLines()
{
	if (!g_sdk.local || !g_sdk.local->IsAlive())
		return;

	C_BaseCombatWeapon* pCombatWeapon = g_sdk.local->m_hActiveWeapon().Get();
	if (pCombatWeapon)
	{
		if (pCombatWeapon->IsSniper())
		{
			if (g_sdk.local->m_bIsScoped())
			{
				if (g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SCOPE])
				{
					int32_t iScreenSizeX, iScreenSizeY;
					g_interfaces.engine->GetScreenSize(iScreenSizeX, iScreenSizeY);

					auto cone = g_sdk.local->m_hActiveWeapon().Get()->GetSpread() + g_sdk.local->m_hActiveWeapon().Get()->GetInaccuracy();

					if (cone <= 0.f) return;

					auto size = cone * iScreenSizeY * 0.7f;

					g_Render->RenderLine(0, iScreenSizeY / 2, iScreenSizeX / 2 - size, iScreenSizeY / 2, Color(0, 0, 0), 1.0f);
					g_Render->RenderLine(iScreenSizeX, iScreenSizeY / 2, iScreenSizeX / 2 + size, iScreenSizeY / 2, Color(0, 0, 0), 1.0f);
					g_Render->RenderLine(iScreenSizeX / 2, 0, iScreenSizeX / 2, iScreenSizeY / 2 - size, Color(0, 0, 0), 1.0f);
					g_Render->RenderLine(iScreenSizeX / 2, iScreenSizeY, iScreenSizeX / 2, iScreenSizeY / 2 + size, Color(0, 0, 0), 1.0f);
				}
			}
		}
	}
}

void C_World::Grenades()
{
	if (!g_sdk.local)
		return;

	if (!g_cfg->m_bPredictGrenades)
		return;

	static auto last_server_tick = g_interfaces.client_state->m_ClockDriftMgr().m_nServerTick;
	if (last_server_tick != g_interfaces.client_state->m_ClockDriftMgr().m_nServerTick) {
		g_GrenadePrediction->get_list().clear();

		last_server_tick = g_interfaces.client_state->m_ClockDriftMgr().m_nServerTick;
	}

	for (int32_t i = 1; i < g_interfaces.entity_list->GetHighestEntityIndex(); i++)
	{
		C_BaseEntity* pBaseEntity = static_cast<C_BaseEntity*>(g_interfaces.entity_list->GetClientEntity(i));
		if (!pBaseEntity || pBaseEntity->IsDormant())
			continue;

		const auto client_class = pBaseEntity->GetClientClass();
		if (pBaseEntity->GetClientClass()->m_ClassID == ClassId_CInferno)
		{
			float_t flSpawnTime = *(float_t*)((DWORD)(pBaseEntity)+0x2D8);
			float_t flPercentage = ((flSpawnTime + 7.0f) - g_interfaces.globals->m_curtime) / 7.0f;

			Vector vecScreenPosition = Vector(0, 0, 0);
			if (!g_interfaces.debug_overlay->ScreenPosition(pBaseEntity->m_vecOrigin(), vecScreenPosition))
			{
				ImVec2 vecTextSize = g_sdk.fonts.m_BigIcons->CalcTextSizeA(20.0f, FLT_MAX, NULL, _S("l"));
				g_Render->RenderText(_S("l"), ImVec2(vecScreenPosition.x - vecTextSize.x / 2 + 1, vecScreenPosition.y - vecTextSize.y / 2), Color::White, false, true, g_sdk.fonts.m_BigIcons);

				//if ( g_cfg->m_GrenadeTimers )
				{
					g_Render->RenderRectFilled(vecScreenPosition.x - 21, vecScreenPosition.y + 14, vecScreenPosition.x + 21, vecScreenPosition.y + 20, Color(0.0f, 0.0f, 0.0f, 100.0f));
					g_Render->RenderRectFilled(vecScreenPosition.x - 20, vecScreenPosition.y + 15, vecScreenPosition.x - 19.0f + 39.0f * flPercentage, vecScreenPosition.y + 19, Color(g_cfg->m_GrenadeWarningTimer));
				}

				//g_Render->RenderCircle3D( pBaseEntity->m_vecOrigin( ), 32, 170, Color( 200, 0, 0 ) );
			}
		}
		else if (pBaseEntity->GetClientClass()->m_ClassID == 157)
		{
			float_t flSpawnTime = *(float_t*)((DWORD)(pBaseEntity)+0x2D8);
			if (flSpawnTime > 0.0f)
			{
				float_t flPercentage = ((flSpawnTime + 17.0f) - g_interfaces.globals->m_curtime) / 17.0f;

				Vector vecScreenPosition = Vector(0, 0, 0);
				if (!g_interfaces.debug_overlay->ScreenPosition(pBaseEntity->m_vecOrigin(), vecScreenPosition))
				{
					ImVec2 vecTextSize = g_sdk.fonts.m_BigIcons->CalcTextSizeA(20.0f, FLT_MAX, NULL, _S("k"));
					g_Render->RenderText(_S("k"), ImVec2(vecScreenPosition.x - vecTextSize.x / 2 + 1, vecScreenPosition.y - vecTextSize.y / 2), Color::White, false, true, g_sdk.fonts.m_BigIcons);

					//if ( g_cfg->m_GrenadeTimers )
					{
						g_Render->RenderRectFilled(vecScreenPosition.x - 21, vecScreenPosition.y + 14, vecScreenPosition.x + 21, vecScreenPosition.y + 20, Color(0.0f, 0.0f, 0.0f, 100.0f));
						g_Render->RenderRectFilled(vecScreenPosition.x - 20, vecScreenPosition.y + 15, vecScreenPosition.x - 19.0f + 39.0f * flPercentage, vecScreenPosition.y + 19, Color(g_cfg->m_GrenadeWarningTimer));
					}

					//g_Render->RenderCircle3D( pBaseEntity->m_vecOrigin( ), 32, 170, Color( 0, 128, 255 ) );
				}
			}
		}

		if (!client_class
			|| client_class->m_ClassID != 114 && client_class->m_ClassID != ClassId_CBaseCSGrenadeProjectile)
			continue;

		if (client_class->m_ClassID == ClassId_CBaseCSGrenadeProjectile) {
			const auto model = pBaseEntity->GetModel();
			if (!model)
				continue;

			const auto studio_model = g_interfaces.model_info->GetStudiomodel(model);
			if (!studio_model
				/*|| std::string_view( studio_model->szName ).find( "fraggrenade" ) == std::string::npos */)
				continue;

			if (std::string_view(studio_model->szName).find("thrown") != std::string::npos ||
				client_class->m_ClassID == ClassId_CBaseCSGrenadeProjectile || client_class->m_ClassID == ClassId_CDecoyProjectile || client_class->m_ClassID == ClassId_CMolotovProjectile)
			{
				g_NadeTracer->AddTracer((C_BasePlayer*)pBaseEntity, Color::White, 60);
				g_NadeTracer->Draw();
			}
		}

		const auto handle = pBaseEntity->GetRefEHandle().ToLong();
		if (pBaseEntity->m_nExplodeEffectTickBegin())
		{
			g_GrenadePrediction->get_list().erase(handle);
			continue;
		}

		if (g_GrenadePrediction->get_list().find(handle) == g_GrenadePrediction->get_list().end()) {
			g_GrenadePrediction->get_list()[handle] =
				C_GrenadePrediction::data_t
				(
					reinterpret_cast<C_BaseCombatWeapon*>(pBaseEntity)->m_hThrower().Get(),
					client_class->m_ClassID == 114 ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
					pBaseEntity->m_vecOrigin(),
					reinterpret_cast<C_BasePlayer*>(pBaseEntity)->m_vecVelocity(),
					pBaseEntity->m_flCreationTime(),
					TIME_TO_TICKS(reinterpret_cast<C_BasePlayer*>(pBaseEntity)->m_flSimulationTime() - pBaseEntity->m_flCreationTime())
				);
		}

		if (g_GrenadePrediction->get_list().at(handle).draw())
			continue;

		g_GrenadePrediction->get_list().erase(handle);
	}

	g_GrenadePrediction->get_local_data().draw();
}

void C_World::RemoveHandShaking()
{
	if (g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_HAND_SHAKING])
		return g_sdk.convars.m_ClWpnSwayAmount->SetValue(0.0f);

	return g_sdk.convars.m_ClWpnSwayAmount->SetValue(1.6f);
}

void C_World::BombTimer(C_GameEvent* pEvent, C_BaseEntity* entity)
{
	if (!g_cfg->m_bBombTimer)
		return;

	bool bomb_timer_enable = false;

	bomb_timer_enable = true;
	if (!strcmp(pEvent->GetName(), _S("bomb_defused")))
	bomb_timer_enable = false;

	if (!bomb_timer_enable)
		return;

	static auto mp_c4timer = g_interfaces.cvar->FindVar(_S("mp_c4timer"));
	auto bomb = (CCSBomb*)entity;

	auto c4timer = mp_c4timer->GetFloat();
	auto bomb_timer = bomb->m_flC4Blow() - g_interfaces.globals->m_curtime;

	if (bomb_timer < 0.0f)
		return;

	static int width, height;
	g_interfaces.engine->GetScreenSize(width, height);

	auto factor = bomb_timer / c4timer * height;

	auto red_factor = (int)(255.0f - bomb_timer / c4timer * 255.0f);
	auto green_factor = (int)(bomb_timer / c4timer * 255.0f);

	g_Render->RenderRectFilled(0, height - factor, 26, factor, Color(red_factor, green_factor, 0, 100));

	auto text_position = height - factor + 11;

	if (text_position > height - 9)
		text_position = height - 9;

	Vector screen;

	if (Math::WorldToScreen(entity->GetAbsOrigin(), screen))
		g_Render->RenderText(_S("BOMB"), ImVec2(screen.x, screen.y), Color(255, 255, 255), false, false, g_sdk.fonts.m_SegoeUI);
}

void C_World::LeftHandKnife()
{
	static auto left_knife = g_interfaces.cvar->FindVar(_S("cl_righthand"));

	if (!g_sdk.local || !g_sdk.local->IsAlive() || !g_cfg->m_bLeftKnifeHand)
	{
		left_knife->SetValue(1);
		return;
	}

	auto weapon = g_sdk.local->m_hActiveWeapon().Get();
	if (!weapon) return;

	left_knife->SetValue(!weapon->IsKnife());
}

void C_World::RemoveSmokeAndPostProcess()
{
	static std::vector< std::string > aMaterialList =
	{
		_S("particle/vistasmokev1/vistasmokev1_emods"),
		_S("particle/vistasmokev1/vistasmokev1_emods_impactdust"),
		_S("particle/vistasmokev1/vistasmokev1_fire"),
		_S("particle/vistasmokev1/vistasmokev1_smokegrenade"),
	};

	for (auto strSmokeMaterial : aMaterialList)
	{
		C_Material* pMaterial = g_interfaces.material_system->FindMaterial(strSmokeMaterial.c_str(), _S(TEXTURE_GROUP_OTHER));
		if (!pMaterial || pMaterial->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW))
			continue;

		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SMOKE]);
	}

	if (*(int32_t*)(*reinterpret_cast <uint32_t**>((uint32_t)(g_sdk.address_list.m_SmokeCount))) != 0)
	{
		if (g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SMOKE])
			*(int32_t*)(*reinterpret_cast <uint32_t**>((uint32_t)(g_sdk.address_list.m_SmokeCount))) = 0;
	}

	**reinterpret_cast <bool**> (reinterpret_cast <uint32_t> (g_sdk.address_list.m_PostProcess)) = g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_POSTPROCESS];
}

void C_World::RemoveShadows()
{
	g_sdk.convars.m_ClFootContactShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
	g_sdk.convars.m_ClCsmStaticPropShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
	g_sdk.convars.m_ClCsmWorldShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
	g_sdk.convars.m_ClCsmShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
	g_sdk.convars.m_ClCsmViewmodelShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
	g_sdk.convars.m_ClCsmSpriteShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
	g_sdk.convars.m_ClCsmRopeShadows->SetValue(!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_SHADOWS]);
}

void C_World::DrawBulletTracers()
{
	for (int32_t iPosition = 0; iPosition < m_BulletTracers.size(); iPosition++)
	{
		auto Trace = &m_BulletTracers[iPosition];

		Color aBulletColor = Color(g_cfg->m_EnemyTracers);
		if (Trace->m_bIsLocalTrace)
			aBulletColor = Color(g_cfg->m_LocalTracers);

		BeamInfo_t BeamInfo = BeamInfo_t();

		BeamInfo.m_vecStart = Trace->m_vecStartPosition;
		if (Trace->m_bIsLocalTrace)
			BeamInfo.m_vecStart = Vector(Trace->m_vecStartPosition.x, Trace->m_vecStartPosition.y, Trace->m_vecStartPosition.z - 2.0f);

		BeamInfo.m_vecEnd = Trace->m_vecEndPosition;
		BeamInfo.m_nModelIndex = -1;
		BeamInfo.m_flHaloScale = 0.0f;
		BeamInfo.m_flLife = 4.0f;
		BeamInfo.m_flFadeLength = 0.0f;
		BeamInfo.m_flWidth = 0.75f;
		BeamInfo.m_flEndWidth = 0.75f;
		BeamInfo.m_flAmplitude = 1.f;

		BeamInfo.m_nStartFrame = 0;
		BeamInfo.m_flRed = aBulletColor.r();
		BeamInfo.m_flGreen = aBulletColor.g();
		BeamInfo.m_flBlue = aBulletColor.b();
		BeamInfo.m_flBrightness = aBulletColor.a();

		BeamInfo.m_bRenderable = true;
		BeamInfo.m_nSegments = 2;
		BeamInfo.m_nFlags = 0;

		BeamInfo.m_flFrameRate = 0.0f;
		BeamInfo.m_pszModelName = "sprites/physbeam.vmt";
		BeamInfo.m_nType = TE_BEAMPOINTS;

		Beam_t* Beam = g_interfaces.view_render_beams->CreateBeamPoints(BeamInfo);
		if (Beam)
			g_interfaces.view_render_beams->DrawBeam(Beam);

		m_BulletTracers.erase(m_BulletTracers.begin() + iPosition);
	}
}

void C_World::DrawClientImpacts()
{
	if (!g_cfg->m_bDrawClientImpacts)
		return;

	auto& aClientImpactList = *(CUtlVector< ClientImpact_t >*)((uintptr_t)(g_sdk.local) + 0x11C50);
	for (auto Impact = aClientImpactList.Count(); Impact > m_iLastProcessedImpact; --Impact)
		g_interfaces.debug_overlay->BoxOverlay(
			aClientImpactList[Impact - 1].m_vecPosition,
			Vector(-2.0f, -2.0f, -2.0f),
			Vector(2.0f, 2.0f, 2.0f),
			QAngle(0.0f, 0.0f, 0.0f),
			g_cfg->m_ClientImpacts->r(),
			g_cfg->m_ClientImpacts->g(),
			g_cfg->m_ClientImpacts->b(),
			g_cfg->m_ClientImpacts->a(),
			4.0f);

	if (aClientImpactList.Count() != m_iLastProcessedImpact)
		m_iLastProcessedImpact = aClientImpactList.Count();
}

void C_World::ViewmodelDistansePfix()
{
	if (g_cfg->m_iViewmodelDistanceFix)
	{
		auto viewFOV = (float)g_cfg->m_iViewmodelDistanceFix + 68.0f;
		static auto viewFOVcvar = g_interfaces.cvar->FindVar(_S("viewmodel_fov"));

		if (viewFOVcvar->GetFloat() != viewFOV)
		{
			*(float*)((DWORD)&viewFOVcvar->m_fnChangeCallbacks + 0xC) = 0.0f;
			viewFOVcvar->SetValue(viewFOV);
		}
	}
}

void C_World::PingModulation()
{
	int value = g_cfg->m_iPingSpike / 2;
	ConVar* net_fakelag = g_interfaces.cvar->FindVar(_S("net_fakelag"));
	net_fakelag->SetValue(value);
}

void C_World::DisablePanorama()
{
	if (g_cfg->m_bPanoramaBlur)
	{
		g_interfaces.cvar->FindVar(_S("@panorama_disable_blur"))->SetValue(true);
	}
	else
	{
		g_interfaces.cvar->FindVar(_S("@panorama_disable_blur"))->SetValue(false);
	}
}

void C_World::SkyboxChanger()
{
	std::string strSkyBox = g_interfaces.cvar->FindVar(_S("sv_skyname"))->GetString();
	switch (g_cfg->m_iSkybox)
	{
	case 1:
		strSkyBox = _S("cs_tibet");
		break;
	case 2:
		strSkyBox = _S("cs_baggage_skybox_");
		break;
	case 3:
		strSkyBox = _S("italy");
		break;
	case 4:
		strSkyBox = _S("jungle");
		break;
	case 5:
		strSkyBox = _S("office");
		break;
	case 6:
		strSkyBox = _S("sky_cs15_daylight01_hdr");
		break;
	case 7:
		strSkyBox = _S("sky_cs15_daylight02_hdr");
		break;
	case 8:
		strSkyBox = _S("vertigoblue_hdr");
		break;
	case 9:
		strSkyBox = _S("vertigo");
		break;
	case 10:
		strSkyBox = _S("sky_day02_05_hdr");
		break;
	case 11:
		strSkyBox = _S("nukeblank");
		break;
	case 12:
		strSkyBox = _S("sky_venice");
		break;
	case 13:
		strSkyBox = _S("sky_cs15_daylight03_hdr");
		break;
	case 14:
		strSkyBox = _S("sky_cs15_daylight04_hdr");
		break;
	case 15:
		strSkyBox = _S("sky_csgo_cloudy01");
		break;
	case 16:
		strSkyBox = _S("sky_csgo_night02");
		break;
	case 17:
		strSkyBox = _S("sky_csgo_night02b");
		break;
	case 18:
		strSkyBox = _S("sky_csgo_night_flat");
		break;
	case 19:
		strSkyBox = _S("sky_dust");
		break;
	case 20:
		strSkyBox = _S("vietnam");
		break;
	case 21:
		strSkyBox = g_cfg->m_szCustomSkybox;
		break;
	}

	g_sdk.convars.m_3DSky->SetValue(false);
	if (g_cfg->m_iSkybox <= 0)
		g_sdk.convars.m_3DSky->SetValue(true);

	return g_Tools->SetSkybox(strSkyBox.c_str());
}

void C_World::PenetrationCrosshair()
{
	if (!g_sdk.local || !g_sdk.local->IsAlive() || !g_cfg->m_bPenetrationCrosshair)
		return;

	C_BaseCombatWeapon* pWeapon = g_sdk.local->m_hActiveWeapon().Get();
	if (!pWeapon || !pWeapon->IsGun())
		return;

	C_CSWeaponData* pWeaponData = pWeapon->GetWeaponData();
	if (!pWeaponData)
		return;

	QAngle angLocalAngles;
	g_interfaces.engine->GetViewAngles(&angLocalAngles);

	Vector vecDirection;
	Math::AngleVectors(angLocalAngles, vecDirection);

	int nScreenSizeX, nScreenSizeY;
	g_interfaces.engine->GetScreenSize(nScreenSizeX, nScreenSizeY);

	Color aColor = Color::Red;
	if (g_AutoWall->IsPenetrablePoint(g_sdk.local_data.m_vecShootPosition, g_sdk.local_data.m_vecShootPosition + (vecDirection * pWeaponData->m_flRange)))
		aColor = Color::Green;

	return g_Render->RenderRectFilled((nScreenSizeX / 2) - 2, (nScreenSizeY / 2) - 2, (nScreenSizeX / 2) + 2, (nScreenSizeY / 2) + 2, aColor);
}