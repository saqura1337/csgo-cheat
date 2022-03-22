#pragma once
#include "../Game/UserCmd.hpp"

#define MULTIPLAYER_BACKUP 150

class bf_write;
class bf_read;

class C_Input
{
public:
	char pad0[0xC]; // 0x0
	bool m_trackir_available; // 0xC
	bool m_mouse_initialized; // 0xD
	bool m_mouse_active; // 0xE
	char pad1[0x9E]; // 0xF
	bool m_bCameraInThirdPerson; // 0xAD
	char pad2[0x2]; // 0xAE
	Vector m_vecCameraOffset; // 0xB0
	char pad3[0x38]; // 0xBC
	C_UserCmd* m_commands; // 0xF4
	C_VerifiedUserCmd* m_verified_commands; // 0xF8   

	C_UserCmd* C_Input::GetUserCmd(int iSequenceNumber)
	{
		return GetVirtual < C_UserCmd* (__thiscall*)(void*, int, int) >(this, 8)(this, 0, iSequenceNumber);
	}

	C_UserCmd* C_Input::GetUserCmd(int iSlot, int iSequenceNumber)
	{
		return GetVirtual < C_UserCmd* (__thiscall*)(void*, int, int) >(this, 8)(this, iSlot, iSequenceNumber);
	}

	C_VerifiedUserCmd* C_Input::GetVerifiedCmd(int iSequenceNumber)
	{
		auto verifiedCommands = *(C_VerifiedUserCmd**)(reinterpret_cast<uint32_t>(this) + 0xF8);
		return &verifiedCommands[iSequenceNumber % MULTIPLAYER_BACKUP];
	}
};