#include "LogManager.hpp"
#include "../Render.hpp"
#include "../SDK/Math/Math.hpp"

void C_LogManager::Instance()
{
	for (int32_t i = 0; i < m_Logs.size(); i++)
	{
		C_LogData* LogData = &m_Logs[i];
		if (!LogData)
			continue;

		float_t flTimeDelta = g_interfaces.globals->m_realtime - LogData->m_flCreationTime;
		if (flTimeDelta >= 5.0f)
		{
			m_Logs.erase(m_Logs.begin() + i);
			continue;
		}

		if (flTimeDelta >= 4.75f)
		{
			LogData->m_flTextAlpha = std::clamp(((5.0f - flTimeDelta) / 0.25f) * 255.0f, 0.0f, 255.0f);
			LogData->m_flBackAlpha = std::clamp(((5.0f - flTimeDelta) / 0.25f) * 50.0f, 0.0f, 50.0f);
			LogData->m_flSpacing = std::clamp(((5.0f - flTimeDelta) / 0.25f) * 24.0f, 0.0f, 24.0f);
		}
		else if (flTimeDelta <= 0.25f)
		{
			LogData->m_flTextAlpha = std::clamp((1.0f - (0.25f - flTimeDelta) / 0.25f) * 255.0f, 0.0f, 255.0f);
			LogData->m_flBackAlpha = std::clamp((1.0f - (0.25f - flTimeDelta) / 0.25f) * 50.0f, 0.0f, 50.0f);
			LogData->m_flSpacing = std::clamp((1.0f - (0.25f - flTimeDelta) / 0.25f) * 24.0f, 0.0f, 24.0f);
		}

		float_t flSpacing = 0.0f;
		if (i)
		{
			for (int i2 = 0; i2 < i; i2++)
				flSpacing += m_Logs[i2].m_flSpacing;
		}

		constexpr uint8_t aColor[4] = { 77, 125, 253, 255 };

		Color aIconColor = Color(0, 128, 255, (int)(LogData->m_flTextAlpha));
		if (!LogData->m_bPrinted)
		{
			//g_Interfaces.m_CVar->ConsoleColorPrintf( aColor, _S( "[ " ) );
			//g_Interfaces.m_CVar->ConsoleColorPrintf( aColor, _S( "reborn hack" ) );
			//g_Interfaces.m_CVar->ConsoleColorPrintf( aColor, _S( " ] " ) );
			g_interfaces.cvar->ConsoleColorPrintf(aColor, LogData->m_LogString.c_str());
			g_interfaces.cvar->ConsoleColorPrintf(aColor, _S("\n"));

			LogData->m_bPrinted = true;
		}

		float_t flTextLength = 28 + g_sdk.fonts.m_LogFont->CalcTextSizeA(16.0f, FLT_MAX, NULL, LogData->m_LogString.c_str()).x;

		//g_Render->RenderRectFilled( 0, flSpacing, flTextLength, flSpacing + 22.0f, Color( 0, 0, 0, ( int )( LogData->m_flBackAlpha ) ) );
		//g_Render->RenderRectFilled(0, flSpacing, 2, flSpacing + 22.0f, Color(LogData->m_LogColor));

		//if ( LogData->m_LogIcon == _S( "d" ) )
		//	g_Render->RenderText( LogData->m_LogIcon, ImVec2( 4, flSpacing + 2 ), aIconColor, false, false, g_Sdk.m_Fonts.m_BombIcon );
		//else
		//	g_Render->RenderText( LogData->m_LogIcon, ImVec2( 4, flSpacing + 4 ), aIconColor, false, false, g_Sdk.m_Fonts.m_LogIcons );

		g_Render->RenderText(LogData->m_LogString, ImVec2(4, flSpacing + 2), LogData->m_LogColor, false, false, g_sdk.fonts.m_LogFont);
	}
}

void C_LogManager::PushLog(std::string LogText, std::string LogIcon, Color color)
{
	C_LogData LogData;

	LogData.m_flCreationTime = g_interfaces.globals->m_realtime;
	LogData.m_flSpacing = 0.0f;
	LogData.m_flTextAlpha = 0.0f;
	LogData.m_LogIcon = LogIcon;
	LogData.m_LogString = LogText;
	LogData.m_LogColor = color;

	m_Logs.emplace_front(LogData);
	while (m_Logs.size() > 10)
		m_Logs.pop_back();
}