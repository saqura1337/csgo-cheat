#include "../Hooks.hpp"

void __fastcall C_Hooks::hkProcessMovement( LPVOID pEcx, uint32_t, C_BasePlayer* pPlayer, C_MoveData* pMoveData )
{
	pMoveData->m_bGameCodeMovedPlayer = false;
	return g_sdk.hooks.originals.m_ProcessMovement( pEcx, pPlayer, pMoveData );
}

bool __fastcall C_Hooks::hkTraceFilterForHeadCollision( LPVOID pEcx, uint32_t, C_BasePlayer* pPlayer, LPVOID pTraceParams )
{
	if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
		return g_sdk.hooks.originals.m_TraceFilterForHeadCollision( pEcx, pPlayer, pTraceParams );

	if ( !pPlayer || !pPlayer->IsPlayer( ) || pPlayer->EntIndex( ) - 1 > 63 || pPlayer == g_sdk.local )
		return g_sdk.hooks.originals.m_TraceFilterForHeadCollision( pEcx, pPlayer, pTraceParams );

	if ( fabsf( pPlayer->m_vecOrigin( ).z - g_sdk.local->m_vecOrigin( ).z ) < 10.0f )
		return false;

	return g_sdk.hooks.originals.m_TraceFilterForHeadCollision( pEcx, pPlayer, pTraceParams );
}