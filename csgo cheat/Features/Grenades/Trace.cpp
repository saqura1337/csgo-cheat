#include "Trace.h"

void CNadeTracer::AddTracer(C_BasePlayer* pGrenade, Color colorTrace, unsigned int iMaxLength)
{
	m_setGrenadeExists.insert(pGrenade);
	if (m_mapGrenades.find(pGrenade) == m_mapGrenades.end())
		m_mapGrenades[pGrenade] = CTraceObj(colorTrace, iMaxLength);

	if (m_mapGrenades[pGrenade].m_timerPointLock.diff() > 0.025f) //25 ms
	{
		m_mapGrenades[pGrenade].m_vecTracePoints.push_back(pGrenade->m_vecOrigin());
		if (m_mapGrenades[pGrenade].m_vecTracePoints.size() > m_mapGrenades[pGrenade].m_iMaxLength)
			m_mapGrenades[pGrenade].m_vecTracePoints.erase(m_mapGrenades[pGrenade].m_vecTracePoints.begin());

		m_mapGrenades[pGrenade].m_timerPointLock.init();
	}
}

void CNadeTracer::Draw()
{
	Vector prev;
	Vector nadeStart, nadeEnd;
	bool bInit = false;

	for (auto& traceObj : m_mapGrenades)
	{
		for (auto& vecPos : traceObj.second.m_vecTracePoints)
		{
			if (Math::WorldToScreen(prev, nadeStart) && Math::WorldToScreen(vecPos, nadeEnd))
			{
				if (bInit)
					g_Render->RenderLine(nadeStart.x, nadeStart.y, nadeEnd.x, nadeEnd.y, traceObj.second.m_colorTrace, 1.f);

				bInit = true;
			}

			prev = vecPos;
		}
		bInit = false;
	}
}

void CNadeTracer::Clear()
{
	for (auto it = m_mapGrenades.begin(); it != m_mapGrenades.end(); ++it)
	{
		if (m_setGrenadeExists.find((*it).first) == m_setGrenadeExists.end())
		{
			it = m_mapGrenades.erase(it);
			if (it == m_mapGrenades.end())
				break;
		}
	}

	m_setGrenadeExists.clear();
}

CTraceObj::CTraceObj(Color colorTrace, unsigned int iMaxLength)
{
	m_colorTrace = colorTrace;
	m_iMaxLength = iMaxLength;
	m_timerPointLock.init();
}