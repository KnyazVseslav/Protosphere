#pragma once

#ifndef INCLUDE_HLSL
#define INCLUDE_HLSL

#include <d3dcommon.h>

#include <string>




class IncludeHLSL: public ID3DInclude
{

public:

	IncludeHLSL(const char* system_path = "");

	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType,
						   LPCSTR pFileName,
						   LPCVOID pParentData,
						   LPCVOID* pData,
						   UINT* pBytes);
	
	HRESULT __stdcall Close(LPCVOID pData);

private:

	std::wstring m_system_path;
	std::wstring m_root_path;


};


#endif

