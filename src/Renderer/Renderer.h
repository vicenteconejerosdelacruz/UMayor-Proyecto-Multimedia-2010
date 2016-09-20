#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9Shader.h>
#include "VertexBuffer.h"
#include "FaceBuffer.h"
#include "Model.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4x4.h"
#include "Cursor.h"
#include <vector>
#include <map>

#define g_d3dDevice g_renderer->data.d3ddev

struct VertexDeclarationContainer
{
	D3DVERTEXELEMENT9*				m_elements;
	LPDIRECT3DVERTEXDECLARATION9	m_vertexDeclaration;
	struct _VS
	{
		D3DXMACRO*						m_macros;
		LPD3DXBUFFER					m_CompilatedShader;
		LPD3DXCONSTANTTABLE				m_ShaderConstantList;
		LPDIRECT3DVERTEXSHADER9			m_Shader;
		_VS()
		{
			m_macros = NULL;
			m_CompilatedShader = NULL;
			m_ShaderConstantList = NULL;
			m_Shader = NULL;
		}
	}VS;
	struct _PS
	{
		D3DXMACRO*						m_macros;
		LPD3DXBUFFER					m_CompilatedShader;
		LPD3DXCONSTANTTABLE				m_ShaderConstantList;
		LPDIRECT3DPIXELSHADER9			m_Shader;
		_PS()
		{
			m_macros = NULL;
			m_CompilatedShader = NULL;
			m_ShaderConstantList = NULL;
			m_Shader = NULL;
		}
	}PS;

	VertexDeclarationContainer()
	{
		m_elements=NULL;
		m_vertexDeclaration=NULL;
	}
};

template<unsigned int ATT,WORD Stream,WORD Offset,BYTE Type,BYTE Method,BYTE Usage,BYTE UsageIndex>
struct FillDeclaration
{
	static void Fill(LPD3DVERTEXELEMENT9& elem,D3DXMACRO** VSmacro=NULL,D3DXMACRO** PSmacro=NULL)
	{
		elem->Stream = Stream;
		elem->Offset = Offset;
		elem->Type = Type;
		elem->Method = Method;
		elem->Usage = Usage;
		elem->UsageIndex = UsageIndex;
		elem++;
		if(VSmacro)
		{
			(*VSmacro)->Name = shader_code<ATT>::code.c_str();
			(*VSmacro)->Definition = NULL;
			(*VSmacro)++;
		}
		if(PSmacro)
		{
			(*PSmacro)->Name = shader_code<ATT>::code.c_str();
			(*PSmacro)->Definition = NULL;
			(*PSmacro)++;
		}
	}
};

template<WORD Stream,WORD Offset,BYTE Type,BYTE Method,BYTE Usage,BYTE UsageIndex>
struct FillDeclaration<0,Stream,Offset,Type,Method,Usage,UsageIndex>
{
	static void Fill(LPD3DVERTEXELEMENT9& elem,D3DXMACRO** VSmacro=NULL,D3DXMACRO** PSmacro=NULL)
	{
	}
};

struct RENDERERData
{
	// global declarations
	LPDIRECT3D9					d3d;				// the pointer to our Direct3D interface
	LPDIRECT3DDEVICE9			d3ddev;		// the pointer to the device class
	HWND						hWnd;

	MATRIX4X4 WorldTransformation;
	MATRIX4X4 ViewTransformation;
	MATRIX4X4 ProjectionTransformation;	
	MATRIX4X4 WorldViewProjectionTransformation;

	LPDIRECT3DVERTEXSHADER9     m_vertexShader;
	LPDIRECT3DPIXELSHADER9		m_pixelShader;

	RENDERERData()
	{
		d3d = NULL;
		d3ddev = NULL;
		hWnd = NULL;
		WorldTransformation.SetIdentity();
		ViewTransformation.SetIdentity();
		ProjectionTransformation.SetIdentity();
		WorldViewProjectionTransformation.SetIdentity();
		m_vertexShader = NULL;
		m_pixelShader = NULL;

	}
};

enum MENU_COMMANDS
{
	 MC_OPEN_FILE
	,MC_RECOMPILE_SHADER
};

class RENDERER
{
public:
	RENDERERData data;	

	std::vector<Model*>	m_modelList;

	Cursor m_Cursor;

	bool m_doRecompile;
	unsigned int resX,resY;

	RENDERER(void);
	~RENDERER(void);

	void InitWindow(HINSTANCE hInstance,int nCmdShow);
	void InitVertexes();
	void CreateDevice();
	void Render();
	void ShutDown();
	void LoadModel(char* fileNameFullPath);
	void SetCamera(Vector3 Pos,Vector3 Fw,Vector3 Up=Vector3::Up);
	void RecompileShaders();
	void LocateCameraToWatchModel(Model* model);
	unsigned int GetResolutionX(){return resX;}
	unsigned int GetResolutionY(){return resY;}
};

extern RENDERER* g_renderer;
extern std::map<unsigned int,VertexDeclarationContainer> s_VertexDeclarations;

#endif