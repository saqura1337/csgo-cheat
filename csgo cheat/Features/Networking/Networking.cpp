#include "Networking.hpp"
#include "../Exploits/Exploits.hpp"
#include "../Log Manager/LogManager.hpp"
#include "../Packet/PacketManager.hpp"

inline bool IsVectorValid( Vector vecOriginal, Vector vecCurrent )
{
	Vector vecDelta = vecOriginal - vecCurrent;
	for ( int i = 0; i < 3; i++ )
	{
		if ( fabsf( vecDelta[ i ] ) > 0.03125f )
			return false;
	}

	return true;
}

inline bool IsAngleValid( QAngle vecOriginal, QAngle vecCurrent )
{
	QAngle vecDelta = vecOriginal - vecCurrent;
	for ( int i = 0; i < 3; i++ )
	{
		if ( fabsf( vecDelta[ i ] ) > 0.03125f )
			return false;
	}

	return true;
}

inline bool IsFloatValid( float_t flOriginal, float_t flCurrent )
{
	if ( fabsf( flOriginal - flCurrent ) > 0.03125f )
		return false;

	return true;
}

void C_Networking::OnPacketEnd( C_ClientState* m_ClientState )
{
	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
		return;

	if ( *( int32_t* )( ( DWORD )( m_ClientState ) + 0x16C ) != *( int32_t* )( ( DWORD )( m_ClientState ) + 0x164 ) )
		return;

	return this->RestoreNetvarData( g_interfaces.client_state->m_nLastCommandAck( ) );
}

void C_Networking::StartNetwork( )
{
	if ( !g_interfaces.client_state || !g_interfaces.client_state->m_pNetChannel( ) )
		return;

	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
		return;

	m_Sequence = g_interfaces.client_state->m_pNetChannel( )->m_iOutSequenceNr;
}

void C_Networking::FinishNetwork( )
{
	if ( !g_interfaces.client_state || !g_interfaces.client_state->m_pNetChannel( ) )
		return;

	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
		return;

	if (g_PacketManager->GetModifablePacket())
	{
		auto choked_packets = g_interfaces.client_state->m_pNetChannel()->m_iChokedCommands;

		if (choked_packets >= 0)
		{
			auto ticks_allowed = g_ExploitSystem->GetShiftAmount();
			auto command_number = g_PacketManager->GetModifableCommand()->m_nCommand - choked_packets;

			do
			{
				auto command = &g_interfaces.input->m_commands[g_PacketManager->GetModifableCommand()->m_nCommand - MULTIPLAYER_BACKUP * (command_number / MULTIPLAYER_BACKUP) - choked_packets];

				if (!command || command->m_nTickCount > g_interfaces.globals->m_tickcount + 72)
				{
					if (--ticks_allowed < 0)
						ticks_allowed = 0;

					g_ExploitSystem->SetShiftAmount(ticks_allowed);
				}

				++command_number;
				--choked_packets;
			} while (choked_packets >= 0);
		}
	}

	if (!g_PacketManager->GetModifablePacket() && !(*g_interfaces.game_rules)->IsValveDS())
	{
		auto net_channel = g_interfaces.client_state->m_pNetChannel();

		if (net_channel->m_iChokedCommands > 0 && !(net_channel->m_iChokedCommands % 4))
		{
			auto backup_choke = net_channel->m_iChokedCommands;
			net_channel->m_iChokedCommands = 0;

			net_channel->SendDatagram(NULL);
			--net_channel->m_iOutSequenceNr;

			net_channel->m_iChokedCommands = backup_choke;
		}
	}
}

void C_Networking::ProcessInterpolation( ClientFrameStage_t Stage, bool bPostFrame )
{
	if ( Stage != ClientFrameStage_t::FRAME_RENDER_START )
		return;

	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
		return;

	if ( !bPostFrame )
	{
		m_FinalPredictedTick = g_sdk.local->m_nFinalPredictedTick( );
		m_flInterp = g_interfaces.globals->m_interpolation_amount;

		g_sdk.local->m_nFinalPredictedTick( ) = g_sdk.local->m_nTickBase( );
		if ( g_ExploitSystem->CanSkipInterpolation( ) )
			g_interfaces.globals->m_interpolation_amount = 0.0f;
		
		return;
	}

	g_sdk.local->m_nFinalPredictedTick( ) = m_FinalPredictedTick;
	g_interfaces.globals->m_interpolation_amount = 0.0f;
}

void C_Networking::SaveNetvarData( int nCommand )
{
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_nTickbase				= g_sdk.local->m_nTickBase( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flDuckAmount				= g_sdk.local->m_flDuckAmount( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flDuckSpeed				= g_sdk.local->m_flDuckSpeed( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_vecOrigin				= g_sdk.local->m_vecOrigin( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_vecVelocity				= g_sdk.local->m_vecVelocity( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_vecBaseVelocity			= g_sdk.local->m_vecBaseVelocity( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flFallVelocity			= g_sdk.local->m_flFallVelocity( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_vecViewOffset			= g_sdk.local->m_vecViewOffset( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_angAimPunchAngle			= g_sdk.local->m_aimPunchAngle( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_vecAimPunchAngleVel		= g_sdk.local->m_aimPunchAngleVel( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_angViewPunchAngle		= g_sdk.local->m_viewPunchAngle( );

    C_BaseCombatWeapon* pWeapon = g_sdk.local->m_hActiveWeapon( ).Get( );
    if ( !pWeapon )
	{
		m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flRecoilIndex = 0.0f;
		m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flAccuracyPenalty = 0.0f;
		m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flPostponeFireReadyTime = 0.0f;

		return;
	}

	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flRecoilIndex = pWeapon->m_flRecoilIndex( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flAccuracyPenalty = pWeapon->m_fAccuracyPenalty( );
	m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ].m_flPostponeFireReadyTime = pWeapon->m_flPostponeFireReadyTime( );
}

void C_Networking::RestoreNetvarData( int nCommand )
{
	volatile auto aNetVars = &m_aCompressData[ nCommand % MULTIPLAYER_BACKUP ];
	if ( aNetVars->m_nTickbase != g_sdk.local->m_nTickBase( ) )
		return;

	if ( IsVectorValid( aNetVars->m_vecVelocity, g_sdk.local->m_vecVelocity( ) ) )
		g_sdk.local->m_vecVelocity( )							=	aNetVars->m_vecVelocity;

	if ( IsVectorValid( aNetVars->m_vecBaseVelocity, g_sdk.local->m_vecBaseVelocity( ) ) )
		g_sdk.local->m_vecBaseVelocity( )						=	aNetVars->m_vecBaseVelocity;
	
	if ( IsVectorValid( aNetVars->m_vecViewOffset, g_sdk.local->m_vecViewOffset( ) ) )
		g_sdk.local->m_vecViewOffset( )							=	aNetVars->m_vecViewOffset;

	if ( IsAngleValid( aNetVars->m_angAimPunchAngle, g_sdk.local->m_aimPunchAngle( ) ) )
		g_sdk.local->m_aimPunchAngle( )							=	aNetVars->m_angAimPunchAngle;
		
	if ( IsVectorValid( aNetVars->m_vecAimPunchAngleVel, g_sdk.local->m_aimPunchAngleVel( ) ) )
		g_sdk.local->m_aimPunchAngleVel( )						=	aNetVars->m_vecAimPunchAngleVel;

	if ( IsAngleValid( aNetVars->m_angViewPunchAngle, g_sdk.local->m_viewPunchAngle( ) ) )
		g_sdk.local->m_viewPunchAngle( )						=	aNetVars->m_angViewPunchAngle;

	if ( IsFloatValid( aNetVars->m_flFallVelocity, g_sdk.local->m_flFallVelocity( ) ) )
		g_sdk.local->m_flFallVelocity( )						=	aNetVars->m_flFallVelocity;

	if ( IsFloatValid( aNetVars->m_flDuckAmount, g_sdk.local->m_flDuckAmount( ) ) )
		g_sdk.local->m_flDuckAmount( )							=	aNetVars->m_flDuckAmount;

	if ( IsFloatValid( aNetVars->m_flDuckSpeed, g_sdk.local->m_flDuckSpeed( ) ) )
		g_sdk.local->m_flDuckSpeed( )							=	aNetVars->m_flDuckSpeed;

	if ( g_sdk.local->m_hActiveWeapon( ).Get( ) )
	{
		if ( IsFloatValid( aNetVars->m_flAccuracyPenalty, g_sdk.local->m_hActiveWeapon( ).Get( )->m_fAccuracyPenalty( ) ) )
			g_sdk.local->m_hActiveWeapon( ).Get( )->m_fAccuracyPenalty( ) = aNetVars->m_flAccuracyPenalty;
		
		if ( IsFloatValid( aNetVars->m_flRecoilIndex, g_sdk.local->m_hActiveWeapon( ).Get( )->m_flRecoilIndex( ) ) )
			g_sdk.local->m_hActiveWeapon( ).Get( )->m_flRecoilIndex( ) = aNetVars->m_flRecoilIndex;
	}

	if ( g_sdk.local->m_vecViewOffset( ).z > 64.0f )
		g_sdk.local->m_vecViewOffset( ).z = 64.0f;
	else if ( g_sdk.local->m_vecViewOffset( ).z <= 46.05f )
		g_sdk.local->m_vecViewOffset( ).z = 46.0f;
	
	if ( g_sdk.local->m_fFlags( ) & FL_ONGROUND )
		g_sdk.local->m_flFallVelocity( ) = 0.0f;
}

void C_Networking::UpdateLatency( )
{
	C_NetChannelInfo* pNetChannelInfo = g_interfaces.engine->GetNetChannelInfo( );
	if ( !pNetChannelInfo )
		return;

	m_Latency = pNetChannelInfo->GetLatency( FLOW_OUTGOING ) + pNetChannelInfo->GetLatency( FLOW_INCOMING );
}

int32_t C_Networking::GetServerTick( )
{	
	int32_t nExtraChoke = 0;
	if ( g_sdk.packet.m_bFakeDuck )
		nExtraChoke = 14 - g_interfaces.client_state->m_nChokedCommands( );

	return g_interfaces.globals->m_tickcount + TIME_TO_TICKS( m_Latency ) + nExtraChoke - g_ExploitSystem->GetShiftAmount( );
}

int32_t C_Networking::GetTickRate( )
{
	return ( int32_t )( 1.0f / g_interfaces.globals->m_intervalpertick );
}

float_t C_Networking::GetLatency( )
{
	return m_Latency;
}

void C_Networking::ResetData( )
{
	m_Latency = 0.0f;
	m_TickRate = 0;

	m_aCompressData = { };
}