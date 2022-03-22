#include "../Hooks.hpp"
#include "../Settings.hpp"

void __fastcall C_Hooks::hkGetColorModulation( LPVOID pEcx, uint32_t, float_t* flRed, float_t* flGreen, float_t* flBlue )
{
	C_Material* pMaterial = ( C_Material* )( pEcx );
	if ( !pMaterial || pMaterial->IsErrorMaterial( ) )
		return g_sdk.hooks.originals.m_GetColorModulation( pEcx, flRed, flGreen, flBlue );

	g_sdk.hooks.originals.m_GetColorModulation( pEcx, flRed, flGreen, flBlue );
	if ( strstr( pMaterial->GetTextureGroupName( ), _S( TEXTURE_GROUP_WORLD ) ) )
	{
		*flRed = g_cfg->m_WorldModulation->r( ) / 255.0f;
		*flGreen = g_cfg->m_WorldModulation->g( ) / 255.0f;
		*flBlue = g_cfg->m_WorldModulation->b( ) / 255.0f;
	}
	else if ( strstr( pMaterial->GetTextureGroupName( ), _S( TEXTURE_GROUP_STATIC_PROP ) ) )
	{
		*flRed = g_cfg->m_PropModulation->r( ) / 255.0f;
		*flGreen = g_cfg->m_PropModulation->g( ) / 255.0f;
		*flBlue = g_cfg->m_PropModulation->b( ) / 255.0f;
	}
	else if ( strstr( pMaterial->GetTextureGroupName( ), _S( TEXTURE_GROUP_SKYBOX ) ) )
	{
		*flRed = g_cfg->m_SkyModulation->r( ) / 255.0f;
		*flGreen = g_cfg->m_SkyModulation->g( ) / 255.0f;
		*flBlue = g_cfg->m_SkyModulation->b( ) / 255.0f;
	}
}

float_t C_Hooks::hkGetAlphaModulation( LPVOID pEcx, uint32_t )
{
	C_Material* pMaterial = ( C_Material* )( pEcx );
	if ( !pMaterial || pMaterial->IsErrorMaterial( ) )
		return g_sdk.hooks.originals.m_GetAlphaModulation( pEcx );

	const char* strName = pMaterial->GetTextureGroupName( );
	if ( strstr( pMaterial->GetTextureGroupName( ), _S( TEXTURE_GROUP_WORLD ) ) )
		return g_cfg->m_WorldModulation->a( ) / 255.0f;
	else if ( strstr( pMaterial->GetTextureGroupName( ), _S( TEXTURE_GROUP_STATIC_PROP ) ) )
		return g_cfg->m_PropModulation->a( ) / 255.0f;
	else if ( strstr( pMaterial->GetTextureGroupName( ), _S( TEXTURE_GROUP_SKYBOX ) ) )
		return g_cfg->m_SkyModulation->a( ) / 255.0f;

	return g_sdk.hooks.originals.m_GetAlphaModulation( pEcx );
}

LPVOID __fastcall C_Hooks::hkGetClientModelRenderable( LPVOID pEcx, uint32_t )
{	
	return nullptr;
}

bool __cdecl C_Hooks::hkIsUsingDebugStaticProps( )
{
	return true;
}