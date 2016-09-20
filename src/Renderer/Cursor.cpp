#include "stdafx.h"
#include "Renderer.h"
#include "Cursor.h"
#include "VBCreator.h"


Cursor::Cursor()
{
	m_texture = NULL;
	m_width = 0;
	m_height = 0;
	m_x = 0;
	m_y = 0;
	m_enable = false;
}

bool Cursor::Init(unsigned int width,unsigned int height)
{
	m_width = width;
	m_height = height;

	HRESULT res = D3DXCreateTextureFromFile(g_d3dDevice,"data\\art\\gui\\hand-pointer.png",&m_texture);

	SAFERELEASE(VS.m_CompilatedShader)
	SAFERELEASE(VS.m_ShaderConstantList)
	SAFERELEASE(PS.m_CompilatedShader)
	SAFERELEASE(PS.m_ShaderConstantList)

	LPD3DXBUFFER error=NULL;
	res = D3DXCompileShaderFromFile("cursor.sha",VS.m_macros,NULL,"main_vs","vs_3_0",0,&VS.m_CompilatedShader,&error,&VS.m_ShaderConstantList);

	if(res!=S_OK)
	{
		static char errorCode[1024];
		memset(errorCode,0,sizeof(errorCode));
		memcpy(errorCode,error->GetBufferPointer(),error->GetBufferSize());
		MessageBox(NULL,errorCode,"FAIL!",MB_OK);
	}
	else
	{
		res = g_d3dDevice->CreateVertexShader((DWORD*)VS.m_CompilatedShader->GetBufferPointer(),&VS.m_Shader);
	}

	res = D3DXCompileShaderFromFile("cursor.sha",PS.m_macros,NULL,"main_ps","ps_3_0",0,&PS.m_CompilatedShader,&error,&PS.m_ShaderConstantList);

	if(res!=S_OK)
	{
		static char errorCode[1024];
		memset(errorCode,0,sizeof(errorCode));
		memcpy(errorCode,error->GetBufferPointer(),error->GetBufferSize());
		MessageBox(NULL,errorCode,"FAIL!",MB_OK);
	}
	else
	{
		res = g_d3dDevice->CreatePixelShader((DWORD*)PS.m_CompilatedShader->GetBufferPointer(),&PS.m_Shader);
	}

	return true;
}
bool Cursor::Render()
{
	if(!m_enable)
		return false;

	float centerX	= -1.0f+2.0f*((float)m_x)/(float)m_width;
	float centerY	=  1.0f-2.0f*((float)m_y)/(float)m_height;
	float cursorHalfWidth	= 0.07f;
	float cursorHalfHeight	= 0.1f;

	Vertex<ATT_POS|ATT_UV0> vertexes[]=
	{
		 {{centerX - cursorHalfWidth,centerY - cursorHalfHeight,0.0f},{ 0.0f, 1.0f}}
		,{{centerX - cursorHalfWidth,centerY + cursorHalfHeight,0.0f},{ 0.0f, 0.0f}}
		,{{centerX + cursorHalfWidth,centerY - cursorHalfHeight,0.0f},{ 1.0f, 1.0f}}		
		,{{centerX + cursorHalfWidth,centerY + cursorHalfHeight,0.0f},{ 1.0f, 0.0f}}
	};

	HRESULT res;

	res = g_d3dDevice->SetVertexDeclaration(s_VertexDeclarations[ATT_POS|ATT_UV0].m_vertexDeclaration);
	res = g_d3dDevice->SetVertexShader(VS.m_Shader);
	res = g_d3dDevice->SetPixelShader(PS.m_Shader);

	res = g_d3dDevice->SetTexture(0,m_texture);
	res = g_d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,(sizeof(vertexes)/sizeof(vertexes[0]))-2,&vertexes,sizeof(vertexes[0]));

	return true;
}