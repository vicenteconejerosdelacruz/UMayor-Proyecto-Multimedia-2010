#include "stdafx.h"
#include "def.h"
#include <windows.h>
#include <windowsx.h>

#include "Renderer.h"
#include "VBCreator.h"
#include <map>
#include <vector>
#include <string>

#define VERTEX_MEMBER(_type,_name)\
template<unsigned int T,int hasMember=has_##_name<T>::value>\
struct get_##_name##_offset\
{\
	enum {offset = 0};\
};\
template<unsigned int T>\
struct get_##_name##_offset<T,1>\
{\
	enum {offset = offsetof(Vertex<T>,_name)};\
};\

VERTEX_MEMBER(float3,pos);
VERTEX_MEMBER(uint,diffuse);
VERTEX_MEMBER(float3,normal);
VERTEX_MEMBER(float3,tangent);
VERTEX_MEMBER(float3,bitangent);
VERTEX_MEMBER(float2,uv0);
VERTEX_MEMBER(float2,uv1);
VERTEX_MEMBER(float2,uv2);
VERTEX_MEMBER(float2,uv3);
#undef VERTEX_MEMBER

RENDERER* g_renderer=NULL;
extern char FileDirectory[MAX_PATH];
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
vbCreatorMap s_VBCreator;

D3DVIEWPORT9 viewData = { 0, 0, 0, 0, 0.0f, 1.0f };

std::string shader_code<ATT_POS			>::code = "_HAS_POS";
std::string shader_code<ATT_DIFFUSE		>::code = "_HAS_DIFFUSE";
std::string shader_code<ATT_NORMAL		>::code = "_HAS_NORMAL";
std::string shader_code<ATT_TANGENT		>::code = "_HAS_TANGENT";
std::string shader_code<ATT_BITANGENT	>::code = "_HAS_BITANGENT";
std::string shader_code<ATT_UV0			>::code = "_HAS_UV0";
std::string shader_code<ATT_UV1			>::code = "_HAS_UV1";
std::string shader_code<ATT_UV2			>::code = "_HAS_UV2";
std::string shader_code<ATT_UV3			>::code = "_HAS_UV3";

std::map<unsigned int,VertexDeclarationContainer> s_VertexDeclarations;

template<unsigned int _type>
struct CreateVertexDeclaration
{
	static void Create()
	{
		HRESULT res = 0;
		unsigned int nElements =	 0
									+has_pos<_type>::value
									+has_diffuse<_type>::value
									+has_normal<_type>::value
									+has_tangent<_type>::value
									+has_bitangent<_type>::value
									+has_uv0<_type>::value
									+has_uv1<_type>::value
									+has_uv2<_type>::value
									+has_uv3<_type>::value;

		s_VertexDeclarations[_type].VS.m_macros = SAFENEW(D3DXMACRO[nElements+2]);
		s_VertexDeclarations[_type].PS.m_macros = SAFENEW(D3DXMACRO[nElements+2]);

		D3DXMACRO* macroVS = s_VertexDeclarations[_type].VS.m_macros;
		D3DXMACRO* macroPS = s_VertexDeclarations[_type].PS.m_macros;

		macroVS->Name="_VERTEX_SHADER";
		macroVS->Definition="";
		macroVS++;

		macroPS->Name="_PIXEL_SHADER";
		macroPS->Definition="";
		macroPS++;

		D3DVERTEXELEMENT9* dec = SAFENEW(D3DVERTEXELEMENT9[nElements+1]);
		D3DVERTEXELEMENT9* decPtr = dec;
		
		FillDeclaration<ATT_POS&_type		,0		,get_pos_offset<_type>::offset			,D3DDECLTYPE_FLOAT3		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_POSITION	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_DIFFUSE&_type	,0		,get_diffuse_offset<_type>::offset		,D3DDECLTYPE_D3DCOLOR	,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_COLOR		,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_NORMAL&_type	,0		,get_normal_offset<_type>::offset		,D3DDECLTYPE_FLOAT3		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_NORMAL	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_TANGENT&_type	,0		,get_tangent_offset<_type>::offset		,D3DDECLTYPE_FLOAT3		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_TANGENT	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_BITANGENT&_type	,0		,get_bitangent_offset<_type>::offset	,D3DDECLTYPE_FLOAT3		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_BINORMAL	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_UV0&_type		,0		,get_uv0_offset<_type>::offset			,D3DDECLTYPE_FLOAT2		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_TEXCOORD	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_UV1&_type		,0		,get_uv1_offset<_type>::offset			,D3DDECLTYPE_FLOAT2		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_TEXCOORD	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_UV2&_type		,0		,get_uv2_offset<_type>::offset			,D3DDECLTYPE_FLOAT2		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_TEXCOORD	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<ATT_UV3&_type		,0		,get_uv3_offset<_type>::offset			,D3DDECLTYPE_FLOAT2		,D3DDECLMETHOD_DEFAULT	,D3DDECLUSAGE_TEXCOORD	,0>::Fill(decPtr,&macroVS,&macroPS);
		FillDeclaration<1					,0xFF	,0										,D3DDECLTYPE_UNUSED		,0						,0						,0>::Fill(decPtr);

		macroVS->Definition=NULL;
		macroVS->Name=NULL;
		macroPS->Definition=NULL;
		macroPS->Name=NULL;

		s_VertexDeclarations[_type].m_elements = dec;
		res = g_renderer->data.d3ddev->CreateVertexDeclaration(dec,&s_VertexDeclarations[_type].m_vertexDeclaration);
	}
};

RENDERER::RENDERER(void)
{
	m_doRecompile = false;
	resX=resY=0;
}

RENDERER::~RENDERER(void)
{
}

void RENDERER::InitWindow(HINSTANCE hInstance,int nCmdShow)
{

    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = "WindowClass";

    RegisterClassEx(&wc);

	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	resX = desktop.right;
	resY = desktop.bottom;
	viewData.Height = resY;
	viewData.Width = resX;

    data.hWnd = CreateWindowEx(NULL,
                          "WindowClass",
                          "WiiMote Umayor",
                          WS_OVERLAPPEDWINDOW,
                          0, 0,
                          resX, resY,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(data.hWnd, nCmdShow);

	D3DXMatrixPerspectiveFovLH((D3DXMATRIX*)&data.ProjectionTransformation.m,D3DXToRadian(45),(FLOAT)RESOLUTION_X / (FLOAT)RESOLUTION_Y,1.0f,1000.0f);
}

void RENDERER::InitVertexes()
{
	s_VBCreator[ATT_POS]=CreateVertexBuffer<ATT_POS>;
	s_VBCreator[ATT_POS|ATT_UV0]=CreateVertexBuffer<ATT_POS|ATT_UV0>;
	s_VBCreator[ATT_POS|ATT_DIFFUSE]=CreateVertexBuffer<ATT_POS|ATT_DIFFUSE>;
	s_VBCreator[ATT_POS|ATT_NORMAL]=CreateVertexBuffer<ATT_POS|ATT_NORMAL>;
	s_VBCreator[ATT_POS|ATT_NORMAL|ATT_DIFFUSE]=CreateVertexBuffer<ATT_POS|ATT_NORMAL|ATT_DIFFUSE>;
	s_VBCreator[ATT_POS|ATT_NORMAL|ATT_UV0]=CreateVertexBuffer<ATT_POS|ATT_NORMAL|ATT_UV0>;
	s_VBCreator[ATT_POS|ATT_NORMAL|ATT_TANGENT|ATT_BITANGENT|ATT_UV0]=CreateVertexBuffer<ATT_POS|ATT_NORMAL|ATT_TANGENT|ATT_BITANGENT|ATT_UV0>;

	CreateVertexDeclaration<ATT_POS>::Create();
	CreateVertexDeclaration<ATT_POS|ATT_UV0>::Create();
	CreateVertexDeclaration<ATT_POS|ATT_DIFFUSE>::Create();
	CreateVertexDeclaration<ATT_POS|ATT_NORMAL>::Create();
	CreateVertexDeclaration<ATT_POS|ATT_NORMAL|ATT_DIFFUSE>::Create();
	CreateVertexDeclaration<ATT_POS|ATT_NORMAL|ATT_UV0>::Create();
	CreateVertexDeclaration<ATT_POS|ATT_NORMAL|ATT_TANGENT|ATT_BITANGENT|ATT_UV0>::Create();

	RecompileShaders();
}

void RENDERER::CreateDevice()
{
	data.d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = data.hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferWidth = RESOLUTION_X;
	d3dpp.BackBufferHeight = RESOLUTION_Y;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // create a device class using this information and the info from the d3dpp stuct
   data.d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      data.hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &data.d3ddev);
	data.d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);    // turn off the 3D lighting
	data.d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	data.d3ddev->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000001);
	data.d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE); 
	data.d3ddev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	InitVertexes();
	LoadModel("data\\art\\models\\Collada\\duck.dae");
	//LoadModel("data\\art\\models\\IRRMesh\\spider.irrmesh");
	//LoadModel("data\\art\\models\\3DS\\jeep1.3ds");
	//LoadModel("data\\art\\models\\B3D\\turtle1.b3d");
	m_Cursor.Init(RESOLUTION_X,RESOLUTION_Y);
}

void RENDERER::Render()
{
	if(m_doRecompile)
	{
		m_doRecompile=false;
		RecompileShaders();
	}

   // clear the window to a deep blue
	data.d3ddev->SetViewport(&viewData);
    data.d3ddev->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
	data.d3ddev->BeginScene();    // begins the 3D scene

	D3DXMatrixMultiply((D3DXMATRIX*)data.WorldViewProjectionTransformation.m,(D3DXMATRIX*)data.ViewTransformation.m,(D3DXMATRIX*)data.ProjectionTransformation.m);

	for(unsigned int i=0;i<m_modelList.size();i++)
	{
		m_modelList[i]->Render();
	}

	m_Cursor.Render();

	g_d3dDevice->SetVertexShader(NULL);
	g_d3dDevice->SetPixelShader(NULL);
    data.d3ddev->EndScene();    // ends the 3D scene
    data.d3ddev->Present(NULL, NULL, NULL, NULL);   // displays the created frame on the screen
}

void RENDERER::ShutDown()
{
    data.d3ddev->Release();    // close and release the 3D device
    data.d3d->Release();    // close and release Direct3D
}

void RENDERER::LoadModel(char* fileNameFullPath)
{
	m_modelList.clear();

	Model *model = new Model();
	if(model->Load(fileNameFullPath))
	{
		m_modelList.push_back(model);
		LocateCameraToWatchModel(model);
	}
	else
		delete model;

	
}

void RENDERER::SetCamera(Vector3 Pos,Vector3 Fw,Vector3 Up)
{
	Vector3 right=Fw^Up;
	Vector3 rUp = right^Fw;

	data.ViewTransformation.SetView(Pos,Fw,rUp);
}

void RENDERER::RecompileShaders()
{
	g_d3dDevice->SetVertexShader(NULL);
	g_d3dDevice->SetPixelShader(NULL);
	g_d3dDevice->SetStreamSource(0,NULL,0,0);

	for(std::map<unsigned int,VertexDeclarationContainer>::iterator it = s_VertexDeclarations.begin();it!=s_VertexDeclarations.end();it++)
	{
		SAFERELEASE(it->second.VS.m_CompilatedShader)
		SAFERELEASE(it->second.VS.m_ShaderConstantList)
		SAFERELEASE(it->second.PS.m_CompilatedShader)
		SAFERELEASE(it->second.PS.m_ShaderConstantList)

		LPD3DXBUFFER error=NULL;
		HRESULT res=0;
		res = D3DXCompileShaderFromFile("shader.sha",it->second.VS.m_macros,NULL,"main_vs","vs_3_0",0,&it->second.VS.m_CompilatedShader,&error,&it->second.VS.m_ShaderConstantList);

		if(res!=S_OK)
		{
			static char errorCode[1024];
			memset(errorCode,0,sizeof(errorCode));
			memcpy(errorCode,error->GetBufferPointer(),error->GetBufferSize());
			MessageBox(NULL,errorCode,"FAIL!",MB_OK);
		}
		else
		{
			res = g_d3dDevice->CreateVertexShader((DWORD*)it->second.VS.m_CompilatedShader->GetBufferPointer(),&it->second.VS.m_Shader);
		}

		res = D3DXCompileShaderFromFile("shader.sha",it->second.PS.m_macros,NULL,"main_ps","ps_3_0",0,&it->second.PS.m_CompilatedShader,&error,&it->second.PS.m_ShaderConstantList);

		if(res!=S_OK)
		{
			static char errorCode[1024];
			memset(errorCode,0,sizeof(errorCode));
			memcpy(errorCode,error->GetBufferPointer(),error->GetBufferSize());
			MessageBox(NULL,errorCode,"FAIL!",MB_OK);
		}
		else
		{
			res = g_d3dDevice->CreatePixelShader((DWORD*)it->second.PS.m_CompilatedShader->GetBufferPointer(),&it->second.PS.m_Shader);
		}	
	}
}

extern Vector3 g_Pos;
extern Vector3 g_Fw;

void RENDERER::LocateCameraToWatchModel(Model* model)
{
	AABBOX& Box = model->getAABBox();
	Vector3 Center((Box.m_corners[0]+Box.m_corners[1])*0.5f,(Box.m_corners[1]+Box.m_corners[2])*0.5f,(Box.m_corners[3]+Box.m_corners[4])*0.5f);

	float zMin = min(Box.m_corners[4],Box.m_corners[5]);
	float h = (Box.m_corners[3]-Box.m_corners[2])/2.0f;
	float d = h/tanf(RADIAN(10));

	g_Fw.Set(0,0,1);
	g_Pos.Set(Center.x,Center.y,-d);	
	
}