#include "Fakelag.hpp"
#include "Networking.hpp"
#include "../Packet/PacketManager.hpp"
#include "../Settings.hpp"

#include "../Animations/LocalAnimations.hpp"
#include "../Animations/LagCompensation.hpp"
#include "../RageBot/RageBot.hpp"
#include "../Exploits/Exploits.hpp"

void C_FakeLag::Instance( )
{
	if ( g_sdk.packet.m_bFakeDuck )
		return;

	int32_t iChokeAmount = -1;
	if ( g_cfg->m_bAntiAim )
		iChokeAmount = 1;

	if ( g_cfg->m_bFakeLagEnabled )
	{
		iChokeAmount = g_cfg->m_iLagLimit;
		if ( g_sdk.local->m_fFlags( ) & FL_ONGROUND )
		{
			if ( g_sdk.local->m_vecVelocity( ).Length2D( ) > 23.40f )
				if ( g_cfg->m_aFakelagTriggers[ FAKELAG_MOVE ] )
					iChokeAmount = g_cfg->m_iTriggerLimit;
		}
		else if ( g_cfg->m_aFakelagTriggers[ FAKELAG_AIR ] )
			iChokeAmount = g_cfg->m_iTriggerLimit;

		if ( m_iForcedChoke )
			iChokeAmount = m_iForcedChoke;
	}

	if ( g_interfaces.client_state->m_nChokedCommands( ) < iChokeAmount )
		g_PacketManager->GetModifablePacket( ) = false;

	if ( g_interfaces.client_state->m_nChokedCommands( ) > 13 || ( ( *g_interfaces.game_rules )->IsValveDS( ) && g_interfaces.client_state->m_nChokedCommands( ) > 5 ) ) 
		g_PacketManager->GetModifablePacket( ) = true;

	//if ( ( *g_interfaces.game_rules )->IsFreezePeriod( ) )
	//	g_PacketManager->GetModifablePacket( ) = true;

	C_BaseCombatWeapon* pCombatWeapon = g_sdk.local->m_hActiveWeapon( ).Get( );
	if ( pCombatWeapon )
	{
		if ( g_PacketManager->GetModifableCommand( )->m_nButtons & IN_ATTACK )
		{
			bool bIsRevolver = false;
			if ( pCombatWeapon )
				if ( pCombatWeapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER )
					bIsRevolver = true;

			int32_t nShiftAmount = g_ExploitSystem->GetShiftAmount( );
			if ( nShiftAmount > 9 || bIsRevolver )
				nShiftAmount = 0;

			bool bCanFire = g_sdk.local->CanFire( nShiftAmount, bIsRevolver );
			if ( bCanFire )
				g_PacketManager->GetModifablePacket( ) = true;
		}
	}

	if ( g_interfaces.client_state->m_nChokedCommands( ) > g_sdk.packet.m_MaxChoke )
		g_PacketManager->GetModifablePacket( ) = true;
	
	if ( g_PacketManager->GetModifablePacket( ) )
		m_iForcedChoke = 0;

	if ( g_cfg->m_bFakeLagEnabled )
	{
		if ( g_cfg->m_aFakelagTriggers[ FAKELAG_PEEK ] )
		{
			if ( g_sdk.peek.m_bIsPeeking )
			{
				if ( !m_iForcedChoke )
					g_PacketManager->GetModifablePacket( ) = true;

				m_iForcedChoke = g_cfg->m_iTriggerLimit;
			}
		}
	}

	if ( m_bDidForceTicksAllowed )
		return;

	return this->ForceTicksAllowedForProcessing( );
}

void C_FakeLag::ForceTicksAllowedForProcessing( )
{
	g_PacketManager->GetModifablePacket( ) = false;
	if ( g_interfaces.client_state->m_nChokedCommands( ) < 14 )
		return;

	m_bDidForceTicksAllowed = true;
}

void C_FakeLag::ResetData( )
{
	m_bDidForceTicksAllowed = false;
	m_iForcedChoke = NULL;
}