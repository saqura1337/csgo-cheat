#include "../Hooks.hpp"
#include "../Render.hpp"
#include "../Features/Model/Model.hpp"
#include "../Features/Visuals/World.hpp"

void __fastcall C_Hooks::hkLockCursor( LPVOID pEcx, uint32_t )
{
	if ( g_Menu->IsMenuOpened( ) )
		return g_interfaces.surface->UnlockCursor( );

	return g_sdk.hooks.originals.m_LockCursor( pEcx );
}

void __fastcall C_Hooks::hkPaintTraverse( LPVOID pEcx, uint32_t, VGUI::VPANEL pPanel, bool bForceRepaint, bool bAllowForce )
{
	if ( strstr( g_interfaces.vgui_panel->GetName( pPanel ), _S( "HudZoom" ) ) )
		if ( g_cfg->m_aWorldRemovals[ REMOVALS_VISUAL_SCOPE ] )
			return;

	g_sdk.hooks.originals.m_PaintTraverse( pEcx, pPanel, bForceRepaint, bAllowForce );

	if (g_interfaces.engine->IsInGame())
	{
		SkinChanger::model_indexes.clear();
		SkinChanger::player_model_indexes.clear();
	}

	static VGUI::VPANEL pPanelID = NULL;
	if ( !pPanelID )  
	{
		if ( !strcmp( g_interfaces.vgui_panel->GetName( pPanel ), _S( "FocusOverlayPanel" ) ) )
			pPanelID = pPanel;
		
	}
	else if ( pPanelID == pPanel )
	{
		g_World->PreserveKillfeed( );
		if ( g_Menu->IsMenuOpened( ) )
			g_DrawModel->Instance( );

		g_Render->Instance( );
	}
}