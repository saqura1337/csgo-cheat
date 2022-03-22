#include "PacketManager.hpp"
#include "../Prediction/EnginePrediction.hpp"
#include "../Animations/LocalAnimations.hpp"
#include "../RageBot/Antiaim.hpp"
#include "../RageBot/RageBot.hpp"
#include "../Settings.hpp"
#include "../BuyBot/BuyBot.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Features/Movement/AutoPeek.hpp"
#include "../Networking/Networking.hpp"
#include "../Exploits/Exploits.hpp"

bool C_PacketManager::SetupPacket( int32_t iSequence, bool* pbSendPacket )
{
	this->m_pUserCmd = g_interfaces.input->GetUserCmd( iSequence );
	if ( !this->m_pUserCmd || !this->m_pUserCmd->m_nCommand )
		return false;

	if ( !g_sdk.local->IsAlive( ) )
		return false;

	this->m_pbSendPacket = pbSendPacket;
	return true;
}

void C_PacketManager::FinishPacket( int32_t iSequence )
{
	C_VerifiedUserCmd* pVerifiedCommand = g_interfaces.input->GetVerifiedCmd( iSequence );
	if ( !pVerifiedCommand )
		return g_PredictionSystem->ResetPacket( );

	g_sdk.packet.m_bInCreateMove = false;

	//if ( g_interfaces.client_state->m_nChokedCommands( ) > 13 )
	//	*m_pbSendPacket = true;

	g_AutoPeek->Instance( );
	g_RageBot->RestorePlayers( );

	if (g_cfg->m_bAntiUntrusted || (*g_interfaces.game_rules)->IsValveDS())
	{
		Math::Normalize3(m_pUserCmd->m_angViewAngles);
		Math::ClampAngles(m_pUserCmd->m_angViewAngles);
	}

	Math::FixMovement(m_pUserCmd);
	/*
	if (*m_pbSendPacket && g_interfaces.client_state->m_pNetChannel())
	{
		auto choked_packets = g_interfaces.client_state->m_pNetChannel()->m_iChokedCommands;

		if (choked_packets >= 0)
		{
			auto ticks_allowed = g_ExploitSystem->GetShiftAmount();
			auto command_number = m_pUserCmd->m_nCommand - choked_packets;

			do
			{
				auto command = &g_interfaces.input->m_commands[m_pUserCmd->m_nCommand - MULTIPLAYER_BACKUP * (command_number / MULTIPLAYER_BACKUP) - choked_packets];

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

	if (!*m_pbSendPacket && !(*g_interfaces.game_rules)->IsValveDS())
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
	}*/

	pVerifiedCommand->m_Cmd = *m_pUserCmd;
	pVerifiedCommand->m_CRC = m_pUserCmd->GetChecksum();

	// process the net
	if (*m_pbSendPacket)
		m_aCommandList.emplace_back(m_pUserCmd->m_nCommand);

	g_BuyBot->OnCreateMove();

	// анимируем локал плеера
	g_LocalAnimations->Instance( );

	// резетаем данные из предикшна
	return g_PredictionSystem->ResetPacket( );
}

void C_PacketManager::FinishNetwork( )
{
	
}

bool C_PacketManager::ShouldProcessPacketStart( int32_t iCommand )
{
	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
		return true;

	for ( auto m_Cmd = m_aCommandList.begin( ); m_Cmd != m_aCommandList.end( ); m_Cmd++ )
	{
		if ( *m_Cmd != iCommand )
			continue;
	
		m_aCommandList.erase( m_Cmd );
		return true;
	}

	return false;
}

C_UserCmd* C_PacketManager::GetModifableCommand( )
{
	return this->m_pUserCmd;
}

bool& C_PacketManager::GetModifablePacket( )
{
	return *this->m_pbSendPacket;
}