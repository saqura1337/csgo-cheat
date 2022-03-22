//#pragma once
//
//#include "../SDK/Includes.hpp"
//#include "../Settings.hpp"
//#include "../Animations/LagCompensation.hpp"
//#include "../Packet/PacketManager.hpp"
//#include "../../SDK/Math/Math.hpp"
//
//class LegitBot
//{
//public:
//	QAngle CalculateRelativeAngle(const Vector& source, const Vector& destination, const QAngle view_angles) noexcept
//	{
//		const auto angles = Math::CalcAngle(source, destination);
//
//		const auto temp = angles - view_angles;
//		auto temp_angle = QAngle(temp.pitch, temp.yaw, temp.roll);
//		temp_angle.Normalize();
//		temp_angle.Clamp();
//
//		return QAngle(temp_angle.pitch, temp_angle.yaw, temp_angle.roll);
//	}
//
//	bool IsVisible(C_BasePlayer* m_player, const Vector start, const Vector end)
//	{
//		if (!m_player)
//		{
//			return false;
//		}
//
//		CGameTrace tr;
//		Ray_t ray;
//		static CTraceFilter trace_filter;
//		trace_filter.pSkip = g_sdk.local;
//
//		ray.Init(start, end);
//
//		g_interfaces.trace->TraceRay(ray, 0x4600400B, &trace_filter, &tr);
//
//		return tr.hit_entity == m_player || tr.fraction >= 0.99f;
//	}
//
//	bool CrosshairOnEnemy(const QAngle view_angles)
//	{
//		const auto start_pos = g_sdk.local->GetEyePosition();
//		Vector direction;
//		Math::AngleVectors(view_angles + g_sdk.local->m_aimPunchAngle(), direction);
//
//		const auto end_pos = start_pos + direction * 8096;
//
//		Ray_t ray;
//		ray.Init(start_pos, end_pos);
//		CTraceFilter filter;
//		filter.pSkip = g_sdk.local;
//		trace_t exit_trace{};
//		g_interfaces.trace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &exit_trace);
//
//		auto* hit_entity = static_cast<C_BasePlayer*>(exit_trace.hit_entity);
//
//		C_PlayerInfo info;
//		if (!g_interfaces.engine->GetPlayerInfo(hit_entity->EntIndex(), &info))
//		{
//			return false;
//		}
//
//		return hit_entity && hit_entity->m_iTeamNum() != g_sdk.local->m_iTeamNum();
//	}
//
//	virtual void normalize_angles(QAngle& angles);
//	virtual void Run();
//	virtual void FindTarget(float& best_fov, Vector& best_target, QAngle aim_punch);
//	virtual void RunTriggerBot();
//	virtual void Rcs(QAngle& angle, const bool is_player, const bool shots_fired);
//	C_LegitSettings m_LegitSettings;
//};
//
//inline LegitBot* g_LegitBot = new LegitBot();