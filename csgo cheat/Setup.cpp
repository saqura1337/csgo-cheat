#include "Setup.hpp"
#include "Hooks.hpp"
#include "Render.hpp"
#include "Features/Visuals/Chams.hpp"

#include "Multithread/threading.h"
#include "Tools/Tools.hpp"
#include "get_user_info.hpp"

#include "JSON.hpp"
#include "curl/curl.h"
#include "MD5.hpp"

void C_SetUp::Instance(HINSTANCE hInstance)
{
	while (!GetModuleHandleA(_S("serverbrowser.dll")))
		Sleep(100);

	// init sdk
	SetupModuleList();
	SetupInterfaceList();
	SetupAddressList();
	SetupConvarList();
	SetupImGui();

	// привет от детей шлюх модул€р сука
	//MultiThread::InitThreads( );

	// setup menu
	g_Menu->Initialize();

	// setup render
	g_Render->Initialize();

	setup_skins();

	// create materials
	g_Chams->CreateMaterials();

	// scan netvars
	g_NetvarManager->Instance();

	// setup hooks
	SetupHooks();

	// get out of thread
	return ExitThread(EXIT_SUCCESS);
}

#define CREATE_HOOK( Address, Function, Original ) if ( MH_CreateHook( ( LPVOID )( Address ), ( LPVOID )( Function ), reinterpret_cast< void** >( &Original ) ) ) MessageBoxA( NULL, _S( #Function ), _S( #Function ), 64 );
void C_SetUp::SetupHooks()
{
	if (MH_Initialize() != MH_OK)
		return;

	/* tweex - note: */
	// didnt use "static", because cheat will randomly crash
	// for exaple: if change main weapon -> crash

	auto pPresent = (LPVOID)((*(uintptr_t**)(g_interfaces.direct_device))[17]);
	auto pReset = (LPVOID)((*(uintptr_t**)(g_interfaces.direct_device))[16]);
	auto pInPrediction = (LPVOID)((*(uintptr_t**)(g_interfaces.prediction))[14]);
	auto pIsHLTV = (LPVOID)((*(uintptr_t**)(g_interfaces.engine))[93]);
	auto pIsPaused = (LPVOID)((*(uintptr_t**)(g_interfaces.engine))[90]);
	auto pFireEvents = (LPVOID)((*(uintptr_t**)(g_interfaces.engine))[59]);
	auto pIsConnected = (LPVOID)((*(uintptr_t**)(g_interfaces.engine))[27]);
	auto pGetAspectRatio = (LPVOID)((*(uintptr_t**)(g_interfaces.engine))[101]);
	auto pListLeavesInBox = (LPVOID)((*(uintptr_t**)(g_interfaces.engine->GetBSPTreeQuery()))[6]);
	auto pLockCursor = (LPVOID)((*(uintptr_t**)(g_interfaces.surface))[67]);
	auto pDispatchUserMessage = (LPVOID)((*(uintptr_t**)(g_interfaces.client))[38]);
	auto pWriteUsercmdDeltaToBuffer = (LPVOID)((*(uintptr_t**)(g_interfaces.client))[24]);
	auto pPaintTraverse = (LPVOID)((*(uintptr_t**)(g_interfaces.vgui_panel))[41]);
	auto pProcessMovement = (LPVOID)((*(uintptr_t**)(g_interfaces.game_movement))[1]);
	auto pDrawModelExecute_Studio = (LPVOID)((*(uintptr_t**)(g_interfaces.studio_render))[29]);
	auto pDrawModelExecute_Model = (LPVOID)((*(uintptr_t**)(g_interfaces.model_render))[21]);
	auto pSvCheats_GetBool = (LPVOID)((*(uintptr_t**)(g_interfaces.cvar->FindVar(_S("sv_cheats"))))[13]);
	auto pClDoResetLatch_GetBool = (LPVOID)((*(uintptr_t**)(g_interfaces.cvar->FindVar(_S("cl_pred_doresetlatch"))))[13]);
	auto pCreateMove = (LPVOID)((*(uintptr_t**)(g_interfaces.client))[22]);

	LPVOID pCL_Move = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("55 8B EC 81 EC 64 01 00 00 53 56 8A F9"));
	LPVOID pFrameStageNotify = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0 A2"));
	LPVOID pPacketStart = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ?")) - 0x20;
	LPVOID pPacketEnd = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ?"));
	LPVOID pModifyEyePosition = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14 83 7F 60"));
	LPVOID pDoExtraBoneProcessing = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"));
	LPVOID pTraceFilterForHeadCollision = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 56 8B 75 0C 57 8B F9 F7 C6 ? ? ? ?"));
	LPVOID pSetupBones = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F0 B8 D8"));
	LPVOID pUpdateClientSideAnimation = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74"));
	LPVOID pStandardBlendingRules = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F0 B8 F8 10"));
	LPVOID pPhysicsSimulate = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23"));
	LPVOID pDoPostScreenEffects = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 8B 49 18 56 8B"));
	LPVOID pOverrideView = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 8B 4D 04 83 EC 58"));
	LPVOID pSendNetMessage = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("55 8B EC 83 EC 08 56 8B F1 8B 4D 04 E8"));
	LPVOID pCalcView = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 14 53 56 57 FF 75 18"));
	LPVOID pGetColorModulation = g_Tools->FindPattern(g_sdk.modules.mat_sys_dll, _S("55 8B EC 83 EC ? 56 8B F1 8A 46"));
	LPVOID pGetAlphaModulation = g_Tools->FindPattern(g_sdk.modules.mat_sys_dll, _S("56 8B F1 8A 46 20 C0 E8 02 A8 01 75 0B 6A 00 6A 00 6A 00 E8 ? ? ? ? 80 7E 22 05 76 0E"));
	LPVOID pShouldDrawFOG = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 8B 0D ? ? ? ? 83 EC 0C 8B 01 53 56 57 FF"));
	LPVOID pGetClientModelRenderable = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("56 8B F1 80 BE FC 26"));
	LPVOID pIsUsingDebugStaticProps = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("8B 0D ? ? ? ? 81 F9 ? ? ? ? 75 ? A1 ? ? ? ? 35 ? ? ? ? EB ? 8B 01 FF 50 ? 83 F8 ? 0F 85 ? ? ? ? 8B 0D"));
	LPVOID pPerformScreenOverlay = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 51 A1 ? ? ? ? 53 56 8B D9"));
	LPVOID pCalcViewmodelBob = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9"));

	CREATE_HOOK(pPresent, C_Hooks::hkPresent, g_sdk.hooks.originals.m_Present);
	CREATE_HOOK(pReset, C_Hooks::hkReset, g_sdk.hooks.originals.m_Reset);
	CREATE_HOOK(pSvCheats_GetBool, C_Hooks::hkSvCheats_GetBool, g_sdk.hooks.originals.m_SvCheats_GetBool);
	CREATE_HOOK(pClDoResetLatch_GetBool, C_Hooks::hkClDoResetLatch_GetBool, g_sdk.hooks.originals.m_ClDoResetLatch_GetBool);
	CREATE_HOOK(pInPrediction, C_Hooks::hkInPrediction, g_sdk.hooks.originals.m_InPrediction);
	CREATE_HOOK(pIsHLTV, C_Hooks::hkIsHLTV, g_sdk.hooks.originals.m_IsHLTV);
	CREATE_HOOK(pIsPaused, C_Hooks::hkIsPaused, g_sdk.hooks.originals.m_IsPaused);
	CREATE_HOOK(pFireEvents, C_Hooks::hkFireEvents, g_sdk.hooks.originals.m_FireEvents);
	CREATE_HOOK(pCL_Move, C_Hooks::hkCL_Move, g_sdk.hooks.originals.m_CL_Move);
	CREATE_HOOK(pCreateMove, C_Hooks::hkCreateMove_Proxy, g_sdk.hooks.originals.m_CreateMove);
	CREATE_HOOK(pFrameStageNotify, C_Hooks::hkFrameStageNotify, g_sdk.hooks.originals.m_FrameStageNotify);
	CREATE_HOOK(pPacketStart, C_Hooks::hkPacketStart, g_sdk.hooks.originals.m_PacketStart);
	CREATE_HOOK(pPacketEnd, C_Hooks::hkPacketEnd, g_sdk.hooks.originals.m_PacketEnd);
	CREATE_HOOK(pModifyEyePosition, C_Hooks::hkModifyEyePosition, g_sdk.hooks.originals.m_ModifyEyePosition);
	CREATE_HOOK(pDoExtraBoneProcessing, C_Hooks::hkDoExtraBoneProcessing, g_sdk.hooks.originals.m_DoExtraBoneProcessing);
	CREATE_HOOK(pLockCursor, C_Hooks::hkLockCursor, g_sdk.hooks.originals.m_LockCursor);
	CREATE_HOOK(pPaintTraverse, C_Hooks::hkPaintTraverse, g_sdk.hooks.originals.m_PaintTraverse);
	CREATE_HOOK(pTraceFilterForHeadCollision, C_Hooks::hkTraceFilterForHeadCollision, g_sdk.hooks.originals.m_TraceFilterForHeadCollision);
	CREATE_HOOK(pProcessMovement, C_Hooks::hkProcessMovement, g_sdk.hooks.originals.m_ProcessMovement);
	CREATE_HOOK(pSetupBones, C_Hooks::hkSetupBones, g_sdk.hooks.originals.m_SetupBones);
	CREATE_HOOK(pUpdateClientSideAnimation, C_Hooks::hkUpdateClientSideAnimation, g_sdk.hooks.originals.m_UpdateClientSideAnimation);
	CREATE_HOOK(pStandardBlendingRules, C_Hooks::hkStandardBlendingRules, g_sdk.hooks.originals.m_StandardBlendingRules);
	CREATE_HOOK(pPhysicsSimulate, C_Hooks::hkPhysicsSimulate, g_sdk.hooks.originals.m_PhysicsSimulate);
	CREATE_HOOK(pDoPostScreenEffects, C_Hooks::hkDoPostScreenEffects, g_sdk.hooks.originals.m_DoPostScreenEffects);
	CREATE_HOOK(pOverrideView, C_Hooks::hkOverrideView, g_sdk.hooks.originals.m_OverrideView);
	CREATE_HOOK(pSendNetMessage, C_Hooks::hkSendNetMessage, g_sdk.hooks.originals.m_SendNetMessage);
	CREATE_HOOK(pIsConnected, C_Hooks::hkIsConnected, g_sdk.hooks.originals.m_IsConnected);
	CREATE_HOOK(pCalcView, C_Hooks::hkCalcView, g_sdk.hooks.originals.m_CalcView);
	CREATE_HOOK(pDrawModelExecute_Model, C_Hooks::hkDrawModelExecute_Model, g_sdk.hooks.originals.m_DrawModelExecute_Model);
	CREATE_HOOK(pDrawModelExecute_Studio, C_Hooks::hkDrawModelExecute_Studio, g_sdk.hooks.originals.m_DrawModelExecute_Studio);
	CREATE_HOOK(pGetColorModulation, C_Hooks::hkGetColorModulation, g_sdk.hooks.originals.m_GetColorModulation);
	CREATE_HOOK(pGetAlphaModulation, C_Hooks::hkGetAlphaModulation, g_sdk.hooks.originals.m_GetAlphaModulation);
	CREATE_HOOK(pGetClientModelRenderable, C_Hooks::hkGetClientModelRenderable, g_sdk.hooks.originals.m_Useless);
	CREATE_HOOK(pIsUsingDebugStaticProps, C_Hooks::hkIsUsingDebugStaticProps, g_sdk.hooks.originals.m_IsUsingDebugStaticProps);
	CREATE_HOOK(pDispatchUserMessage, C_Hooks::hkDispatchUserMessage, g_sdk.hooks.originals.m_DispatchUserMessage);
	CREATE_HOOK(pPerformScreenOverlay, C_Hooks::hkPerformScreenOverlay, g_sdk.hooks.originals.m_PerformScreenOverlay);
	CREATE_HOOK(pCalcViewmodelBob, C_Hooks::hkCalcViewmodelBob, g_sdk.hooks.originals.m_CalcViewmodelBob);
	CREATE_HOOK(pShouldDrawFOG, C_Hooks::hkShouldDrawFOG, g_sdk.hooks.originals.m_ShouldDrawFog);
	CREATE_HOOK(pListLeavesInBox, C_Hooks::hkListLeavesInBox, g_sdk.hooks.originals.m_ListLeavesInBox);
	CREATE_HOOK(pGetAspectRatio, C_Hooks::hkGetScreenSizeAspectRatio, g_sdk.hooks.originals.m_GetScreenSizeAspectRatio);
	CREATE_HOOK(pWriteUsercmdDeltaToBuffer, C_Hooks::hkWriteUsercmdDeltaToBuffer, g_sdk.hooks.originals.m_WriteUsercmdDeltaToBuffer);

	/* removed hooks */
	/* reason - outdated signature
	LPVOID pGetViewmodelFOV = g_Tools->FindPattern(g_sdk.modules.client_dll, _S( "55 8B EC 8B 4D 04 83 EC 08 57 8B C1 83 C0 08" ));
	CREATE_HOOK(pGetViewmodelFOV,	C_Hooks::hkGetViewmodelFOV,	g_sdk.hooks.originals.m_GetViewmodelFOV); */
	/* reason - no need
	LPVOID pPolakThing = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("E8 ? ? ? ? 8B CE E8 ? ? ? ? 33 FF"));
	LPVOID pSetupVelocity			= g_Tools->FindPattern(g_sdk.modules.client_dll, _S("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0"));
	CREATE_HOOK(pSetupVelocity, C_Hooks::hkSetupVelocity, g_sdk.hooks.originals.setup_velocity) // not working
	LPVOID pRunCommand					= (LPVOID)((*(uintptr_t**)(g_interfaces.prediction))[19]);
	CREATE_HOOK(pRunCommand, C_Hooks::hkRunCommand, g_sdk.hooks.originals.runcommand); */

	MH_EnableHook(MH_ALL_HOOKS);

	C_CustomEventListener* pEventListener = new C_CustomEventListener();
	g_interfaces.event_manager->AddListener(pEventListener, _S("bomb_beginplant"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("bomb_begindefuse"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("bomb_defused"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("bullet_impact"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("player_hurt"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("player_death"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("weapon_fire"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("item_purchase"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("round_start"), false);
	g_interfaces.event_manager->AddListener(pEventListener, _S("round_end"), false);

	g_sdk.hooks.originals.m_FlashDuration = new C_RecvHook(g_NetvarManager->GetNetProp(_S("DT_CSPlayer"), _S("m_flFlashDuration")), C_Hooks::hkFlashDuration);
}

void C_SetUp::SetupImGui()
{
	D3DDEVICE_CREATION_PARAMETERS lParams;
	g_interfaces.direct_device->GetCreationParameters(&lParams);

	HWND hCSGO = lParams.hFocusWindow;
	if (!hCSGO)
		return;

	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	ImGui::GetStyle().WindowRounding = 10.0f;

	if (!ImGui_ImplDX9_Init(g_interfaces.direct_device))
		return;

	if (!ImGui_ImplWin32_Init(hCSGO))
		return;

	g_sdk.render.m_OldWndProc = SetWindowLongA(hCSGO, GWLP_WNDPROC, (LONG_PTR)(C_Hooks::hkWndProc));
}

void C_SetUp::SetupAddressList()
{
	g_sdk.address_list.m_ClDoResetLatch_GetBool_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("85 C0 75 28 8B 0D ? ? ? ? 81"));
	g_sdk.address_list.m_SvCheats_GetBool_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("85 C0 75 30 38 86"));
	g_sdk.address_list.m_IsPaused_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("0F B6 0D ? ? ? ? 84 C0 0F 44"));
	g_sdk.address_list.m_InPrediction_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05"));
	g_sdk.address_list.m_SetupVelocity_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0"));
	g_sdk.address_list.m_AccumulateLayers_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("84 C0 75 0D F6 87"));
	g_sdk.address_list.m_IsConnected_Call = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("84 C0 75 05 B0 01 5F"));
	g_sdk.address_list.m_SetupBonesForAttachmentQueries = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 14 83 3D ? ? ? ? ? 53"));
	g_sdk.address_list.m_DisableRenderTargetAllocationForever = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("80 B9 ? ? ? ? ? 74 0F")) + 0x2;
	g_sdk.address_list.m_CreateModel = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("53 8B D9 56 57 8D 4B 04 C7 03 ? ? ? ? E8 ? ? ? ? 6A"));
	g_sdk.address_list.m_LookupSequence = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 56 8B F1 83 BE  4C 29 00 00 00 75 14 8B 46 04 8D 4E 04 FF 50 20 85 C0 74 07 8B CE E8 BD 65"));
	g_sdk.address_list.m_SetAbsAngles = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8"));
	g_sdk.address_list.m_SetAbsOrigin = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 51 53  56 57 8B F1 E8 1F"));
	g_sdk.address_list.m_SetMergedMDL = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 57 8B F9 8B 0D ? ? ? ? 85 C9 75"));
	g_sdk.address_list.m_PredictionPlayer = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("89 35 ? ? ? ? F3 0F 10 48 20")) + 0x2;
	g_sdk.address_list.m_PredictionSeed = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04")) + 0x2;
	g_sdk.address_list.m_GetSequenceActivity = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 53 8B 5D 08 56 8B F1 83"));
	g_sdk.address_list.m_SetupBones_AttachmentHelper = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4"));
	g_sdk.address_list.m_InvalidateBoneCache = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81"));
	g_sdk.address_list.m_SequenceDuration = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 56 8B F1 83 BE 4C 29 00 00 00 75 14 8B 46 04 8D 4E 04 FF 50 20 85 C0 74 07 8B CE E8 BD"));
	g_sdk.address_list.m_SequenceDescriptor = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 79 04 00 75"));
	g_sdk.address_list.m_GetFirstSequenceAnimationTag = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 56 8B F1 83 BE  4C 29 00 00 00 75 14 8B 46 04 8D 4E 04 FF 50 20 85 C0 74 07 8B CE E8 6D"));
	g_sdk.address_list.m_KeyValuesFindKey = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 1C 53 8B D9 85 DB"));
	g_sdk.address_list.m_KeyValuesGetString = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 8B 5D 08"));
	g_sdk.address_list.m_KeyValuesSetString = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC A1 ? ? ? ? 53 56 57 8B F9 8B 08 8B 01"));
	g_sdk.address_list.m_KeyValuesLoadFromBuffer = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04"));
	g_sdk.address_list.m_InvalidatePhysicsRecursive = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3"));
	g_sdk.address_list.m_UpdateClientSideAnimation = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74"));
	g_sdk.address_list.m_PostProcess = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("83 EC 4C 80 3D")) + 0x5;
	g_sdk.address_list.m_SmokeCount = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("A3 ? ? ? ? 57 8B CB")) + 0x1;
	g_sdk.address_list.m_IsBreakableEntity = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 51 56 8B F1 85 F6 74 68"));
	g_sdk.address_list.m_FindHudElement = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	g_sdk.address_list.m_FindHudElement_Ptr = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("B9 ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 5D 08")) + 0x1;
	g_sdk.address_list.m_ClipRayToHitbox = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 F3 0F 10 42"));
	g_sdk.address_list.m_ClipTraceToEntity = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC D8 00 00 00"));
	g_sdk.address_list.m_TraceFilterSimple = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3D;
	g_sdk.address_list.m_TraceToExit = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 30 F3 0F 10 75"));
	g_sdk.address_list.m_WriteUsercmd = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D"));
	g_sdk.address_list.m_ClearDeathList = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 0C 53 56 8B 71"));
	g_sdk.address_list.m_TraceFilterSkipTwoEntities = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 81 EC BC 00 00 00 56 8B F1 8B 86")) + 550;
	g_sdk.address_list.m_HostShouldRun_Call = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("84 C0 0F 84 E0 01 00 00 A1 E8"));
	g_sdk.address_list.m_DispatchSounds = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("56 8B 35 ?? ?? ?? ?? 57  8B 3D ?? ?? ?? ?? 66 90 83 Fe FF 74 3D 6B"));
	g_sdk.address_list.m_LoadSkybox = g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
	g_sdk.address_list.m_PhysicsRunThink = g_Tools->FindPattern(g_sdk.modules.client_dll, _S("55 8B EC 83 EC 10 53 56 57 8B F9 8B 87"));
}

void C_SetUp::SetupConvarList()
{
	g_sdk.convars.m_3DSky = g_interfaces.cvar->FindVar(_S("r_3dsky"));
	g_sdk.convars.m_WeaponRecoilScale = g_interfaces.cvar->FindVar(_S("weapon_recoil_scale"));
	g_sdk.convars.m_Yaw = g_interfaces.cvar->FindVar(_S("m_yaw"));
	g_sdk.convars.m_Pitch = g_interfaces.cvar->FindVar(_S("m_pitch"));
	g_sdk.convars.m_Sensitivity = g_interfaces.cvar->FindVar(_S("sensitivity"));
	g_sdk.convars.m_ClInterp = g_interfaces.cvar->FindVar(_S("cl_interp"));
	g_sdk.convars.m_DamageBulletPenetration = g_interfaces.cvar->FindVar(_S("ff_damage_bullet_penetration"));
	g_sdk.convars.m_DamageReduction = g_interfaces.cvar->FindVar(_S("ff_damage_reduction_bullets"));
	g_sdk.convars.m_ClInterpRatio = g_interfaces.cvar->FindVar(_S("cl_interp_ratio"));
	g_sdk.convars.m_ClUpdateRate = g_interfaces.cvar->FindVar(_S("cl_updaterate"));
	g_sdk.convars.m_SvMinUpdateRate = g_interfaces.cvar->FindVar(_S("sv_minupdaterate"));
	g_sdk.convars.m_SvMaxUpdateRate = g_interfaces.cvar->FindVar(_S("sv_maxupdaterate"));
	g_sdk.convars.m_SvAcceleration = g_interfaces.cvar->FindVar(_S("sv_accelerate"));
	g_sdk.convars.m_SvClientMinInterpRatio = g_interfaces.cvar->FindVar(_S("sv_client_min_interp_ratio"));
	g_sdk.convars.m_SvClientMaxInterpRatio = g_interfaces.cvar->FindVar(_S("sv_client_max_interp_ratio"));
	g_sdk.convars.m_SvGravity = g_interfaces.cvar->FindVar(_S("sv_gravity"));
	g_sdk.convars.m_SvFriction = g_interfaces.cvar->FindVar(_S("sv_friction"));
	g_sdk.convars.m_SvStopSpeed = g_interfaces.cvar->FindVar(_S("sv_stopspeed"));
	g_sdk.convars.m_SvFootsteps = g_interfaces.cvar->FindVar(_S("sv_footsteps"));
	g_sdk.convars.m_ClWpnSwayAmount = g_interfaces.cvar->FindVar(_S("cl_wpn_sway_scale"));
	g_sdk.convars.m_ClCsmShadows = g_interfaces.cvar->FindVar(_S("cl_csm_shadows"));
	g_sdk.convars.m_ClFootContactShadows = g_interfaces.cvar->FindVar(_S("cl_foot_contact_shadows"));
	g_sdk.convars.m_ClCsmStaticPropShadows = g_interfaces.cvar->FindVar(_S("cl_csm_static_prop_shadows"));
	g_sdk.convars.m_ClCsmWorldShadows = g_interfaces.cvar->FindVar(_S("cl_csm_world_shadows"));
	g_sdk.convars.m_ClCsmViewmodelShadows = g_interfaces.cvar->FindVar(_S("cl_csm_viewmodel_shadows"));
	g_sdk.convars.m_ClCsmSpriteShadows = g_interfaces.cvar->FindVar(_S("cl_csm_sprite_shadows"));
	g_sdk.convars.m_ClCsmRopeShadows = g_interfaces.cvar->FindVar(_S("cl_csm_rope_shadows"));
	g_sdk.convars.m_ViewmodelX = g_interfaces.cvar->FindVar(_S("viewmodel_offset_x"));
	g_sdk.convars.m_ViewmodelY = g_interfaces.cvar->FindVar(_S("viewmodel_offset_y"));
	g_sdk.convars.m_ViewmodelZ = g_interfaces.cvar->FindVar(_S("viewmodel_offset_z"));
	g_sdk.convars.m_RainDensity = g_interfaces.cvar->FindVar(_S("r_raindensity"));
	g_sdk.convars.m_RainSpeed = g_interfaces.cvar->FindVar(_S("r_rainspeed"));
	g_sdk.convars.m_RainLength = g_interfaces.cvar->FindVar(_S("r_rainlength"));
	g_sdk.convars.m_RainWidth = g_interfaces.cvar->FindVar(_S("r_rainwidth"));
	g_sdk.convars.m_RainAlpha = g_interfaces.cvar->FindVar(_S("r_rainalpha"));
	g_sdk.convars.m_RainSideVel = g_interfaces.cvar->FindVar(_S("r_RainSideVel"));
	g_sdk.convars.m_WeaponDebugShowSpread = g_interfaces.cvar->FindVar(_S("weapon_debug_spread_show"));

	g_sdk.convars.m_ViewmodelX->m_fnChangeCallbacks.m_Size = 0;
	g_sdk.convars.m_ViewmodelY->m_fnChangeCallbacks.m_Size = 0;
	g_sdk.convars.m_ViewmodelZ->m_fnChangeCallbacks.m_Size = 0;

	g_sdk.convars.m_ViewmodelX->m_fMinVal = INT_MIN;
	g_sdk.convars.m_ViewmodelY->m_fMinVal = INT_MIN;
	g_sdk.convars.m_ViewmodelZ->m_fMinVal = INT_MIN;

	g_sdk.convars.m_ViewmodelX->m_fMaxVal = INT_MAX;
	g_sdk.convars.m_ViewmodelY->m_fMaxVal = INT_MAX;
	g_sdk.convars.m_ViewmodelZ->m_fMaxVal = INT_MAX;
}

void C_SetUp::SetupModuleList()
{
	g_sdk.modules.client_dll = GetModuleHandleA(_S("client.dll"));
	g_sdk.modules.engine_dll = GetModuleHandleA(_S("engine.dll"));
	g_sdk.modules.tier_dll = GetModuleHandleA(_S("tier0.dll"));
	g_sdk.modules.localize_dll = GetModuleHandleA(_S("localize.dll"));
	g_sdk.modules.file_system_dll = GetModuleHandleA(_S("filesystem_stdio.dll"));
	g_sdk.modules.shader_dll = GetModuleHandleA(_S("shaderapidx9.dll"));
	g_sdk.modules.vgui_dll = GetModuleHandleA(_S("vguimatsurface.dll"));
	g_sdk.modules.vgui2_dll = GetModuleHandleA(_S("vgui2.dll"));
	g_sdk.modules.physics_dll = GetModuleHandleA(_S("vphysics.dll"));
	g_sdk.modules.vstd_dll = GetModuleHandleA(_S("vstdlib.dll"));
	g_sdk.modules.mat_sys_dll = GetModuleHandleA(_S("materialsystem.dll"));
	g_sdk.modules.data_cache_dll = GetModuleHandleA(_S("datacache.dll"));
	g_sdk.modules.studio_render_dll = GetModuleHandleA(_S("studiorender.dll"));

	// s/o to my homies polak, kittenpopo gang
	long long amongus = 0x69690004C201B0;
	static std::string sig = _S("55 8B EC 56 8B F1 33 C0 57 8B 7D 08"); // \xE8\x00\x00\x00\x00\x84\xC0\x75\x15\xFF\x75\x10

	LPCWSTR modules[]
	{
		L"client.dll",
		L"engine.dll",
		L"server.dll",
		L"studiorender.dll",
		L"materialsystem.dll"
	};

	for (auto base : modules)
		WriteProcessMemory(GetCurrentProcess(), g_Tools->FindPattern(GetModuleHandleW(base), sig), &amongus, 5, 0);
}

void C_SetUp::SetupInterfaceList()
{
	g_interfaces.entity_list = reinterpret_cast<C_ClientEntityList*>(g_Tools->GetInterface(g_sdk.modules.client_dll, _S("VClientEntityList003")));
	g_interfaces.client = reinterpret_cast<C_BaseClientDLL*>(g_Tools->GetInterface(g_sdk.modules.client_dll, _S("VClient018")));
	g_interfaces.prediction = reinterpret_cast<C_Prediction*>(g_Tools->GetInterface(g_sdk.modules.client_dll, _S("VClientPrediction001")));
	g_interfaces.game_movement = reinterpret_cast<C_GameMovement*>(g_Tools->GetInterface(g_sdk.modules.client_dll, _S("GameMovement001")));
	g_interfaces.model_render = reinterpret_cast<C_ModelRender*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("VEngineModel016")));
	g_interfaces.model_info = reinterpret_cast<C_ModelInfoClient*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("VModelInfoClient004")));
	g_interfaces.event_manager = reinterpret_cast<C_GameEventManager*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("GAMEEVENTSMANAGER002")));
	g_interfaces.render_view = reinterpret_cast<C_RenderView*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("VEngineRenderView014")));
	g_interfaces.engine = reinterpret_cast<C_EngineClient*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("VEngineClient014")));
	g_interfaces.engine_sound = reinterpret_cast<C_EngineSound*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("IEngineSoundClient003")));
	g_interfaces.trace = reinterpret_cast<C_EngineTrace*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("EngineTraceClient004")));
	g_interfaces.debug_overlay = reinterpret_cast<C_DebugOverlay*>(g_Tools->GetInterface(g_sdk.modules.engine_dll, _S("VDebugOverlay004")));
	g_interfaces.surface = reinterpret_cast<C_Surface*>(g_Tools->GetInterface(g_sdk.modules.vgui_dll, _S("VGUI_Surface031")));
	g_interfaces.vgui_panel = reinterpret_cast<C_Panel*>(g_Tools->GetInterface(g_sdk.modules.vgui2_dll, _S("VGUI_Panel009")));
	g_interfaces.material_system = reinterpret_cast<C_MaterialSystem*>(g_Tools->GetInterface(g_sdk.modules.mat_sys_dll, _S("VMaterialSystem080")));
	g_interfaces.prop_physics = reinterpret_cast<C_PhysicsSurfaceProps*>(g_Tools->GetInterface(g_sdk.modules.physics_dll, _S("VPhysicsSurfaceProps001")));
	g_interfaces.localize = reinterpret_cast<C_Localize*>(g_Tools->GetInterface(g_sdk.modules.localize_dll, _S("Localize_001")));
	g_interfaces.file_system = reinterpret_cast<C_FileSystem*>(g_Tools->GetInterface(g_sdk.modules.file_system_dll, _S("VBaseFileSystem011")));
	g_interfaces.mdl_cache = reinterpret_cast<C_MDLCache*>(g_Tools->GetInterface(g_sdk.modules.data_cache_dll, _S("MDLCache004")));
	g_interfaces.cvar = reinterpret_cast<C_Cvar*>(g_Tools->GetInterface(g_sdk.modules.vstd_dll, _S("VEngineCvar007")));
	g_interfaces.studio_render = reinterpret_cast<C_StudioRender*>(g_Tools->GetInterface(g_sdk.modules.studio_render_dll, _S("VStudioRender026")));

	g_interfaces.mem_alloc = *(C_MemAlloc**)(GetProcAddress(g_sdk.modules.tier_dll, _S("g_pMemAlloc")));
	g_interfaces.client_state = **(C_ClientState***)(g_Tools->FindPattern(g_sdk.modules.engine_dll, _S("A1 ? ? ? ? 8B 80 ? ? ? ? C3")) + 0x1);
	g_interfaces.move_helper = **(C_MoveHelper***)(g_Tools->FindPattern(g_sdk.modules.client_dll, _S("8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01")) + 0x2);
	g_interfaces.globals = **(C_GlobalVarsBase***)(g_Tools->FindPattern(g_sdk.modules.client_dll, _S("A1 ? ? ? ? 5E 8B 40 10")) + 0x1);
	g_interfaces.direct_device = **(IDirect3DDevice9***)(g_Tools->FindPattern(g_sdk.modules.shader_dll, _S("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 0x1);
	g_interfaces.input = *(C_Input**)(g_Tools->FindPattern(g_sdk.modules.client_dll, _S("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10")) + 0x1);
	g_interfaces.game_rules = *(C_GameRules***)(g_Tools->FindPattern(g_sdk.modules.client_dll, _S("E8 ? ? ? ? A1 ? ? ? ? 85 C0 0F 84 ? ? ? ?")) + 0x6);
	g_interfaces.view_render_beams = *(C_ViewRenderBeams**)(g_Tools->FindPattern(g_sdk.modules.client_dll, _S("A1 ? ? ? ? FF 10 A1 ? ? ? ? B9")) + 0x1);
	g_interfaces.glow_manager = *(IGlowObjectManager**)(g_Tools->FindPattern(g_sdk.modules.client_dll, _S("0F 11 05 ? ? ? ? 83 C8 01")) + 0x3);
}