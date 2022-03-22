#include "../Hooks.hpp"

bool __fastcall C_Hooks::hkClDoResetLatch_GetBool( LPVOID pEcx, uint32_t )
{
	if ( reinterpret_cast < DWORD >( _ReturnAddress( ) ) == reinterpret_cast < DWORD >( g_sdk.address_list.m_ClDoResetLatch_GetBool_Call ) )
		return false;

	return g_sdk.hooks.originals.m_ClDoResetLatch_GetBool( pEcx );
}

bool __fastcall C_Hooks::hkSvCheats_GetBool( LPVOID pEcx, uint32_t )
{
	if ( reinterpret_cast < DWORD >( _ReturnAddress( ) ) == reinterpret_cast < DWORD >( g_sdk.address_list.m_SvCheats_GetBool_Call ) )
		return true;

	return g_sdk.hooks.originals.m_SvCheats_GetBool( pEcx );
}