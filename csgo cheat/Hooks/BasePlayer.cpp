#include "../Hooks.hpp"
#include "../Settings.hpp"
#include "../Features/Animations/Animations.hpp"
#include "../Features/Animations/LocalAnimations.hpp"
#include "../Features/Animations/BoneManager.hpp"
#include "../Features/Networking/Networking.hpp"
#include "../Features/Exploits/Exploits.hpp"
#include "../Features/RageBot/RageBot.hpp"

void __fastcall C_Hooks::hkDoExtraBoneProcessing( LPVOID pEcx, uint32_t, C_StudioHDR* pHdr, Vector* vecVector, Quaternion* pSomething, matrix3x4_t* aMatrix, LPVOID aList, LPVOID pContext )
{
	return;
}

void __fastcall C_Hooks::hkStandardBlendingRules( LPVOID pEcx, uint32_t, C_StudioHDR* pStudioHDR, Vector* vecPosition, Quaternion* quatern, float_t flTime, int32_t iBoneMask )
{
	C_BasePlayer* pPlayer = ( C_BasePlayer* )( pEcx );
	if ( !pPlayer || !pPlayer->IsPlayer( ) || pPlayer->EntIndex( ) - 1 > 63 || !pPlayer->IsAlive( ) || !g_sdk.local )
		return g_sdk.hooks.originals.m_StandardBlendingRules( pEcx, pStudioHDR, vecPosition, quatern, flTime, iBoneMask );

	if ( pPlayer->m_iTeamNum( ) == g_sdk.local->m_iTeamNum( ) )
		if ( pPlayer != g_sdk.local )
			return g_sdk.hooks.originals.m_StandardBlendingRules( pEcx, pStudioHDR, vecPosition, quatern, flTime, iBoneMask );

	iBoneMask |= BONE_USED_BY_HITBOX;
	return g_sdk.hooks.originals.m_StandardBlendingRules( pEcx, pStudioHDR, vecPosition, quatern, flTime, iBoneMask );
}

void __fastcall C_Hooks::hkCalcViewmodelBob( LPVOID pEcx, uint32_t, Vector& vecViewBob )
{
	if ( pEcx != g_sdk.local || !g_cfg->m_aWorldRemovals[ REMOVALS_VISUAL_LANDING_BOB ] ) 
		return g_sdk.hooks.originals.m_CalcViewmodelBob( pEcx, vecViewBob );
}

void __fastcall C_Hooks::hkUpdateClientSideAnimation( LPVOID pEcx, uint32_t )
{
	C_BasePlayer* pPlayer = ( C_BasePlayer* )( pEcx );
	if ( !pPlayer || !pPlayer->IsPlayer( ) || pPlayer->EntIndex( ) - 1 > 63 || !pPlayer->IsAlive( ) || !g_sdk.local )
		return g_sdk.hooks.originals.m_UpdateClientSideAnimation( pEcx );

	if ( pPlayer->m_iTeamNum( ) == g_sdk.local->m_iTeamNum( ) )
	{
		if ( pPlayer != g_sdk.local )
			return g_sdk.hooks.originals.m_UpdateClientSideAnimation( pEcx );
	}
	else
		pPlayer->SetAbsoluteOrigin( pPlayer->m_vecOrigin( ) );

	if ( !g_sdk.animation_data.m_bAnimationUpdate )
	{
		if ( pPlayer == g_sdk.local )
			return g_LocalAnimations->OnUpdateClientSideAnimation( );

		return g_AnimationSync->OnUpdateClientSideAnimation( ( C_BasePlayer* )( pEcx ) );
	}
		
	return g_sdk.hooks.originals.m_UpdateClientSideAnimation( pEcx );
}

bool __fastcall C_Hooks::hkSetupBones( LPVOID pEcx, uint32_t, matrix3x4_t* aMatrix, int32_t iMaxBones, int32_t iBoneMask, float_t flCurrentTime )
{
	C_BasePlayer* pPlayer = ( C_BasePlayer* )( ( uintptr_t )( pEcx ) - 0x4 );
	if ( !pPlayer || !pPlayer->IsPlayer( ) || pPlayer->EntIndex( ) - 1 > 63 || !pPlayer->IsAlive( ) || !g_sdk.local )
		return g_sdk.hooks.originals.m_SetupBones( pEcx, aMatrix, iMaxBones, iBoneMask, flCurrentTime );

	if ( pPlayer->m_iTeamNum( ) == g_sdk.local->m_iTeamNum( ) )
		if ( pPlayer != g_sdk.local )
			return g_sdk.hooks.originals.m_SetupBones( pEcx, aMatrix, iMaxBones, iBoneMask, flCurrentTime );

	if ( g_sdk.animation_data.m_bSetupBones )
		return g_sdk.hooks.originals.m_SetupBones( pEcx, aMatrix, iMaxBones, iBoneMask, flCurrentTime );
	else if ( aMatrix )
	{
		if ( pPlayer == g_sdk.local )
			return g_LocalAnimations->GetCachedMatrix( aMatrix );
		else
			return g_AnimationSync->GetCachedMatrix( pPlayer, aMatrix );
	}

	return true;
}

int __fastcall C_Hooks::hkSetupVelocity(LPVOID pEcx, uint32_t)
{
	C_BasePlayer* pPlayer = (C_BasePlayer*)((uintptr_t)(pEcx) - 0x4);
	static auto& lagrecord = g_LagCompensation->GetPlayerRecords(pPlayer->EntIndex());

	if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->EntIndex() - 1 > 63 || !pPlayer->IsAlive() || !g_sdk.local || !lagrecord.empty())
		return g_sdk.hooks.originals.setup_velocity(pEcx);

	auto& record = lagrecord.front();

	if (!record.m_bAnimResolved)
		return g_sdk.hooks.originals.setup_velocity(pEcx);
}

void __fastcall C_Hooks::hkPhysicsSimulate( LPVOID pEcx, uint32_t )
{
	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) || pEcx != g_sdk.local )
		return g_sdk.hooks.originals.m_PhysicsSimulate( pEcx );

	int32_t iSimulationTick = *( int32_t* )( ( uintptr_t )( pEcx ) + 0x2AC );
	if ( iSimulationTick == g_interfaces.globals->m_tickcount )
		return;

	C_CommandContext* pCommandContext = reinterpret_cast < C_CommandContext* >( ( uintptr_t )( g_sdk.local ) + 0x350C );
	if ( !pCommandContext || !pCommandContext->m_bNeedsProcessing )
		return;

	g_sdk.local->m_nTickBase( ) = g_ExploitSystem->GetNetworkTickbase( pCommandContext->m_nCommandNumber );

	g_sdk.hooks.originals.m_PhysicsSimulate( pEcx );
	
	// store viewmodel
	g_PredictionSystem->SaveViewmodelData( );
	
	// save netvar data
	return g_Networking->SaveNetvarData( pCommandContext->m_nCommandNumber );
}

void __fastcall C_Hooks::hkCalcView( LPVOID pEcx, uint32_t, Vector& vecEyeOrigin, QAngle& angEyeAngles, float_t& zNear, float_t& zFar, float_t& flFov )
{
	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) || pEcx != g_sdk.local )
		return g_sdk.hooks.originals.m_CalcView( pEcx, vecEyeOrigin, angEyeAngles, zNear, zFar, flFov );

	QAngle angAimPunchAngle = g_sdk.local->m_aimPunchAngle( );
	QAngle angViewPunchAngle = g_sdk.local->m_viewPunchAngle( );

	if ( g_cfg->m_aWorldRemovals[ REMOVALS_VISUAL_PUNCH ] )
		g_sdk.local->m_aimPunchAngle( ) = QAngle( 0, 0, 0 );

	if ( g_cfg->m_aWorldRemovals[ REMOVALS_VISUAL_KICK ] )
		g_sdk.local->m_viewPunchAngle( ) = QAngle( 0, 0, 0 );
		 
	g_sdk.hooks.originals.m_CalcView( pEcx, vecEyeOrigin, angEyeAngles, zNear, zFar, flFov );
	
	if ( g_cfg->m_aWorldRemovals[ REMOVALS_VISUAL_PUNCH ] )
		g_sdk.local->m_aimPunchAngle( ) = angAimPunchAngle;

	if ( g_cfg->m_aWorldRemovals[ REMOVALS_VISUAL_KICK ] )
		g_sdk.local->m_viewPunchAngle( ) = angViewPunchAngle;

	if ( g_sdk.packet.m_bVisualFakeDuck )
		vecEyeOrigin.z = g_sdk.local->GetAbsOrigin( ).z + g_interfaces.game_movement->GetPlayerViewOffset( false ).z;
}