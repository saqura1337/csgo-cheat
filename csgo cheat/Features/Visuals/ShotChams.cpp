#include "ShotChams.hpp"
#include "Chams.hpp"

void C_ShotChams::OnRageBotFire( C_BasePlayer* pPlayer )
{
    static int nSkin = FindInDataMap( pPlayer->GetPredDescMap( ), FNV32( "m_nSkin" ) );
    static int nBody = FindInDataMap( pPlayer->GetPredDescMap( ), FNV32( "m_nBody" ) );

    IClientRenderable* pRenderable = pPlayer->GetClientRenderable( );
    if ( !pRenderable )
        return;

    const model_t* pModel = pPlayer->GetModel( );
    if ( !pModel )
        return;
    
    C_ShotChamsData& NewShotData = m_aShotChams.emplace_back( );

    NewShotData.m_RenderInfo.origin = pPlayer->GetAbsOrigin( );
    NewShotData.m_RenderInfo.angles = pPlayer->GetAbsAngles( );
    NewShotData.m_flTimeToDraw = g_interfaces.globals->m_realtime + 4.0f;
    NewShotData.m_DrawState.m_pStudioHdr = g_interfaces.model_info->GetStudiomodel( pModel );
    NewShotData.m_DrawState.m_pStudioHWData = g_interfaces.mdl_cache->GetHardwareData( pModel->studio );
    NewShotData.m_DrawState.m_pRenderable = pRenderable;
    NewShotData.m_DrawState.m_drawFlags = 0;
    NewShotData.m_DrawState.m_pModelToWorld = &NewShotData.m_aWorldMatrix;

    NewShotData.m_RenderInfo.pRenderable = pRenderable;
    NewShotData.m_RenderInfo.pModel = pModel;
    NewShotData.m_RenderInfo.pLightingOffset = nullptr;
    NewShotData.m_RenderInfo.pLightingOrigin = nullptr;
    NewShotData.m_RenderInfo.hitboxset = pPlayer->m_nHitboxSet( );
    NewShotData.m_RenderInfo.skin = ( int32_t )( ( uintptr_t )( pPlayer ) + nSkin );
    NewShotData.m_RenderInfo.body = ( int32_t )( ( uintptr_t )( pPlayer ) + nBody );
    NewShotData.m_RenderInfo.entity_index = pPlayer->EntIndex( );
    NewShotData.m_RenderInfo.instance = GetVirtual< ModelInstanceHandle_t( __thiscall* )( void* ) >( pRenderable, 30 )( pRenderable );
    NewShotData.m_RenderInfo.flags = 0x1;
    NewShotData.m_RenderInfo.pModelToWorld = &NewShotData.m_aWorldMatrix;
    
    std::memcpy( NewShotData.m_aMatrix.data( ), pPlayer->m_CachedBoneData( ).Base( ), pPlayer->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );

    Vector vecAngles = Vector( NewShotData.m_RenderInfo.angles.pitch, NewShotData.m_RenderInfo.angles.yaw, NewShotData.m_RenderInfo.angles.roll );
    NewShotData.m_aWorldMatrix.AngleMatrix( vecAngles, NewShotData.m_RenderInfo.origin );
}

void C_ShotChams::OnDrawModel( )
{
    if ( !g_sdk.local || !g_sdk.local->IsAlive( ) )
    {
        if ( !m_aShotChams.empty( ) )
            m_aShotChams.clear( );

        return;
    }

    C_ChamsSettings Settings = g_cfg->m_aChamsSettings[ 3 ];
    if ( !Settings.m_bRenderChams )
        return;

    for ( int i = 0; i < m_aShotChams.size( ); i++ )
    {
        if ( g_interfaces.globals->m_realtime - m_aShotChams[ i ].m_flTimeToDraw < 0.0f && g_interfaces.entity_list->GetClientEntity( m_aShotChams[ i ].m_RenderInfo.entity_index ) )
            continue;

        m_aShotChams.erase( m_aShotChams.begin( ) + i );
    }

    g_sdk.model.m_bShotChams = true;
    for ( auto it = m_aShotChams.begin( ); it != m_aShotChams.end( ); it++ )
    {
        if ( !it->m_DrawState.m_pModelToWorld || !it->m_DrawState.m_pRenderable || !it->m_DrawState.m_pStudioHdr || !it->m_DrawState.m_pStudioHWData ||
            !it->m_RenderInfo.pRenderable || !it->m_RenderInfo.pModelToWorld || !it->m_RenderInfo.pModel || !g_interfaces.entity_list->GetClientEntity( it->m_RenderInfo.entity_index ) ) 
            continue;
        
        float_t flProgress = ( it->m_flTimeToDraw - g_interfaces.globals->m_realtime ) / 0.2f;
        if ( it->m_flTimeToDraw - g_interfaces.globals->m_realtime > 0.2f )
            flProgress = 1.0f;

        float_t flFirstAlpha = Settings.m_Color[ 3 ] * flProgress;
        float_t flSecondAlpha = Settings.m_aModifiersColors[ 0 ][ 3 ] * flProgress;
        float_t flThirdAlpha = Settings.m_aModifiersColors[ 1 ][ 3 ] * flProgress;
        float_t flFourthAlpha = Settings.m_aModifiersColors[ 2 ][ 3 ] * flProgress;

        if ( Settings.m_aModifiers[ 0 ] )
        {
            g_Chams->ApplyMaterial( 2, true, Settings.m_aModifiersColors[ 0 ], false, true );

            g_interfaces.render_view->SetBlend( flSecondAlpha );
            g_sdk.hooks.originals.m_DrawModelExecute_Model( g_interfaces.model_render, GetVirtual < IMatRenderContext*( __thiscall* )( void* ) >( g_interfaces.material_system, 115 )( g_interfaces.material_system ), it->m_DrawState, it->m_RenderInfo, it->m_aMatrix.data( ) );
        }
           
        if ( Settings.m_aModifiers[ 1 ] )
        {
            g_Chams->ApplyMaterial( 3, true, Settings.m_aModifiersColors[ 1 ], false, true );

            g_interfaces.render_view->SetBlend( flThirdAlpha );
            g_sdk.hooks.originals.m_DrawModelExecute_Model( g_interfaces.model_render, GetVirtual < IMatRenderContext*( __thiscall* )( void* ) >( g_interfaces.material_system, 115 )( g_interfaces.material_system ), it->m_DrawState, it->m_RenderInfo, it->m_aMatrix.data( ) );
        }

        if ( Settings.m_aModifiers[ 2 ] )
        {
            g_Chams->ApplyMaterial( 4, true, Settings.m_aModifiersColors[ 2 ], false, true );
          
            g_interfaces.render_view->SetBlend( flFourthAlpha );
            g_sdk.hooks.originals.m_DrawModelExecute_Model( g_interfaces.model_render, GetVirtual < IMatRenderContext*( __thiscall* )( void* ) >( g_interfaces.material_system, 115 )( g_interfaces.material_system ), it->m_DrawState, it->m_RenderInfo, it->m_aMatrix.data( ) );
        }

        if ( Settings.m_aModifiers[ 3 ] )
        {
            g_Chams->ApplyMaterial( 5, true, Settings.m_aModifiersColors[ 3 ], false, true );
          
            g_interfaces.render_view->SetBlend( flFourthAlpha );
            g_sdk.hooks.originals.m_DrawModelExecute_Model( g_interfaces.model_render, GetVirtual < IMatRenderContext*( __thiscall* )( void* ) >( g_interfaces.material_system, 115 )( g_interfaces.material_system ), it->m_DrawState, it->m_RenderInfo, it->m_aMatrix.data( ) );
        }

        g_Chams->ApplyMaterial( Settings.m_iMainMaterial, true, Settings.m_Color, false, true );

        g_interfaces.render_view->SetBlend( flFirstAlpha );

        g_sdk.hooks.originals.m_DrawModelExecute_Model( g_interfaces.model_render, GetVirtual < IMatRenderContext*( __thiscall* )( void* ) >( g_interfaces.material_system, 115 )( g_interfaces.material_system ), it->m_DrawState, it->m_RenderInfo, it->m_aMatrix.data( ) );
        g_interfaces.model_render->ForcedMaterialOverride( nullptr );
    }
    g_sdk.model.m_bShotChams = false;
}