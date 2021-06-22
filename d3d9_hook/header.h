#ifndef HEADER_H
#define HEADER_H

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

typedef HRESULT(WINAPI* tDrawIndexedPrimitive)(LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
tDrawIndexedPrimitive oDrawIndexedPrimitive;

BYTE codeFragment_drawindex[5] = { 0, 0, 0, 0, 0 };
BYTE jump_drawindex[5] = { 0, 0, 0, 0, 0 };
DWORD savedProtection_drawindex = 0;

DWORD drawindexOffset = 0;

bool isEnabled = false;

#endif