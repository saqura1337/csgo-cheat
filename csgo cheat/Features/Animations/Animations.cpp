#include "Animations.hpp"
#include "BoneManager.hpp"
#include "../SDK/Math/Math.hpp"

void C_AnimationSync::Instance(ClientFrameStage_t Stage)
{
	if (Stage != ClientFrameStage_t::FRAME_NET_UPDATE_END)
		return;

	for (int32_t iPlayerID = 1; iPlayerID <= g_interfaces.globals->m_maxclients; iPlayerID++)
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(iPlayerID);
		if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->m_iTeamNum() == g_sdk.local->m_iTeamNum())
		{
			g_sdk.resolver_data.m_AnimResoled[iPlayerID] = false;
			g_sdk.resolver_data.m_MissedShots[iPlayerID] = 0;
			g_sdk.resolver_data.m_LastMissedShots[iPlayerID] = 0;

			continue;
		}

		bool bHasPreviousRecord = false;
		if (pPlayer->m_flOldSimulationTime() >= pPlayer->m_flSimulationTime())
		{
			if (pPlayer->m_flOldSimulationTime() > pPlayer->m_flSimulationTime())
				this->UnmarkAsDormant(iPlayerID);

			continue;
		}

		auto& LagRecords = g_sdk.m_CachedPlayerRecords[iPlayerID];
		if (LagRecords.empty())
			continue;

		C_LagRecord PreviousRecord = m_PreviousRecord[iPlayerID];
		if (TIME_TO_TICKS(fabs(pPlayer->m_flSimulationTime() - PreviousRecord.m_SimulationTime)) <= 17)
			bHasPreviousRecord = true;

		C_LagRecord& LatestRecord = LagRecords.back();
		if (this->HasLeftOutOfDormancy(iPlayerID))
			bHasPreviousRecord = false;

		if (LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flCycle == PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flCycle)
		{
			pPlayer->m_flSimulationTime() = pPlayer->m_flOldSimulationTime();
			continue;
		}

		LatestRecord.m_UpdateDelay = TIME_TO_TICKS(pPlayer->m_flSimulationTime() - this->GetPreviousRecord(iPlayerID).m_SimulationTime);
		if (LatestRecord.m_UpdateDelay > 17)
			LatestRecord.m_UpdateDelay = 1;

		C_PlayerInfo PlayerInfo;
		g_interfaces.engine->GetPlayerInfo(iPlayerID, &PlayerInfo);

		if (PlayerInfo.m_bIsFakePlayer || LatestRecord.m_UpdateDelay < 1)
			LatestRecord.m_UpdateDelay = 1;

		this->RecalculateVelocity(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord);

		// ������� ��������� ������
		std::array < C_AnimationLayer, ANIMATION_LAYER_COUNT > AnimationLayers;
		std::array < float_t, MAXSTUDIOPOSEPARAM > PoseParameters;
		C_CSGOPlayerAnimationState AnimationState;

		// ������� ������
		memcpy(AnimationLayers.data(), pPlayer->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
		memcpy(PoseParameters.data(), pPlayer->m_aPoseParameters().data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);
		memcpy(&AnimationState, pPlayer->m_PlayerAnimStateCSGO(), sizeof(AnimationState));

		// �������� �������� � ���� ����
		//pPlayer->UpdateSetupVelocityAndSetupLean(AnimationState.m_flFootYaw, AnimationState.m_flEyePitch);

		// �������� ������ ��� �����������
		for (int32_t i = ROTATE_LEFT; i <= ROTATE_LOW_RIGHT; i++)
		{
			// �������� ������ ���� ����
			this->UpdatePlayerAnimations(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, i);

			// ��������� ��������� ������
			memcpy(LatestRecord.m_AnimationLayers.at(i).data(), pPlayer->m_AnimationLayers(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);

			// ������� ����� �������
			memcpy(pPlayer->m_AnimationLayers(), AnimationLayers.data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);

			// ������� �����
			if (i < ROTATE_LOW_LEFT)
				g_BoneManager->BuildMatrix(pPlayer, LatestRecord.m_Matricies[i].data(), true);

			// �������� ��������� ������
			memcpy(pPlayer->m_aPoseParameters().data(), PoseParameters.data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);
			memcpy(pPlayer->m_PlayerAnimStateCSGO(), &AnimationState, sizeof(AnimationState));
		}

		// �������� ���������
		//this->ResolveInit(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord);

		if (!LatestRecord.m_bIsShooting)
		{
			if (LatestRecord.m_UpdateDelay > 1 /*&& !(pPlayer->GetSequenceActivity(LatestRecord.m_AnimationLayers.at(ROTATE_LEFT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_nSequence) == ANIMATION_LAYER_WEAPON_ACTION)*/)
			{
				// if player on ground
				bool bOnGround = pPlayer->m_PlayerAnimStateCSGO()->m_bOnGround && pPlayer->m_PlayerAnimStateCSGO()->m_flDurationInAir == 0.f;
				float_t flLowerBodyTimer = 0.f;

				// get player angle
				float_t flFeetDelta = Math::AngleNormalize(Math::AngleDiff(Math::AngleNormalize(pPlayer->m_flLowerBodyYaw()), Math::AngleNormalize(pPlayer->m_angEyeAngles().yaw)));
				float_t flDelta = Math::AngleDiff(pPlayer->m_angEyeAngles().yaw, pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw);

				if (pPlayer->m_PlayerAnimStateCSGO()->m_vecVelocity.Length2D() > 1.0f && bOnGround) // moving
				{
					float_t flLeftDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LEFT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
					float_t flLowLeftDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LOW_LEFT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
					float_t flLowRightDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LOW_RIGHT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
					float_t flRightDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_RIGHT).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);
					float_t flCenterDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_CENTER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate);

					LatestRecord.m_bAnimResolved = false;
					{
						if (flLeftDelta > flCenterDelta && flLeftDelta > flLowLeftDelta)
							LatestRecord.m_RotationMode = ROTATE_LEFT;

						if (flRightDelta > flCenterDelta && flRightDelta > flLowRightDelta)
							LatestRecord.m_RotationMode = ROTATE_RIGHT;

						if (flLowLeftDelta > flLeftDelta && flLowLeftDelta > flCenterDelta)
							LatestRecord.m_RotationMode = ROTATE_LOW_LEFT;

						if (flLowRightDelta > flRightDelta && flLowRightDelta > flCenterDelta)
							LatestRecord.m_RotationMode = ROTATE_LOW_RIGHT;
					}
					LatestRecord.m_bAnimResolved = true;

					pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::ApproachAngle(
						pPlayer->m_angEyeAngles().yaw,
						pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw,
						((pPlayer->m_PlayerAnimStateCSGO()->m_flWalkToRunTransition * 20.0f) + 30.0f)
						* pPlayer->m_PlayerAnimStateCSGO()->m_flLastUpdateTime);

					//if (pPlayer->m_angEyeAngles().roll >= 45.f)
					//	pPlayer->m_angEyeAngles().roll = 0.f;

					if (bOnGround && (g_interfaces.globals->m_curtime > flLowerBodyTimer && flFeetDelta > 35.f))
					{
						flLowerBodyTimer = g_interfaces.globals->m_curtime + (1.1f * 0.2f); // synchronize with CSGO_ANIM_LOWER_REALIGN_DELAY
						pPlayer->m_flLowerBodyYaw() = pPlayer->m_angEyeAngles().yaw;
					}

					if (LatestRecord.m_bAnimResolved && LatestRecord.m_RotationMode != ROTATE_SERVER)
						g_sdk.resolver_data.m_LastBruteSide[iPlayerID] = LatestRecord.m_RotationMode;

					if (LatestRecord.m_RotationMode == ROTATE_SERVER)
						LatestRecord.m_RotationMode = g_sdk.resolver_data.m_LastBruteSide[iPlayerID];
				}

				//else if (!bOnGround)
				//{
				//	// ufff yaaa
				//	float_t flLeftDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LEFT).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate);
				//	float_t flLowLeftDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LOW_LEFT).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate);
				//	float_t flLowRightDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_LOW_RIGHT).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate);
				//	float_t flRightDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_RIGHT).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate);
				//	float_t flCenterDelta = fabsf(LatestRecord.m_AnimationLayers.at(ROTATE_CENTER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate - LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate);

				//	LatestRecord.m_bAnimResolved = false;
				//	{
				//		if (flLeftDelta > flCenterDelta && flLeftDelta > flLowLeftDelta)
				//			LatestRecord.m_RotationMode = ROTATE_LEFT;

				//		if (flRightDelta > flCenterDelta && flRightDelta > flLowRightDelta)
				//			LatestRecord.m_RotationMode = ROTATE_RIGHT;

				//		if (flLowLeftDelta > flLeftDelta && flLowLeftDelta > flCenterDelta)
				//			LatestRecord.m_RotationMode = ROTATE_LOW_LEFT;

				//		if (flLowRightDelta > flRightDelta && flLowRightDelta > flCenterDelta)
				//			LatestRecord.m_RotationMode = ROTATE_LOW_RIGHT;
				//	}
				//	LatestRecord.m_bAnimResolved = true;

				//	//if (pPlayer->m_angEyeAngles().roll >= 45.f)
				//	//	pPlayer->m_angEyeAngles().roll = 0.f;

				//	if (LatestRecord.m_bAnimResolved && LatestRecord.m_RotationMode != ROTATE_SERVER)
				//		g_sdk.resolver_data.m_LastBruteSide[iPlayerID] = LatestRecord.m_RotationMode;

				//	if (LatestRecord.m_RotationMode == ROTATE_SERVER)
				//		LatestRecord.m_RotationMode = g_sdk.resolver_data.m_LastBruteSide[iPlayerID];
				//}
				else if (bOnGround && pPlayer->m_PlayerAnimStateCSGO()->m_vecVelocity.Length() < 1.0f)
				{
					// the player has just moved from a state of RUNNING to a state of STANDING
					bool bStoppedMoving = pPlayer->GetSequenceActivity(LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_nSequence) == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING;

					//if player has delta > 120 (max_desync_delta);
					bool bBalanceAdjust = LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_flCycle < 1.f
						&& LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_flWeight > 0.01f
						&& pPlayer->GetSequenceActivity(LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_nSequence) == ACT_CSGO_IDLE_TURN_BALANCEADJUST;

					// get balance adjust
					// thanks to my homies llama & valve
					// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/cstrike15/csgo_playeranimstate.cpp#L2380
					if (bOnGround && !pPlayer->m_PlayerAnimStateCSGO()->m_flLadderWeight && !pPlayer->m_PlayerAnimStateCSGO()->m_flLandAnimMultiplier)
					{
						if (pPlayer->m_PlayerAnimStateCSGO()->m_flLastUpdateIncrement > 0.0)
						{
							if (pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw < pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw)
							{
								if (flDelta >= 180.0)
									flDelta = flDelta - 360.0;
							}
							else if (flDelta <= -180.0)
								flDelta = flDelta + 360.0;

							if ((flFeetDelta / pPlayer->m_PlayerAnimStateCSGO()->m_flLastUpdateIncrement) > 120.0)
							{
								LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_flCycle = 0.0;
								LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_flWeight = 0.0;
								bBalanceAdjust = true;
							}
						}
					}

					if (bStoppedMoving && bBalanceAdjust && g_sdk.resolver_data.m_LastBruteSide[iPlayerID] < 0)
					{
						if (flFeetDelta > 0.0f)
							LatestRecord.m_RotationMode = ROTATE_LEFT;
						else
							LatestRecord.m_RotationMode = ROTATE_RIGHT;

						//if (LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_flWeight == 0.0f
						//	&& LatestRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ADJUST).m_flCycle == 0.0f)
						//{
						//	LatestRecord.m_RotationMode = std::clamp((2 * (flDelta <= 0.f) - 1), 1, 3);
						//}

						g_sdk.resolver_data.m_LastBruteSide[iPlayerID] = LatestRecord.m_RotationMode;
					}

					pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw = Math::ApproachAngle(
						pPlayer->m_flLowerBodyYaw(),
						pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw,
						pPlayer->m_PlayerAnimStateCSGO()->m_flLastUpdateTime * 100.0f);

					//if (pPlayer->m_angEyeAngles().roll >= 45.f)
					//	pPlayer->m_angEyeAngles().roll = 0.f;

					if (bOnGround && g_interfaces.globals->m_curtime > flLowerBodyTimer && flFeetDelta > 35.0f)
					{
						flLowerBodyTimer = g_interfaces.globals->m_curtime + 1.1f;
						pPlayer->m_flLowerBodyYaw() = pPlayer->m_angEyeAngles().yaw;
					}
				}

				g_sdk.resolver_data.m_AnimResoled[iPlayerID] = LatestRecord.m_bAnimResolved;

				// change current angle, if shot missed
				if (LatestRecord.m_RotationMode == g_sdk.resolver_data.m_BruteSide[iPlayerID])
				{
					if (g_sdk.resolver_data.m_MissedShots[iPlayerID] > 0)
					{
						int iNewRotation = 0;
						switch (LatestRecord.m_RotationMode)
						{
						case ROTATE_LEFT: iNewRotation = ROTATE_RIGHT; break;
						case ROTATE_RIGHT: iNewRotation = ROTATE_LOW_RIGHT; break;
						case ROTATE_LOW_RIGHT: iNewRotation = ROTATE_LOW_LEFT; break;
						case ROTATE_LOW_LEFT: iNewRotation = ROTATE_LEFT; break;
						}

						LatestRecord.m_RotationMode = iNewRotation;
					}
				}

				this->UpdatePlayerAnimations(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, LatestRecord.m_RotationMode);
			}
			else
				this->UpdatePlayerAnimations(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, ROTATE_SERVER);
		}
		else
			this->UpdatePlayerAnimations(pPlayer, LatestRecord, PreviousRecord, bHasPreviousRecord, ROTATE_SERVER);

		// write more information
		// https://www.unknowncheats.me/forum/3364562-post11.html

		// ������ ���������� �����
		memcpy(pPlayer->m_AnimationLayers(), AnimationLayers.data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);

		// ������ ����
		memcpy(LatestRecord.m_PoseParameters.data(), pPlayer->m_aPoseParameters().data(), sizeof(float_t) * MAXSTUDIOPOSEPARAM);

		// ������� �����
		g_BoneManager->BuildMatrix(pPlayer, LatestRecord.m_Matricies[ROTATE_SERVER].data(), false);

		// ������ �����
		for (int i = 0; i < MAXSTUDIOBONES; i++)
			m_BoneOrigins[iPlayerID][i] = pPlayer->GetAbsOrigin() - LatestRecord.m_Matricies[ROTATE_SERVER][i].GetOrigin();

		// �������� �����
		memcpy(m_CachedMatrix[iPlayerID].data(), LatestRecord.m_Matricies[ROTATE_SERVER].data(), sizeof(matrix3x4_t) * MAXSTUDIOBONES);

		// ����� ����� � ��������
		this->UnmarkAsDormant(iPlayerID);
	}
}
void C_AnimationSync::RecalculateVelocity(C_BasePlayer* pPlayer, C_LagRecord& LagRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord)
{
	Vector vecVelocity = LagRecord.m_Velocity;
	float_t flWeightSpeed = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flWeight;

	if (LagRecord.m_Flags & FL_ONGROUND
		&& LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flWeight > 0.0f
		&& LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flWeight < 1.0f)
	{
		if (float_t flValue = (1.0f - LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flWeight) * 0.35f; flValue > 0.0f && flValue < 1.0f)
			LagRecord.m_AnimationSpeed = flValue + 0.55f;
		else
			LagRecord.m_AnimationSpeed = -1.f;
	}

	if (!this->HasLeftOutOfDormancy(pPlayer->EntIndex()) && bHasPreviousRecord)
	{
		//	calculate new velocity based on (new_origin - old_origin) / (new_time - old_time) formula.
		if (LagRecord.m_UpdateDelay > 1 && LagRecord.m_UpdateDelay <= 20)
			vecVelocity = (LagRecord.m_Origin - PreviousRecord.m_Origin) * (1.0f / TICKS_TO_TIME(LagRecord.m_UpdateDelay));

		if (abs(vecVelocity.x) < 0.001f)
			vecVelocity.x = 0.0f;
		if (abs(vecVelocity.y) < 0.001f)
			vecVelocity.y = 0.0f;
		if (abs(vecVelocity.z) < 0.001f)
			vecVelocity.z = 0.0f;

		if (_fdtest(&vecVelocity.x) > 0
			|| _fdtest(&vecVelocity.y) > 0
			|| _fdtest(&vecVelocity.z) > 0)
			vecVelocity.Zero();

		float_t flCurrentDirection = RAD2DEG(atan2f(vecVelocity.y, vecVelocity.x));
		float_t flPreviousDirection = RAD2DEG(atan2f(PreviousRecord.m_Velocity.y, PreviousRecord.m_Velocity.x));

		if (LagRecord.m_Flags & FL_ONGROUND
			&& vecVelocity.Length2D() >= 0.1f
			&& abs(Math::NormalizeAngle(flCurrentDirection - flPreviousDirection)) < 1.0f
			&& abs(LagRecord.m_DuckAmount - PreviousRecord.m_DuckAmount) <= 0.0f
			&& LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate > PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate
			&& flWeightSpeed > PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flWeight)
		{
			if (flWeightSpeed <= 0.7f && flWeightSpeed > 0.0f)
			{
				if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate == 0.0f)
					vecVelocity.Zero();
				else
				{
					if (vecVelocity.Length2D() != 0.0f)
					{
						float_t flMaxSpeedMultiply = 1;
						if (LagRecord.m_Flags & 6)
							flMaxSpeedMultiply = 0.34f;
						else if (pPlayer->m_bIsWalking())
							flMaxSpeedMultiply = 0.52f;

						vecVelocity.x = (vecVelocity.x / vecVelocity.Length2D()) * (flWeightSpeed * (pPlayer->GetMaxPlayerSpeed() * flMaxSpeedMultiply));
						vecVelocity.y = (vecVelocity.y / vecVelocity.Length2D()) * (flWeightSpeed * (pPlayer->GetMaxPlayerSpeed() * flMaxSpeedMultiply));
					}
				}
			}
		}

		if (LagRecord.m_Flags & FL_ONGROUND && vecVelocity.Length2D() > 0.1f && LagRecord.m_UpdateDelay > 1)
		{
			if (LagRecord.m_AnimationSpeed > 0)
			{
				if (pPlayer->m_hActiveWeapon().Get())
				{
					vecVelocity.x *= (LagRecord.m_AnimationSpeed * pPlayer->GetMaxPlayerSpeed()) / vecVelocity.Length2D();
					vecVelocity.y *= (LagRecord.m_AnimationSpeed * pPlayer->GetMaxPlayerSpeed()) / vecVelocity.Length2D();
				}
			}
		}

		if (!(LagRecord.m_Flags & FL_ONGROUND))
		{
			vecVelocity = (LagRecord.m_Origin - this->GetPreviousRecord(pPlayer->EntIndex()).m_Origin) * (1.0f / TICKS_TO_TIME(LagRecord.m_UpdateDelay));

			float_t flWeight = 1.0f - LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flWeight;
			if (flWeight > 0.0f)
			{
				float_t flPreviousRate = this->GetPreviousRecord(pPlayer->EntIndex()).m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flPlaybackRate;
				float_t flCurrentRate = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_flPlaybackRate;

				if (flPreviousRate == flCurrentRate)
				{
					int32_t iPreviousSequence = this->GetPreviousRecord(pPlayer->EntIndex()).m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_nSequence;
					int32_t iCurrentSequence = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_ALIVELOOP).m_nSequence;

					if (iPreviousSequence == iCurrentSequence)
					{
						float_t flSpeedNormalized = (flWeight / 2.8571432f) + 0.55f;
						if (flSpeedNormalized > 0.0f)
						{
							float_t flSpeed = flSpeedNormalized * pPlayer->GetMaxPlayerSpeed();
							if (flSpeed > 0.0f)
								if (vecVelocity.Length2D() > 0.0f)
									vecVelocity = (vecVelocity / vecVelocity.Length()) * flSpeed;
						}
					}
				}

				vecVelocity.z -= g_sdk.convars.m_SvGravity->GetFloat() * 0.5f * TICKS_TO_TIME(LagRecord.m_UpdateDelay);
			}
			else
				vecVelocity.z = 0.0f;
		}
	}
	else if (this->HasLeftOutOfDormancy(pPlayer->EntIndex()))
	{
		float_t flLastUpdateTime = LagRecord.m_SimulationTime - g_interfaces.globals->m_intervalpertick;
		if (pPlayer->m_fFlags() & FL_ONGROUND)
		{
			pPlayer->m_PlayerAnimStateCSGO()->m_bLanding = false;
			pPlayer->m_PlayerAnimStateCSGO()->m_bOnGround = true;

			float_t flLandTime = 0.0f;
			if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flCycle > 0.0f &&
				LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flPlaybackRate > 0.0f)
			{
				int32_t iLandActivity = pPlayer->GetSequenceActivity(LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_nSequence);
				if (iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY)
				{
					flLandTime = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flCycle / LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flPlaybackRate;
					if (flLandTime > 0.0f)
						flLastUpdateTime = LagRecord.m_SimulationTime - flLandTime;
				}
			}

			LagRecord.m_Velocity.z = 0.0f;
		}
		else
		{
			float_t flJumpTime = 0.0f;
			if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flCycle > 0.0f &&
				LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate > 0.0f)
			{
				int32_t iJumpActivity = pPlayer->GetSequenceActivity(LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_nSequence);
				if (iJumpActivity == ACT_CSGO_JUMP)
				{
					flJumpTime = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flCycle / LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate;
					if (flJumpTime > 0.0f)
						flLastUpdateTime = LagRecord.m_SimulationTime - flJumpTime;
				}
			}

			pPlayer->m_PlayerAnimStateCSGO()->m_bOnGround = false;
			pPlayer->m_PlayerAnimStateCSGO()->m_flDurationInAir = flJumpTime - g_interfaces.globals->m_intervalpertick;
		}

		if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flPlaybackRate < 0.00001f)
			LagRecord.m_Velocity.Zero();
		else
		{
			if (flWeightSpeed > 0.0f && flWeightSpeed < 0.95f)
			{
				float_t flMaxSpeed = pPlayer->GetMaxPlayerSpeed();
				if (vecVelocity.Length() > 0.0f)
				{
					float_t flMaxSpeedMultiply = 1.0f;
					if (pPlayer->m_fFlags() & 6)
						flMaxSpeedMultiply = 0.34f;
					else if (pPlayer->m_bIsWalking())
						flMaxSpeedMultiply = 0.52f;

					LagRecord.m_Velocity = (LagRecord.m_Velocity / pPlayer->m_vecVelocity().Length()) * (flWeightSpeed * (flMaxSpeed * flMaxSpeedMultiply));
				}
			}
		}

		pPlayer->m_PlayerAnimStateCSGO()->m_flLastUpdateTime = flLastUpdateTime;
	}

	pPlayer->m_vecVelocity() = vecVelocity;
	pPlayer->m_vecAbsVelocity() = pPlayer->m_vecVelocity();

	if (pPlayer->m_fFlags() & FL_ONGROUND && pPlayer->m_vecVelocity().Length() > 0.0f && flWeightSpeed <= 0.0f)
		pPlayer->m_vecVelocity().Zero();

	return pPlayer->InvalidatePhysicsRecursive(VELOCITY_CHANGED); // velocity changed
}

void C_AnimationSync::UpdatePlayerAnimations(C_BasePlayer* pPlayer, C_LagRecord& LagRecord, C_LagRecord PreviousRecord, bool bHasPreviousRecord, int32_t iRotationMode)
{
	float_t flCurTime = g_interfaces.globals->m_curtime;
	float_t flRealTime = g_interfaces.globals->m_realtime;
	float_t flAbsFrameTime = g_interfaces.globals->m_absframetime;
	float_t flFrameTime = g_interfaces.globals->m_frametime;
	float_t iFrameCount = g_interfaces.globals->m_framecount;
	float_t iTickCount = g_interfaces.globals->m_tickcount;
	float_t flInterpolationAmount = g_interfaces.globals->m_interpolation_amount;

	float_t flLowerBodyYaw = LagRecord.m_LowerBodyYaw;
	float_t flDuckAmount = LagRecord.m_DuckAmount;
	int32_t iFlags = LagRecord.m_Flags;
	int32_t iEFlags = pPlayer->m_iEFlags();

	C_PlayerInfo PlayerInfo;
	g_interfaces.engine->GetPlayerInfo(pPlayer->EntIndex(), &PlayerInfo);

	//g_interfaces.globals->m_curtime = pPlayer->m_flSimulationTime();
	//g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;

	if (bHasPreviousRecord)
	{
		pPlayer->m_PlayerAnimStateCSGO()->m_flStrafeChangeCycle = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO()->m_flStrafeChangeWeight = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO()->m_nStrafeSequence = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE).m_nSequence;
		pPlayer->m_PlayerAnimStateCSGO()->m_flPrimaryCycle = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO()->m_flMoveWeight = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO()->m_flAccelerationWeight = PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_LEAN).m_flWeight;
		memcpy(pPlayer->m_AnimationLayers(), PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
	}
	else
	{
		pPlayer->m_PlayerAnimStateCSGO()->m_flStrafeChangeCycle = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO()->m_flStrafeChangeWeight = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO()->m_nStrafeSequence = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE).m_nSequence;
		pPlayer->m_PlayerAnimStateCSGO()->m_flPrimaryCycle = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flCycle;
		pPlayer->m_PlayerAnimStateCSGO()->m_flMoveWeight = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_MOVE).m_flWeight;
		pPlayer->m_PlayerAnimStateCSGO()->m_flAccelerationWeight = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_LEAN).m_flWeight;
		memcpy(pPlayer->m_AnimationLayers(), LagRecord.m_AnimationLayers.at(ROTATE_SERVER).data(), sizeof(C_AnimationLayer) * ANIMATION_LAYER_COUNT);
	}

	if (LagRecord.m_UpdateDelay > 1)
	{
		int32_t iActivityTick = 0;
		int32_t iActivityType = 0;

		if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flWeight > 0.0f && PreviousRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flWeight <= 0.0f)
		{
			int32_t iLandSequence = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_nSequence;
			if (iLandSequence > 2)
			{
				int32_t iLandActivity = pPlayer->GetSequenceActivity(iLandSequence);
				if (iLandActivity == ACT_CSGO_LAND_LIGHT || iLandActivity == ACT_CSGO_LAND_HEAVY)
				{
					float_t flCurrentCycle = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flCycle;
					float_t flCurrentRate = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flPlaybackRate;

					if (flCurrentCycle > 0.0f && flCurrentRate > 0.0f)
					{
						float_t flLandTime = (flCurrentCycle / flCurrentRate);
						if (flLandTime > 0.0f)
						{
							iActivityTick = TIME_TO_TICKS(LagRecord.m_SimulationTime - flLandTime) + 1;
							iActivityType = ACTIVITY_LAND;
						}
					}
				}
			}
		}

		if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flCycle > 0.0f && LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate > 0.0f)
		{
			int32_t iJumpSequence = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_nSequence;
			if (iJumpSequence > 2)
			{
				int32_t iJumpActivity = pPlayer->GetSequenceActivity(iJumpSequence);
				if (iJumpActivity == ACT_CSGO_JUMP)
				{
					float_t flCurrentCycle = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flCycle;
					float_t flCurrentRate = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate;

					if (flCurrentCycle > 0.0f && flCurrentRate > 0.0f)
					{
						float_t flJumpTime = (flCurrentCycle / flCurrentRate);
						if (flJumpTime > 0.0f)
						{
							iActivityTick = TIME_TO_TICKS(LagRecord.m_SimulationTime - flJumpTime) + 1;
							iActivityType = ACTIVITY_JUMP;
						}
					}
				}
			}
		}

		if (LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flCycle > 0.0f)
			LagRecord.m_bJumped = true;

		for (int32_t iSimulationTick = 1; iSimulationTick <= LagRecord.m_UpdateDelay; iSimulationTick++)
		{
			float_t flSimulationTime = PreviousRecord.m_SimulationTime + TICKS_TO_TIME(iSimulationTick);
			g_interfaces.globals->m_curtime = flSimulationTime;
			g_interfaces.globals->m_realtime = flSimulationTime;
			g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;
			g_interfaces.globals->m_absframetime = g_interfaces.globals->m_intervalpertick;
			g_interfaces.globals->m_framecount = TIME_TO_TICKS(flSimulationTime);
			g_interfaces.globals->m_tickcount = TIME_TO_TICKS(flSimulationTime);
			g_interfaces.globals->m_interpolation_amount = 0.0f;

			// fix retard abobus animation
			if (auto duck_amount_per_tick = (pPlayer->m_flDuckAmount() - PreviousRecord.m_DuckAmount) / LagRecord.m_UpdateDelay)
				pPlayer->m_flDuckAmount() = PreviousRecord.m_DuckAmount + duck_amount_per_tick * (float)iSimulationTick;

			//pPlayer->m_flDuckAmount() = Interpolate(PreviousRecord.m_DuckAmount, LagRecord.m_DuckAmount, iSimulationTick, LagRecord.m_UpdateDelay);
			pPlayer->m_vecVelocity() = Interpolate(PreviousRecord.m_Velocity, LagRecord.m_Velocity, iSimulationTick, LagRecord.m_UpdateDelay);
			pPlayer->m_vecAbsVelocity() = Interpolate(PreviousRecord.m_Velocity, LagRecord.m_Velocity, iSimulationTick, LagRecord.m_UpdateDelay);

			if (iSimulationTick < LagRecord.m_UpdateDelay)
			{
				int32_t iCurrentSimulationTick = TIME_TO_TICKS(flSimulationTime);
				if (iActivityType > ACTIVITY_NONE)
				{
					bool bIsOnGround = pPlayer->m_fFlags() & FL_ONGROUND;
					if (iActivityType == ACTIVITY_JUMP)
					{
						if (iCurrentSimulationTick == iActivityTick - 1)
							bIsOnGround = true;
						else if (iCurrentSimulationTick == iActivityTick)
						{
							// reset animation layer
							pPlayer->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle = 0.0f;
							pPlayer->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_nSequence = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_nSequence;
							pPlayer->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL).m_flPlaybackRate;

							// reset player ground state
							bIsOnGround = false;
						}
					}
					else if (iActivityType == ACTIVITY_LAND)
					{
						if (iCurrentSimulationTick == iActivityTick - 1)
							bIsOnGround = false;
						else if (iCurrentSimulationTick == iActivityTick)
						{
							// reset animation layer
							pPlayer->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flCycle = 0.0f;
							pPlayer->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_nSequence = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_nSequence;
							pPlayer->m_AnimationLayers()[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB].m_flPlaybackRate = LagRecord.m_AnimationLayers.at(ROTATE_SERVER).at(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB).m_flPlaybackRate;

							// reset player ground state
							bIsOnGround = true;
						}
					}

					if (bIsOnGround)
						pPlayer->m_fFlags() |= FL_ONGROUND;
					else
						pPlayer->m_fFlags() &= ~FL_ONGROUND;
				}

				//ResolveInit(pPlayer, LagRecord, PreviousRecord, iRotationMode);

				if (!PlayerInfo.m_bIsFakePlayer)
				{
					switch (iRotationMode)
					{
					case ROTATE_LEFT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
						Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw - pPlayer->GetMinDesyncDelta()); break;
					case ROTATE_RIGHT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
						Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw + pPlayer->GetMaxDesyncDelta()); break;
					case ROTATE_LOW_LEFT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
						Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw - pPlayer->GetMaxDesyncDelta()); break;
					case ROTATE_LOW_RIGHT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
						Math::NormalizeAngle(pPlayer->m_angEyeAngles().yaw + pPlayer->GetMinDesyncDelta()); break;
					default: break;
					}
				}
			}
			else
			{
				pPlayer->m_vecVelocity() = LagRecord.m_Velocity;
				pPlayer->m_vecAbsVelocity() = LagRecord.m_Velocity;
				pPlayer->m_flDuckAmount() = LagRecord.m_DuckAmount;
				pPlayer->m_fFlags() = LagRecord.m_Flags;
			}

			//if (pPlayer->m_AnimState()->m_nLastUpdateFrame > g_interfaces.globals->m_framecount - 1)
			//	pPlayer->m_AnimState()->m_nLastUpdateFrame = g_interfaces.globals->m_framecount - 1;

			bool bClientSideAnimation = pPlayer->m_bClientSideAnimation();
			pPlayer->m_bClientSideAnimation() = true;

			for (int32_t iLayer = NULL; iLayer < ANIMATION_LAYER_COUNT; iLayer++)
				pPlayer->m_AnimationLayers()[iLayer].m_pOwner = pPlayer;

			g_sdk.animation_data.m_bAnimationUpdate = true;
			pPlayer->UpdateClientSideAnimation();
			g_sdk.animation_data.m_bAnimationUpdate = false;

			pPlayer->m_bClientSideAnimation() = bClientSideAnimation;
		}
	}
	else
	{
		g_interfaces.globals->m_curtime = LagRecord.m_SimulationTime;
		g_interfaces.globals->m_realtime = LagRecord.m_SimulationTime;
		g_interfaces.globals->m_frametime = g_interfaces.globals->m_intervalpertick;
		g_interfaces.globals->m_absframetime = g_interfaces.globals->m_intervalpertick;
		g_interfaces.globals->m_framecount = TIME_TO_TICKS(LagRecord.m_SimulationTime);
		g_interfaces.globals->m_tickcount = TIME_TO_TICKS(LagRecord.m_SimulationTime);
		g_interfaces.globals->m_interpolation_amount = 0.0f;

		pPlayer->m_vecVelocity() = LagRecord.m_Velocity;
		pPlayer->m_vecAbsVelocity() = LagRecord.m_Velocity;

		//ResolveInit(pPlayer, LagRecord, PreviousRecord, iRotationMode);
		if (!PlayerInfo.m_bIsFakePlayer)
		{
			switch (iRotationMode)
			{
			case ROTATE_LEFT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
				Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw - pPlayer->GetMinDesyncDelta()); break;
			case ROTATE_RIGHT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
				Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw + pPlayer->GetMaxDesyncDelta()); break;
			case ROTATE_LOW_LEFT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
				Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw - pPlayer->GetMaxDesyncDelta()); break;
			case ROTATE_LOW_RIGHT: pPlayer->m_PlayerAnimStateCSGO()->m_flFootYaw =
				Math::NormalizeAngle(pPlayer->m_PlayerAnimStateCSGO()->m_flEyeYaw + pPlayer->GetMinDesyncDelta()); break;
			default: break;
			}
		}
		bool bClientSideAnimation = pPlayer->m_bClientSideAnimation();
		pPlayer->m_bClientSideAnimation() = true;

		for (int32_t iLayer = NULL; iLayer < ANIMATION_LAYER_COUNT; iLayer++)
			pPlayer->m_AnimationLayers()[iLayer].m_pOwner = pPlayer;

		g_sdk.animation_data.m_bAnimationUpdate = true;
		pPlayer->UpdateClientSideAnimation();
		g_sdk.animation_data.m_bAnimationUpdate = false;

		pPlayer->m_bClientSideAnimation() = bClientSideAnimation;
	}

	pPlayer->m_flLowerBodyYaw() = flLowerBodyYaw;
	pPlayer->m_flDuckAmount() = flDuckAmount;
	pPlayer->m_iEFlags() = iEFlags;
	pPlayer->m_fFlags() = iFlags;

	g_interfaces.globals->m_curtime = flCurTime;
	g_interfaces.globals->m_realtime = flRealTime;
	g_interfaces.globals->m_absframetime = flAbsFrameTime;
	g_interfaces.globals->m_frametime = flFrameTime;
	g_interfaces.globals->m_framecount = iFrameCount;
	g_interfaces.globals->m_tickcount = iTickCount;
	g_interfaces.globals->m_interpolation_amount = flInterpolationAmount;

	return pPlayer->InvalidatePhysicsRecursive(ANIMATION_CHANGED | ANGLES_CHANGED);
}

bool C_AnimationSync::GetCachedMatrix(C_BasePlayer* pPlayer, matrix3x4_t* aMatrix)
{
	memcpy(aMatrix, m_CachedMatrix[pPlayer->EntIndex()].data(), sizeof(matrix3x4_t) * pPlayer->m_CachedBoneData().Count());
	return true;
}

void C_AnimationSync::OnUpdateClientSideAnimation(C_BasePlayer* pPlayer)
{
	for (int i = 0; i < MAXSTUDIOBONES; i++)
		m_CachedMatrix[pPlayer->EntIndex()][i].SetOrigin(pPlayer->GetAbsOrigin() - m_BoneOrigins[pPlayer->EntIndex()][i]);

	memcpy(pPlayer->m_CachedBoneData().Base(), m_CachedMatrix[pPlayer->EntIndex()].data(), sizeof(matrix3x4_t) * pPlayer->m_CachedBoneData().Count());
	memcpy(pPlayer->GetBoneAccessor().GetBoneArrayForWrite(), m_CachedMatrix[pPlayer->EntIndex()].data(), sizeof(matrix3x4_t) * pPlayer->m_CachedBoneData().Count());

	return pPlayer->SetupBones_AttachmentHelper();
}