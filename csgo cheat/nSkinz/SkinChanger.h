#pragma once
#include "../SDK/Includes.hpp"
#include "Memory.h"

class GameEvent;	
extern Memory memory;

namespace SkinChanger 
{
    void run(ClientFrameStage_t stage) noexcept;
    void scheduleHudUpdate() noexcept;
    void overrideHudIcon(C_GameEvent* event) noexcept;

	struct PaintKit
	{
		int id;
		std::string name;
		std::string skin_name;

		PaintKit(int id, std::string&& name, std::string&& skin_name) noexcept : id(id), name(name), skin_name(skin_name)
		{

		}

		auto operator<(const PaintKit& other) const noexcept
		{
			return name < other.name;
		}
	};

	extern std::unordered_map <std::string, int> model_indexes;
	extern std::unordered_map <std::string, int> player_model_indexes;

	extern std::vector <PaintKit> skinKits;
	extern std::vector <PaintKit> gloveKits;
	extern std::vector <PaintKit> displayKits;
}

__forceinline void setup_skins()
{
	auto items = std::ifstream(_S("csgo/scripts/items/items_game_cdn.txt"));
	auto gameItems = std::string(std::istreambuf_iterator <char> { items }, std::istreambuf_iterator <char> { });

	if (!items.is_open())
		return;

	items.close();
	memory.initialize();

	for (auto i = 0; i <= memory.itemSchema()->paintKits.lastElement; i++)
	{
		auto paintKit = memory.itemSchema()->paintKits.memory[i].value;

		if (paintKit->id == 9001)
			continue;

		auto itemName = g_interfaces.localize->FindSafe(paintKit->itemName.buffer + 1);
		auto itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

		if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
		{
			if (paintKit->id < 10000)
			{
				if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } + '='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } + '=') == std::string::npos)
				{
					if (auto weaponName = gameItems.rfind(_S("weapon_"), pos); weaponName != std::string::npos)
					{
						name.back() = ' ';
						name += '(' + gameItems.substr(weaponName + 7, pos - weaponName - 7) + ')';
					}
				}
				SkinChanger::skinKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
			else
			{
				std::string_view gloveName{ paintKit->name.buffer };
				name.back() = ' ';
				name += '(' + std::string{ gloveName.substr(0, gloveName.find('_')) } + ')';
				SkinChanger::gloveKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
		}
	}

	std::sort(SkinChanger::skinKits.begin(), SkinChanger::skinKits.end());
	std::sort(SkinChanger::gloveKits.begin(), SkinChanger::gloveKits.end());
}