#include "BoneManager.hpp"

void C_BoneManager::BuildMatrix( C_BasePlayer* pPlayer, matrix3x4_t* aMatrix, bool bSafeMatrix )
{
	std::array < C_AnimationLayer, 13 > aAnimationLayers;

	float_t flCurTime = g_interfaces.globals->m_curtime;
	float_t flRealTime = g_interfaces.globals->m_realtime;
	float_t flFrameTime = g_interfaces.globals->m_frametime;
	float_t flAbsFrameTime = g_interfaces.globals->m_absframetime;
	int32_t iFrameCount = g_interfaces.globals->m_framecount;
	int32_t iTickCount = g_interfaces.globals->m_tickcount;
	float_t flInterpolation = g_interfaces.globals->m_interpolation_amount;

	g_interfaces.globals->m_curtime = pPlayer->m_flSimulationTime( );
	g_interfaces.globals->m_realtime = pPlayer->m_flSimulationTime( );
	g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;
	g_interfaces.globals->m_absframetime = g_interfaces.globals->m_intervalpertick;
	g_interfaces.globals->m_framecount = INT_MAX;
	g_interfaces.globals->m_tickcount = TIME_TO_TICKS( pPlayer->m_flSimulationTime( ) );
	g_interfaces.globals->m_interpolation_amount = 0.0f;

	int32_t nClientEffects = pPlayer->m_nClientEffects( );
	int32_t nLastSkipFramecount = pPlayer->m_nLastSkipFramecount( );
	int32_t nOcclusionMask = pPlayer->m_nOcclusionMask( );
	int32_t nOcclusionFrame = pPlayer->m_nOcclusionFrame( );
	int32_t iEffects = pPlayer->m_fEffects( );
	bool bJiggleBones = pPlayer->m_bJiggleBones( );
	bool bMaintainSequenceTransition = pPlayer->m_bMaintainSequenceTransition( );
	Vector vecAbsOrigin = pPlayer->GetAbsOrigin( );

	int32_t iMask = BONE_USED_BY_ANYTHING;
	if ( bSafeMatrix )
		iMask = BONE_USED_BY_HITBOX;

	//std::memcpy( aAnimationLayers.data( ), pPlayer->m_AnimationLayers( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

	pPlayer->InvalidateBoneCache( );
	pPlayer->GetBoneAccessor( ).m_ReadableBones = NULL;
	pPlayer->GetBoneAccessor( ).m_WritableBones = NULL;

	if ( pPlayer->m_PlayerAnimStateCSGO( ) )
		pPlayer->m_PlayerAnimStateCSGO( )->m_pWeaponLast = pPlayer->m_PlayerAnimStateCSGO( )->m_pWeapon;

	pPlayer->m_nOcclusionFrame( ) = 0;
	pPlayer->m_nOcclusionMask( ) = 0;
	pPlayer->m_nLastSkipFramecount( ) = 0;

	if ( pPlayer != g_sdk.local )
		pPlayer->SetAbsoluteOrigin( pPlayer->m_vecOrigin( ) );

	pPlayer->m_fEffects( ) |= EF_NOINTERP;
	pPlayer->m_nClientEffects( ) |= 2;
	pPlayer->m_bJiggleBones( ) = false;
	pPlayer->m_bMaintainSequenceTransition( ) = false;
	
	pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_LEAN ].m_flWeight = 0.0f;
	if ( bSafeMatrix )
		pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_ADJUST ].m_pOwner = NULL;
	else if ( pPlayer == g_sdk.local )
	{
		if ( pPlayer->GetSequenceActivity( pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_ADJUST ].m_nSequence ) == ACT_CSGO_IDLE_TURN_BALANCEADJUST )
		{
			pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_ADJUST ].m_flCycle = 0.0f;
			pPlayer->m_AnimationLayers( )[ ANIMATION_LAYER_ADJUST ].m_flWeight = 0.0f;
		}
	}

	g_sdk.animation_data.m_bSetupBones = true;
	pPlayer->SetupBones( aMatrix, MAXSTUDIOBONES, iMask, pPlayer->m_flSimulationTime( ) );
	g_sdk.animation_data.m_bSetupBones = false;

	pPlayer->m_bMaintainSequenceTransition( ) = bMaintainSequenceTransition;
	pPlayer->m_nClientEffects( ) = nClientEffects;
	pPlayer->m_bJiggleBones( ) = bJiggleBones;
	pPlayer->m_fEffects( ) = iEffects;
	pPlayer->m_nLastSkipFramecount( ) = nLastSkipFramecount;
	pPlayer->m_nOcclusionFrame( ) = nOcclusionFrame;
	pPlayer->m_nOcclusionMask( ) = nOcclusionMask;
	
	if ( pPlayer != g_sdk.local )
		pPlayer->SetAbsoluteOrigin( vecAbsOrigin );

	//std::memcpy( pPlayer->m_AnimationLayers( ), aAnimationLayers.data( ), sizeof( C_AnimationLayer ) * ANIMATION_LAYER_COUNT );

	g_interfaces.globals->m_curtime = flCurTime;
	g_interfaces.globals->m_realtime = flRealTime;
	g_interfaces.globals->m_frametime = flFrameTime;
	g_interfaces.globals->m_absframetime = flAbsFrameTime;
	g_interfaces.globals->m_framecount = iFrameCount;
	g_interfaces.globals->m_tickcount = iTickCount;
	g_interfaces.globals->m_interpolation_amount = flInterpolation;
}