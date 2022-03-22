#include "HitMarker.hpp"
#include "../Settings.hpp"
#include "../Render.hpp"

void C_HitMarker::Instance( )
{
	if ( !g_cfg->m_bHitMarker )
	{
		if ( !m_aHitMarkers.empty( ) )
			m_aHitMarkers.clear( );

		return;
	}

	if ( m_aHitMarkers.empty( ) )
		return;

	float_t flTime = g_interfaces.globals->m_realtime;
	for ( int i = 0; i < m_aHitMarkers.size( ); i++ )
	{
		auto HitMarker = &m_aHitMarkers[ i ];
		if ( flTime - HitMarker->m_flTime > 4.0f )
		{
			m_aHitMarkers.erase( m_aHitMarkers.begin( ) + i );
			continue;
		}

		Vector vecScreenPosition = Vector( 0, 0, 0 );
		if ( g_interfaces.debug_overlay->ScreenPosition( HitMarker->m_vecOrigin, vecScreenPosition ) )
			continue;

		g_Render->RenderLine( vecScreenPosition.x - 4.0f, vecScreenPosition.y - 4.0f, vecScreenPosition.x - 1, vecScreenPosition.y - 1, Color( 255, 255, 255, 120 ), 1.0f );
		g_Render->RenderLine( vecScreenPosition.x - 4.0f, vecScreenPosition.y + 4.0f, vecScreenPosition.x - 1, vecScreenPosition.y + 1, Color( 255, 255, 255, 120 ), 1.0f );
		g_Render->RenderLine( vecScreenPosition.x + 4.0f, vecScreenPosition.y - 4.0f, vecScreenPosition.x + 1, vecScreenPosition.y - 1, Color( 255, 255, 255, 120 ), 1.0f );
		g_Render->RenderLine( vecScreenPosition.x + 4.0f, vecScreenPosition.y + 4.0f, vecScreenPosition.x + 1, vecScreenPosition.y + 1, Color( 255, 255, 255, 120 ), 1.0f );
	}
}

void C_HitMarker::OnRageBotFire( Vector vecOrigin )
{
	if ( !g_cfg->m_bHitMarker )
	{
		if ( !m_aHitMarkers.empty( ) )
			m_aHitMarkers.clear( );

		return;
	}

	HitMarker_t& HitMarker = m_aHitMarkers.emplace_back( );
	HitMarker.m_flTime = g_interfaces.globals->m_realtime;
	HitMarker.m_vecOrigin = vecOrigin;
}