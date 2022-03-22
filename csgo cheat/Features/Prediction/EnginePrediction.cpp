#include "EnginePrediction.hpp"
#include "../Animations/LocalAnimations.hpp"
#include "../Packet/PacketManager.hpp"

void C_PredictionSystem::Instance( )
{
	m_flOldCurtime = g_interfaces.globals->m_curtime;
	m_flOldFrametime = g_interfaces.globals->m_frametime;
	
	m_bInPrediction_Backup = g_interfaces.prediction->m_bInPrediction( );
	m_bIsFirstTimePredicted_Backup = g_interfaces.prediction->m_bIsFirstTimePredicted( );
	
	if (!m_LastPredictedCmd || m_LastPredictedCmd->m_bHasBeenPredicted)
		m_nTickBase++;
	else
		m_nTickBase = g_sdk.local->m_nTickBase();

	g_interfaces.prediction->m_bInPrediction( ) = true;
	g_interfaces.prediction->m_bIsFirstTimePredicted( ) = false;

	return g_PredictionSystem->Repredict( );
}

void C_PredictionSystem::Repredict()
{
	g_interfaces.globals->m_curtime = m_nTickBase * g_interfaces.globals->m_intervalpertick;
	if (!(g_sdk.local->m_fFlags() & FL_FROZEN))
		g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;

	float_t flRecoilIndex = 0.0f;
	float_t flAccuracyPenalty = 0.0f;
	float_t flInaccuracy = 0.0f;  
	float_t flSpread = 0.0f;

	Vector vecPreviousOrigin = g_sdk.local->GetAbsOrigin();
	if (g_sdk.local->m_hActiveWeapon().Get())
	{
		flRecoilIndex = g_sdk.local->m_hActiveWeapon().Get()->m_flRecoilIndex();
		flAccuracyPenalty = g_sdk.local->m_hActiveWeapon().Get()->m_fAccuracyPenalty();
		flInaccuracy = g_sdk.local->m_hActiveWeapon().Get()->GetInaccuracy();
		flSpread = g_sdk.local->m_hActiveWeapon().Get()->GetSpread();
	}

	// correct player data
	g_sdk.local->m_vecAbsVelocity() = g_sdk.local->m_vecVelocity();
	g_sdk.local->SetAbsoluteOrigin(g_sdk.local->m_vecOrigin());

	*(C_UserCmd**)((uintptr_t)(g_sdk.local) + 0x3348) = g_PacketManager->GetModifableCommand();
	(**((C_BasePlayer***)(g_sdk.address_list.m_PredictionPlayer))) = g_sdk.local;
	(*(*(int32_t**)(g_sdk.address_list.m_PredictionSeed))) = g_PacketManager->GetModifableCommand()->m_iRandomSeed;
	
	//C_MoveData move_data;
	//memset(&move_data, 0, sizeof(C_MoveData));

	// correct move data.
	//m_MoveData.m_flForwardMove = g_PacketManager->GetModifableCommand()->m_flForwardMove;
	//m_MoveData.m_flSideMove = g_PacketManager->GetModifableCommand()->m_flSideMove;
	//m_MoveData.m_flUpMove = g_PacketManager->GetModifableCommand()->m_flUpMove;
	//m_MoveData.m_nButtons = g_PacketManager->GetModifableCommand()->m_nButtons;
	//m_MoveData.m_vecViewAngles = Vector(
	//	g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch,
	//	g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw,
	//	g_PacketManager->GetModifableCommand()->m_angViewAngles.roll
	//);
	//m_MoveData.m_vecAngles = Vector(
	//	g_PacketManager->GetModifableCommand()->m_angViewAngles.pitch,
	//	g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw,
	//	g_PacketManager->GetModifableCommand()->m_angViewAngles.roll
	//);
	//m_MoveData.m_nImpulseCommand = g_PacketManager->GetModifableCommand()->m_nImpulse;
	
	// start of prediction
	g_interfaces.game_movement->StartTrackPredictionErrors(g_sdk.local);
	
	//g_PacketManager->GetModifableCommand()->m_nButtons |= *reinterpret_cast<uint32_t*>(uint32_t(g_sdk.local) + 0x3330);
	//m_MoveData.m_nButtons = g_PacketManager->GetModifableCommand()->m_nButtons;
	//int buttonsChanged = g_PacketManager->GetModifableCommand()->m_nButtons ^ *reinterpret_cast<int*>(uint32_t(g_sdk.local) + 0x3208);
	//*reinterpret_cast<int*>(uint32_t(g_sdk.local) + 0x31FC) = (uint32_t(g_sdk.local) + 0x3208);
	//*reinterpret_cast<int*>(uint32_t(g_sdk.local) + 0x3208) = g_PacketManager->GetModifableCommand()->m_nButtons;
	//*reinterpret_cast<int*>(uint32_t(g_sdk.local) + 0x3200) = g_PacketManager->GetModifableCommand()->m_nButtons & buttonsChanged;
	//*reinterpret_cast<int*>(uint32_t(g_sdk.local) + 0x3204) = buttonsChanged & ~g_PacketManager->GetModifableCommand()->m_nButtons;

	//if (auto iNextThinkTick = g_sdk.local->m_nNextThinkTick(); iNextThinkTick > 0 && iNextThinkTick <= m_nTickBase)
	//{
	//	iNextThinkTick = TICK_NEVER_THINK;
	//	g_sdk.local->Think();
	//}

	g_interfaces.move_helper->SetHost(g_sdk.local);
	
	// start movement
	g_interfaces.prediction->SetupMove(g_sdk.local, g_PacketManager->GetModifableCommand(), g_interfaces.move_helper, &m_MoveData);
	g_interfaces.prediction->CheckMovingGround(g_sdk.local, g_interfaces.globals->m_frametime);
	g_interfaces.prediction->SetLocalViewAngles(g_PacketManager->GetModifableCommand()->m_angViewAngles);

	//m_MoveData = move_data;
	
	// run movement
	g_interfaces.game_movement->ProcessMovement(g_sdk.local, &m_MoveData);
	
	// finish movement
	g_interfaces.prediction->FinishMove(g_sdk.local, g_PacketManager->GetModifableCommand(), &m_MoveData);
	
	// end of prediction
	g_interfaces.move_helper->SetHost(nullptr);
	g_interfaces.game_movement->FinishTrackPredictionErrors(g_sdk.local);

	// update weapon accuracy
	//g_sdk.local->m_hActiveWeapon().Get()->UpdateAccuracyPenalty();
	
	// restore player
	*(C_UserCmd**)((uintptr_t)(g_sdk.local) + 0x3348) = NULL;
	(**((C_BasePlayer***)(g_sdk.address_list.m_PredictionPlayer))) = NULL;
	(*(*(int32_t**)(g_sdk.address_list.m_PredictionSeed))) = -1;

	m_LastPredictedCmd = g_PacketManager->GetModifableCommand();
	
	// update weapon accuracy
	if (g_sdk.local->m_hActiveWeapon().Get())
	{
		g_sdk.local->m_hActiveWeapon().Get()->m_flRecoilIndex() = flRecoilIndex;
		g_sdk.local->m_hActiveWeapon().Get()->m_fAccuracyPenalty() = flAccuracyPenalty;
		g_sdk.local->m_hActiveWeapon().Get()->UpdateAccuracyPenalty();
	}
	//else if (!g_sdk.local->m_hActiveWeapon().Get()) 
	//	flSpread = flInaccuracy = 0.f;

	g_LocalAnimations->SetupShootPosition();
	return g_sdk.local->SetAbsoluteOrigin(vecPreviousOrigin);
}

void C_PredictionSystem::ResetData( )
{
	m_aNetvarData = { };
	
	m_flOldCurtime = 0.0f;
	m_flOldFrametime = 0.0f;
	m_flVelocityModifier = 1.0f;
	m_iLastCommand = -1;
	m_bInPrediction_Backup = false;
	m_bIsFirstTimePredicted_Backup = false;
	m_LastPredictedCmd = NULL;
	m_MoveData = C_MoveData( );
	m_nTickBase = 0;
}

void C_PredictionSystem::SaveVelocityModifier( )
{
	m_flVelocityModifier = g_sdk.local->m_flVelocityModifier( );
}

void C_PredictionSystem::SaveCommand( int32_t nCommand )
{
	m_iLastCommand = nCommand;
}

float_t C_PredictionSystem::GetVelocityModifier( int32_t nCommand )
{
	float_t flVelocityModifier = m_flVelocityModifier + ( min( ( ( nCommand - 1 ) - m_iLastCommand ), 1 ) * ( g_interfaces.globals->m_intervalpertick * 0.4f ) );
	if ( flVelocityModifier > 1.0f )
		return 1.0f;

	if ( !( g_sdk.local->m_fFlags( ) & FL_ONGROUND ) )
		return g_sdk.local->m_flVelocityModifier( );

	return flVelocityModifier;
}

void C_PredictionSystem::SaveNetvars( int32_t nCommand )
{
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_fFlags						= g_sdk.local->m_fFlags( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_hGroundEntity				= g_sdk.local->m_hGroundEntity( ).Get( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flDuckAmount				= g_sdk.local->m_flDuckAmount( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flDuckSpeed				= g_sdk.local->m_flDuckSpeed( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecOrigin					= g_sdk.local->m_vecOrigin( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecVelocity				= g_sdk.local->m_vecVelocity( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecBaseVelocity			= g_sdk.local->m_vecBaseVelocity( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flFallVelocity				= g_sdk.local->m_flFallVelocity( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecViewOffset				= g_sdk.local->m_vecViewOffset( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_angAimPunchAngle			= g_sdk.local->m_aimPunchAngle( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecAimPunchAngleVel		= g_sdk.local->m_aimPunchAngleVel( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_angViewPunchAngle			= g_sdk.local->m_viewPunchAngle( );

    C_BaseCombatWeapon* pWeapon = g_sdk.local->m_hActiveWeapon( ).Get( );
    if ( !pWeapon )
		return;

	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flRecoilIndex = pWeapon->m_flRecoilIndex( );
	m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flAccuracyPenalty = pWeapon->m_fAccuracyPenalty( );
}

void C_PredictionSystem::RestoreNetvars( int32_t nCommand )
{
	g_sdk.local->m_fFlags( )							= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_fFlags;
    g_sdk.local->m_hGroundEntity( )						= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_hGroundEntity.Get( );
    g_sdk.local->m_flDuckAmount( )						= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flDuckAmount;
    g_sdk.local->m_flDuckSpeed( )						= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flDuckSpeed;
    g_sdk.local->m_vecOrigin( )							= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecOrigin;
    g_sdk.local->m_vecVelocity( )						= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecVelocity;
    g_sdk.local->m_vecBaseVelocity( )					= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecBaseVelocity;
    g_sdk.local->m_flFallVelocity( )					= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flFallVelocity;
    g_sdk.local->m_vecViewOffset( )						= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecViewOffset;
    g_sdk.local->m_aimPunchAngle( )						= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_angAimPunchAngle;
    g_sdk.local->m_aimPunchAngleVel( )					= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_vecAimPunchAngleVel;
    g_sdk.local->m_viewPunchAngle( )					= m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_angViewPunchAngle;

    C_BaseCombatWeapon* pWeapon = g_sdk.local->m_hActiveWeapon( ).Get( );
    if ( !pWeapon )
        return;
    
    pWeapon->m_flRecoilIndex( ) = m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flRecoilIndex;
    pWeapon->m_fAccuracyPenalty( ) = m_aNetvarData[ nCommand % MULTIPLAYER_BACKUP ].m_flAccuracyPenalty;
}

void C_PredictionSystem::ResetPacket( )
{
	g_interfaces.globals->m_curtime = m_flOldCurtime;
	g_interfaces.globals->m_frametime = m_flOldFrametime;

	g_interfaces.prediction->m_bInPrediction( ) = m_bInPrediction_Backup;
	g_interfaces.prediction->m_bIsFirstTimePredicted( ) = m_bIsFirstTimePredicted_Backup;
}

void C_PredictionSystem::UpdatePacket( )
{
	if ( g_interfaces.client_state->m_nDeltaTick( ) < 0 )
		return;

	return g_interfaces.prediction->Update( g_interfaces.client_state->m_nDeltaTick( ), g_interfaces.client_state->m_nDeltaTick( ) > 0, g_interfaces.client_state->m_nLastCommandAck( ), g_interfaces.client_state->m_nChokedCommands( ) + g_interfaces.client_state->m_nLastOutgoingCommand( ) );
}

void C_PredictionSystem::SaveViewmodelData( )
{
	C_BaseViewModel* hViewmodel = g_sdk.local->m_hViewModel( ).Get( );
	if ( !hViewmodel )
		return;

	m_iAnimationParity = hViewmodel->m_iAnimationParity( );
	m_iSequence = hViewmodel->m_iSequence( );
	m_flCycle = hViewmodel->m_flCycle( );
	m_flAnimTime = hViewmodel->m_flAnimTime( );
}

void C_PredictionSystem::AdjustViewmodelData( )
{
	C_BaseViewModel* hViewmodel = g_sdk.local->m_hViewModel( ).Get( );
	if ( !hViewmodel )
		return;

	if ( m_iSequence != hViewmodel->m_iSequence( ) || m_iAnimationParity != hViewmodel->m_iAnimationParity( ) )
		return;

	hViewmodel->m_flCycle( ) = m_flCycle;
	hViewmodel->m_flAnimTime( ) = m_flAnimTime;
}