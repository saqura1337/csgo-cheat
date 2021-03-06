#include "Weather.hpp"

void C_WeatherSystem::Instance(ClientFrameStage_t Stage)
{
    if (Stage != ClientFrameStage_t::FRAME_NET_UPDATE_END)
        return;

    if (!g_cfg->m_CustomWeather)
    {
        if (m_WeatherData.m_bHasBeenCreated)
        {
            m_WeatherData.m_bHasBeenCreated = false;
            if (m_WeatherData.m_Networkable)
            {
                m_WeatherData.m_Networkable->Release();
                m_WeatherData.m_Networkable = NULL;
            }
        }

        return;
    }

    g_sdk.convars.m_RainSideVel->SetValue(30.0f);
    g_sdk.convars.m_RainAlpha->SetValue(1.0f);
    g_sdk.convars.m_RainSpeed->SetValue(static_cast <float_t>(g_cfg->m_RainSpeed));
    g_sdk.convars.m_RainWidth->SetValue(static_cast <float_t>(g_cfg->m_RainWidth) / 100.0f);
    g_sdk.convars.m_RainLength->SetValue(static_cast <float_t>(g_cfg->m_RainLength) / 1000.0f);

    if (!m_WeatherData.m_bHasBeenCreated)
    {
        m_WeatherData.m_Networkable = this->CreateWeatherEntity();
        return;
    }

    C_Precipitation* m_Precipitation = (C_Precipitation*)(m_WeatherData.m_Networkable->GetIClientUnknown()->GetBaseEntity());
    if (!m_Precipitation)
        return;

    m_Precipitation->PreDataUpdate(NULL);
    m_Precipitation->OnPreDataChanged(NULL);

    m_Precipitation->m_nPrecipitationType() = g_cfg->m_WeatherType;
    m_Precipitation->GetCollideable()->OBBMins() = Vector(-32768.0f, -32768.0f, -32768.0f);
    m_Precipitation->GetCollideable()->OBBMaxs() = Vector(32768.0f, 32768.0f, 32768.0f);

    m_Precipitation->OnPostDataChanged(NULL);
    m_Precipitation->PostDataUpdate(NULL);
}

void C_WeatherSystem::ResetData()
{
    m_WeatherData.m_bHasBeenCreated = false;
}

IClientNetworkable* C_WeatherSystem::CreateWeatherEntity()
{
    ClientClass* Class = g_interfaces.client->GetAllClasses();
    while ((int32_t)(Class->m_ClassID) != 138)
        Class = Class->m_pNext;

    IClientNetworkable* Networkable = Class->m_pCreateFn(g_interfaces.entity_list->GetHighestEntityIndex() + 1, g_Tools->RandomInt(0, 4096));
    if (!Networkable || !Networkable->GetIClientUnknown()->GetBaseEntity())
        return NULL;

    m_WeatherData.m_bHasBeenCreated = true;
    return Networkable;
}