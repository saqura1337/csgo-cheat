#include "LocalAnimations.hpp"
#include "BoneManager.hpp"
#include "../Packet/PacketManager.hpp"
#include "../Prediction/EnginePrediction.hpp"

#include "../Settings.hpp"
#include "../SDK/Math/Math.hpp"
#include "../Exploits/Exploits.hpp"

void C_LocalAnimations::Instance()
{
	float_t flCurtime = g_interfaces.globals->m_curtime;
	float_t flRealTime = g_interfaces.globals->m_realtime;
	float_t flAbsFrameTime = g_interfaces.globals->m_absframetime;
	float_t flFrameTime = g_interfaces.globals->m_frametime;
	float_t flInterpolationAmount = g_interfaces.globals->m_interpolation_amount;
	float_t iTickCount = g_interfaces.globals->m_tickcount;
	float_t iFrameCount = g_interfaces.globals->m_framecount;

	g_sdk.local->m_fFlags() = g_PredictionSystem->GetNetvars(g_PacketManager->GetModifableCommand()->m_nCommand).m_fFlags;
	if (g_sdk.local->m_flSpawnTime() != g_sdk.local_data.m_flSpawnTime)
	{
		g_sdk.local_data.m_iFlags[0] = g_sdk.local_data.m_iFlags[1] = g_sdk.local->m_fFlags();
		g_sdk.local_data.m_iMoveType[0] = g_sdk.local_data.m_iMoveType[1] = g_sdk.local->GetMoveType();
		g_sdk.local_data.m_flSpawnTime = g_sdk.local->m_flSpawnTime();

		std::memcpy(&g_sdk.local_data.m_FakeAnimationState, g_sdk.local->m_PlayerAnimStateCSGO(), sizeof(C_CSGOPlayerAnimationState));
		std::memcpy(g_sdk.local_data.m_FakeAnimationLayers.data(), g_sdk.local->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
		std::memcpy(g_sdk.local_data.m_FakePoseParameters.data(), g_sdk.local->m_aPoseParameters().data(), sizeof(float_t) * 24);
	}

	int32_t iFlags = g_sdk.local->m_fFlags();
	float_t flLowerBodyYaw = g_sdk.local->m_flLowerBodyYaw();
	float_t flDuckSpeed = g_sdk.local->m_flDuckSpeed();
	float_t flDuckAmount = g_sdk.local->m_flDuckAmount();
	QAngle angVisualAngles = g_sdk.local->m_angVisualAngles();

	g_interfaces.globals->m_curtime = TICKS_TO_TIME(g_PacketManager->GetModifableCommand()->m_nTickCount);
	g_interfaces.globals->m_realtime = TICKS_TO_TIME(g_PacketManager->GetModifableCommand()->m_nTickCount);
	g_interfaces.globals->m_absframetime = g_interfaces.globals->m_intervalpertick;
	g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;
	g_interfaces.globals->m_tickcount = g_PacketManager->GetModifableCommand()->m_nTickCount;
	g_interfaces.globals->m_framecount = g_PacketManager->GetModifableCommand()->m_nTickCount;
	g_interfaces.globals->m_interpolation_amount = 0.0f;

	g_sdk.local->m_vecAbsVelocity() = g_sdk.local->m_vecVelocity();
	g_sdk.local->m_angVisualAngles() = g_PacketManager->GetModifableCommand()->m_angViewAngles;

	g_sdk.local->m_flThirdpersonRecoil() = g_sdk.local->m_aimPunchAngle().pitch * g_sdk.convars.m_WeaponRecoilScale->GetFloat();
	if (flRealTime - g_sdk.local_data.m_flShotTime <= 0.25f && g_cfg->m_bHoldFireAnimation)
		if (g_PacketManager->GetModifablePacket())
			g_sdk.local->m_angVisualAngles() = g_sdk.local_data.m_angForcedAngles;

	if (g_sdk.local_data.m_bDidShotAtChokeCycle)
		if (g_PacketManager->GetModifablePacket())
			g_sdk.local->m_angVisualAngles() = g_sdk.local_data.m_angShotChokedAngle;
	g_sdk.local->m_angVisualAngles().roll = 0.0f;

	g_sdk.local->m_flLowerBodyYaw() = g_sdk.local_data.m_flLowerBodyYaw;
	if (g_sdk.local->m_fFlags() & FL_FROZEN /*|| (*g_interfaces.game_rules)->IsFreezePeriod()*/)
		g_sdk.local->m_flLowerBodyYaw() = flLowerBodyYaw;

	this->DoAnimationEvent(0);

	bool bClientSideAnimation = g_sdk.local->m_bClientSideAnimation();
	g_sdk.local->m_bClientSideAnimation() = true;

	g_sdk.animation_data.m_bAnimationUpdate = true;
	g_sdk.local->UpdateClientSideAnimation();
	g_sdk.animation_data.m_bAnimationUpdate = false;

	g_sdk.local->m_bClientSideAnimation() = bClientSideAnimation;

	std::memcpy(g_sdk.local_data.m_PoseParameters.data(), g_sdk.local->m_aPoseParameters().data(), sizeof(float_t) * 24);
	std::memcpy(g_sdk.local_data.m_AnimationLayers.data(), g_sdk.local->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);

	if (g_sdk.local->m_PlayerAnimStateCSGO()->m_flVelocityLengthXY > 0.1f || fabs(g_sdk.local->m_PlayerAnimStateCSGO()->m_flVelocityLengthZ) > 100.0f)
	{
		g_sdk.local_data.m_flNextLowerBodyYawUpdateTime = flCurtime + 0.22f;
		if (g_sdk.local_data.m_flLowerBodyYaw != Math::NormalizeAngle(g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw))
			g_sdk.local_data.m_flLowerBodyYaw = g_sdk.local->m_flLowerBodyYaw() = Math::NormalizeAngle(g_sdk.local->m_PlayerAnimStateCSGO()->m_flEyeYaw);
	}
	else if (flCurtime > g_sdk.local_data.m_flNextLowerBodyYawUpdateTime)
	{
		if (float_t flAngleDifference = Math::AngleDiff(Math::NormalizeAngle(g_sdk.local->m_PlayerAnimStateCSGO()->m_flFootYaw), Math::NormalizeAngle(g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw)); 
			fabsf(flAngleDifference) > 35.0f)
		{
			g_sdk.local_data.m_flNextLowerBodyYawUpdateTime = flCurtime + 1.1f;
			if (g_sdk.local_data.m_flLowerBodyYaw != Math::NormalizeAngle(g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw))
				g_sdk.local_data.m_flLowerBodyYaw = g_sdk.local->m_flLowerBodyYaw() = Math::NormalizeAngle(g_PacketManager->GetModifableCommand()->m_angViewAngles.yaw);
		}
	}

	if (g_PacketManager->GetModifablePacket())
	{
		C_CSGOPlayerAnimationState AnimationState;
		std::memcpy(&AnimationState, g_sdk.local->m_PlayerAnimStateCSGO(), sizeof(C_CSGOPlayerAnimationState));

		// sorry, static legs func. work only on client
		if (g_cfg->m_bStaticLegs)	
			g_sdk.local->m_aPoseParameters()[6] = 1; // [JUMP_FALL]
		
		// hideshots matrix
		//bool bShouldSetupMatrix = true;
		//if ( g_ExploitSystem->GetActiveExploit( ) == HIDESHOTS )
		//	if ( g_ExploitSystem->GetShiftCommand( ) == g_PacketManager->GetModifableCommand( )->m_nCommand )
		//		bShouldSetupMatrix = false;

		g_BoneManager->BuildMatrix(g_sdk.local, g_sdk.local_data.m_aMainBones.data(), false);

		// desync layers
		std::memcpy(g_sdk.local->m_AnimationLayers(), GetFakeAnimationLayers().data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
		std::memcpy(g_sdk.local->m_PlayerAnimStateCSGO(), &g_sdk.local_data.m_FakeAnimationState, sizeof(C_CSGOPlayerAnimationState));
		std::memcpy(g_sdk.local->m_aPoseParameters().data(), g_sdk.local_data.m_FakePoseParameters.data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);

		// desync update
		int32_t iSimulationTicks = g_interfaces.client_state->m_nChokedCommands() + 1;
		for (int32_t iSimulationTick = 1; iSimulationTick <= iSimulationTicks; iSimulationTick++)
		{
			int32_t iTickCount = g_PacketManager->GetModifableCommand()->m_nTickCount - (iSimulationTicks - iSimulationTick);
			g_interfaces.globals->m_curtime = TICKS_TO_TIME(iTickCount);
			g_interfaces.globals->m_realtime = TICKS_TO_TIME(iTickCount);
			g_interfaces.globals->m_absframetime = g_interfaces.globals->m_intervalpertick;
			g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;
			g_interfaces.globals->m_tickcount = iTickCount;
			g_interfaces.globals->m_framecount = iTickCount;

			g_sdk.local->m_vecAbsVelocity() = g_sdk.local->m_vecVelocity();
			g_sdk.local->m_flThirdpersonRecoil() = g_sdk.local->m_aimPunchAngle().pitch * g_sdk.convars.m_WeaponRecoilScale->GetFloat();

			g_sdk.local->m_angVisualAngles() = g_PacketManager->GetFakeAngles();
			if ((iSimulationTicks - iSimulationTick) < 1)
			{
				if (flRealTime - g_sdk.local_data.m_flShotTime <= 0.25f && g_cfg->m_bHoldFireAnimation)
					g_sdk.local->m_angVisualAngles() = g_sdk.local_data.m_angForcedAngles;

				if (g_sdk.local_data.m_bDidShotAtChokeCycle)
					g_sdk.local->m_angVisualAngles() = g_sdk.local_data.m_angShotChokedAngle;

				g_sdk.local->m_angVisualAngles().roll = 0.0f;
			}

			this->DoAnimationEvent(1);

			bool bClientSideAnimation = g_sdk.local->m_bClientSideAnimation();
			g_sdk.local->m_bClientSideAnimation() = true;

			g_sdk.animation_data.m_bAnimationUpdate = true;
			g_sdk.local->UpdateClientSideAnimation();
			g_sdk.animation_data.m_bAnimationUpdate = false;

			g_sdk.local->m_bClientSideAnimation() = bClientSideAnimation;
		}

		// build desync matrix
		g_BoneManager->BuildMatrix(g_sdk.local, g_sdk.local_data.m_aDesyncBones.data(), false);

		// copy lag matrix
		std::memcpy(g_sdk.local_data.m_aLagBones.data(), g_sdk.local_data.m_aDesyncBones.data(), sizeof(matrix3x4_t) * MAXSTUDIOBONES);

		// save layers
		std::memcpy(&g_sdk.local_data.m_FakeAnimationState, g_sdk.local->m_PlayerAnimStateCSGO(), sizeof(C_CSGOPlayerAnimationState));
		std::memcpy(g_sdk.local_data.m_FakeAnimationLayers.data(), g_sdk.local->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
		std::memcpy(g_sdk.local_data.m_FakePoseParameters.data(), g_sdk.local->m_aPoseParameters().data(), sizeof(float_t) * 24);

		// restore layers
		std::memcpy(g_sdk.local->m_AnimationLayers(), GetAnimationLayers().data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
		std::memcpy(g_sdk.local->m_PlayerAnimStateCSGO(), &AnimationState, sizeof(C_CSGOPlayerAnimationState));
		std::memcpy(g_sdk.local->m_aPoseParameters().data(), g_sdk.local_data.m_PoseParameters.data(), sizeof(float_t) * 24);

		// move matrixes
		for (int i = 0; i < MAXSTUDIOBONES; i++)
			g_sdk.local_data.m_vecBoneOrigins[i] = g_sdk.local->GetAbsOrigin() - g_sdk.local_data.m_aMainBones[i].GetOrigin();

		for (int i = 0; i < MAXSTUDIOBONES; i++)
			g_sdk.local_data.m_vecFakeBoneOrigins[i] = g_sdk.local->GetAbsOrigin() - g_sdk.local_data.m_aDesyncBones[i].GetOrigin();

		if (g_cfg->m_bStaticLegs)
		{
			g_sdk.local->m_aPoseParameters()[6] = 1;
		}

		// reset angles
		g_sdk.local_data.m_bDidShotAtChokeCycle = false;
		g_sdk.local_data.m_angShotChokedAngle = QAngle(0, 0, 0);
	}

	g_sdk.local->m_fFlags() = iFlags;
	g_sdk.local->m_flDuckAmount() = flDuckAmount;
	g_sdk.local->m_flDuckSpeed() = flDuckSpeed;
	g_sdk.local->m_flLowerBodyYaw() = flLowerBodyYaw;
	g_sdk.local->m_angVisualAngles() = angVisualAngles;

	// restore globals
	g_interfaces.globals->m_curtime = flCurtime;
	g_interfaces.globals->m_realtime = flRealTime;
	g_interfaces.globals->m_absframetime = flAbsFrameTime;
	g_interfaces.globals->m_frametime = flFrameTime;
	g_interfaces.globals->m_tickcount = iTickCount;
	g_interfaces.globals->m_framecount = iFrameCount;
	g_interfaces.globals->m_interpolation_amount = flInterpolationAmount;
}

void C_LocalAnimations::SetupShootPosition()
{
	std::memcpy(g_sdk.local->m_AnimationLayers(), g_LocalAnimations->GetAnimationLayers().data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(g_sdk.local->m_aPoseParameters().data(), g_sdk.local_data.m_PoseParameters.data(), sizeof(float_t) * 24);

	float flOldBodyPitch = g_sdk.local->m_aPoseParameters()[12];
	Vector vecOldOrigin = g_sdk.local->GetAbsOrigin();

	g_sdk.local->SetAbsoluteAngles(QAngle(0.0f, g_sdk.local->m_PlayerAnimStateCSGO()->m_flFootYaw, 0.0f));
	g_sdk.local->SetAbsoluteOrigin(g_sdk.local->m_vecOrigin());

	matrix3x4_t aMatrix[MAXSTUDIOBONES];

	g_sdk.local->m_aPoseParameters()[12] = (g_sdk.local->m_angEyeAngles().pitch + 89.0f) / 178.0f;
	g_BoneManager->BuildMatrix(g_sdk.local, aMatrix, true);
	g_sdk.local->m_aPoseParameters()[12] = flOldBodyPitch;

	g_sdk.local->SetAbsoluteOrigin(vecOldOrigin);
	std::memcpy(g_sdk.local->m_CachedBoneData().Base(), aMatrix, sizeof(matrix3x4_t) * g_sdk.local->m_CachedBoneData().Count());

	g_sdk.local->ForceBoneCache();
	g_sdk.local_data.m_vecShootPosition = g_sdk.local->GetShootPosition();
}

bool C_LocalAnimations::GetCachedMatrix(matrix3x4_t* aMatrix)
{
	std::memcpy(aMatrix, g_sdk.local_data.m_aMainBones.data(), sizeof(matrix3x4_t) * g_sdk.local->m_CachedBoneData().Count());
	return true;
}

std::array < matrix3x4_t, 128 > C_LocalAnimations::GetDesyncMatrix()
{
	return g_sdk.local_data.m_aDesyncBones;
}

std::array < matrix3x4_t, 128 > C_LocalAnimations::GetLagMatrix()
{
	return g_sdk.local_data.m_aLagBones;
}

void C_LocalAnimations::DoAnimationEvent(int type)
{
	if (/*(*(g_interfaces.game_rules))->IsFreezePeriod()* ||*/ (g_sdk.local->m_fFlags() & FL_FROZEN))
	{
		g_sdk.local_data.m_iMoveType[type] = MOVETYPE_NONE;
		g_sdk.local_data.m_iFlags[type] = FL_ONGROUND;
	}

	C_AnimationLayer* pLandOrClimbLayer = &g_sdk.local->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	if (!pLandOrClimbLayer)
		return;

	C_AnimationLayer* pJumpOrFallLayer = &g_sdk.local->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	if (!pJumpOrFallLayer)
		return;

	if (g_sdk.local_data.m_iMoveType[type] != MOVETYPE_LADDER && g_sdk.local->GetMoveType() == MOVETYPE_LADDER)
		g_sdk.local->m_PlayerAnimStateCSGO()->SetLayerSequence(pLandOrClimbLayer, ACT_CSGO_CLIMB_LADDER);
	else if (g_sdk.local_data.m_iMoveType[type] == MOVETYPE_LADDER && g_sdk.local->GetMoveType() != MOVETYPE_LADDER)
		g_sdk.local->m_PlayerAnimStateCSGO()->SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_FALL);
	else
	{
		if (g_sdk.local->m_fFlags() & FL_ONGROUND)
		{
			if (!(g_sdk.local_data.m_iFlags[type] & FL_ONGROUND))
				g_sdk.local->m_PlayerAnimStateCSGO()->SetLayerSequence(pLandOrClimbLayer, g_sdk.local->m_PlayerAnimStateCSGO()->m_flDurationInAir > 1.0f && type == 0 ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT);
		}
		else if (g_sdk.local_data.m_iFlags[type] & FL_ONGROUND)
		{
			if (g_sdk.local->m_vecVelocity().z > 0.0f)
				g_sdk.local->m_PlayerAnimStateCSGO()->SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_JUMP);
			else
				g_sdk.local->m_PlayerAnimStateCSGO()->SetLayerSequence(pJumpOrFallLayer, ACT_CSGO_FALL);
		}
	}

	g_sdk.local_data.m_iMoveType[type] = g_sdk.local->GetMoveType();
	g_sdk.local_data.m_iFlags[type] = g_PredictionSystem->GetNetvars(g_PacketManager->GetModifableCommand()->m_nCommand).m_fFlags;
}

void C_LocalAnimations::OnUpdateClientSideAnimation()
{
	for (int i = 0; i < MAXSTUDIOBONES; i++)
		g_sdk.local_data.m_aMainBones[i].SetOrigin(g_sdk.local->GetAbsOrigin() - g_sdk.local_data.m_vecBoneOrigins[i]);

	for (int i = 0; i < MAXSTUDIOBONES; i++)
		g_sdk.local_data.m_aDesyncBones[i].SetOrigin(g_sdk.local->GetAbsOrigin() - g_sdk.local_data.m_vecFakeBoneOrigins[i]);

	std::memcpy(g_sdk.local->m_CachedBoneData().Base(), g_sdk.local_data.m_aMainBones.data(), sizeof(matrix3x4_t) * g_sdk.local->m_CachedBoneData().Count());
	std::memcpy(g_sdk.local->GetBoneAccessor().GetBoneArrayForWrite(), g_sdk.local_data.m_aMainBones.data(), sizeof(matrix3x4_t) * g_sdk.local->m_CachedBoneData().Count());

	return g_sdk.local->SetupBones_AttachmentHelper();
}

std::array< C_AnimationLayer, 13 > C_LocalAnimations::GetAnimationLayers()
{
	std::array< C_AnimationLayer, 13 > aOutput;

	std::memcpy(aOutput.data(), g_sdk.local->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(&aOutput.at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL), &g_sdk.local_data.m_AnimationLayers.at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL), sizeof(C_AnimationLayer));
	std::memcpy(&aOutput.at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB), &g_sdk.local_data.m_AnimationLayers.at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB), sizeof(C_AnimationLayer));
	std::memcpy(&aOutput.at(ANIMATION_LAYER_ALIVELOOP), &g_sdk.local_data.m_AnimationLayers.at(ANIMATION_LAYER_ALIVELOOP), sizeof(C_AnimationLayer));
	std::memcpy(&aOutput.at(ANIMATION_LAYER_LEAN), &g_sdk.local_data.m_AnimationLayers.at(ANIMATION_LAYER_LEAN), sizeof(C_AnimationLayer));

	return aOutput;
}

std::array< C_AnimationLayer, 13 > C_LocalAnimations::GetFakeAnimationLayers()
{
	std::array< C_AnimationLayer, 13 > aOutput;

	std::memcpy(aOutput.data(), g_sdk.local->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(&aOutput.at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL), &g_sdk.local_data.m_FakeAnimationLayers.at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL), sizeof(C_AnimationLayer));
	std::memcpy(&aOutput.at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB), &g_sdk.local_data.m_FakeAnimationLayers.at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB), sizeof(C_AnimationLayer));
	std::memcpy(&aOutput.at(ANIMATION_LAYER_ALIVELOOP), &g_sdk.local_data.m_FakeAnimationLayers.at(ANIMATION_LAYER_ALIVELOOP), sizeof(C_AnimationLayer));
	std::memcpy(&aOutput.at(ANIMATION_LAYER_LEAN), &g_sdk.local_data.m_FakeAnimationLayers.at(ANIMATION_LAYER_LEAN), sizeof(C_AnimationLayer));

	return aOutput;
}

void C_LocalAnimations::ResetData()
{
	g_sdk.local_data.m_aDesyncBones = { };
	g_sdk.local_data.m_aMainBones = { };

	g_sdk.local_data.m_vecNetworkedOrigin = Vector(0, 0, 0);
	g_sdk.local_data.m_angShotChokedAngle = QAngle(0, 0, 0);
	g_sdk.local_data.m_vecBoneOrigins.fill(Vector(0, 0, 0));
	g_sdk.local_data.m_vecFakeBoneOrigins.fill(Vector(0, 0, 0));

	g_sdk.local_data.m_bDidShotAtChokeCycle = false;

	g_sdk.local_data.m_AnimationLayers.fill(C_AnimationLayer());
	g_sdk.local_data.m_FakeAnimationLayers.fill(C_AnimationLayer());

	g_sdk.local_data.m_PoseParameters.fill(0.0f);
	g_sdk.local_data.m_FakePoseParameters.fill(0.0f);

	g_sdk.local_data.m_flShotTime = 0.0f;
	g_sdk.local_data.m_angForcedAngles = QAngle(0, 0, 0);

	g_sdk.local_data.m_flLowerBodyYaw = 0.0f;
	g_sdk.local_data.m_flNextLowerBodyYawUpdateTime = 0.0f;
	g_sdk.local_data.m_flSpawnTime = 0.0f;

	g_sdk.local_data.m_iFlags[0] = g_sdk.local_data.m_iFlags[0] = 0;
	g_sdk.local_data.m_iMoveType[0] = g_sdk.local_data.m_iMoveType[1] = 0;
}