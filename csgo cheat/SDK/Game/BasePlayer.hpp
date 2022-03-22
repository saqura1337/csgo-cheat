#pragma once
#include "AnimationState.hpp"
#include "BoneAccessor.hpp"

class C_BasePlayer : public C_BaseEntity
{
public:
	PushVirtual(GetLayerSequenceCycleRate(C_AnimationLayer* AnimationLayer, int32_t iLayerSequence), 223, float_t(__thiscall*)(void*, C_AnimationLayer*, int32_t), AnimationLayer, iLayerSequence);

	NETVAR(m_vecVelocity, Vector, FNV32("DT_BasePlayer"), FNV32("m_vecVelocity[0]"));
	NETVAR(m_vecViewOffset, Vector, FNV32("DT_BasePlayer"), FNV32("m_vecViewOffset[0]"));
	NETVAR(m_aimPunchAngleVel, Vector, FNV32("DT_BasePlayer"), FNV32("m_aimPunchAngleVel"));
	NETVAR(m_viewPunchAngle, QAngle, FNV32("DT_BasePlayer"), FNV32("m_viewPunchAngle"));
	NETVAR(m_aimPunchAngle, QAngle, FNV32("DT_BasePlayer"), FNV32("m_aimPunchAngle"));
	NETVAR(m_angEyeAngles, QAngle, FNV32("DT_CSPlayer"), FNV32("m_angEyeAngles[0]"));
	NETVAR(m_nTickBase, int32_t, FNV32("DT_BasePlayer"), FNV32("m_nTickBase"));
	NETVAR(m_nNextThinkTick, int32_t, FNV32("DT_BasePlayer"), FNV32("m_nNextThinkTick"));
	NETVAR(m_fLifeState, int32_t, FNV32("DT_BasePlayer"), FNV32("m_lifeState"));
	NETVAR(m_iHealth, int32_t, FNV32("DT_BasePlayer"), FNV32("m_iHealth"));
	NETVAR(m_iHideHUD, int32_t, FNV32("DT_BasePlayer"), FNV32("m_iHideHUD"));
	NETVAR(m_fFlags, int32_t, FNV32("DT_BasePlayer"), FNV32("m_fFlags"));
	NETVAR(m_iObserverMode, int32_t, FNV32("DT_BasePlayer"), FNV32("m_iObserverMode"));
	NETVAR(m_ArmourValue, int32_t, FNV32("DT_CSPlayer"), FNV32("m_ArmorValue"));
	NETVAR(m_iAccount, int32_t, FNV32("DT_CSPlayer"), FNV32("m_iAccount"));
	NETVAR(m_iMoveState, int32_t, FNV32("DT_CSPlayer"), FNV32("m_iMoveState"));
	NETVAR(m_flFallVelocity, float_t, FNV32("DT_BasePlayer"), FNV32("m_flFallVelocity"));
	NETVAR(m_flLowerBodyYaw, float_t, FNV32("DT_CSPlayer"), FNV32("m_flLowerBodyYawTarget"));
	NETVAR(m_flVelocityModifier, float_t, FNV32("DT_CSPlayer"), FNV32("m_flVelocityModifier"));
	NETVAR(m_flThirdpersonRecoil, float_t, FNV32("DT_CSPlayer"), FNV32("m_flThirdpersonRecoil"));
	NETVAR(m_flNextAttack, float_t, FNV32("DT_BaseCombatCharacter"), FNV32("m_flNextAttack"));
	NETVAR(m_flDuckSpeed, float_t, FNV32("DT_BasePlayer"), FNV32("m_flDuckSpeed"));
	NETVAR(m_flDuckAmount, float_t, FNV32("DT_BasePlayer"), FNV32("m_flDuckAmount"));
	NETVAR(m_hObserverTarget, CHandle < C_BasePlayer >, FNV32("DT_BasePlayer"), FNV32("m_hObserverTarget"));
	NETVAR(m_hGroundEntity, CHandle < C_BaseEntity >, FNV32("DT_BasePlayer"), FNV32("m_hGroundEntity"));
	NETVAR(m_hActiveWeapon, CHandle < C_BaseCombatWeapon >, FNV32("DT_BaseCombatCharacter"), FNV32("m_hActiveWeapon"));
	NETVAR(m_hViewModel, CHandle < C_BaseViewModel >, FNV32("DT_BasePlayer"), FNV32("m_hViewModel[0]"));
	NETVAR(m_bHasHeavyArmor, bool, FNV32("DT_CSPlayer"), FNV32("m_bHasHeavyArmor"));
	NETVAR(m_bHasHelmet, bool, FNV32("DT_CSPlayer"), FNV32("m_bHasHelmet"));
	NETVAR(m_bIsScoped, bool, FNV32("DT_CSPlayer"), FNV32("m_bIsScoped"));
	NETVAR(m_bGunGameImmunity, bool, FNV32("DT_CSPlayer"), FNV32("m_bGunGameImmunity"));
	NETVAR(m_bIsWalking, bool, FNV32("DT_CSPlayer"), FNV32("m_bIsWalking"));
	NETVAR(m_bStrafing, bool, FNV32("DT_CSPlayer"), FNV32("m_bStrafing"));
	NETVAR(m_bClientSideAnimation, bool, FNV32("DT_BaseAnimating"), FNV32("m_bClientSideAnimation"));
	NETVAR(m_nHitboxSet, int32_t, FNV32("DT_BaseAnimating"), FNV32("m_nHitboxSet"));

	float_t GetYawModifer()
	{
		//CCSGOPlayerAnimState::SetupVelocity
		// ref: https://prnt.sc/26jlokg

		// improve me:
		/*
		 CCSGOPlayerAnimState::Update;
		 ref: https://prnt.sc/26jlbf6
		*/

		auto animstate = this->m_PlayerAnimStateCSGO();

		if (/*!this || */!animstate)
			return 0.0f;

		float_t v38 = animstate->m_flSpeedAsPortionOfWalkTopSpeed;
		float_t v39, v40, v41, v42, v43, v44;

		if (v38 >= 0.0)
			v39 = fminf(v38, 1.0);
		else
			v39 = 0.0;
		v40 = ((animstate->m_flWalkToRunTransition * -0.30000001) - 0.19999999) * v39;
		v41 = animstate->m_flAnimDuckAmount;
		v42 = v40 + 1.0;

		if (v41 > 0.0)
		{
			v43 = animstate->m_flSpeedAsPortionOfCrouchTopSpeed;
			if (v43 >= 0.0)
				v44 = fminf(v43, 1.0);
			else
				v44 = 0.0;
			v42 = v42 + ((v41 * v44) * (0.5 - v42));
		}

		return v42;
	}

	float_t GetFeetDelta()
	{
		float_t v33 = -360.0;
		this->m_PlayerAnimStateCSGO()->m_flFootYawLast = this->m_PlayerAnimStateCSGO()->m_flFootYaw;
		float_t v34 = this->m_PlayerAnimStateCSGO()->m_flFootYaw;
		float_t v106 = -360.0;

		if (v34 >= -360.0)
		{
			v33 = fminf(v34, 360.0);
			v106 = v33;
		}

		this->m_PlayerAnimStateCSGO()->m_flFootYaw = v33;

		float_t v35 = this->m_PlayerAnimStateCSGO()->m_flEyeYaw - v33;
		float_t v114 = fmod(v35, 360.0);
		float_t v37 = v114;

		if (this->m_PlayerAnimStateCSGO()->m_flEyeYaw <= v106)
		{
			if (v114 <= -180.0)
				v37 = v114 + 360.0;
		}
		else if (v114 >= 180.0)
		{
			v37 = v114 - 360.0;
		}

		return v37;
	}

	float_t GetMaxDesyncDelta()
	{
		//if (!this)
		//	return 0.f;

		return GetYawModifer() * m_PlayerAnimStateCSGO()->m_flAimYawMax;
	}

	float_t GetMinDesyncDelta()
	{
		//if (!this)
		//	return 0.f;

		return GetYawModifer() * m_PlayerAnimStateCSGO()->m_flAimYawMin;
	}

	const char* m_szLastPlaceName()
	{
		//if (!this)
		//	return 0;

		return (const char*)((DWORD)(this) + 0x35C4);
	}

	CBaseHandle* m_hMyWeapons()
	{
		//if (!this)
		//	return 0;

		return (CBaseHandle*)((DWORD)(this) + 0x2E08);
	}

	CBaseHandle* m_hMyWearables()
	{
		//if (!this)
		//	return 0;

		return (CBaseHandle*)((DWORD)(this) + 0x2F14);
	}

	__forceinline void UpdateClientSideAnimation()
	{
		//if (!this)
		//	return;

		return ((void(__thiscall*)(LPVOID))(g_sdk.address_list.m_UpdateClientSideAnimation))(this);
	}

	__forceinline void InvalidatePhysicsRecursive(int32_t iFlags)
	{
		//if (!this)
		//	return;

		// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/baseentity_shared.cpp#L1632
		((void(__thiscall*)(LPVOID, int32_t))(g_sdk.address_list.m_InvalidatePhysicsRecursive))(this, iFlags);
	}

	__forceinline mstudioseqdesc_t& GetSequenceDescription(int32_t iSequence)
	{
		return *((mstudioseqdesc_t * (__thiscall*)(void*, int32_t))(g_sdk.address_list.m_SequenceDescriptor))(this->GetStudioHdr(), iSequence);
	}

	__forceinline int32_t GetFirstSequenceAnimationTag(int32_t iSequence, int32_t iAnimationTag)
	{
		//if (!this)
		//	return 0;

		return ((int32_t(__thiscall*)(void*, int, int, int))(g_sdk.address_list.m_GetFirstSequenceAnimationTag))(this, iSequence, iAnimationTag, 1);
	}

	__forceinline float_t SequenceDuration(int32_t iSequence)
	{
		//if (!this)
		//	return 0.0f;

		return ((float_t(__thiscall*)(void*, int32_t))(g_sdk.address_list.m_SequenceDuration))(this, iSequence);
	}

	__forceinline void SetupBones_AttachmentHelper()
	{
		//if (!this)
		//	return;

		return ((void(__thiscall*)(LPVOID, LPVOID))(g_sdk.address_list.m_SetupBones_AttachmentHelper))(this, this->GetStudioHdr());
	}

	__forceinline int32_t GetSequenceActivity(int32_t iSequence)
	{
		//if (!this)
		//	return 0;

		studiohdr_t* pStudioHDR = g_interfaces.model_info->GetStudiomodel(this->GetModel());
		if (!pStudioHDR)
			return -1;

		return ((int32_t(__fastcall*)(void*, void*, int))(g_sdk.address_list.m_GetSequenceActivity))(this, pStudioHDR, iSequence);
	}

	__forceinline float_t GetMaxPlayerSpeed()
	{
		//if (!this)
		//	return 0.0f;

		float_t m_flMaxSpeed = 0.f;

		if (C_BaseCombatWeapon* pWeapon = this->m_hActiveWeapon().Get(); pWeapon)
		{
			if (C_CSWeaponData* pWeaponData = pWeapon->GetWeaponData(); pWeaponData)
				m_flMaxSpeed = this->m_bIsScoped() ? pWeaponData->m_flMaxPlayerSpeedAlt : pWeaponData->m_flMaxPlayerSpeed;
		}

		return m_flMaxSpeed;
	}

	__forceinline bool CanFire(int32_t ShiftAmount = 0, bool bCheckRevolver = false)
	{
		//if (!this)
		//	return false;

		C_BaseCombatWeapon* pCombatWeapon = this->m_hActiveWeapon().Get();
		if (!pCombatWeapon || (pCombatWeapon->m_iItemDefinitionIndex() != WEAPON_TASER && !pCombatWeapon->IsGun()))
			return true;

		float_t flServerTime = (this->m_nTickBase() - ShiftAmount) * g_interfaces.globals->m_intervalpertick;
		if (pCombatWeapon->m_iClip1() <= 0)
			return false;

		if (bCheckRevolver)
			if (pCombatWeapon->m_flPostponeFireReadyTime() >= flServerTime || pCombatWeapon->m_Activity() != 208)
				return false;

		if (this->m_flNextAttack() > flServerTime)
			return false;

		return pCombatWeapon->m_flNextPrimaryAttack() <= flServerTime;
	}

	Vector WorldSpaceCenter()
	{
		if (!this)
			return Vector(0, 0, 0);

		Vector vecOrigin = m_vecOrigin();

		Vector vecMins = this->GetCollideable()->OBBMins() + vecOrigin;
		Vector vecMaxs = this->GetCollideable()->OBBMaxs() + vecOrigin;

		Vector vecSize = vecMaxs - vecMins;
		vecSize /= 2.0f;
		vecSize += vecMins;
		return vecSize;
	}

	__forceinline int32_t& m_nFinalPredictedTick()
	{
		return *(int32_t*)((DWORD)(this) + 0x3434);
	}

	static C_BasePlayer* GetPlayerByIndex(int32_t index)
	{
		return static_cast <C_BasePlayer*> (g_interfaces.entity_list->GetClientEntity(index));
	}

	bool IsAlive()
	{
		/*
		* CCSGOPlayerAnimState::Update
		* ida ref: https://prnt.sc/26pb4g6
		* source: https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L253
		*/

		//if (!this)
		//	return false;

		return this->m_iHealth() > 0 && this->m_fLifeState() == LIFE_ALIVE;
	}

	typedef void(__thiscall* GetShootPosition_t)(LPVOID, Vector*);
	Vector GetShootPosition()
	{
		Vector vecShootPosition = Vector(0, 0, 0);
		if (!this)
			return vecShootPosition;

		GetVirtual < GetShootPosition_t >(this, 285)(this, &vecShootPosition);
		return vecShootPosition;
	}

	typedef void(__thiscall* Think_t)();
	void Think()
	{
		if (!this)
			return;

		GetVirtual<Think_t>(this, 139)();
	}

	typedef void(__thiscall* PreThink_t)();
	void PreThink()
	{
		if (!this)
			return;

		GetVirtual<PreThink_t>(this, 318)();
	}

	//using PhysicsRunThink_t = bool(__thiscall*)(void*, int);
	//bool PhysicsRunThink(int nThinkMethod)
	//{
	//	if (!this)
	//		return false;

	//	static auto oPhysicsRunThink = reinterpret_cast<PhysicsRunThink_t>(g_sdk.address_list.m_PhysicsRunThink);
	//	return oPhysicsRunThink(this, nThinkMethod);
	//}

	__forceinline void InvalidateBoneCache()
	{
		//if (!this)
		//	return;

		int32_t iModelBoneCounter = **(int32_t**)(((DWORD)(g_sdk.address_list.m_InvalidateBoneCache)) + 10);

		// [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]
		*(uintptr_t*)((DWORD)this + 0x2690) = iModelBoneCounter - 1;
		*(uintptr_t*)((DWORD)this + 0x2928) = 0xFF7FFFFF;
		*(uintptr_t*)((DWORD)this + 0x26AC) = 0;
		//*(uintptr_t*)((DWORD)this + 0x20) = 0;
	}

	__forceinline void ForceBoneCache()
	{
		//if (!this)
		//	return;

		int32_t iModelBoneCounter = **(int32_t**)(((DWORD)(g_sdk.address_list.m_InvalidateBoneCache)) + 10);
		*(uintptr_t*)((DWORD)this + 0x2690) = iModelBoneCounter;
	}

	CUSTOM_OFFSET(m_CachedBoneData, CUtlVector < matrix3x4_t >, FNV32("CachedBoneData"), 0x2914);
	CUSTOM_OFFSET(GetBoneAccessor, C_BoneAccessor, FNV32("BoneAccessor"), 0x26A8);
	CUSTOM_OFFSET(m_angVisualAngles, QAngle, FNV32("VisualAngles"), 0x31E8);
	CUSTOM_OFFSET(m_flSpawnTime, float_t, FNV32("SpawnTime"), 0x103C0);
	CUSTOM_OFFSET(GetMoveType, int32_t, FNV32("MoveType"), 0x25C);
	CUSTOM_OFFSET(m_nClientEffects, int32_t, FNV32("ClientEffects"), 0x68);
	CUSTOM_OFFSET(m_nLastSkipFramecount, int32_t, FNV32("LastSkipFramecount"), 0xA68);
	CUSTOM_OFFSET(m_nOcclusionFrame, int32_t, FNV32("OcclusionFrame"), 2608);
	CUSTOM_OFFSET(m_nOcclusionMask, int32_t, FNV32("OcclusionMask"), 2600);
	CUSTOM_OFFSET(m_pInverseKinematics, LPVOID, FNV32("InverseKinematics"), 9840);
	CUSTOM_OFFSET(m_bJiggleBones, bool, FNV32("JiggleBones"), 0x2930);
	CUSTOM_OFFSET(m_bMaintainSequenceTransition, bool, FNV32("MaintainSequenceTransition"), 0x9F0);

	C_StudioHDR* GetStudioHdr()
	{
		return *(C_StudioHDR**)((DWORD)(this) + 0x2950);
	}

	C_AnimationLayer* m_AnimationLayers()
	{
		return *(C_AnimationLayer**)((DWORD)(this) + 0x2990);
	}

	C_CSGOPlayerAnimationState* m_PlayerAnimStateCSGO()
	{
		/*
		* ida ref: https://prnt.sc/26pb622
		*/
		return *(C_CSGOPlayerAnimationState**)((DWORD)(this) + 0x9960);
	}

	std::array < float_t, 24 >& m_aPoseParameters()
	{
		return *(std::array < float_t, 24 >*)((uintptr_t)(this) + 0x2778);
	}
};