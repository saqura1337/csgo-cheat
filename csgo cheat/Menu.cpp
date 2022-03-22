#include <d3d9.h>
#include "Tools/Obfuscation/XorStr.hpp"
#include "SDK/Globals.hpp"
#include "Data/Ava.h"

#include "../Features/Model/Model.hpp"
#include <d3dx9.h>
#pragma comment (lib, "d3dx9.lib")

#include "../Render.hpp"
#include "../Config.hpp"
#include "../Settings.hpp"
#include "../Features/RageBot/RageBot.hpp"
#include "../Features/Log Manager/LogManager.hpp"
#include "../Data/Back.h"
#include "../Features/RageBot/Antiaim.hpp"

IDirect3DTexture9* all_Skins[36];

struct tab_Struct
{
	float size;
	bool active;
};

bool Tab(const char* label, const ImVec2& size_arg, const bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_Size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_Size.x + style.FramePadding.x * 2.0f, label_Size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y - 10));
	ImGui::ItemSize(ImVec2(size.x + 10, size.y), style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

	static std::map<ImGuiID, tab_Struct> selected_animation;
	auto it_Selected = selected_animation.find(ImGui::GetItemID());

	if (it_Selected == selected_animation.end())
	{
		selected_animation.insert({ ImGui::GetItemID(), {0.0f, false} });
		it_Selected = selected_animation.find(ImGui::GetItemID());
	}
	it_Selected->second.size = ImClamp(it_Selected->second.size + (5.f * ImGui::GetIO().DeltaTime * (selected || hovered ? 1.f : -1.f)), 0.0f, 1.f);

	ImU32 color_text = ImGui::GetColorU32(ImLerp(ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.0f), ImVec4(255 / 255.f, 255 / 255.f, 255 / 255.f, 1.0f), it_Selected->second.size));

	//window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y + size_arg.y / 3), ImVec2(bb.Max.x, bb.Max.y - size_arg.y / 3), ImGui::GetColorU32(ImVec4(26 / 255.f, 26 / 255.f, 30 / 255.f, 1.0f)), 3);
	//window->DrawList->AddRect(ImVec2(bb.Min.x - 1, (bb.Min.y + size_arg.y / 3) - 1), ImVec2(bb.Max.x + 1, (bb.Max.y - size_arg.y / 3) + 1), ImGui::GetColorU32(ImVec4(38 / 255.f, 38 / 255.f, 42 / 255.f, 1.0f)), 3);//outline
		// hover animation

	if (selected)
	{
		window->DrawList->AddRectFilled(ImVec2(bb.Min.x + 1, bb.Max.y + 1), ImVec2(bb.Max.x + 1, bb.Max.y + 2), ImColor(124, 252, 0, 255));
	}

	window->DrawList->AddText(ImVec2(bb.Min.x + size_arg.x / 2 - ImGui::CalcTextSize(label).x / 2, bb.Min.y + size_arg.y / 2 - ImGui::CalcTextSize(label).y / 2 - 1), color_text, label);

	return pressed;
}

static float tab_alpha = 0.0f;
static float tab_padding = 0.0f;

void C_Menu::Instance()
{
	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha > 0.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha - fc / 255.f, 0.f, 1.f);
	}

	if (m_bIsMenuOpened && ImGui::GetStyle().Alpha < 1.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha + fc / 255.f, 0.f, 1.f);
	}

	this->DrawSpectatorList();
	this->DrawKeybindList();
	this->WaterMark();

	if (!m_bIsMenuOpened && ImGui::GetStyle().Alpha < 0.1f)
		return;

	int32_t iScreenSizeX, iScreenSizeY;
	g_interfaces.engine->GetScreenSize(iScreenSizeX, iScreenSizeY);

	ImGui::SetNextWindowPos(ImVec2((iScreenSizeX / 2) - 325, (iScreenSizeY / 2) - 220), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(797, 500), ImVec2(797, 500));
	ImGui::Begin(this->GetMenuName(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

	auto draw = ImGui::GetWindowDrawList();
	auto pos = ImGui::GetWindowPos();

	static int tab = 0;

	static bool draw_anim = false;
	static int anim_tab = 0;
	static int sub_tab = 0;

	tab_padding = ImClamp(tab_padding + (75.f * ImGui::GetIO().DeltaTime * (draw_anim && tab != anim_tab ? 1.f : -1.f)), 0.0f, 15.f);
	tab_alpha = ImClamp(tab_alpha + (5.f * ImGui::GetIO().DeltaTime * (draw_anim && tab != anim_tab ? 1.f : -1.f)), 0.0f, 1.f);

	if (tab_alpha >= 1.0f && tab_padding >= 10.0f)
	{
		draw_anim = false;
		tab = anim_tab;
	}

	ImGui::SetCursorPos({ 130,-8 });
	ImGui::BeginGroup();
	{
		if (Tab("Legitbot", ImVec2(ImGui::CalcTextSize("Legitbot").x + 15, 60), anim_tab == 0))
			anim_tab = 0, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Ragebot", ImVec2(ImGui::CalcTextSize("Ragebot").x + 15, 60), anim_tab == 1))
			anim_tab = 1, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Anti aim", ImVec2(ImGui::CalcTextSize("Anti aim").x + 15, 60), anim_tab == 2))
			anim_tab = 2, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Players", ImVec2(ImGui::CalcTextSize("Players").x + 15, 60), anim_tab == 3))
			anim_tab = 3, draw_anim = true;
		ImGui::SameLine();
		if (Tab("World", ImVec2(ImGui::CalcTextSize("World").x + 15, 60), anim_tab == 4))
			anim_tab = 4, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Misc", ImVec2(ImGui::CalcTextSize("Misc").x + 15, 60), anim_tab == 5))
			anim_tab = 5, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Configs", ImVec2(ImGui::CalcTextSize("Configs").x + 15, 60), anim_tab == 6))
			anim_tab = 6, draw_anim = true;
		ImGui::SameLine();
		if (Tab("Skins", ImVec2(ImGui::CalcTextSize("Skins").x + 15, 60), anim_tab == 8))
			anim_tab = 7, draw_anim = true;
	}
	ImGui::EndGroup();

	int32_t x = ImGui::GetCurrentWindow()->Pos.x + 4.5f;
	int32_t y = ImGui::GetCurrentWindow()->Pos.y;

	ImGui::SetCursorPos({ 15,75 + tab_padding });

	ImGui::PushClipRect(ImVec2(pos.x + 1, pos.y + 1), ImVec2(pos.x + 797, pos.y + 500), false);
	ImGui::BeginGroup();
	{
		switch (tab)
		{
		case 0:
			//	this->DrawLegitTab();
			break;
		case 1:
			this->DrawRageTab();
			break;
		case 2:
			this->DrawAntiAimTab();
			break;
		case 3:
			this->DrawPlayersTab();
			break;
		case 4:
			this->DrawWorldTab();
			break;
		case 5:
			this->DrawMiscTab();
			break;
		case 6:
			this->DrawConfigTab();
			break;
		case 7:
			this->DrawInventoryTab();
			break;
		}
	}
	ImGui::EndGroup();
	ImGui::PopClipRect();

	/*switch (tab)
	{
	case 0:
	ImGui::SetCursorPos({ 0, 55 });
	ImGui::BeginGroup();
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(43, 0));
		if (ImGui::subtub("Main", subtab == 0)) subtab = 0;
		if (ImGui::subtub("Accuracy", subtab == 1)) subtab = 1;
		if (ImGui::subtub("Exploits", subtab == 2)) subtab = 2;
		ImGui::PopStyleVar();
	}
	ImGui::EndGroup();
	break;
	}

	ImGui::GetOverlayDrawList()->AddRectFilled({ pos.x + 3,pos.y + 43 }, { pos.x + 797,pos.y + 500 }, ImGui::GetColorU32(ImVec4(3 / 255.0f, 16 / 255.0f, 32 / 255.0f, 170 / 255.f)), 3);
	ImGui::GetOverlayDrawList()->AddRectFilled({ pos.x + 3,pos.y + 43 }, { pos.x + 797,pos.y + 450 }, ImGui::GetColorU32(ImVec4(22 / 255.0f, 22 / 255.0f, 26 / 255.0f, tab_alpha)), 3);

	if (ImGui::Button("Keybinds", ImVec2(100, 20)))
	{
	const char* txt = g_cfg->m_aDoubleTap->m_iModeSelected == 1 ? "Toggle" : "Hold";
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(pos.x + 500, pos.y + 200), ImVec2(pos.x + 500, pos.y + 200), ImColor(255, 255, 255, 255));
	ImGui::Text("Doubletap mode ");
	ImGui::Text(txt);
	}*/
	ImGui::SetCursorPos(ImVec2(30, 0 - 10));
	//	ImGui::Image(g_Menu->GetTexture(), ImVec2(70, 70));
	ImGui::End();
}

//void C_Menu::DrawLegitTab()
//{
//	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
//	ImGui::PopFont();
//
//	static int wep = 0;
//
//	std::vector< const char* > aLegitWeapons =
//	{
//		("Auto"),
//		("Scout"),
//		("AWP"),
//		("Deagle"),
//		("Revolver"),
//		("Pistol"),
//		("Rifle")
//	};
//
//	std::vector< const char* > aHitboxes =
//	{
//		("Head"),
//		("Chest"),
//		("Arms"),
//		("Pelvis"),
//		("Stomach"),
//		("Legs"),
//	};
//
//	// Start Main tab
//	ImGui::SetCursorPos(ImVec2(80, 78 + tab_padding));
//	ImGui::BeginChild(_S("Main"), ImVec2(251, 189));
//	{
//		ImGui::Checkbox(_S("Enable Legitbot"), &g_cfg->m_aLegitSettings[wep].m_bEnabled);
//		ImGui::SingleSelect(_S("Current Weapon"), &wep, aLegitWeapons);
//		if (ImGui::BeginCombo(_S("Hitbox"), _S("Select"), 0, aHitboxes.size()))
//		{
//			for (int i = 0; i < aHitboxes.size(); i++)
//			ImGui::Selectable(aHitboxes[i], &g_cfg->m_aLegitSettings[wep].m_Hitboxes, ImGuiSelectableFlags_DontClosePopups);
//
//		ImGui::EndCombo();
//		}
//
//		ImGui::Checkbox(_S("Remove Recoil"), &g_cfg->m_aLegitSettings[wep].m_bRemoveRecoil);
//		ImGui::Checkbox(_S("Auto Fire"), &g_cfg->m_aLegitSettings[wep].m_bAutoFire);
//		ImGui::Checkbox(_S("Auto Scope"), &g_cfg->m_aLegitSettings[wep].m_bAutoScope);
//		ImGui::Checkbox(_S("Backtrack"), &g_cfg->m_aLegitSettings[wep].m_bLegitBacktrack);
//	}
//	ImGui::EndChild();
//
//	// Start Main tab
//	ImGui::SetCursorPos(ImVec2(80, 290 + tab_padding));
//	ImGui::BeginChild(_S("Rcs"), ImVec2(251, 140));
//	{
//		ImGui::Checkbox(_S("Enable Rcs"), &g_cfg->m_aLegitSettings[wep].m_bRcsEnable);
//		ImGui::SliderFloat(_S("Rcs X"), &g_cfg->m_aLegitSettings[wep].m_flRcs_x, 0, 3);
//		ImGui::SliderFloat(_S("Rcs Y"), &g_cfg->m_aLegitSettings[wep].m_flRcs_y, 0, 3);
//		ImGui::SliderFloat(_S("Smooth"), &g_cfg->m_aLegitSettings[wep].m_flRcsSmooth, 1.f, 6.f);
//	}
//	ImGui::EndChild();
//
//	// Start Main tab
//	ImGui::SetCursorPos(ImVec2(408, 290 + tab_padding));
//	ImGui::BeginChild(_S("Silent"), ImVec2(251, 70));
//	{
//		ImGui::Checkbox(_S("Enable Silent"), &g_cfg->m_aLegitSettings[wep].m_bSilent);
//		ImGui::SliderFloat(_S("Silent Fov"), &g_cfg->m_aLegitSettings[wep].m_flSilentFov, 0, 10);
//	}
//	ImGui::EndChild();
//
//	// Start Main tab
//	ImGui::SetCursorPos(ImVec2(408, 78 + tab_padding));
//	ImGui::BeginChild(_S("Fov"), ImVec2(251, 189));
//	{
//		ImGui::SliderFloat(_S("Fov"), &g_cfg->m_aLegitSettings[wep].m_flFov, 0, 10);
//		ImGui::SliderFloat(_S("Smooth"), &g_cfg->m_aLegitSettings[wep].m_flSmooth, 0, 6);
//	}
//	ImGui::EndChild();
//
//}

void C_Menu::DrawRageTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;
	static int wep = 0;
	ImGui::PopFont();
	//ImGui::SetCursorPos(ImVec2(6, 75));
	//ImGui::PushFont(g_sdk.fonts.m_WeaponIcon);
	//ImGui::BeginGroup();
	//{
	//	if (ImGui::subtub(_S("Y"), wep == 0)) wep = 0; ImGui::SameLine(); // AUTO
	//	if (ImGui::subtub(_S("a"), wep == 1)) wep = 1; ImGui::SameLine(); // SCOUT
	//	if (ImGui::subtub(_S("Z"), wep == 2)) wep = 2; ImGui::SameLine(); // AWP
	//	if (ImGui::subtub(_S("A"), wep == 3)) wep = 3; ImGui::SameLine(); // DEAGLE
	//	if (ImGui::subtub(_S("B"), wep == 5)) wep = 5; ImGui::SameLine(); // PISTOL
	//	if (ImGui::subtub(_S("S"), wep == 6)) wep = 6; ImGui::SameLine(); // RIFLE
	//}
	//ImGui::EndGroup();
	//ImGui::PopFont();
	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
//	ImGui::BeginChild("", ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	std::vector< const char* > aRageWeapons =
	{
		("Auto"),
		("Scout"),
		("AWP"),
		("Deagle"),
		("Revolver"),
		("Pistol"),
		("Rifle")
	};

	std::vector< const char* > aHitboxes =
	{
		("Head"),
		("Neck"),
		("Chest"),
		("Arms"),
		("Pelvis"),
		("Stomach"),
		("Legs"),
	};

	//// Draw Model
	//if (g_DrawModel->GetTexture())
	//{
	//	// Draw Background Rect
	//	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y), ImVec2(vecWindowPosition.x + 950 + 150, vecWindowPosition.y + 440), ImColor(3, 16, 32, 170), 4.0f);
	//	// Draw our model
	//	ImGui::GetForegroundDrawList()->AddImage(
	//		g_DrawModel->GetTexture()->pTextureHandles[0]->lpRawTexture,
	//		ImVec2(vecWindowPosition.x + 610 + 150, vecWindowPosition.y - 130),
	//		ImVec2(vecWindowPosition.x + 610 + 150 + g_DrawModel->GetTexture()->GetActualWidth(), vecWindowPosition.y + g_DrawModel->GetTexture()->GetActualHeight() - 130),
	//		ImVec2(0, 0), ImVec2(1, 1),
	//		ImColor(1.0f, 1.0f, 1.0f, 1.0f));

	//	//ImGui::SetCursorPos(ImVec2(vecWindowPosition.x + 100, 0));
	//	//ImGui::ImageButton(this->GetTexture(), ImVec2(15,15));
	//	//for (int i = 0; i < aHitboxes.size(); i++)
	//	//	ImGui::Selectable(aHitboxes[i], &g_cfg->m_aRageSettings[wep].m_Hitboxes[i], ImGuiSelectableFlags_Disabled);
	//	//ImGui::EndCombo();
	//}

	// Start Main tab
	ImGui::SetCursorPos(ImVec2(80, 78 + tab_padding));
	ImGui::BeginChild(_S("Main"), ImVec2(iChildDoubleSizeX, 150));
	{
		bool meme = false;
		ImGui::Checkbox(_S("Enable"), &g_cfg->m_aRageSettings[wep].m_bEnabled);
		//ImGui::Checkbox(_S("Silent Aim"), &meme);
		ImGui::SliderInt(_S("FOV"), &g_cfg->m_aRageSettings[wep].m_iFOV, 1, 180);
		ImGui::SingleSelect(_S("Current Weapon"), &wep, aRageWeapons);

		if (ImGui::BeginCombo(_S("Hitboxes"), _S("Select"), 0, aHitboxes.size()))
		{
			for (int i = 0; i < aHitboxes.size(); i++)
				ImGui::Selectable(aHitboxes[i], &g_cfg->m_aRageSettings[wep].m_Hitboxes[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo(_S("Multipoints"), _S("Select"), 0, aHitboxes.size()))
		{
			for (int i = 0; i < aHitboxes.size(); i++)
				ImGui::Selectable(aHitboxes[i], &g_cfg->m_aRageSettings[wep].m_Multipoints[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	// End Main tab

	// Start Accuracy tab
	ImGui::SetCursorPos(ImVec2(80, 280 + tab_padding)); // 468
	ImGui::BeginChild(_S("Accuracy "), ImVec2(iChildDoubleSizeX, 200));
	{
		ImGui::SliderInt(_S("Hitchance"), &g_cfg->m_aRageSettings[wep].m_iHitChance, 0, 100);

		ImGui::SliderInt(_S("Head Scale"), &g_cfg->m_aRageSettings[wep].m_iHeadScale, 0, 100);
		ImGui::SliderInt(_S("Body Scale"), &g_cfg->m_aRageSettings[wep].m_iBodyScale, 0, 100);

		ImGui::Checkbox(_S("Safe Point"), &g_cfg->m_aRageSettings[wep].m_bPreferSafe);
		ImGui::Checkbox(_S("Body Aim"), &g_cfg->m_aRageSettings[wep].m_bPreferBody);

		ImGui::Text(_S("Force Safe Point"));
		ImGui::Keybind(_S("Force Safepoints "), &g_cfg->m_aSafePoint->m_iKeySelected, &g_cfg->m_aSafePoint->m_iModeSelected);
	}
	ImGui::EndChild();
	ImGui::SameLine();
	// End Accuracy tab

	// Start Exploits tab
	ImGui::SetCursorPos(ImVec2(408, 78 + tab_padding));
	ImGui::BeginChild(_S("Exploits "), ImVec2(iChildDoubleSizeX, 82));
	{
		std::vector < const char* > aDoubleTap =
		{
			"Teleport",
			"Lag peek"
			//"Fakelag"
		};

		ImGui::Text(_S("Double Tap"));
		ImGui::Keybind(_S("Double tap"), &g_cfg->m_aDoubleTap->m_iKeySelected, &g_cfg->m_aDoubleTap->m_iModeSelected);

		ImGui::Text(_S("Hide shots"));
		ImGui::Keybind(_S("Hide Shots"), &g_cfg->m_aHideShots->m_iKeySelected, &g_cfg->m_aHideShots->m_iModeSelected);

		if (ImGui::BeginCombo(_S("Doubletap Options"), _S("Select"), 0, aDoubleTap.size()))
		{
			for (int i = 0; i < aDoubleTap.size(); i++)
				ImGui::Selectable(aDoubleTap[i], &g_cfg->m_aRageSettings[wep].m_DoubleTapOptions[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}
	}
	ImGui::EndChild();
	// End Exploits tab

	// Start Min Damage tab
	ImGui::SetCursorPos(ImVec2(408, 285 + tab_padding));
	ImGui::BeginChild(_S("Min. Damage"), ImVec2(iChildDoubleSizeX, 110));
	{
		ImGui::SliderInt(_S("Min. Damage"), &g_cfg->m_aRageSettings[wep].m_iMinDamage, 0, 100);
		ImGui::SliderInt(_S("Min. Damage Override"), &g_cfg->m_aRageSettings[wep].m_iMinDamageOverride, 0, 100);
		ImGui::Text(_S("Damage Override Key"));
		ImGui::Keybind(_S("Damage override"), &g_cfg->m_aMinDamage->m_iKeySelected, &g_cfg->m_aMinDamage->m_iModeSelected);
	}
	ImGui::EndChild();
	// End Min Damage tab

	// Start Misc tab
	ImGui::SetCursorPos(ImVec2(408, 185 + tab_padding));
	ImGui::BeginChild(_S("Misc"), ImVec2(iChildDoubleSizeX, 80));
	{
		ImGui::Checkbox(_S("Auto Stop"), &g_cfg->m_aRageSettings[wep].m_bAutoStop);
		ImGui::Checkbox(_S("Auto Scope"), &g_cfg->m_aRageSettings[wep].m_bAutoScope);

		std::vector < const char* > aAutoStop =
		{
			"Force accuracy",
			"Early"
		};

		if (ImGui::BeginCombo(_S("Autostop options"), _S("Select"), 0, aAutoStop.size()))
		{
			for (int i = 0; i < aAutoStop.size(); i++)
				ImGui::Selectable(aAutoStop[i], &g_cfg->m_aRageSettings[wep].m_AutoStopOptions[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}
	}
	ImGui::EndChild();
	// End Misc tab
}

void C_Menu::DrawAntiAimTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;

	int iChildSizeX = (800 - ImGui::CalcTextSize(this->GetMenuName()).x - 40) - 30;
	int iChildSizeY = (565 - 30) - 200;

	ImGui::PopFont();

	int iChildPosFirstX = vecWindowPosition.x + iMainTextSize + 15 + 40;
	int iChildPosSecondX = vecWindowPosition.x + iMainTextSize + 30 + iChildDoubleSizeX + 40;

	int iChildPosFirstY = vecWindowPosition.y + 15;
	int iChildPosSecondY = vecWindowPosition.y + 15 + iChildDoubleSizeY + 15;

	// Start Main Tab
	ImGui::SetCursorPos(ImVec2(80, 78 + tab_padding));
	ImGui::BeginChild(_S("Main"), ImVec2(iChildDoubleSizeX, 205));
	{
		ImGui::Checkbox(_S("Enable"), g_cfg->m_bAntiAim);

		std::vector < const char* > PitchModes = { _S("None"), _S("Down"), _S("Up"), _S("Fake down"), _S("Fake up") };
		ImGui::SingleSelect(_S("Pitch"), g_cfg->m_iPitchMode, PitchModes);
		ImGui::SliderInt(_S("Yaw Add Offset"), g_cfg->m_iYawAddOffset, 0, 180);
		ImGui::SliderInt(_S("Jitter Amount"), g_cfg->m_iJitterAmount, 0, 180);

		ImGui::Text(_S("Manual Left"));
		ImGui::Keybind(_S("123"), &g_cfg->m_aManualLeft->m_iKeySelected, &g_cfg->m_aManualLeft->m_iModeSelected);

		ImGui::Text(_S("Manual Back"));
		ImGui::Keybind(_S("1234"), &g_cfg->m_aManualBack->m_iKeySelected, &g_cfg->m_aManualBack->m_iModeSelected);

		ImGui::Text(_S("Manual Right"));
		ImGui::Keybind(_S("12346"), &g_cfg->m_aManualRight->m_iKeySelected, &g_cfg->m_aManualRight->m_iModeSelected);
	}
	ImGui::EndChild();
	//	ImGui::SameLine();
		// End Main Tab

		// Start Fake Angle Tab
	ImGui::SetCursorPos(ImVec2(408, 78 + tab_padding));
	ImGui::BeginChild(_S("Fake Angle"), ImVec2(iChildDoubleSizeX, 130));
	{
		ImGui::SliderInt(_S("Left Limit"), g_cfg->m_iLeftFakeLimit, 0, 60);
		ImGui::SliderInt(_S("Right Limit"), g_cfg->m_iRightFakeLimit, 0, 60);

		std::vector < std::string > InverterConditions = { _S("Stand"), _S("Move"), _S("Air") };
		if (ImGui::BeginCombo(_S("Jitter Conditions"), _S("Select"), 0, InverterConditions.size()))
		{
			for (int i = 0; i < InverterConditions.size(); i++)
				ImGui::Selectable(InverterConditions[i].c_str(), &g_cfg->m_aInverterConditions[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}

		ImGui::Text(_S("Inverter"));
		ImGui::Keybind(_S("InvertButton"), &g_cfg->m_aInverter->m_iKeySelected, &g_cfg->m_aInverter->m_iModeSelected);
	}
	ImGui::EndChild();
	//ImGui::SameLine();
	// End Fake Angle Tab

	// Start Angle Tab
	ImGui::SetCursorPos(ImVec2(408, 232 + tab_padding));
	ImGui::BeginChild(_S("Angle"), ImVec2(iChildDoubleSizeX, 51));
	{
		ImGui::Checkbox(_S("At Targets"), g_cfg->m_bAntiAimAtTargets);
		ImGui::Checkbox(_S("Auto Direction"), g_cfg->m_bAutoDirection);
	}
	ImGui::EndChild();
	//End Angle Tab

	// Start Fake Lag Tab
	ImGui::SetCursorPos(ImVec2(80, 310 + tab_padding));
	ImGui::BeginChild(_S("Fake Lag"), ImVec2(iChildDoubleSizeX, 150));
	{
		ImGui::Checkbox(_S("Enable Fake Lag"), g_cfg->m_bFakeLagEnabled);
		ImGui::SliderInt(_S("Lag Limit"), g_cfg->m_iLagLimit, 1, 14);

		std::vector < std::string > aLagTriggers
			=
		{
			_S("Move"),
			_S("Air"),
			_S("Peek")
		};

		if (ImGui::BeginCombo(_S("Lag Triggers"), _S("Select"), 0, aLagTriggers.size()))
		{
			for (int i = 0; i < aLagTriggers.size(); i++)
				ImGui::Selectable(aLagTriggers[i].c_str(), &g_cfg->m_aFakelagTriggers[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}

		ImGui::SliderInt(_S("Trigger Limit"), g_cfg->m_iTriggerLimit, 1, 14);
	}
	//ImGui::SameLine();
	ImGui::EndChild();
	//End Fake lag Tab

	// Start Misc Tab
	ImGui::SetCursorPos(ImVec2(408, 310 + tab_padding));
	ImGui::BeginChild(_S("Extra"), ImVec2(iChildDoubleSizeX, 150));
	{
		ImGui::Text(_S("Slow Walk"));
		ImGui::Keybind(_S("SW"), &g_cfg->m_aSlowwalk->m_iKeySelected, &g_cfg->m_aSlowwalk->m_iModeSelected);

		ImGui::Text(_S("Fake Duck"));
		ImGui::Keybind(_S("FD"), &g_cfg->m_aFakeDuck->m_iKeySelected, &g_cfg->m_aFakeDuck->m_iModeSelected);

		std::vector < const char* > aLegMovement
			=
		{
			_S("Default"),
			_S("Always slide"),
			_S("Always break")
		};

		ImGui::SingleSelect(_S("Leg Movement"), g_cfg->m_iLegMovement, aLegMovement);
		ImGui::Checkbox(_S("Static Legs In Air"), g_cfg->m_bStaticLegs);
		//ImGui::Checkbox(_S("Pitch Null On Land"), g_cfg->m_bPitchNull);
	}
	ImGui::EndChild();
	//ImGui::SameLine();
	// End Misc Tab
}

void C_Menu::DrawInventoryTab()
{
	//auto active_weapon = g_sdk.local->m_hActiveWeapon().Get()->m_iItemDefinitionIndex();
	//int iChildDoubleSizeX = 251;

	//ImGui::SetCursorPos(ImVec2(108, 310 + tab_padding));
	//ImGui::BeginChild(_S("Extra"), ImVec2(iChildDoubleSizeX + 100, 350));
	//{
	//	if (ImGui::InputInt("Paint Kit", &g_cfg->skins.skinChanger[active_weapon].paintKit, 1, 100));
	//		SkinChanger::scheduleHudUpdate();
	//}

	//ImGui::EndChild();
}

void C_Menu::DrawPlayersTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;
	ImGui::PopFont();

	//ImGui::SetNextWindowPos(ImVec2(iChildPosFirstX, iChildPosFirstY));
	ImGui::SetCursorPos(ImVec2(80, 78 + tab_padding));
	ImGui::BeginChild("ESP", ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));

	static int iPlayerESPType = 0;

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::SingleSelect(_S("Player Group"), &iPlayerESPType, { _S("Enemy"), _S("Team"), _S("Local") });

	static C_PlayerSettings* Settings = NULL;
	switch (iPlayerESPType)
	{
	case 0: Settings = g_cfg->m_Enemies; break;
	case 1: Settings = g_cfg->m_Teammates; break;
	case 2: Settings = g_cfg->local; break;
	}

	ImGui::Checkbox(_S("Box"), &Settings->m_BoundaryBox);
	this->DrawColorEdit4(_S("Box##color"), &Settings->m_aBoundaryBox);

	ImGui::Checkbox(_S("Name"), &Settings->m_RenderName);
	this->DrawColorEdit4(_S("Name##color"), &Settings->m_aNameColor);

	ImGui::Checkbox(_S("Health Bar"), &Settings->m_RenderHealthBar);
	this->DrawColorEdit4(_S("m_aHealth##color"), &Settings->m_aHealthBar);

	ImGui::Checkbox(_S("Health bar Text"), &Settings->m_RenderHealthText);
	this->DrawColorEdit4(_S("m_aHealthText##color"), &Settings->m_aHealthText);

	ImGui::Checkbox(_S("Ammo Bar"), &Settings->m_RenderAmmoBar);
	this->DrawColorEdit4(_S("m_aAmmoBar##color"), &Settings->m_aAmmoBar);

	ImGui::Checkbox(_S("Ammo Bar Text"), &Settings->m_RenderAmmoBarText);
	this->DrawColorEdit4(_S("m_aAmmoBarText##color"), &Settings->m_aAmmoBarText);

	ImGui::Checkbox(_S("Weapon Text"), &Settings->m_RenderWeaponText);
	this->DrawColorEdit4(_S("m_aWeaponText##color"), &Settings->m_aWeaponText);

	ImGui::Checkbox(_S("Weapon Icon"), &Settings->m_RenderWeaponIcon);
	this->DrawColorEdit4(_S("m_aWeaponIcon##color"), &Settings->m_aWeaponIcon);

	ImGui::Checkbox(_S("Out Of View Arrows"), g_cfg->m_bOutOfViewArrows);
	this->DrawColorEdit4(_S("awerqweqw2e123412er412q4##color"), g_cfg->m_aOutOfViewArrows);
	//ImGui::SliderInt(_S("Out of view arrows size"), g_cfg->m_bOutOfViewArrowsSize, 1, 100);
	//ImGui::SliderInt(_S("Out of view arrows radius"), g_cfg->m_bOutOfViewArrowsPoint, 1, 100);

	const char* aFlags[6]
		=
	{
		"Scoped",
		"Armor",
		"Flashed",
		"Location",
		"Money",
		"Fake duck"
	};

	for (int i = 0; i < 6; i++)
	{
		ImGui::Checkbox(aFlags[i], &Settings->m_Flags[i]);
		this->DrawColorEdit4(("##" + std::to_string(i)).c_str(), &Settings->m_Colors[i]);
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::SetCursorPos(ImVec2(408, 78 + tab_padding));
	ImGui::BeginChild(_S("Chams"), ImVec2(iChildDoubleSizeX, 170));
	static int32_t iChamsGroup = 0;
	ImGui::SingleSelect(_S("Chams group"), &iChamsGroup, {
		_S("Enemy Visible"),
		_S("Enemy Vnvisble"),
		_S("Backtrack"),
		_S("On Shot Chams"),
		_S("Team Visible"),
		_S("Team Vnvisible"),
		_S("Local"),
		_S("Desync"),
		_S("Weapons"),
		_S("Arms")
		});

	ImGui::Checkbox(_S("Enable Chams"), &g_cfg->m_aChamsSettings[iChamsGroup].m_bRenderChams);
	this->DrawColorEdit4(_S("##qweqwe"), &g_cfg->m_aChamsSettings[iChamsGroup].m_Color);

	ImGui::SingleSelect(_S("Material"), &g_cfg->m_aChamsSettings[iChamsGroup].m_iMainMaterial, { _S("Flat"), _S("Regular"), _S("Glow"), _S("Glow"), _S("Pearlescent")});

	ImGui::Checkbox(_S("Enable Glow"), &g_cfg->m_aChamsSettings[iChamsGroup].m_aModifiers[0]);
	this->DrawColorEdit4(_S("##512414 color"), &g_cfg->m_aChamsSettings[iChamsGroup].m_aModifiersColors[0]);

	ImGui::Checkbox(_S("Enable armsrace"), &g_cfg->m_aChamsSettings[iChamsGroup].m_aModifiers[1]);
	this->DrawColorEdit4(_S("##235235 color"), &g_cfg->m_aChamsSettings[iChamsGroup].m_aModifiersColors[1]);
	/*ImGui::Checkbox( _S( "Enable glass" ), &g_cfg->m_aChamsSettings[ iChamsGroup ].m_aModifiers[ 2 ] );
	this->DrawColorEdit4( _S( "##4124124 color" ), &g_cfg->m_aChamsSettings[ iChamsGroup ].m_aModifiersColors[ 2 ] );
	ImGui::Checkbox( _S( "Enable pulsation" ), &g_cfg->m_aChamsSettings[ iChamsGroup ].m_aModifiers[ 3 ] );
	this->DrawColorEdit4( _S( "##123123 color" ), &g_cfg->m_aChamsSettings[ iChamsGroup ].m_aModifiersColors[ 3 ] );*/
	ImGui::Checkbox(_S("Ragdoll Chams"), g_cfg->m_bDrawRagdolls);
	ImGui::Checkbox(_S("Attachment Chams"), g_cfg->m_bAttachmentChams);
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(408, 275 + tab_padding));
	ImGui::BeginChild(_S("Glow"), ImVec2(iChildDoubleSizeX, 205));

	ImGui::Checkbox(_S("Player Glow"), &Settings->m_bRenderGlow);
	this->DrawColorEdit4(_S("##m_aGlowcolor"), &Settings->m_aGlow);
	ImGui::SingleSelect(_S("Glow style##1"), &Settings->m_iGlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::Checkbox(_S("C4 Glow"), g_cfg->m_bRenderC4Glow);
	this->DrawColorEdit4(_S("##C4Glowcolor"), g_cfg->m_aC4Glow);
	ImGui::SingleSelect(_S("Glow style##m_iC4GlowStyle"), g_cfg->m_iC4GlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::Checkbox(_S("Dropped Weapon Glow"), g_cfg->m_bRenderDroppedWeaponGlow);
	this->DrawColorEdit4(_S("m_aDroppedWeaponGlow##color"), g_cfg->m_aDroppedWeaponGlow);
	ImGui::SingleSelect(_S("Glow style##m_iDroppedWeaponGlowStyle"), g_cfg->m_iDroppedWeaponGlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::Checkbox(_S("Projectiles Glow"), g_cfg->m_bRenderProjectileGlow);
	this->DrawColorEdit4(_S("##Projectile"), g_cfg->m_aProjectileGlow);
	ImGui::SingleSelect(_S("Glow style##m_iProjectileGlowStyle"), g_cfg->m_iProjectileGlowStyle, { _S("Outline"), _S("Thin"), _S("Cover"), _S("Cover Pulse") });

	ImGui::EndChild();

	if (iChamsGroup < 8)
		g_DrawModel->SetChamsSettings(g_cfg->m_aChamsSettings[iChamsGroup]);

	g_DrawModel->SetGlow(Settings->m_iGlowStyle);
	if (!Settings->m_bRenderGlow)
		g_DrawModel->SetGlow(-1);
	else
		g_DrawModel->SetGlowColor(Color(Settings->m_aGlow));

	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y + 2), ImVec2(vecWindowPosition.x + 950 + 150, vecWindowPosition.y + 440), ImColor(3, 16, 32, 170), 4.0f);
	if (g_DrawModel->GetTexture())
	{
		ImGui::GetForegroundDrawList()->AddImage(
			g_DrawModel->GetTexture()->pTextureHandles[0]->lpRawTexture,
			ImVec2(vecWindowPosition.x + 610 + 150, vecWindowPosition.y - 130),
			ImVec2(vecWindowPosition.x + 610 + 150 + g_DrawModel->GetTexture()->GetActualWidth(), vecWindowPosition.y + g_DrawModel->GetTexture()->GetActualHeight() - 130),
			ImVec2(0, 0), ImVec2(1, 1),
			ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// render box
	Color aBox = Color(Settings->m_aBoundaryBox);
	if (Settings->m_BoundaryBox)
	{
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 694 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 886 + 170, vecWindowPosition.y + 386), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 696 + 150, vecWindowPosition.y + 41), ImVec2(vecWindowPosition.x + 884 + 170, vecWindowPosition.y + 384), ImColor(0, 0, 0, 255));
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 885 + 170, vecWindowPosition.y + 385), ImColor(aBox.r(), aBox.g(), aBox.b(), aBox.a()));
	}

	// render name
	Color aName = Color(Settings->m_aNameColor);
	if (Settings->m_RenderName)
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("Name")).x / 2, vecWindowPosition.y + 22), ImColor(aName.r(), aName.g(), aName.b(), aName.a()), _S("Name"));
		ImGui::PopFont();
	}

	// render health
	Color aHealthBar = Color(Settings->m_aHealthBar);
	if (Settings->m_RenderHealthBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 689 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 693 + 150, vecWindowPosition.y + 385), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 690 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 692 + 150, vecWindowPosition.y + 385), ImColor(aHealthBar.r(), aHealthBar.g(), aHealthBar.b(), aHealthBar.a()));
	}

	Color aHealthText = Color(Settings->m_aHealthText);
	if (Settings->m_RenderHealthText)
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);

		if (Settings->m_RenderHealthBar)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 150 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 691 + 150 - ImGui::CalcTextSize(_S("100")).x, vecWindowPosition.y + 37), ImColor(aHealthText.r(), aHealthText.g(), aHealthText.b(), aHealthText.a()), _S("100"));

		ImGui::PopFont();
	}

	Color aFlagSColor = Color(Settings->m_Colors[0]);
	if (Settings->m_Flags[0])
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 350 - ImGui::CalcTextSize(_S("SCOPED")).x, vecWindowPosition.y + 40), ImColor(aFlagSColor.r(), aFlagSColor.g(), aFlagSColor.b(), aFlagSColor.a()), _S("SCOPED"));

		ImGui::PopFont();
	}

	//Color aFlagAColor = Color(Settings->m_Colors[1]);
	//if (Settings->m_Flags[1])
	//{
	//	if (!g_sdk.local)
	//		return;

	//	auto kevlar = g_sdk.local->m_ArmourValue() > 0;
	//	auto helmet = g_sdk.local->m_bHasHelmet();

	//	const char* text;

	//	if (helmet && kevlar)
	//		text = "q";
	//	else if (kevlar)
	//		text = "p";

	//	ImGui::PushFont(g_sdk.fonts.m_WeaponIcon);
	//	ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 350 - ImGui::CalcTextSize(text).x, vecWindowPosition.y + 43), ImColor(aFlagAColor.r(), aFlagAColor.g(), aFlagAColor.b(), aFlagAColor.a()), text);

	//	ImGui::PopFont();
	//}

	Color aFlagFColor = Color(Settings->m_Colors[2]);
	if (Settings->m_Flags[2])
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 356 - ImGui::CalcTextSize(_S("FLASHED")).x, vecWindowPosition.y + 52), ImColor(aFlagFColor.r(), aFlagFColor.g(), aFlagFColor.b(), aFlagFColor.a()), _S("FLASHED"));

		ImGui::PopFont();
	}

	Color aFlagLColor = Color(Settings->m_Colors[3]);
	if (Settings->m_Flags[3])
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 364 - ImGui::CalcTextSize(_S("LOCATION")).x, vecWindowPosition.y + 64), ImColor(aFlagLColor.r(), aFlagLColor.g(), aFlagLColor.b(), aFlagLColor.a()), _S("LOCATION"));

		ImGui::PopFont();
	}

	Color aFlagMColor = Color(Settings->m_Colors[4]);
	if (Settings->m_Flags[4])
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 338 - ImGui::CalcTextSize(_S("$1000")).x, vecWindowPosition.y + 75), ImColor(aFlagMColor.r(), aFlagMColor.g(), aFlagMColor.b(), aFlagMColor.a()), _S("100 $"));

		ImGui::PopFont();
	}

	Color aFlagFDColor = Color(Settings->m_Colors[5]);
	if (Settings->m_Flags[5])
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 687 + 321 - ImGui::CalcTextSize(_S("FD")).x, vecWindowPosition.y + 85), ImColor(aFlagFDColor.r(), aFlagFDColor.g(), aFlagFDColor.b(), aFlagFDColor.a()), _S("FD"));

		ImGui::PopFont();
	}

	Color aWeaponText = Color(Settings->m_aWeaponText);
	if (Settings->m_RenderWeaponText)
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("SSG08")).x / 2, vecWindowPosition.y + 385 + 6), ImColor(aWeaponText.r(), aWeaponText.g(), aWeaponText.b(), aWeaponText.a()), _S("SSG08"));
		ImGui::PopFont();
	}

	Color aWeaponIcon = Color(Settings->m_aWeaponIcon);
	if (Settings->m_RenderWeaponIcon)
	{
		ImGui::PushFont(g_sdk.fonts.m_WeaponIcon);

		if (Settings->m_RenderWeaponText)
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("a")).x / 2, vecWindowPosition.y + 385 + 22), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("a"));
		else
			ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 715 + 150 + 85 - ImGui::CalcTextSize(_S("a")).x / 2, vecWindowPosition.y + 385 + 8), ImColor(aWeaponIcon.r(), aWeaponIcon.g(), aWeaponIcon.b(), aWeaponIcon.a()), _S("a"));

		ImGui::PopFont();
	}

	Color aAmmoBar = Color(Settings->m_aAmmoBar);
	if (Settings->m_RenderAmmoBar)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 387), ImVec2(vecWindowPosition.x + 906 + 150, vecWindowPosition.y + 391), ImColor(0, 0, 0, 100));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 388), ImVec2(vecWindowPosition.x + 905 + 150, vecWindowPosition.y + 390), ImColor(aAmmoBar.r(), aAmmoBar.g(), aAmmoBar.b(), aAmmoBar.a()));
	}

	Color aAmmoText = Color(Settings->m_aAmmoBarText);
	if (Settings->m_RenderAmmoBarText)
	{
		ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(vecWindowPosition.x + 902 + 150 + ImGui::CalcTextSize(_S("13")).x / 2, vecWindowPosition.y + 386), ImColor(aAmmoText.r(), aAmmoText.g(), aAmmoText.b(), aAmmoText.a()), _S("13"));
		ImGui::PopFont();
	}
}

static int Selected = 0;

void C_Menu::DrawMiscTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iMainTextSize = ImGui::CalcTextSize(this->GetMenuName()).x;
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(408, 228 + tab_padding));
	ImGui::BeginChild(_S("Movement"), ImVec2(iChildDoubleSizeX, 120));
	{
		ImGui::Checkbox(_S("Auto jump"), g_cfg->m_bBunnyHop);
		ImGui::Checkbox(_S("Auto strafe"), g_cfg->m_bAutoStrafe);
		ImGui::Checkbox(_S("Fast stop"), g_cfg->m_bFastStop);
		ImGui::Text(_S("Edge jump"));
		ImGui::Keybind(_S("Edge jump Bind"), &g_cfg->m_aEdgeJump->m_iKeySelected, &g_cfg->m_aEdgeJump->m_iModeSelected);
		ImGui::Text(_S("Auto peek"));
		ImGui::Keybind(_S("Auto peek"), &g_cfg->m_aAutoPeek->m_iKeySelected, &g_cfg->m_aAutoPeek->m_iModeSelected);
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::SetCursorPos(ImVec2(408, 78 + tab_padding));
	ImGui::BeginChild(_S("Interface"), ImVec2(iChildDoubleSizeX, 120));
	{
		std::vector < std::string > aLogItems = { _S("Hit"), _S("Harm"), _S("Purchase"), _S("Bomb"), _S("Miss") };
		if (ImGui::BeginCombo(_S("Event logs"), _S("Select"), ImGuiComboFlags_HeightSmall, aLogItems.size()))
		{
			ImGui::Selectable(aLogItems[0].c_str(), g_cfg->m_bLogHurts.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
			ImGui::Selectable(aLogItems[1].c_str(), g_cfg->m_bLogHarms.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
			ImGui::Selectable(aLogItems[2].c_str(), g_cfg->m_bLogPurchases.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
			ImGui::Selectable(aLogItems[3].c_str(), g_cfg->m_bLogBomb.GetPtr(), ImGuiSelectableFlags_DontClosePopups);
			ImGui::Selectable(aLogItems[4].c_str(), g_cfg->m_bLogMisses.GetPtr(), ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}

		ImGui::Checkbox(_S("Spectator list"), g_cfg->m_bSpectatorList);
		ImGui::Checkbox(_S("Keybind list"), g_cfg->m_bDrawKeyBindList);
		ImGui::Checkbox(_S("Watermark"), g_cfg->m_bWaterMark);
		ImGui::Checkbox(_S("Disable panorama blur"), g_cfg->m_bPanoramaBlur);
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::SetCursorPos(ImVec2(80, 78 + tab_padding));
	ImGui::BeginChild(_S("Other"), ImVec2(iChildDoubleSizeX, 300));
	{
		ImGui::Checkbox(_S("Anti Untrusted"), g_cfg->m_bAntiUntrusted);

		std::vector <const char*> t_model =
		{
			_S("None"),
	_S("Getaway Sally | The Professionals"),
	_S("Number K | The Professionals"),
	_S("Little Kev | The Professionals"),
	_S("Safecracker Voltzmann | The Professionals"),
	_S("Bloody Darryl The Strapped | The Professionals"),
	_S("Sir Bloody Loudmouth Darryl | The Professionals"),
	_S("Sir Bloody Darryl Royale | The Professionals"),
	_S("Sir Bloody Darryl Royale | The Professionals"),
	_S("Sir Bloody Miami Darryl | The Professionals"),
	_S("Soldier | Phoenix"),
	_S("Slingshot | Phoenix"),
	_S("Enforcer | Phoenix"),
	_S("Mr. Muhlik | Elite Crew"),
	_S("Prof. Shahmat | Elite Crew"),
	_S("Osiris | Elite Crew"),
	_S("Ground Rebel | Elite Crew"),
	_S("The Elite Mr. Muhlik | Elite Crew"),
	_S("Trapper | Guerrilla Warfare"),
	_S("Trapper Aggressor | Guerrilla Warfare"),
	_S("Vypa Sista of the Revolution | Guerrilla Warfare"),
	_S("Col. Mangos Dabisi | Guerrilla Warfare"),
	_S("Arno The Overgrown | Guerrilla Warfare"),
	_S("'Medium Rare' Crasswater | Guerrilla Warfare"),
	_S("Crasswater The Forgotten | Guerrilla Warfare"),
	_S("Elite Trapper Solman | Guerrilla Warfare"),
	_S("'The Doctor' Romanov | Sabre"),
	_S("Blackwolf | Sabre"),
	_S("Maximus | Sabre"),
	_S("Dragomir | Sabre"),
	_S("Rezan The Ready | Sabre"),
	_S("Rezan the Redshirt | Sabre"),
	_S("Dragomir | Sabre Footsoldier"),
		};

		std::vector <const char*> ct_model =
		{
	_S("None"),
	_S("Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman"),
	_S("Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman"),
	_S("Lieutenant Rex Krikey | SEAL Frogman"),
	_S("Michael Syfers | FBI Sniper"),
	_S("Operator | FBI SWAT"),
	_S("Special Agent Ava | FBI"),
	_S("Markus Delrow | FBI HRT"),
	_S("Sous-Lieutenant Medic | Gendarmerie Nationale"),
	_S("Chem-Haz Capitaine | Gendarmerie Nationale"),
	_S("Chef d'Escadron Rouchard | Gendarmerie Nationale"),
	_S("Aspirant | Gendarmerie Nationale"),
	_S("Officer Jacques Beltram | Gendarmerie Nationale"),
	_S("D Squadron Officer | NZSAS"),
	_S("B Squadron Officer | SAS"),
	_S("Seal Team 6 Soldier | NSWC SEAL"),
	_S("Buckshot | NSWC SEAL"),
	_S("Lt. Commander Ricksaw | NSWC SEAL"),
	_S("Lt. Commander Ricksaw | NSWC SEAL"),
	_S("'Blueberries' Buckshot | NSWC SEAL"),
	_S("3rd Commando Company | KSK"),
	_S("'Two Times' McCoy | TACP Cavalry"),
	_S("'Two Times' McCoy | USAF TACP"),
	_S("Primeiro Tenente | Brazilian 1st Battalion"),
	_S("Cmdr. Mae 'Dead Cold' Jamison | SWAT"),
	_S("1st Lieutenant Farlow | SWAT"),
	_S("John 'Van Healen' Kask | SWAT"),
	_S("Bio-Haz Specialist | SWAT"),
	_S("Sergeant Bombson | SWAT"),
	_S("Chem-Haz Specialist | SWAT"),
	_S("Lieutenant 'Tree Hugger' Farlow | SWAT"),
		};

		ImGui::Checkbox(_S("Hold fire animation"), g_cfg->m_bHoldFireAnimation);
		ImGui::Checkbox(_S("Filter server ads"), g_cfg->m_bAdBlock);
		ImGui::Checkbox(_S("Filter console"), g_cfg->m_bFilterConsole);
		ImGui::SingleSelect(_S("T model"), g_cfg->m_nModelT, t_model);
		ImGui::SingleSelect(_S("CT model"), g_cfg->m_nModelCT, ct_model);

		//ImGui::Checkbox(_S("Fix scope sensivity"), g_cfg->m_fixZomSens);
		ImGui::Checkbox(_S("Unlock convars"), g_cfg->m_bUnhideConvars);
		ImGui::Checkbox(_S("Reveal ranks"), g_cfg->m_bRevealRanks);
		ImGui::Checkbox(_S("Unlock inventory"), g_cfg->m_bUnlockInventoryAccess);

		ImGui::Checkbox(_S("Clantag"), g_cfg->m_bTagChanger);
		ImGui::Checkbox(_S("Hitsound"), g_cfg->m_bHitSound);
	//	ImGui::Checkbox(_S("Killsound"), g_cfg->m_bKillsound);

		ImGui::Checkbox(_S("Penetration crosshair"), g_cfg->m_bPenetrationCrosshair);
		ImGui::Checkbox(_S("Force crosshair"), g_cfg->m_bForceCrosshair);
		ImGui::Checkbox(_S("Preserve killfeed"), g_cfg->m_bPreserveKillfeed);
		ImGui::SliderInt(_S("Ping"), g_cfg->m_iPingSpike, 0, 200);
	}
	ImGui::EndChild();
	ImGui::SameLine();
}

void C_Menu::DrawWorldTab()
{
	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
	int iChildDoubleSizeX = 251;
	int iChildDoubleSizeY = 363;
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(408, 298 + tab_padding));
	ImGui::BeginChild(_S("View settings"), ImVec2(iChildDoubleSizeX, 175));
	{
		ImGui::Text(_S("Thirdperson"));
		ImGui::Keybind(_S("ThirdPerson Bind"), &g_cfg->m_aThirdPerson->m_iKeySelected, &g_cfg->m_aThirdPerson->m_iModeSelected);
		ImGui::SliderInt(_S("Distance"), g_cfg->m_iThirdPersonDistance, 50, 300);
		ImGui::SliderInt(_S("Field of view"), g_cfg->m_iCameraDistance, 90, 140);
		ImGui::Checkbox(_S("Force distance while scoped"), g_cfg->m_bOverrideFOVWhileScoped);
		ImGui::Checkbox(_S("Left hand knife"), g_cfg->m_bLeftKnifeHand);
		ImGui::SliderFloat(_S("Aspect ratio"), g_cfg->m_flAspectRatio, 0.01f, 3.0f);
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::SetCursorPos(ImVec2(408, 78 + tab_padding));
	ImGui::BeginChild(_S("Viewmodel"), ImVec2(iChildDoubleSizeX, 190));
	{
		ImGui::SliderInt(_S("Viewmodel distance"), g_cfg->m_iViewmodelDistanceFix, 0, 89);
		ImGui::SliderInt(_S("Viewmodel X"), g_cfg->m_iViewmodelX, -10, 10);
		ImGui::SliderInt(_S("Viewmodel Y"), g_cfg->m_iViewmodelY, -10, 10);
		ImGui::SliderInt(_S("Viewmodel Z"), g_cfg->m_iViewmodelZ, -10, 10);
		ImGui::SliderInt(_S("Viewmodel Roll"), g_cfg->m_iViewmodelRoll, -90, 90);
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::SetCursorPos(ImVec2(80, 78 + tab_padding));
	ImGui::BeginChild(_S("World"), ImVec2(iChildDoubleSizeX, 410));
	{
		std::vector < const char* > aSkyboxList =
		{
			_S("None"),	_S("Tibet"),_S("Baggage"),_S("Italy"),_S("Aztec"),_S("Vertigo"),_S("Daylight"),_S("Daylight 2"),_S("Clouds"),_S("Clouds 2"),_S("Gray"),_S("Clear"),_S("Canals"),_S("Cobblestone"),_S("Assault"),_S("Clouds dark"),_S("Night"),_S("Night 2"),_S("Night flat"),_S("Dusty"),_S("Rainy"),_S("Custom")
		};
	/*	ImGui::BeginChild(_S("Weather"), ImVec2(iChildDoubleSizeX, iChildDoubleSizeY));
		ImGui::Checkbox(_S("Enable weather"), g_cfg->m_CustomWeather);

		ImGui::SliderInt(_S("Rain length"), g_cfg->m_RainLength, 1, 100);
		ImGui::SliderInt(_S("Rain width"), g_cfg->m_RainWidth, 1, 100);
		ImGui::SliderInt(_S("Rain speed"), g_cfg->m_RainSpeed, 1, 2000);*/
		ImGui::Text(_S("World color"));
		this->DrawColorEdit4(_S("##123123"), g_cfg->m_WorldModulation);

		ImGui::Text(_S("Props color"));
		this->DrawColorEdit4(_S("##11233"), g_cfg->m_PropModulation);

		ImGui::Text(_S("Skybox color"));
		this->DrawColorEdit4(_S("##51223"), g_cfg->m_SkyModulation);

		ImGui::SingleSelect(_S("Skybox changer"), g_cfg->m_iSkybox.GetPtr(), aSkyboxList);
		if (g_cfg->m_iSkybox == aSkyboxList.size() - 1)
		{
			static char aSkyBox[32];
			if (!g_cfg->m_szCustomSkybox->empty())
				strcpy(aSkyBox, g_cfg->m_szCustomSkybox.Get().c_str());

			if (ImGui::InputText(_S("##324234124"), aSkyBox, 32))
				g_cfg->m_szCustomSkybox.Get() = aSkyBox;
		}

		ImGui::Checkbox(_S("Sun Set Mode"), g_cfg->bSunSetMode);
		ImGui::SliderInt(_S("Sunset X"), g_cfg->m_iSunSetX, -100, 100);
		ImGui::SliderInt(_S("Sunset Y"), g_cfg->m_iSunSetY, -100, 100);
		ImGui::SliderInt(_S("Sunset Z"), g_cfg->m_iSunSetZ, -100, 100);

		ImGui::Checkbox(_S("Hitmarker"), g_cfg->m_bHitMarker);
		ImGui::Checkbox(_S("Damage marker"), g_cfg->m_bDamageMarker);

		ImGui::Checkbox(_S("Client bullet impacts"), g_cfg->m_bDrawClientImpacts);
		this->DrawColorEdit4(_S("##41242354"), g_cfg->m_ClientImpacts);
		ImGui::Checkbox(_S("Server bullet impacts"), g_cfg->m_bDrawServerImpacts);
		this->DrawColorEdit4(_S("##412423154"), g_cfg->m_ServerImpacts);

		ImGui::Checkbox(_S("Local bullet tracers"), g_cfg->m_bDrawLocalTracers);
		this->DrawColorEdit4(_S("##43242354"), g_cfg->m_LocalTracers);
		/*	ImGui::Checkbox(_S("Enemy bullet tracers"), g_cfg->m_bDrawEnemyTracers);
			this->DrawColorEdit4(_S("##432423154"), g_cfg->m_EnemyTracers);*/

		std::vector < std::string > aWorldRemovals =
		{
			_S("Visual punch"),
			_S("Visual kick"),
			_S("Scope"),
			_S("Zoom"),
			_S("Smoke"),
			_S("Fire"),
			_S("Flash"),
			_S("Post process"),
			_S("Fog"),
			_S("Shadows"),
			_S("Landing bob"),
			_S("Hand shaking"),
			_S("Zoom")
		};

		ImGui::Checkbox(_S("Bomb timer"), g_cfg->m_bBombTimer);
		ImGui::Checkbox(_S("Grenade prediction"), g_cfg->m_bPredictGrenades);
		this->DrawColorEdit4(_S("##1234142124"), g_cfg->m_GrenadeWarning);

		if (ImGui::BeginCombo(_S("Removals"), _S("Select"), 0, 7))
		{
			for (int i = 0; i < aWorldRemovals.size(); i++)
				ImGui::Selectable(aWorldRemovals[i].c_str(), &g_cfg->m_aWorldRemovals[i], ImGuiSelectableFlags_DontClosePopups);

			ImGui::EndCombo();
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
}

void C_Menu::DrawConfigTab()
{
	static std::string selected_cfg = "";
	static char cfg_name[32];

	ImGui::BeginChild(_S("Config Settings"), ImVec2(351, 363));
	{
		if (ImGui::InputText(_S(""), cfg_name, 32)) selected_cfg = std::string(cfg_name);

		if (ImGui::Button(_S("Create config"), ImVec2(332, 25)))
		{
			std::ofstream(selected_cfg + ".cfg", std::ios_base::trunc);
			g_ConfigSystem->SaveConfig((selected_cfg + (std::string)(".cfg")).c_str());
		}

		if (ImGui::Button(_S("Save config"), ImVec2(332, 25)))
			g_ConfigSystem->SaveConfig(selected_cfg.c_str());

		if (ImGui::Button(_S("Load config"), ImVec2(332, 25)))
			g_ConfigSystem->LoadConfig(selected_cfg.c_str());
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild(_S("Config list"), ImVec2(371, 363));
	{
		{
			for (auto cfg : g_ConfigSystem->GetConfigList())
				if (ImGui::Selectable(cfg.c_str(), cfg == selected_cfg))
					selected_cfg = cfg;
		}
	}
	ImGui::EndChild();
}

void C_Menu::DrawColorEdit4(const char* strLabel, Color* aColor)
{
	float aColour[4] =
	{
		aColor->r() / 255.0f,
		aColor->g() / 255.0f,
		aColor->b() / 255.0f,
		aColor->a() / 255.0f
	};

	if (ImGui::ColorEdit4(strLabel, aColour, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_RGB))
		aColor->SetColor(aColour[0], aColour[1], aColour[2], aColour[3]);
}

void C_Menu::Initialize()
{
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f);
	ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ImVec4(65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f);
	ImGui::GetStyle().AntiAliasedFill = true;
	ImGui::GetStyle().AntiAliasedLines = true;
	ImGui::GetStyle().ScrollbarSize = 6;
	ImGui::GetStyle().WindowRounding = 12;

	//D3DXCreateTextureFromFileInMemoryEx(g_interfaces.direct_device, &avatarka, sizeof(avatarka), 512, 512, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &g_Menu->m_dTexture);
	//D3DXCreateTextureFromFileInMemoryEx(g_interfaces.direct_device, &backgroundcsgo, sizeof(backgroundcsgo), 860, 600, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &g_Menu->m_dTexture2);
	//ImGui::ImageButton(g_Menu->m_dTexture, ImVec2(512, 512));

	//for (auto i = 0; i < g_cfg->skins.skinChanger.size(); i++)
	//	if (!all_Skins[i])
	//		all_Skins[i] = get_Skin_preview(get_wep(i, (i == 0 || i == 1) ? g_cfg->skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_Str(), g_cfg->skins.skinChanger.at(i).skin_name, g_interfaces.direct_device); //-V810
}

#include "../Features/Networking/Networking.hpp"
void C_Menu::WaterMark()
{
	std::string szWatermark = _S("reborn hack") + g_sdk.m_szUzername + _S(" |");
	if (!g_cfg->m_bWaterMark)
		return;

	C_NetChannelInfo* m_NetChannelInfo = g_interfaces.engine->GetNetChannelInfo();
	if (g_interfaces.engine->IsConnected())
	{
		if (m_NetChannelInfo)
		{
			if (m_NetChannelInfo->IsLoopback())
				szWatermark += _S(" local server |");
			else
				szWatermark += " " + (std::string)(m_NetChannelInfo->GetAddress()) + " | ";

			szWatermark += (std::string)(" ") + std::to_string((int)(m_NetChannelInfo->GetAvgLatency(FLOW_OUTGOING) + m_NetChannelInfo->GetAvgLatency(FLOW_INCOMING))) + (std::string)("ms |");
		}
	}
	else
		szWatermark += _S(" unconnected |");

	int nScreenSizeX, nScreenSizeY;
	g_interfaces.engine->GetScreenSize(nScreenSizeX, nScreenSizeY);

	int nTextLength = g_sdk.fonts.m_SegoeUI->CalcTextSizeA(16.0f, FLT_MAX, NULL, szWatermark.c_str()).x + 75;
	szWatermark += _S(" fps: ") + std::to_string((int)(1.0f / ImGui::GetIO().DeltaTime));

	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(nScreenSizeX - nTextLength, 11), ImVec2(nScreenSizeX - 10, 30), ImColor(24.0f / 255.0f, 31.0f / 255.0f, 44.0f / 255.0f, 1.0f));
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(nScreenSizeX - nTextLength, 30), ImVec2(nScreenSizeX - 10, 31), ImColor(0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 255.0f / 255.0f));
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(nScreenSizeX - nTextLength, 11), ImVec2(nScreenSizeX - 10, 30), ImColor(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 100.0f / 255.0f));

	ImGui::PushFont(g_sdk.fonts.m_SegoeUI);
	ImGui::GetOverlayDrawList()->AddText(ImVec2(nScreenSizeX - nTextLength + 7, 12), ImColor(255, 255, 255, 255), szWatermark.c_str());
	ImGui::PopFont();

	//ImGui::BeginGroup();
	//{
	//	ImGui::SetCursorPos(ImVec2(10,10));
	//	ImGui::Image(g_Menu->GetTexture(), ImVec2(10, 10));
	//}
	//ImGui::EndGroup();
}

#include "Tools/Tools.hpp"
#define PUSH_BIND( m_Variable, Name )\
if ( g_Tools->IsBindActive( m_Variable ) )\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = _S( Name );\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent + ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\
else\
{\
	if ( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent == 0.0f )\
		m_BindList[ FNV32( #m_Variable ) ].m_szName = "";\
	m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent = std::clamp( m_BindList[ FNV32( #m_Variable ) ].m_flAlphaPercent - ImGui::GetIO( ).DeltaTime * 10.0f, 0.0f, 1.0f );\
}\

const char* keyss[] = {
	"None",
	"M1",
	"M2",
	"CN",
	"M3",
	"M4",
	"M5",
	"-",
	"BAC",
	"TAB",
	"-",
	"-",
	"CLR",
	"RET",
	"-",
	"-",
	"SHI",
	"CTL",
	"MEN",
	"PAU",
	"CAPS",
	"KAN",
	"-",
	"JUN",
	"FIN",
	"KAN",
	"-",
	"ESC",
	"CON",
	"NCO",
	"ACC",
	"MAD",
	"SPACE",
	"PGU",
	"PGD",
	"END",
	"HOM",
	"LEF",
	"UP",
	"RIG",
	"DOW",
	"SEL",
	"PRI",
	"EXE",
	"PRI",
	"INS",
	"DEL",
	"HEL",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"WIN",
	"WIN",
	"APP",
	"-",
	"SLE",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"NUM",
	"MUL",
	"ADD",
	"SEP",
	"MIN",
	"DEC",
	"DIV",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"F16",
	"F17",
	"F18",
	"F19",
	"F20",
	"F21",
	"F22",
	"F23",
	"F24",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"NUM",
	"SCR",
	"EQU",
	"MAS",
	"TOY",
	"OYA",
	"OYA",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"SHIFT",
	"SHIFT",
	"CTRL",
	"CTRL",
	"ALT",
	"ALT"
};

void C_Menu::DrawKeybindList()
{
	if (!g_cfg->m_bDrawKeyBindList)
		return;

	int m_Last = 0;
	PUSH_BIND(g_cfg->m_aFakeDuck, "Fake Duck");
	PUSH_BIND(g_cfg->m_aDoubleTap, "Double Tap");
	PUSH_BIND(g_cfg->m_aSlowwalk, "Slow Walk");
	PUSH_BIND(g_cfg->m_aHideShots, "Hide Shots");
	PUSH_BIND(g_cfg->m_aSafePoint, "Safe Points");
	PUSH_BIND(g_cfg->m_aInverter, "Inverter");
	PUSH_BIND(g_cfg->m_aAutoPeek, "Auto Peek");
	PUSH_BIND(g_cfg->m_aMinDamage, "Damage Override");
	PUSH_BIND(g_cfg->m_aEdgeJump, "Edge Jump");

	int32_t iCount = 0;
	for (auto& Bind : m_BindList)
	{
		if (Bind.second.m_szName.length())
			iCount++;
	}

	if (iCount <= 0 && !m_bIsMenuOpened)
		return;

	int nAdvancedFlag = 0;
	if (!m_bIsMenuOpened)
		nAdvancedFlag = ImGuiWindowFlags_NoMove;

	if (m_bIsMenuOpened)
	{
		ImGui::DefaultBegin(_S("Keybinds"), g_cfg->m_bDrawKeyBindList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag);
		{
			int32_t x = ImGui::GetCurrentWindow()->Pos.x + 4.5f;
			int32_t y = ImGui::GetCurrentWindow()->Pos.y;

			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + 181, y + 22), ImColor(36 / 255.0f, 37 / 255.0f, 41.0f / 255.0f, 170.f / 255.f));

			ImGui::PushFont(g_sdk.fonts.m_LogFont);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 65, y + 2), ImColor(255, 255, 255), _S("Keybinds"));

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 27), ImColor(255, 255, 255), _S("Double Tap"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 27), ImColor(255, 255, 255), g_cfg->m_aDoubleTap->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 27), ImColor(255, 255, 255), keyss[g_cfg->m_aDoubleTap->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 40), ImColor(255, 255, 255), _S("Hide Shots"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 40), ImColor(255, 255, 255), g_cfg->m_aHideShots->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 40), ImColor(255, 255, 255), keyss[g_cfg->m_aHideShots->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 52), ImColor(255, 255, 255), _S("Auto Peek"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 52), ImColor(255, 255, 255), g_cfg->m_aAutoPeek->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 52), ImColor(255, 255, 255), keyss[g_cfg->m_aAutoPeek->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 64), ImColor(255, 255, 255), _S("Slow Walk"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 64), ImColor(255, 255, 255), g_cfg->m_aSlowwalk->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 64), ImColor(255, 255, 255), keyss[g_cfg->m_aSlowwalk->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 76), ImColor(255, 255, 255), _S("Fake Duck"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 76), ImColor(255, 255, 255), g_cfg->m_aFakeDuck->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 76), ImColor(255, 255, 255), keyss[g_cfg->m_aFakeDuck->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 88), ImColor(255, 255, 255), _S("Inverter"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 88), ImColor(255, 255, 255), g_cfg->m_aInverter->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 88), ImColor(255, 255, 255), keyss[g_cfg->m_aInverter->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 98), ImColor(255, 255, 255), _S("Dmg Override"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 98), ImColor(255, 255, 255), g_cfg->m_aMinDamage->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 98), ImColor(255, 255, 255), keyss[g_cfg->m_aMinDamage->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 110), ImColor(255, 255, 255), _S("Safe Points"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 110), ImColor(255, 255, 255), g_cfg->m_aSafePoint->m_iModeSelected == 0 ? _S("Toggle") : _S("Hold"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 110), ImColor(255, 255, 255), keyss[g_cfg->m_aSafePoint->m_iKeySelected]);

			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, y + 122), ImColor(255, 255, 255), _S("Edge Jump"));
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 85, y + 122), ImColor(255, 255, 255), g_cfg->m_aEdgeJump->m_iKeySelected == 0 ? "Toggle" : "Hold");
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 145, y + 122), ImColor(255, 255, 255), keyss[g_cfg->m_aEdgeJump->m_iKeySelected]);

			ImGui::PopFont();
		}
		ImGui::DefaultEnd();
	}
	else if (!m_bIsMenuOpened)
	{
		ImGui::DefaultBegin(_S("Keybinds"), g_cfg->m_bDrawKeyBindList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag);
		{
			int32_t x = ImGui::GetCurrentWindow()->Pos.x + 4.5f;
			int32_t y = ImGui::GetCurrentWindow()->Pos.y;

			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + 181, y + 22), ImColor(36 / 255.0f, 37 / 255.0f, 41.0f / 255.0f, 170.f / 255.f));
			//ImGui::GetOverlayDrawList( )->AddRectFilled( ImVec2( x, y + 22 ), ImVec2( x + 181, y + 22.27f ), ImColor( 0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 1.0f ) );
			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y + 22.27f), ImVec2(x + 181, y + 22.30f), ImColor(0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 0.65f));

			//ImGui::PushFont( g_sdk.fonts.m_MenuIcons );
			//ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 5, y + 2 ), ImColor( 71, 163, 255 ), _S( "a" ) );
			//ImGui::PopFont( );

			//ImGui::SetCursorPos(ImVec2(10, 10));
			//ImGui::Image(this->GetTexture(), ImVec2(50, 70));

			ImGui::PushFont(g_sdk.fonts.m_LogFont);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 65, y + 2), ImColor(255, 255, 255), _S("Keybinds"));
			ImGui::PopFont();

			for (auto& Bind : m_BindList)
			{
				if (!Bind.second.m_szName.length())
					continue;

				ImGui::PushFont(g_sdk.fonts.m_LogFont);
				ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, 27 + (y + 16 * m_Last)), ImColor(255, 255, 255, static_cast <int>(Bind.second.m_flAlphaPercent * 255.0f)), Bind.second.m_szName.c_str());
				ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 160, 27 + (y + 16 * m_Last)), ImColor(255, 255, 255, static_cast <int>(Bind.second.m_flAlphaPercent * 255.0f)), _S("on"));
				ImGui::PopFont();

				//ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + 181, 27 + (y + 16 * m_Last)), ImColor(255 / 255.0f, 0 / 255.0f, 255 / 255.0f, 1.f));

				m_Last++;
			}
		}
		//ImGui::GetOverlayDrawList()->PopClipRect();
		ImGui::DefaultEnd();
	}
}

void C_Menu::DrawSpectatorList()
{
	std::vector < std::string > vecSpectatorList;
	if (!g_cfg->m_bSpectatorList)
		return;

	if (g_sdk.local && g_sdk.local->IsAlive())
	{
		for (int nPlayerID = 1; nPlayerID <= g_interfaces.globals->m_maxclients; nPlayerID++)
		{
			C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(nPlayerID);
			if (!pPlayer || pPlayer->IsAlive() || !pPlayer->IsPlayer() || pPlayer->IsDormant() || !pPlayer->m_hObserverTarget())
				continue;

			C_PlayerInfo m_TargetInfo;
			g_interfaces.engine->GetPlayerInfo(pPlayer->EntIndex(), &m_TargetInfo);

			vecSpectatorList.emplace_back((std::string)(m_TargetInfo.m_strName));
		}
	}

	if (!m_bIsMenuOpened && vecSpectatorList.empty())
		return;

	int nAdvancedFlag = 0;
	if (!m_bIsMenuOpened)
		nAdvancedFlag = ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowSize(ImVec2(190, m_BindList.empty() ? 0 : 35 + (21.5f * vecSpectatorList.size())));
	ImGui::DefaultBegin(_S("Spectator List"), g_cfg->m_bDrawKeyBindList, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | nAdvancedFlag);
	{
		int32_t x = ImGui::GetCurrentWindow()->Pos.x + 4.5f;
		int32_t y = ImGui::GetCurrentWindow()->Pos.y;

		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + 181, y + 22), ImColor(36 / 255.0f, 37 / 255.0f, 41.0f / 255.0f, 1.0f));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y + 22), ImVec2(x + 181, y + 22.27f), ImColor(0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 1.0f));
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y + 22.27f), ImVec2(x + 181, y + 22.30f), ImColor(0.0f, 115.0f / 255.0f, 222.0f / 255.0f, 0.65f));

		//ImGui::PushFont( g_sdk.fonts.m_UserIcon );
		//ImGui::GetOverlayDrawList( )->AddText( ImVec2( x + 5, y + 2 ), ImColor( 71, 163, 255 ), _S( "a" ) );
		//ImGui::PopFont( );

		ImGui::PushFont(g_sdk.fonts.m_LogFont);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 65, y + 2), ImColor(255, 255, 255), _S("Spectators"));
		ImGui::PopFont();

		int m_Last = 0;
		for (auto& Spectator : vecSpectatorList)
		{
			ImGui::PushFont(g_sdk.fonts.m_LogFont);
			ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 2, 23 + (y + 16 * m_Last)), ImColor(255, 255, 255, 255), Spectator.c_str());
			ImGui::PopFont();

			m_Last++;
		}
	}
	ImGui::DefaultEnd();
}