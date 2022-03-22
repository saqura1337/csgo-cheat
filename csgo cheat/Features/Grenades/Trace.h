#pragma once
#ifndef _CNADETRACER_H
#define _CNADETRACER_H

#include <deque>
#include <vector>
#include <set>
#include <unordered_map>

#include "../Settings.hpp"
#include "../Tools/Tools.hpp"
#include "../Grenades/Warning.hpp"
#include "../SDK/Includes.hpp"

class CTimer
{
private:
	__int64 initialTS, currentTS;
	float secsPerCount;

public:
	CTimer()
	{
		__int64 countsPerSec = initialTS = currentTS = 0;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		if (countsPerSec == 0)
			secsPerCount = 1.0f;
		else
			secsPerCount = 1.0f / (float)countsPerSec;
	}
	void init()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&initialTS);
	}
	float diff()
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTS);
		return ((float)currentTS - (float)initialTS) * secsPerCount;
	}
};

struct CTraceObj
{
	CTraceObj() = default;
	CTraceObj(Color, unsigned int);

	Color m_colorTrace;
	unsigned int m_iMaxLength;
	std::vector<Vector> m_vecTracePoints;
	CTimer m_timerPointLock;
};

class CNadeTracer
{
public:
	std::unordered_map<C_BasePlayer*, CTraceObj> m_mapGrenades;
	std::set<C_BasePlayer*> m_setGrenadeExists;

	void AddTracer(C_BasePlayer*, Color, unsigned int);
	void Draw();
	void Clear();
};

#endif

inline CNadeTracer* g_NadeTracer = new CNadeTracer();
