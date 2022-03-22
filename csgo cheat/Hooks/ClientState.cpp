#include "../Hooks.hpp"
#include "../Features/Packet/PacketManager.hpp"
#include "../Features/Networking/Networking.hpp"

void __fastcall C_Hooks::hkPacketStart( LPVOID pEcx, uint32_t, int32_t iSequence, int32_t iCommand )
{
	if ( g_PacketManager->ShouldProcessPacketStart( iCommand ) )
		return g_sdk.hooks.originals.m_PacketStart( pEcx, iSequence, iCommand );
}

void __fastcall C_Hooks::hkPacketEnd( LPVOID pEcx, uint32_t )
{
	g_Networking->OnPacketEnd( ( C_ClientState* )( pEcx ) );
	return g_sdk.hooks.originals.m_PacketEnd( pEcx );
}