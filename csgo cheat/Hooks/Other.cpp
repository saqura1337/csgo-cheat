#include "../Hooks.hpp"
#include "../Features/Packet/PacketManager.hpp"
#include "../Features/Model/Model.hpp"
#include "../Features/Exploits/Exploits.hpp"
#include "../Features/Networking/Networking.hpp"
#include "../Tools/Tools.hpp"

void __cdecl C_Hooks::hkCL_Move(float_t flFrametime, bool bIsFinalTick)
{
	g_Networking->UpdateLatency();
	g_Networking->StartNetwork();

	if (!g_ExploitSystem->IsAllowedToRun())
		return;

	g_sdk.hooks.originals.m_CL_Move(flFrametime, g_ExploitSystem->IsFinalTick());

	g_ExploitSystem->PerformPackets();
	return g_Networking->FinishNetwork();
}

bool __fastcall C_Hooks::hkDispatchUserMessage(LPVOID pEcx, uint32_t, int32_t iMessageType, int32_t iArgument, int32_t iSecondArgument, LPVOID pData)
{
	if (iMessageType == CS_UM_TextMsg || iMessageType == CS_UM_HudMsg || iMessageType == CS_UM_SayText)
		if (g_cfg->m_bAdBlock && !(*(g_interfaces.game_rules))->IsValveDS())
			return true;

	return g_sdk.hooks.originals.m_DispatchUserMessage(pEcx, iMessageType, iArgument, iSecondArgument, pData);
}

void __fastcall C_Hooks::hkPerformScreenOverlay(LPVOID pEcx, uint32_t, int32_t x, int32_t y, int32_t iWidth, int32_t iHeight)
{
	if (!g_cfg->m_bAdBlock || (*(g_interfaces.game_rules))->IsValveDS())
		return g_sdk.hooks.originals.m_PerformScreenOverlay(pEcx, x, y, iWidth, iHeight);
}

void __cdecl C_Hooks::hkShouldDrawFOG()
{
	if (!g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_FOG])
		return g_sdk.hooks.originals.m_ShouldDrawFog();
}

void C_Hooks::hkFlashDuration(const CRecvProxyData* Data, LPVOID pStruct, LPVOID pOut)
{
	if (g_cfg->m_aWorldRemovals[REMOVALS_VISUAL_FLASH])
	{
		*(float_t*)(pOut) = 0.0f;
		return;
	}

	return g_sdk.hooks.originals.m_FlashDuration->GetOriginal()(Data, pStruct, pOut);
}

bool __cdecl C_Hooks::hkHost_ShouldRun()
{
	return true;
}