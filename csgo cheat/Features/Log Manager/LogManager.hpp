#pragma once
#include <deque>
#include "../SDK/Includes.hpp"

struct C_LogData
{
	float_t m_flCreationTime = 0.f;
	float_t m_flTextAlpha = 0.0f;
	float_t m_flBackAlpha = 50.0f;
	float_t m_flSpacing = 0.0f;

	std::string m_LogString = "";
	std::string m_LogIcon = "";
	Color m_LogColor;

	bool m_bPrinted = false;
};

class C_LogManager
{
public:
	virtual void Instance();
	virtual void PushLog(std::string LogText, std::string LogIcon, Color color);
private:
	std::deque < C_LogData > m_Logs;
};

inline C_LogManager* g_LogManager = new C_LogManager();