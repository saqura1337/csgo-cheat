#include "Autowall.hpp"
#include "../SDK/Math/Math.hpp"

void C_AutoWall::CacheWeaponData()
{
	m_PenetrationData.m_Weapon = g_sdk.local->m_hActiveWeapon().Get();
	if (!m_PenetrationData.m_Weapon)
		return;

	m_PenetrationData.m_WeaponData = m_PenetrationData.m_Weapon->GetWeaponData();
	if (!m_PenetrationData.m_WeaponData)
		return;

	m_PenetrationData.m_flMaxRange = m_PenetrationData.m_WeaponData->m_flRange;
	m_PenetrationData.m_flWeaponDamage = m_PenetrationData.m_WeaponData->m_iDamage;
	m_PenetrationData.m_flPenetrationPower = m_PenetrationData.m_WeaponData->m_flPenetration;
	m_PenetrationData.m_flPenetrationDistance = 3000.0f;
}

bool C_AutoWall::HandleBulletPenetration()
{
	bool bSolidSurf = ((m_PenetrationData.m_EnterTrace.contents >> 3) & CONTENTS_SOLID);
	bool bLightSurf = (m_PenetrationData.m_EnterTrace.surface.flags >> 7) & SURF_LIGHT;

	int nEnterMaterial = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_EnterTrace.surface.surfaceProps)->game.material;
	if (!m_PenetrationData.m_PenetrationCount && !bLightSurf && !bSolidSurf && nEnterMaterial != 9)
	{
		if (nEnterMaterial != 71)
			return false;
	}

	Vector vecEnd;
	if (m_PenetrationData.m_PenetrationCount <= 0 || m_PenetrationData.m_flPenetrationPower <= 0.0f)
		return false;

	if (!TraceToExit(m_PenetrationData.m_EnterTrace.endpos, vecEnd, &m_PenetrationData.m_EnterTrace, &m_PenetrationData.m_ExitTrace))
		if (!(g_interfaces.trace->GetPointContents(vecEnd, 0x600400B) & 0x600400B))
			return false;

	float flDamLostPercent = 0.16;
	float flDamageModifier = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_EnterTrace.surface.surfaceProps)->game.flDamageModifier;
	float flPenetrationModifier = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_EnterTrace.surface.surfaceProps)->game.flPenetrationModifier;

	int iEnterMaterial = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_EnterTrace.surface.surfaceProps)->game.material;
	if (bSolidSurf || bLightSurf || iEnterMaterial == CHAR_TEX_GLASS || iEnterMaterial == CHAR_TEX_GRATE)
	{
		if (iEnterMaterial == CHAR_TEX_GLASS || iEnterMaterial == CHAR_TEX_GRATE)
		{
			flPenetrationModifier = 3.0f;
			flDamLostPercent = 0.05;
		}
		else
			flPenetrationModifier = 1.0f;

		flDamageModifier = 0.99f;
	}
	else if (iEnterMaterial == CHAR_TEX_FLESH && g_sdk.convars.m_DamageReduction->GetFloat() == 0
		&& m_PenetrationData.m_EnterTrace.hit_entity && ((C_BasePlayer*)(m_PenetrationData.m_EnterTrace.hit_entity))->IsPlayer() && ((C_BasePlayer*)(m_PenetrationData.m_EnterTrace.hit_entity))->m_iTeamNum() == g_sdk.local->m_iTeamNum())
	{
		if (g_sdk.convars.m_DamageReduction->GetFloat() == 0)
			return true;

		flPenetrationModifier = g_sdk.convars.m_DamageReduction->GetFloat();
		flDamageModifier = g_sdk.convars.m_DamageReduction->GetFloat();
	}
	else
	{
		float flExitPenetrationModifier = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_ExitTrace.surface.surfaceProps)->game.flPenetrationModifier;
		float flExitDamageModifier = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_ExitTrace.surface.surfaceProps)->game.flDamageModifier;
		flPenetrationModifier = (flPenetrationModifier + flExitPenetrationModifier) / 2;
		flDamageModifier = (flDamageModifier + flExitDamageModifier) / 2;
	}

	int iExitMaterial = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_ExitTrace.surface.surfaceProps)->game.material;
	if (iEnterMaterial == iExitMaterial)
	{
		if (iExitMaterial == CHAR_TEX_WOOD || iExitMaterial == CHAR_TEX_CARDBOARD)
		{
			flPenetrationModifier = 3;
		}
		else if (iExitMaterial == CHAR_TEX_PLASTIC)
		{
			flPenetrationModifier = 2;
		}
	}

	float flTraceDistance = (m_PenetrationData.m_ExitTrace.endpos - m_PenetrationData.m_EnterTrace.endpos).Length();

	float flPenMod = fmax(0.f, (1.f / flPenetrationModifier));

	float flPercentDamageChunk = m_PenetrationData.m_flCurrentDamage * flDamLostPercent;
	float flPenWepMod = flPercentDamageChunk + fmax(0, (3 / m_PenetrationData.m_flPenetrationPower) * 1.25) * (flPenMod * 3.0);

	float flLostDamageObject = ((flPenMod * (flTraceDistance * flTraceDistance)) / 24);
	float flTotalLostDamage = flPenWepMod + flLostDamageObject;

	m_PenetrationData.m_flCurrentDamage -= fmax(flTotalLostDamage, 0.0f);
	if (m_PenetrationData.m_flCurrentDamage <= 0.0f)
		return false;

	m_PenetrationData.m_vecShootPosition = m_PenetrationData.m_ExitTrace.endpos;
	m_PenetrationData.m_PenetrationCount--;

	return true;
}

bool C_AutoWall::TraceToExit(Vector vecStart, Vector& vecEnd, CGameTrace* pEnterTrace, CGameTrace* pExitTrace)
{
	int	nFirstContents = 0;

	std::array < std::uintptr_t, 4 > aFilter
		=
	{
		*(std::uintptr_t*)(g_sdk.address_list.m_TraceFilterSimple),
		(uintptr_t)(g_sdk.local),
		NULL,
		NULL
	};

	float flDistance = 0.0f;
	while (flDistance <= 90.0f)
	{
		// increase distance
		flDistance += 4.0f;

		// calc new end
		vecEnd = vecStart + (m_PenetrationData.m_vecDirection * flDistance);

		// cache contents
		int nContents = g_interfaces.trace->GetPointContents(vecEnd, 0x4600400B, nullptr);
		if (!nFirstContents)
			nFirstContents = nContents;

		Vector vecTraceEnd = vecEnd - (m_PenetrationData.m_vecDirection * 4.0f);
		if (nContents & 0x600400B && (!(nContents & 0x40000000) || nFirstContents == nContents))
			continue;

		// trace line
		g_interfaces.trace->TraceLine(vecEnd, vecTraceEnd, 0x4600400B, g_sdk.local, NULL, pExitTrace);

		// clip trace to player
		ClipTraceToPlayers((C_BasePlayer*)(pExitTrace->hit_entity), vecEnd, vecTraceEnd, pExitTrace, (CTraceFilter*)(aFilter.data()), 0x4600400B);

		// check solid and falgs
		if (pExitTrace->startsolid && pExitTrace->surface.flags & SURF_HITBOX)
		{
			std::array < uintptr_t, 5 > aSkipTwoEntities
				=
			{
				*(std::uintptr_t*)(g_sdk.address_list.m_TraceFilterSkipTwoEntities),
				(uintptr_t)(g_sdk.local),
				NULL,
				NULL,
				(std::uintptr_t)(pExitTrace->hit_entity),
			};

			Ray_t Ray;
			Ray.Init(vecEnd, vecStart);

			g_interfaces.trace->TraceRay(Ray, 0x600400B, (CTraceFilter*)(aSkipTwoEntities.data()), pExitTrace);
			if (pExitTrace->DidHit() && !pExitTrace->startsolid)
			{
				vecEnd = pExitTrace->endpos;
				return true;
			}

			continue;
		}

		if (!pExitTrace->DidHit() || pExitTrace->startsolid)
		{
			if (pEnterTrace->hit_entity != g_interfaces.entity_list->GetClientEntity(NULL))
			{
				if (pExitTrace->hit_entity && ((C_BaseEntity*)(pExitTrace->hit_entity))->IsBreakableEntity())
				{
					pExitTrace->surface.surfaceProps = pEnterTrace->surface.surfaceProps;
					pExitTrace->endpos = vecStart + m_PenetrationData.m_vecDirection;

					return true;
				}
			}

			continue;
		}

		if (pExitTrace->surface.flags & 0x80)
		{
			if (pEnterTrace->hit_entity && ((C_BaseEntity*)(pEnterTrace->hit_entity))->IsBreakableEntity() && pExitTrace->hit_entity && ((C_BaseEntity*)(pExitTrace->hit_entity))->IsBreakableEntity())
			{
				vecEnd = pExitTrace->endpos;
				return true;
			}

			if (!(pEnterTrace->surface.flags & 0x80u))
				continue;
		}

		if (pExitTrace->plane.normal.Dot(m_PenetrationData.m_vecDirection) > 1.0f)
			return false;

		vecEnd -= m_PenetrationData.m_vecDirection * (pExitTrace->fraction * 4.0f);
		return true;
	}

	return false;
}

bool C_AutoWall::SimulateFireBullet()
{
	if (!m_PenetrationData.m_Weapon || !m_PenetrationData.m_WeaponData)
		return false;

	std::array < uintptr_t, 5 > aSkipTwoEntities
		=
	{
		*(uintptr_t*)(g_sdk.address_list.m_TraceFilterSkipTwoEntities),
		(uintptr_t)(g_sdk.local),
		NULL,
		NULL,
		NULL
	};

	C_ClientEntity* m_LastPlayerHit = nullptr;
	while (m_PenetrationData.m_PenetrationCount > 0)
	{
		float flRemains = (m_PenetrationData.m_flMaxRange - m_PenetrationData.m_flCurrentDistance);
		Vector vecEnd = m_PenetrationData.m_vecShootPosition + (m_PenetrationData.m_vecDirection * flRemains);

		aSkipTwoEntities[4] = (uintptr_t)(m_LastPlayerHit);

		Ray_t Ray;
		Ray.Init(m_PenetrationData.m_vecShootPosition, vecEnd);

		g_interfaces.trace->TraceRay(Ray, MASK_SHOT_HULL | CONTENTS_HITBOX, (CTraceFilter*)(aSkipTwoEntities.data()), &m_PenetrationData.m_EnterTrace);

		ClipTraceToPlayers((C_BasePlayer*)(m_PenetrationData.m_EnterTrace.hit_entity), m_PenetrationData.m_vecShootPosition, vecEnd + (m_PenetrationData.m_vecDirection * 40.0f), &m_PenetrationData.m_EnterTrace, (CTraceFilter*)(aSkipTwoEntities.data()), MASK_SHOT_HULL | CONTENTS_HITBOX);

		m_LastPlayerHit = m_PenetrationData.m_EnterTrace.hit_entity;

		surfacedata_t* pEnterSurfaceData = g_interfaces.prop_physics->GetSurfaceData(m_PenetrationData.m_EnterTrace.surface.surfaceProps);
		if ((int)(m_PenetrationData.m_EnterTrace.fraction))
			break;

		float flEnterSurfacePenetrationModifier = pEnterSurfaceData->game.flPenetrationModifier;
		if (m_PenetrationData.m_flCurrentDistance > m_PenetrationData.m_flPenetrationDistance && m_PenetrationData.m_flPenetrationPower > 0.f || flEnterSurfacePenetrationModifier < 0.1f)
			break;

		m_PenetrationData.m_flCurrentDistance += m_PenetrationData.m_EnterTrace.fraction * flRemains;
		m_PenetrationData.m_flCurrentDamage *= std::pow(m_PenetrationData.m_WeaponData->m_flRangeModifier, (m_PenetrationData.m_flCurrentDistance / 500.0f));

		if (m_PenetrationData.m_EnterTrace.hit_entity)
		{
			C_BasePlayer* pPlayer = (C_BasePlayer*)(m_PenetrationData.m_EnterTrace.hit_entity);
			if (pPlayer && pPlayer->IsPlayer())
			{
				if (!pPlayer->IsAlive() || pPlayer->m_iTeamNum() == g_sdk.local->m_iTeamNum())
					return false;

				if (m_PenetrationData.m_EnterTrace.hitgroup != HITGROUP_GEAR && m_PenetrationData.m_EnterTrace.hitgroup != HITGROUP_GENERIC)
				{
					this->ScaleDamage(m_PenetrationData.m_EnterTrace, m_PenetrationData.m_flCurrentDamage);
					return true;
				}
			}
		}

		if (this->HandleBulletPenetration() && m_PenetrationData.m_flCurrentDamage > 0.0f)
			continue;

		break;
	}

	return false;
}

void C_AutoWall::ScaleDamage(CGameTrace Trace, float_t& flDamage)
{
	bool bHasHeavyArmour = ((C_BasePlayer*)(Trace.hit_entity))->m_bHasHeavyArmor();
	switch (Trace.hitgroup)
	{
	case HITGROUP_HEAD:

		if (bHasHeavyArmour)
			flDamage *= 2.0f;
		else
			flDamage *= 4.f;

		break;

	case HITGROUP_CHEST:
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:

		flDamage *= 1.f;
		break;

	case HITGROUP_STOMACH:

		flDamage *= 1.25f;
		break;

	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:

		flDamage *= 0.75f;
		break;
	}

	if (!this->IsArmored(((C_BasePlayer*)(Trace.hit_entity)), Trace.hitgroup))
		return;

	float fl47 = 1.f, flArmorBonusRatio = 0.5f;

	float_t flArmorRatio = m_PenetrationData.m_WeaponData->m_flArmorRatio * 0.5f;
	if (bHasHeavyArmour)
	{
		flArmorBonusRatio = 0.33f;
		flArmorRatio = m_PenetrationData.m_WeaponData->m_flArmorRatio * 0.25f;
		fl47 = 0.33f;
	}

	float flNewDamage = flDamage * flArmorRatio;
	if (bHasHeavyArmour)
		flNewDamage *= 0.85f;

	if (((flDamage - (flDamage * flArmorRatio)) * (fl47 * flArmorBonusRatio)) > ((C_BasePlayer*)(Trace.hit_entity))->m_ArmourValue())
		flNewDamage = flDamage - (((C_BasePlayer*)(Trace.hit_entity))->m_ArmourValue() / flArmorBonusRatio);

	flDamage = flNewDamage;
}

bool C_AutoWall::IsArmored(C_BasePlayer* pPlayer, int32_t nHitGroup)
{
	bool bResult = false;
	if (pPlayer->m_ArmourValue() > 0)
	{
		switch (nHitGroup)
		{
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			bResult = true;
			break;
		case HITGROUP_HEAD:
			bResult = pPlayer->m_bHasHelmet();
			break;
		}
	}

	return bResult;
}

void C_AutoWall::ClipTraceToPlayers(C_BasePlayer* pPlayer, Vector vecStart, Vector vecEnd, CGameTrace* Trace, CTraceFilter* pTraceFilter, uint32_t nMask)
{
	Ray_t Ray;
	Ray.Init(vecStart, vecEnd);

	CGameTrace NewTrace;
	if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->IsDormant() || pPlayer->m_iTeamNum() == g_sdk.local->m_iTeamNum())
		return;

	if (pTraceFilter && !pTraceFilter->ShouldHitEntity(pPlayer, MASK_SHOT_HULL | CONTENTS_HITBOX))
		return;

	float flRange = Math::DistanceToRay(pPlayer->WorldSpaceCenter(), vecStart, vecEnd);
	if (flRange < -60.0f || flRange > 60.0f)
		return;

	g_interfaces.trace->ClipRayToEntity(Ray, nMask, pPlayer, &NewTrace);
	if (NewTrace.fraction > Trace->fraction)
		std::memcpy(Trace, &NewTrace, sizeof(CGameTrace));
}

float C_AutoWall::GetPointDamage(Vector vecShootPosition, Vector vecTargetPosition)
{
	if (!m_PenetrationData.m_Weapon || !m_PenetrationData.m_WeaponData)
		return false;

	m_PenetrationData.m_vecShootPosition = vecShootPosition;
	m_PenetrationData.m_vecTargetPosition = vecTargetPosition;
	m_PenetrationData.m_vecDirection = GetPointDirection(vecShootPosition, vecTargetPosition);
	m_PenetrationData.m_flCurrentDamage = m_PenetrationData.m_flWeaponDamage;
	m_PenetrationData.m_flCurrentDistance = 0.0f;
	m_PenetrationData.m_PenetrationCount = 4;

	m_PenetrationData.m_flDamageModifier = 0.5f;
	m_PenetrationData.m_flPenetrationModifier = 1.0f;

	if (SimulateFireBullet())
		return m_PenetrationData.m_flCurrentDamage;

	return 0.0f;
}

Vector C_AutoWall::GetPointDirection(Vector vecShootPosition, Vector vecTargetPosition)
{
	Vector vecDirection;
	QAngle angDirection;

	Math::VectorAngles(vecTargetPosition - vecShootPosition, angDirection);
	Math::AngleVectors(angDirection, vecDirection);

	vecDirection.NormalizeInPlace();
	return vecDirection;
}

bool CGameTrace::DidHitWorld() const
{
	return hit_entity == g_interfaces.entity_list->GetClientEntity(NULL);
}

bool CGameTrace::DidHitNonWorldEntity() const
{
	return hit_entity != nullptr && !DidHitWorld();
}

bool C_AutoWall::IsPenetrablePoint(Vector vecShootPosition, Vector vecTargetPosition)
{
	QAngle angLocalAngles;
	g_interfaces.engine->GetViewAngles(&angLocalAngles);

	Vector vecDirection;
	Math::AngleVectors(angLocalAngles, vecDirection);

	CGameTrace Trace;
	g_interfaces.trace->TraceLine(vecShootPosition, vecTargetPosition, MASK_SHOT_HULL | CONTENTS_HITBOX, g_sdk.local, NULL, &Trace);

	if ((int)(Trace.fraction))
		return false;

	PenetrationData_t OldPenetrationData = m_PenetrationData;
	m_PenetrationData.m_EnterTrace = Trace;
	m_PenetrationData.m_vecDirection = vecDirection;
	m_PenetrationData.m_flCurrentDistance = 0.0f;
	m_PenetrationData.m_flCurrentDamage = m_PenetrationData.m_flWeaponDamage;
	m_PenetrationData.m_PenetrationCount = 1;
	m_PenetrationData.m_vecShootPosition = vecShootPosition;
	m_PenetrationData.m_vecTargetPosition = vecTargetPosition;

	if (!g_AutoWall->HandleBulletPenetration())
	{
		m_PenetrationData = OldPenetrationData;
		return false;
	}

	m_PenetrationData = OldPenetrationData;
	return true;
}