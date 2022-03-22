#pragma once

class C_BaseEntity : public C_ClientEntity
{
public:
	PushVirtual( GetPredDescMap( ), 17, C_DataMap*( __thiscall* )( void* ) );
	PushVirtual( IsPlayer( ), 158, bool( __thiscall* )( void* ) );
	PushVirtual( SetModelIndex( int i ), 75, void( __thiscall* )( void*, int ), i );

	NETVAR( m_nEntityModelIndex		,	int32_t	, FNV32( "DT_BaseEntity" ), FNV32( "m_nModelIndex" ) );
	NETVAR( m_flSimulationTime		,	float_t	, FNV32( "DT_BaseEntity" ), FNV32( "m_flSimulationTime" ) );
	NETVAR( m_iTeamNum				,	int32_t	, FNV32( "DT_BaseEntity" ), FNV32( "m_iTeamNum" ) );
	NETVAR( m_vecOrigin				,	Vector	, FNV32( "DT_BaseEntity" ), FNV32( "m_vecOrigin" ) );

	void SetAbsoluteAngles( QAngle angViewAngle );
	void SetAbsoluteOrigin( Vector vecAbsOrigin );
	void SetWorldOrigin( Vector vecWorldOrigin );

	DATAMAP( int32_t	,	m_iEFlags			);
	DATAMAP( float_t	,	m_flStamina			);
	DATAMAP( float_t	,	m_flCycle			);
	DATAMAP( int32_t	,	m_fEffects			);
	DATAMAP( Vector		,	m_vecAbsVelocity	);
	DATAMAP( Vector		,	m_vecBaseVelocity	);

	bool IsBreakableEntity( )
	{
		const auto szObjectName = this->GetClientClass( )->m_strNetworkName;
		if ( szObjectName[ 0 ] == 'C' )
		{
			if ( szObjectName[ 7 ] == 't' || szObjectName[ 7 ] == 'b' )
				return true;
		}

		return ( ( bool( __thiscall* )( C_BaseEntity* ) )( g_sdk.address_list.m_IsBreakableEntity ) )( this );
	}

	CUSTOM_OFFSET( m_rgflCoordinateFrame, matrix3x4_t, FNV32( "CoordinateFrame" ), 1092 );
	CUSTOM_OFFSET( m_flOldSimulationTime, float_t, FNV32( "OldSimulationTime" ), 0x26C );
	CUSTOM_OFFSET( m_flFlashTime, float_t, FNV32( "FlashTime" ), 0x10480 );
	CUSTOM_OFFSET( m_nExplodeEffectTickBegin, int32_t, FNV32( "m_nExplodeEffectTickBegin" ), 0x29F4 );
	CUSTOM_OFFSET( m_flCreationTime, float_t, FNV32( "CreationTime" ), 0x2D28 );
	CUSTOM_OFFSET( m_vecMins, Vector, FNV32( "Mins" ), 0x328 );
	CUSTOM_OFFSET( m_vecMaxs, Vector, FNV32( "Maxs" ), 0x334 );
	CUSTOM_OFFSET( m_nRenderMode, int32_t, FNV32( "RenderMode" ), 0x25B );
};