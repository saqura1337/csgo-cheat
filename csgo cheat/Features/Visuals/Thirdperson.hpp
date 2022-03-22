#pragma once
#include "../SDK/Includes.hpp"

class C_ThirdPerson
{
public:
	float_t m_flThirdpersonDistance = 0.0f;
	virtual void Init(const bool fakeducking, const float progress);
	virtual void Thirdperson(const bool fakeducking);
};

inline C_ThirdPerson* g_ThirdPerson = new C_ThirdPerson( );