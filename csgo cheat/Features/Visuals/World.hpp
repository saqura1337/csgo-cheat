#pragma once
#include <vector>
#include "../SDK/Includes.hpp"

struct ClientImpact_t
{
	Vector m_vecPosition;
	float_t m_flTime;
	float_t m_flExpirationTime;
};

struct C_BulletTrace
{
	bool m_bIsLocalTrace = false;

	Vector m_vecStartPosition = Vector(0, 0, 0);
	Vector m_vecEndPosition = Vector(0, 0, 0);
};

class C_World
{
public:
	virtual void Instance(ClientFrameStage_t Stage);
	virtual void SkyboxChanger();
	virtual void DrawClientImpacts();
	virtual void ViewmodelDistansePfix();
	virtual	void PingModulation();
	virtual void DisablePanorama();
	virtual void DrawBulletTracers();
	virtual void OnBulletImpact(C_GameEvent* pEvent);
	virtual void Clantag();
	virtual void Grenades();
	virtual void DrawScopeLines();
	virtual void PenetrationCrosshair();
	virtual void RemoveShadows();
	virtual void RemoveHandShaking();
	virtual	void BombTimer(C_GameEvent* pEvent, C_BaseEntity* entity);
	virtual void LeftHandKnife();
	virtual void RemoveSmokeAndPostProcess();
	virtual void PostFrame(ClientFrameStage_t Stage);
	virtual void OnRageBotFire(Vector vecStartPosition, Vector vecEndPosition);
	virtual void PreserveKillfeed();

	void SunsetMode();

	std::vector < C_BulletTrace > m_BulletTracers = { };

private:
	int32_t m_iLastProcessedImpact = 0;

	bool m_bDidUnlockConvars = false;
};

inline C_World* g_World = new C_World();