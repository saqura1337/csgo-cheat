#include "../Hooks.hpp"
#include "../Features/Exploits/Exploits.hpp"

bool __fastcall C_Hooks::hkInPrediction(LPVOID pEcx, uint32_t)
{
	if (reinterpret_cast <DWORD>(_ReturnAddress()) == reinterpret_cast <DWORD>(g_sdk.address_list.m_InPrediction_Call))
		return false;

	return g_sdk.hooks.originals.m_InPrediction(pEcx);
}

void __fastcall C_Hooks::hkRunCommand(void* ecx, void* edx, C_BasePlayer* player, C_UserCmd* m_pcmd, C_MoveHelper* move_helper)
{
	if (!player || player != g_sdk.local)
		return g_sdk.hooks.originals.runcommand(ecx, player, m_pcmd, move_helper);

	if (m_pcmd->m_nTickCount > g_interfaces.globals->m_tickcount + 72)
	{
		m_pcmd->m_bHasBeenPredicted = true;
		if (!g_interfaces.engine->IsPaused())
			player->m_nTickBase()++;

		return;
	}

	return g_sdk.hooks.originals.runcommand(ecx, player, m_pcmd, move_helper);
}