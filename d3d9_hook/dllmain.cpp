#include "header.h"

//Gets offset to needed function in virtual method table using new created d3d9 device. We also create new empty window to use it to create d3d9 device
void GetDevice9Methods()
{
	HWND hWnd = CreateWindowA("STATIC", "dummy", 0, 0, 0, 0, 0, 0, 0, 0, 0);
	HMODULE hD3D9 = LoadLibrary(L"d3d9");
	IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);

	//Retrieves the current display mode of the adapter
	D3DDISPLAYMODE d3ddm;
	d3d->GetAdapterDisplayMode(0, &d3ddm);

	//Creates d3d9 device with D3DPRESENT_PARAMETERS structure that describes the presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	IDirect3DDevice9* d3dDevice = 0;
	d3d->CreateDevice(0, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice);
	
	//Gets offset from start of d3d device to address of DrawIndexedPrimitive function
	DWORD* vTable = (DWORD*)(*((DWORD*)d3dDevice));
	drawindexOffset = vTable[82] - (DWORD)hD3D9;

	//Releases objects
	d3dDevice->Release();
	d3d->Release();
	FreeLibrary(hD3D9);
	CloseHandle(hWnd);
}

//Hooked function that will be executed instead of original function in the process
HRESULT WINAPI hkDrawIndexedPrimitive(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT PrimitiveCount)
{
	//Restores bytes of original function
	BYTE* codeDest = (BYTE*)oDrawIndexedPrimitive;
	codeDest[0] = codeFragment_drawindex[0];
	*((DWORD*)(codeDest + 1)) = *((DWORD*)(codeFragment_drawindex + 1));

	if (isEnabled)
	{
		if (PrimitiveCount > 2)
		{
			pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
		else
		{
			pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		}
	}
	else
	{
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}

	//Calls original function
	HRESULT res = oDrawIndexedPrimitive(pDevice, PrimType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, PrimitiveCount);

	//Restores bytes of hooked function so process of the game will execute hooked function instead of original function 
	codeDest[0] = jump_drawindex[0];
	*((DWORD*)(codeDest + 1)) = *((DWORD*)(jump_drawindex + 1));
	return res;
}

//Performs code injection
void HookDevice9Methods()
{
	HMODULE hD3D9 = GetModuleHandle(L"d3d9.dll");

	oDrawIndexedPrimitive = (tDrawIndexedPrimitive)((DWORD)hD3D9 + drawindexOffset); //Gets address of original function

	jump_drawindex[0] = 0xE9;
	DWORD addr_drawindex = (DWORD)hkDrawIndexedPrimitive - (DWORD)oDrawIndexedPrimitive - 5; //Gets address of injection (where to jump)
	memcpy(jump_drawindex + 1, &addr_drawindex, sizeof(DWORD)); //Adds address after 'jmp' opcode to get assembler instruction jmp [address]
	memcpy(codeFragment_drawindex, oDrawIndexedPrimitive, 5); //Saves address of original function
	VirtualProtect(oDrawIndexedPrimitive, 8, PAGE_EXECUTE_READWRITE, &savedProtection_drawindex);
	memcpy(oDrawIndexedPrimitive, jump_drawindex, 5); //Replaces address of original function with assembler instruction jmp [address] (jump to injection where hook is)
}

DWORD WINAPI TF(void* lParam)
{
	GetDevice9Methods();
	HookDevice9Methods();
	return 0;
}

DWORD WINAPI KeyboardHook(void* lParam)
{
	while (1)
	{
		if (GetAsyncKeyState(VK_NUMPAD1))
		{
			isEnabled = !isEnabled;
		}
		Sleep(100);
	}
	return 0;
}

int WINAPI DllMain(HINSTANCE hInst, DWORD ul_reason_for_call, void* lpReserver)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, &TF, 0, 0, 0);
		CreateThread(0, 0, &KeyboardHook, 0, 0, 0);
	}
	return 1;
}