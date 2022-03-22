#include <algorithm>
#include <fstream>
#include "SkinChanger.h"
#include "../Settings.hpp"

item_setting* get_by_definition_index(const int definition_index)
{
	for (int i = 0; i < 36; i++)
	{
		if (g_cfg->skins.skinChanger[i].itemId == definition_index || g_cfg->skins.skinChanger[i].itemIdIndex == definition_index)
		{
			return &g_cfg->skins.skinChanger[i];
		}
	}

	return nullptr;
}

bool bModelBackup = false;

static const char* player_model_index_t[] =
{
"models/player/custom_player/legacy/tm_professional_varj.mdl", // Getaway Sally | The Professionals
"models/player/custom_player/legacy/tm_professional_vari.mdl", // Number K | The Professionals
"models/player/custom_player/legacy/tm_professional_varh.mdl", // Little Kev | The Professionals
"models/player/custom_player/legacy/tm_professional_varg.mdl", // Safecracker Voltzmann | The Professionals
"models/player/custom_player/legacy/tm_professional_varf5.mdl", // Bloody Darryl The Strapped | The Professionals
"models/player/custom_player/legacy/tm_professional_varf4.mdl", // Sir Bloody Loudmouth Darryl | The Professionals
"models/player/custom_player/legacy/tm_professional_varf3.mdl", // Sir Bloody Darryl Royale | The Professionals
"models/player/custom_player/legacy/tm_professional_varf2.mdl", // Sir Bloody Darryl Royale | The Professional
"models/player/custom_player/legacy/tm_professional_varf1.mdl", // Sir Bloody Silent Darryl | The Professionals
"models/player/custom_player/legacy/tm_professional_varf.mdl", // Sir Bloody Miami Darryl | The Professionals
"models/player/custom_player/legacy/tm_phoenix_varianti.mdl", // Street Soldier | Phoenix
"models/player/custom_player/legacy/tm_phoenix_varianth.mdl", // Soldier | Phoenix
"models/player/custom_player/legacy/tm_phoenix_variantg.mdl", // Slingshot | Phoenix
"models/player/custom_player/legacy/tm_phoenix_variantf.mdl", // Enforcer | Phoenix
"models/player/custom_player/legacy/tm_leet_variantj.mdl", // Mr. Muhlik | Elite Crew
"models/player/custom_player/legacy/tm_leet_varianti.mdl", // Prof. Shahmat | Elite Crew
"models/player/custom_player/legacy/tm_leet_varianth.mdl", // Osiris | Elite Crew
"models/player/custom_player/legacy/tm_leet_variantg.mdl", // Ground Rebel | Elite Crew
"models/player/custom_player/legacy/tm_leet_variantf.mdl", // The Elite Mr. Muhlik | Elite Crew
"models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl", // Trapper | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl", // Trapper Aggressor | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_variante.mdl", // Vypa Sista of the Revolution | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl", // Col. Mangos Dabisi | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_variantñ.mdl", // Arno The Overgrown | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl", // 'Medium Rare' Crasswater | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl", // Crasswater The Forgotten | Guerrilla Warfare
"models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl", // Elite Trapper Solman | Guerrilla Warfare
"models/player/custom_player/legacy/tm_balkan_varianth.mdl", // 'The Doctor' Romanov | Sabre
"models/player/custom_player/legacy/tm_balkan_variantj.mdl", // Blackwolf | Sabre
"models/player/custom_player/legacy/tm_balkan_varianti.mdl", // Maximus | Sabre
"models/player/custom_player/legacy/tm_balkan_variantf.mdl", // Dragomir | Sabre
"models/player/custom_player/legacy/tm_balkan_variantg.mdl", // Rezan The Ready | Sabre
"models/player/custom_player/legacy/tm_balkan_variantk.mdl", // Rezan the Redshirt | Sabre
"models/player/custom_player/legacy/tm_balkan_variantl.mdl", // Dragomir | Sabre Footsoldier
};

static const char* player_model_index_ct[] =
{
"models/player/custom_player/legacy/ctm_diver_varianta.mdl", // Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman
"models/player/custom_player/legacy/ctm_diver_variantb.mdl", // Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman
"models/player/custom_player/legacy/ctm_diver_variantc.mdl", // Lieutenant Rex Krikey | SEAL Frogman
"models/player/custom_player/legacy/ctm_fbi_varianth.mdl", // Michael Syfers | FBI Sniper
"models/player/custom_player/legacy/ctm_fbi_variantf.mdl", // Operator | FBI SWAT
"models/player/custom_player/legacy/ctm_fbi_variantb.mdl", // Special Agent Ava | FBI
"models/player/custom_player/legacy/ctm_fbi_variantg.mdl", // Markus Delrow | FBI HRT
"models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl", // Sous-Lieutenant Medic | Gendarmerie Nationale
"models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl", // Chem-Haz Capitaine | Gendarmerie Nationale
"models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl", // Chef d'Escadron Rouchard | Gendarmerie Nationale
"models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl", // Aspirant | Gendarmerie Nationale
"models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl", // Officer Jacques Beltram | Gendarmerie Nationale
"models/player/custom_player/legacy/ctm_sas_variantg.mdl", // D Squadron Officer | NZSAS
"models/player/custom_player/legacy/ctm_sas_variantf.mdl", // B Squadron Officer | SAS
"models/player/custom_player/legacy/ctm_st6_variante.mdl", // Seal Team 6 Soldier | NSWC SEAL
"models/player/custom_player/legacy/ctm_st6_variantg.mdl", // Buckshot | NSWC SEAL
"models/player/custom_player/legacy/ctm_st6_varianti.mdl", // Lt. Commander Ricksaw | NSWC SEAL
"models/player/custom_player/legacy/ctm_st6_variantj.mdl", // 'Blueberries' Buckshot | NSWC SEAL
"models/player/custom_player/legacy/ctm_st6_variantk.mdl", // 3rd Commando Company | KSK
"models/player/custom_player/legacy/ctm_st6_variantl.mdl", // 'Two Times' McCoy | TACP Cavalry
"models/player/custom_player/legacy/ctm_st6_variantm.mdl", // 'Two Times' McCoy | USAF TACP
"models/player/custom_player/legacy/ctm_st6_variantn.mdl", // Primeiro Tenente | Brazilian 1st Battalion
"models/player/custom_player/legacy/ctm_swat_variante.mdl", // Cmdr. Mae 'Dead Cold' Jamison | SWAT
"models/player/custom_player/legacy/ctm_swat_variantf.mdl", // 1st Lieutenant Farlow | SWAT
"models/player/custom_player/legacy/ctm_swat_variantg.mdl", // John 'Van Healen' Kask | SWAT
"models/player/custom_player/legacy/ctm_swat_varianth.mdl", // Bio-Haz Specialist | SWAT
"models/player/custom_player/legacy/ctm_swat_varianti.mdl", // Sergeant Bombson | SWAT
"models/player/custom_player/legacy/ctm_swat_variantj.mdl", // Chem-Haz Specialist | SWAT
"models/player/custom_player/legacy/ctm_swat_variantk.mdl" // Lieutenant 'Tree Hugger' Farlow | SWAT
};

Memory memory;

std::unordered_map <std::string, int> SkinChanger::model_indexes;
std::unordered_map <std::string, int> SkinChanger::player_model_indexes;

std::vector <SkinChanger::PaintKit> SkinChanger::skinKits;
std::vector <SkinChanger::PaintKit> SkinChanger::gloveKits;
std::vector <SkinChanger::PaintKit> SkinChanger::displayKits;

static std::unordered_map <std::string_view, const char*> iconOverrides;

void erase_override_if_exists_by_index(const int definition_index) noexcept
{
	if (auto original_item = game_data::get_weapon_info(definition_index))
	{
		if (!original_item->icon)
			return;

		if (const auto override_entry = iconOverrides.find(original_item->icon); override_entry != end(iconOverrides))
			iconOverrides.erase(override_entry);
	}
}

void apply_config_on_attributable_item(C_BaseAttributableItem* item, const item_setting* config, const unsigned xuid_low) noexcept
{
	item->m_iItemIDHigh() = -1; //-V522
	item->m_iAccountID() = xuid_low;
	item->m_flFallbackWear() = config->wear;

	volatile int nQuality = config->quality;
	if (nQuality)
		item->m_iEntityQuality() = nQuality;

	volatile int nPaintKit = config->paintKit;
	if (nPaintKit)
		item->m_nFallbackPaintKit() = nPaintKit;

	volatile int nSeed = config->seed;
	if (nSeed)
		item->m_nFallbackSeed() = nSeed;

	volatile int nStatTrak = config->stat_trak;
	if (nStatTrak)
		item->m_nFallbackStatTrak() = nStatTrak;

	auto& definition_index = *(int16_t*)((DWORD)(item)+0x2FAA);

	if (config->definition_override_index && config->definition_override_index != definition_index)
	{
		if (auto replacement_item = game_data::get_weapon_info(config->definition_override_index))
		{
			auto old_definition_index = definition_index;
			definition_index = config->definition_override_index;

			if (SkinChanger::model_indexes.find(replacement_item->model) == SkinChanger::model_indexes.end())
				SkinChanger::model_indexes.emplace(replacement_item->model, g_interfaces.model_info->GetModelIndex(replacement_item->model));

			item->SetModelIndex(SkinChanger::model_indexes.at(replacement_item->model));
			item->PreDataUpdate(0);

			if (old_definition_index)
				if (auto original_item = game_data::get_weapon_info(old_definition_index); original_item && original_item->icon && replacement_item->icon)
					iconOverrides[original_item->icon] = replacement_item->icon;
		}
	}
	else
		erase_override_if_exists_by_index(definition_index);
}

static auto get_wearable_create_fn() -> CreateClientClassFn
{
	auto classes = g_interfaces.client->GetAllClasses();

	while (classes->m_ClassID != ClassId_CEconWearable)
		classes = classes->m_pNext;

	return classes->m_pCreateFn;
}

static C_BaseAttributableItem* make_glove(int entry, int serial) noexcept
{
	get_wearable_create_fn()(entry, serial);
	auto glove = static_cast <C_BaseAttributableItem*> (g_interfaces.entity_list->GetClientEntity(entry));

	if (!glove)
		return nullptr;

	glove->SetAbsoluteOrigin(Vector(16384.0f, 16384.0f, 16384.0f));
	return glove;
}

static float last_skins_update = 0.0f;

static void post_data_update_start(C_BasePlayer* local) noexcept
{
	C_PlayerInfo player_info;

	if (!g_interfaces.engine->GetPlayerInfo(local->EntIndex(), &player_info))
		return;

	static auto glove_handle = CBaseHandle(0);

	auto wearables = local->m_hMyWearables();
	auto glove_config = get_by_definition_index(GLOVE_T_SIDE);
	auto glove = reinterpret_cast <C_BaseAttributableItem*> (g_interfaces.entity_list->GetClientEntityFromHandle(wearables[0]));

	if (!glove)
	{
		auto our_glove = reinterpret_cast <C_BaseAttributableItem*> (g_interfaces.entity_list->GetClientEntityFromHandle(glove_handle));

		if (our_glove)
		{
			wearables[0] = glove_handle;
			glove = our_glove;
		}
	}

	if (!local->IsAlive())
	{
		if (glove)
		{
			glove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
			glove->GetClientNetworkable()->Release();
		}

		return;
	}

	if (glove_config && glove_config->definition_override_index)
	{
		if (!glove)
		{
			auto entry = g_interfaces.entity_list->GetHighestEntityIndex() + 1;
			auto serial = rand() % 0x1000;

			glove = make_glove(entry, serial);
			wearables[0] = entry | serial << 16;
			glove_handle = wearables[0];
		}

		*reinterpret_cast <int*> (uintptr_t(glove) + 0x64) = -1;
		apply_config_on_attributable_item(glove, glove_config, player_info.m_iXuidLow);
	}

	auto weapons = local->m_hMyWeapons();

	for (auto weapon_handle = 0; weapons[weapon_handle].IsValid(); weapon_handle++)
	{
		auto weapon = (C_BaseCombatWeapon*)g_interfaces.entity_list->GetClientEntityFromHandle(weapons[weapon_handle]); //-V807
		if (!weapon)
			continue;

		auto definition_index = weapon->m_iItemDefinitionIndex();
		auto active_conf = get_by_definition_index(weapon->IsKnife() ? WEAPON_KNIFE : definition_index);
		if (active_conf)
			apply_config_on_attributable_item(weapon, active_conf, player_info.m_iXuidLow);
		else
			erase_override_if_exists_by_index(definition_index);
	}

	auto view_model = (C_BaseViewModel*)g_interfaces.entity_list->GetClientEntityFromHandle(local->m_hViewModel());

	if (!view_model)
		return;

	auto view_model_weapon = (C_BaseCombatWeapon*)g_interfaces.entity_list->GetClientEntityFromHandle(view_model->m_hWeapon());

	if (!view_model_weapon)
		return;

	auto override_info = game_data::get_weapon_info(view_model_weapon->m_iItemDefinitionIndex());

	if (!override_info)
		return;

	auto world_model = (C_BaseCombatWeapon*)g_interfaces.entity_list->GetClientEntityFromHandle(view_model_weapon->m_hWeaponWorldModel());

	if (!world_model)
		return;

	if (SkinChanger::model_indexes.find(override_info->model) == SkinChanger::model_indexes.end())
		SkinChanger::model_indexes.emplace(override_info->model, g_interfaces.model_info->GetModelIndex(override_info->model));

	view_model->m_nEntityModelIndex() = SkinChanger::model_indexes.at(override_info->model);
	world_model->m_nEntityModelIndex() = SkinChanger::model_indexes.at(override_info->model) + 1;
}

static bool UpdateRequired = false;
static bool hudUpdateRequired = false;

static void updateHud() noexcept
{
	if (auto hud_weapons = memory.findHudElement(memory.hud, _S("CCSGO_HudWeaponSelection")) - 0x28)
		for (auto i = 0; i < *(hud_weapons + 0x20); i++)
			i = memory.clearHudWeapon(hud_weapons, i);

	hudUpdateRequired = false;
}

void SkinChanger::run(ClientFrameStage_t stage) noexcept
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!g_sdk.local)
		return;

	post_data_update_start(g_sdk.local);

	if (!g_sdk.local->IsAlive())
	{
		UpdateRequired = false;
		hudUpdateRequired = false;
		return;
	}

	static auto backup_model_index = -1;
	{
		const char** player_model_index = nullptr;
		auto player_model = 0;

		switch (g_sdk.local->m_iTeamNum())
		{
		case 2:
			player_model_index = player_model_index_t;
			player_model = g_cfg->m_nModelT;
			break;
		case 3:
			player_model_index = player_model_index_ct;
			player_model = g_cfg->m_nModelCT;
			break;
		}

		if (player_model)
		{
			if (!bModelBackup)
			{
				auto model = g_sdk.local->GetModel();

				if (model)
				{
					auto studio_model = g_interfaces.model_info->GetStudiomodel(model);

					if (studio_model)
					{
						auto name = _S("models/") + (std::string)studio_model->szName;
						backup_model_index = g_interfaces.model_info->GetModelIndex(name.c_str());
					}
				}
			}

			if (SkinChanger::player_model_indexes.find(player_model_index[player_model - 1]) == SkinChanger::player_model_indexes.end()) //-V522
				SkinChanger::player_model_indexes.emplace(player_model_index[player_model - 1], g_interfaces.model_info->GetModelIndex(player_model_index[player_model - 1]));

			g_sdk.local->SetModelIndex(SkinChanger::player_model_indexes[player_model_index[player_model - 1]]);
			bModelBackup = true;
		}
		else if (bModelBackup)
		{
			g_sdk.local->SetModelIndex(backup_model_index);
			bModelBackup = false;
		}
	}

	if (UpdateRequired)
	{
		UpdateRequired = false;
		hudUpdateRequired = true;

		g_interfaces.client_state->m_nDeltaTick() = -1;
	}
	else if (hudUpdateRequired)
	{
		hudUpdateRequired = false;
		updateHud();
	}
}

void SkinChanger::scheduleHudUpdate() noexcept
{
	//if (!g_Globals.m_LocalPlayer->IsAlive())
		//return;

	if (!g_sdk.local)
		return;

	if (g_interfaces.globals->m_realtime - last_skins_update < 1.0f)
		return;

	UpdateRequired = true;
	last_skins_update = g_interfaces.globals->m_realtime;
}

void SkinChanger::overrideHudIcon(C_GameEvent* event) noexcept
{
	if (auto iconOverride = iconOverrides[event->GetString(_S("weapon"))])
		event->SetString(_S("weapon"), iconOverride);
}