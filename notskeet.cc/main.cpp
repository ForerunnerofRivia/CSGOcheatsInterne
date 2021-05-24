#include <Windows.h>
#include <iostream>
#include <thread>
#include "detours.h"
#include "Patternscaning.h"
#include "offsets.h"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx9.h"
#include "../ImGui/imgui_impl_win32.h"
#include <time.h>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

bool triggerbot_actived;
bool glow_actived;
bool bhop_actived;
bool radarhack_actived;
bool noflash_actived;
bool shootdelay_actived;
float glowennemiescolor[] = { 255.0f, 0.0f, 0.0f, };
float glowteammatecolor[] = { 0.0f, 255.0f, 0.0f, };
float glowopacity = 1.0f;
int shootdelay = 0;

uintptr_t moduleClient;
uintptr_t pLocalPlayer;
uintptr_t LocalPlayer;

//triggerbot

void triggerbotRun()
{
	if (triggerbot_actived)
	{
		if (LocalPlayer != NULL)
		{
			int myTeam = *(int*)(LocalPlayer + hazedumper::netvars::m_iTeamNum);
			int CrossEnt = *(int*)(LocalPlayer + hazedumper::netvars::m_iCrosshairId);
			if (CrossEnt != NULL && CrossEnt > 0 && CrossEnt < 64)
			{
				DWORD entInCross = *(DWORD*)(moduleClient + hazedumper::signatures::dwEntityList + (CrossEnt - 1) * 0x10);
				int EntH = *(int*)(entInCross + hazedumper::netvars::m_iHealth);
				int EntT = *(int*)(entInCross + hazedumper::netvars::m_iTeamNum);
				if (entInCross != NULL && EntH > 0 && myTeam != EntT && shootdelay_actived != true)
				{
					*(int*)(moduleClient + hazedumper::signatures::dwForceAttack) = 6;
				}
				if (entInCross != NULL && EntH > 0 && myTeam != EntT && shootdelay_actived == true)
				{
				std::this_thread::sleep_for(std::chrono::milliseconds(shootdelay));
				*(int*)(moduleClient + hazedumper::signatures::dwForceAttack) = 6;
				}
			}
		}
	}
}

//glowhack

void glow()
{
	if (glow_actived)
	{
		uintptr_t pGlowObjectManager = moduleClient + hazedumper::signatures::dwGlowObjectManager;
		uintptr_t GlowObjectManager = *(uintptr_t*)pGlowObjectManager;
		if (LocalPlayer != NULL && GlowObjectManager != NULL)
		{
			int myTeam = *(int*)(LocalPlayer + hazedumper::netvars::m_iTeamNum);
			for (int i = 0; i < 64; i++)
			{
				DWORD glowEntity = *(DWORD*)(moduleClient + hazedumper::signatures::dwEntityList + i * 0x10);
				if (glowEntity == NULL) continue;
				int glowIndex = *(int*)(glowEntity + hazedumper::netvars::m_iGlowIndex);
				int entityTeam = *(int*)(glowEntity + hazedumper::netvars::m_iTeamNum);
				if (entityTeam == myTeam)
				{
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0x4)) = glowteammatecolor[0];
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0x8)) = glowteammatecolor[1];
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0xC)) = glowteammatecolor[2];
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0x10)) = glowopacity;
				}
				else
				{
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0x4)) = glowennemiescolor[0];
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0x8)) = glowennemiescolor[1];
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0xC)) = glowennemiescolor[2];
					*(float*)((GlowObjectManager)+((glowIndex * 0x38) + 0x10)) = glowopacity;
				}
				*(bool*)((GlowObjectManager)+((glowIndex * 0x38) + 0x24)) = true;
				*(bool*)((GlowObjectManager)+((glowIndex * 0x38) + 0x25)) = false;
				
			}
		}
	}
}


//bhop
void bhop()
{
	if (bhop_actived)
	{
		
		if (LocalPlayer != NULL)
		{
			if(*(int*)(LocalPlayer + hazedumper::netvars::m_fFlags) != 256 && GetAsyncKeyState(VK_SPACE))
			{
				*(int*)(moduleClient + hazedumper::signatures::dwForceJump) = 6;
			}
		}
	}
}

//radarhack
void radarhack()
{
	if (radarhack_actived)
	{
		for (int i = 0; i < 64; i++)
		{
			DWORD entity = *(DWORD*)(moduleClient + hazedumper::signatures::dwEntityList + i * 0x10);
			if (entity != NULL)
			{
				*(bool*)(entity + hazedumper::netvars::m_bSpotted) = true;
			}
		}

	}
}

//noflash
void noflash()
{
	if (noflash_actived)
	{
		if (*(float*)(LocalPlayer + hazedumper::netvars::m_flFlashMaxAlpha) >= 0.0f)
		{
			*(float*)(LocalPlayer + hazedumper::netvars::m_flFlashMaxAlpha) = 0.0f;
		}
	}
}



void Run()
{
	
	glow();
	bhop();
	radarhack();
	noflash();
	triggerbotRun();
}


const char* windowName = "Counter-Strike: Global Offensive";

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* f_EndScene)(IDirect3DDevice9* pDevice); // Our function prototype 
f_EndScene oEndScene; // Original Endscene

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

WNDPROC oWndProc;
bool menu_showed = false;
HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice) // Our hooked endscene
{
	moduleClient = (uintptr_t)GetModuleHandle(TEXT("client_panorama.dll"));
	pLocalPlayer = moduleClient + hazedumper::signatures::dwLocalPlayer;
	LocalPlayer = *(uintptr_t*)pLocalPlayer;

	//création et lancement des threads de hacks
	std::thread RunCheat(Run);
	RunCheat.join();

	if (GetAsyncKeyState(VK_INSERT) & 1) menu_showed = !menu_showed;
	static bool init = true;
	if (init)
	{
		
		Sleep(100);


		
	}
	return oEndScene(pDevice); // Call original ensdcene so the game can draw
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}


DWORD WINAPI MainThread(LPVOID param) // Our main thread
{
	HWND  window = FindWindowA(NULL, windowName);

	oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		return false;

	D3DPRESENT_PARAMETERS d3dpp{ 0 };
	d3dpp.hDeviceWindow = window, d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD, d3dpp.Windowed = TRUE;

	IDirect3DDevice9* Device = nullptr;
	if (FAILED(pD3D->CreateDevice(0, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &Device)))
	{
		pD3D->Release();
		return false;
	}


	void** pVTable = *reinterpret_cast<void***>(Device);

	if (Device)
		Device->Release(), Device = nullptr;

	oEndScene = (f_EndScene)DetourFunction((PBYTE)pVTable[42], (PBYTE)Hooked_EndScene);

	return false;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: // Gets runned when injected
		CreateThread(0, 0, MainThread, hModule, 0, 0); // Creates our thread
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

