#include "../SDK/Includes.hpp"
#include "../Settings.hpp"
#include "../Log Manager/LogManager.hpp"
#include "../BuyBot/BuyBot.hpp"
#include "../Animations/LagCompensation.hpp"
#include "../RageBot/RageBot.hpp"
#include "../Visuals/World.hpp"
#include "../Networking/Fakelag.hpp"

#include "../Visuals/Players.hpp"
#include "../Networking/Networking.hpp"
#include "../Animations/LocalAnimations.hpp"
#include "../Packet/PacketManager.hpp"

void C_CustomEventListener::FireGameEvent(C_GameEvent* pEvent)
{
	if (!g_sdk.local)
		return;

	auto GetHitboxByHitgroup = [](int32_t iHitgroup) -> int
	{
		switch (iHitgroup)
		{
		case HITGROUP_HEAD:
			return HITBOX_HEAD;
		case HITGROUP_CHEST:
			return HITBOX_CHEST;
		case HITGROUP_STOMACH:
			return HITBOX_STOMACH;
		case HITGROUP_LEFTARM:
			return HITBOX_LEFT_HAND;
		case HITGROUP_RIGHTARM:
			return HITBOX_RIGHT_HAND;
		case HITGROUP_LEFTLEG:
			return HITBOX_RIGHT_CALF;
		case HITGROUP_RIGHTLEG:
			return HITBOX_LEFT_CALF;
		default:
			return HITBOX_PELVIS;
		}
	};

	auto GetHitboxNameFromHitgroup = [GetHitboxByHitgroup](int32_t iHitgroup) -> std::string
	{
		switch (GetHitboxByHitgroup(iHitgroup))
		{
		case HITBOX_HEAD:
			return _S("head");
		case HITBOX_CHEST:
			return _S("chest");
		case HITBOX_STOMACH:
			return _S("stomach");
		case HITBOX_PELVIS:
			return _S("pelvis");
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_RIGHT_HAND:
			return _S("left arm");
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
			return _S("right arm");
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
			return _S("left leg");
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
			return _S("right leg");
		case HITBOX_RIGHT_FOOT:
			return _S("left foot");
		case HITBOX_LEFT_FOOT:
			return _S("right foot");
		}
	};

	if (strstr(pEvent->GetName(), _S("weapon_fire")))
		g_RageBot->OnWeaponFire(pEvent);
	else if (strstr(pEvent->GetName(), _S("player_hurt")))
	{
		C_BasePlayer* pHurtPlayer = C_BasePlayer::GetPlayerByIndex(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))));
		if (!pHurtPlayer || !pHurtPlayer->IsPlayer())
			return;

		g_RageBot->OnPlayerHurt(pEvent);
		if (g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("attacker"))) == g_interfaces.engine->GetLocalPlayer())
		{
			if (g_cfg->m_bHitSound)
			{
				g_interfaces.surface->PlaySound_(_S("buttons\\arena_switch_press_02.wav"));
			}

			if (g_cfg->m_bKillsound)
			{
				if (!g_sdk.local || !g_sdk.local->IsAlive())
					return;
				{
					g_interfaces.surface->PlaySound_(_S("buttons\\arena_switch_press_02.wav"));
				}
			}


			if (g_cfg->m_bLogHurts)
			{
				C_PlayerInfo Info;
				g_interfaces.engine->GetPlayerInfo(pHurtPlayer->EntIndex(), &Info);

				std::string strHurtMessage = _S("[ Hit ] ");
				strHurtMessage += _S("entity: ");
				strHurtMessage += Info.m_strName;
				strHurtMessage += _S(" hitgroup: ");
				strHurtMessage += GetHitboxNameFromHitgroup(pEvent->GetInt(_S("hitgroup")));
				strHurtMessage += _S(" dmg: ");
				strHurtMessage += std::to_string(pEvent->GetInt("dmg_health"));
				strHurtMessage += _S("hp remain: ");
				strHurtMessage += std::to_string(pEvent->GetInt("health"));
				strHurtMessage += _S("hp ");

				g_LogManager->PushLog(strHurtMessage, _S("h"), Color(60, 255, 0));
			}
		}
		else if (g_cfg->m_bLogHarms && (g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("attacker"))) != g_interfaces.engine->GetLocalPlayer()) && pHurtPlayer == g_sdk.local)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("attacker"))));
			if (!pPlayer || !pPlayer->IsPlayer())
				return;

			C_PlayerInfo Info;
			g_interfaces.engine->GetPlayerInfo(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("attacker"))), &Info);

			std::string strHurtMessage = _S("[ Harmed ] dmg: ");
			strHurtMessage += std::to_string(pEvent->GetInt(_S("dmg_health"))) + _S("hp from: ") + Info.m_strName + _S(" hitgroup: ");
			strHurtMessage += GetHitboxNameFromHitgroup(pEvent->GetInt(_S("hitgroup")));

			g_LogManager->PushLog(strHurtMessage, _S("h"), Color(255, 128, 0));
		}
	}
	else if (strstr(pEvent->GetName(), _S("bullet_impact")))
	{
		g_RageBot->OnBulletImpact(pEvent);
		g_World->OnBulletImpact(pEvent);
	}
	else if (strstr(pEvent->GetName(), _S("bomb_beginplant")))
	{
		C_PlayerInfo PlayerInfo;
		g_interfaces.engine->GetPlayerInfo(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))), &PlayerInfo);

		if (g_cfg->m_bLogBomb)
			g_LogManager->PushLog(std::string(PlayerInfo.m_strName) + _S(" started planting the bomb"), _S("d"), Color::Red);
	}
	else if (strstr(pEvent->GetName(), _S("bomb_begindefuse")))
	{
		C_PlayerInfo PlayerInfo;
		g_interfaces.engine->GetPlayerInfo(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))), &PlayerInfo);

		if (g_cfg->m_bLogBomb)
			g_LogManager->PushLog(std::string(PlayerInfo.m_strName) + _S(" started defusing the bomb"), _S("d"), Color::Red);
	}
	else if (strstr(pEvent->GetName(), _S("round_start")))
	{
		g_BuyBot->OnRoundStart();
		g_Networking->ResetData();
		g_LocalAnimations->ResetData();
		g_PredictionSystem->ResetData();
		g_RageBot->ResetData();
		g_PlayerESP->ResetData();

		g_sdk.round_info.m_bShouldClearDeathNotices = true;
	}
	else if (strstr(pEvent->GetName(), _S("player_death")))
	{
		if (g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))) != g_interfaces.engine->GetLocalPlayer())
			return;

		std::string weapon_name = pEvent->GetString(_S("weapon"));
		auto weapon = g_sdk.local->m_hActiveWeapon().Get();

		if (weapon && weapon_name.find(_S("knife")) != std::string::npos)
			SkinChanger::overrideHudIcon(pEvent);

		g_sdk.round_info.m_bShouldClearDeathNotices = true;
	}
	else if (strstr(pEvent->GetName(), _S("item_purchase")) && g_cfg->m_bLogPurchases)
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))));
		if (!pPlayer || pPlayer->m_iTeamNum() == g_sdk.local->m_iTeamNum())
			return;

		C_PlayerInfo Info;
		g_interfaces.engine->GetPlayerInfo(g_interfaces.engine->GetPlayerForUserID(pEvent->GetInt(_S("userid"))), &Info);

		std::string strWeaponName = pEvent->GetString(_S("weapon"));
		if (strstr(strWeaponName.c_str(), _S("unknown")) || strstr(strWeaponName.c_str(), _S("assaultsuit")) || strstr(strWeaponName.c_str(), _S("kevlar")))
			return;

		if (strstr(strWeaponName.c_str(), _S("weapon_")))
			strWeaponName.erase(strWeaponName.begin(), strWeaponName.begin() + 7);
		else if (strstr(strWeaponName.c_str(), _S("item_")))
			strWeaponName.erase(strWeaponName.begin(), strWeaponName.begin() + 4);

		if (strstr(strWeaponName.c_str(), _S("_defuser")))
			strWeaponName = _S("defuser");

		std::string strMessage = Info.m_strName;
		strMessage += _S(" bought ");
		strMessage += strWeaponName;
		strMessage += _S(".");

		g_LogManager->PushLog(strMessage, _S("b"), Color::White);
	}
}