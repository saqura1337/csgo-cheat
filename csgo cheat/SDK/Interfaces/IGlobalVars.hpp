#pragma once

class C_GlobalVarsBase
{
public:
	float       m_realtime;
	int         m_framecount;
	float       m_absframetime;
	float       m_absframetimedev;
	float       m_curtime;
	float       m_frametime;
	int         m_maxclients;
	int         m_tickcount;
	float       m_intervalpertick;
	float       m_interpolation_amount;
	int         m_simticks_this_frame;
	int         m_network_protocol;
	void*		m_save_data;
	bool        m_client;
	bool        m_remote_client;
	int         m_timestamp_networkingbase;
	int         m_timestamp_randomize_window;
};