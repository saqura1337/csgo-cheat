#pragma once
#include <vector>
#include "../SDK/Includes.hpp"

struct DamageMarker_t
{
	Vector m_vecOrigin = Vector( 0, 0, 0 );
	float_t m_flTime = 0.0f;
	float_t m_flStartZ = 0.0f;
	float_t m_flDamage = 0.0f;
};

class C_DamageMarker
{
public:
	virtual void OnRageBotFire( Vector vecOrigin, float_t flDamage );
	virtual void Instance( );
private:
	std::vector < DamageMarker_t > m_aDmgMarkers = { };
};

inline C_DamageMarker* g_DmgMarkers = new C_DamageMarker( );