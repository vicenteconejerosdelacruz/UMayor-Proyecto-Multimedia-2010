#include "stdafx.h"
#include <stdio.h>
#include <wiiuse.h>
#include <assimp.h>

#include <windows.h>
#include <windowsx.h>

#include "Renderer/Renderer.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"

Vector3 Vector3::Fw(0,0,1);
Vector3 Vector3::Up(0,1,0);
Vector3 Vector3::xAxis(1,0,0);
Vector3 Vector3::yAxis(0,1,0);
Vector3 Vector3::zAxis(0,0,1);

Vector3 g_Fw(0,0,1);
Vector3 g_Pos(0,0,-10);

char FilePath[MAX_PATH];
char FileDirectory[MAX_PATH];

wiimote** g_wiimotes;

static bool				nunchuk_is_moving = false;
static bool				wiimote_ir_tracking = false;
static float			wiimote_ir_screenCoordX = 0.0f;
static float			wiimote_ir_screenCoordY = 0.0f;
static bool				wiimote_button_one_pressed = false;
static float			yaw = 0.0f;
static float			pitch = 0.0f;
static float			nunchuk_yaw = 0.0f;

HANDLE wiiMoteThread;
void InitWiiMoteThread();
DWORD WINAPI WiiMoteThread(LPVOID iValue);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{

	GetModuleFileName(NULL,FilePath,sizeof(FilePath));
	memcpy(FileDirectory,FilePath,sizeof(FileDirectory));
	char *ptr = FileDirectory + strlen(FileDirectory)-1;
	while(*ptr!='\\')
	{
		ptr--;
	}

	ptr[1]='\0';

	SetCurrentDirectory(FileDirectory);

	g_renderer = new RENDERER();
	g_renderer->InitWindow(hInstance,nCmdShow);
	g_renderer->CreateDevice();

	InitWiiMoteThread();

    MSG msg;
    while(TRUE)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT)
            break;

		g_renderer->SetCamera(g_Pos,g_Fw);
        g_renderer->Render();
    }

	g_renderer->ShutDown();
	delete g_renderer;

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU menu;
	static bool mouseCapture=false;
	static POINT mouseCaptureCoord;

    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } 
		break;
		case WM_CREATE:
			{
				menu = CreateMenu();
				AppendMenu(menu,MF_STRING,MC_OPEN_FILE,"&Open File");
				//AppendMenu(menu,MF_STRING,MC_RECOMPILE_SHADER,"&Recompile Shaders");
				SetMenu(hWnd,menu);
			}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case MC_OPEN_FILE:
				{
					OPENFILENAME ofn;
					char szFileName[MAX_PATH] = "";
					char filterString[1024] = "";

					ZeroMemory(&ofn, sizeof(ofn));

					ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
					ofn.hwndOwner = g_renderer->data.hWnd;
					aiString str;
					aiGetExtensionList(&str);
					strcat(filterString,str.data);
					strcat(filterString+strlen(filterString)+1,str.data);
					ofn.lpstrFilter = filterString;
					ofn.lpstrFile = szFileName;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
					ofn.lpstrDefExt = NULL;
					ofn.lpstrInitialDir = FileDirectory;

					if(GetOpenFileName(&ofn))
					{
						g_renderer->LoadModel(szFileName);
					}
				}
			break;
			case MC_RECOMPILE_SHADER:
				{
					g_renderer->m_doRecompile=true;
				}
				break;
			}
		break;
		case WM_KEYDOWN:
			switch(LOWORD(wParam))
			{
			case VK_UP:
				{
					g_Pos+=g_Fw;
				}
			break;
			case VK_DOWN:
				{
					g_Pos-=g_Fw;
				}
			break;
			case VK_RIGHT:
				{

				}
			break;
			case VK_LEFT:
				{

				}
			break;
			}
			break;
		case WM_LBUTTONDOWN:
			{
				mouseCapture = true;
				GetCursorPos(&mouseCaptureCoord);
				ScreenToClient(hWnd,&mouseCaptureCoord);
			}
			break;
		case WM_LBUTTONUP:
			mouseCapture = false;
			break;
		case WM_MOUSEMOVE:
			if(mouseCapture)
			{
				POINT mousePoint; 
				GetCursorPos(&mousePoint);
				ScreenToClient(hWnd,&mousePoint);
				POINT diff;
				diff.x = mousePoint.x - mouseCaptureCoord.x;
				diff.y = mousePoint.y - mouseCaptureCoord.y;
				mouseCaptureCoord.x = mousePoint.x;
				mouseCaptureCoord.y = mousePoint.y;

				yaw+=diff.x/10.0f;
				pitch-=diff.y/10.0f;

				LIMITTO(pitch,-70.0f,70.0f);

				g_Fw = Vector3::Fw;
				g_Fw.RotateAroundAxis(Vector3::Up,RADIAN(yaw));
				Vector3 FwCUp = g_Fw^(Vector3::Up);
				g_Fw.RotateAroundAxis(FwCUp,RADIAN(pitch));
			}
			break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}

void InitWiiMoteThread()
{
	DWORD threadId;
	wiiMoteThread = CreateThread(NULL,0,WiiMoteThread,NULL,0,&threadId);
}

void handle_event(wiimote_t* wm)
{
	if(IS_PRESSED(wm,WIIMOTE_BUTTON_ONE))
	{
		if(!wiimote_button_one_pressed)
		{
			nunchuk_is_moving = false;
			wiimote_ir_tracking = false;
			wiimote_button_one_pressed = true;
			if(g_renderer->m_modelList.size())
			{
				g_renderer->LocateCameraToWatchModel(g_renderer->m_modelList[0]);
				yaw = 0.0f;
				pitch = 0.0f;
			}
		}
	}
	else
	{
		wiimote_button_one_pressed = false;
	}

	if(wiimote_button_one_pressed)
		return;

	if(WIIUSE_USING_IR(wm))
	{
		if(wm->ir.num_dots>=2 && (wm->ir.x!=0 || wm->ir.y!=0))
		{
			wiimote_ir_tracking = true;
			wiimote_ir_screenCoordX = float(wm->ir.x);
			wiimote_ir_screenCoordY = float(wm->ir.y);
		}
		else
		{
			wiimote_ir_tracking = false;
		}
	}

	if(wm->exp.type == EXP_NUNCHUK)
	{
		nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;

		float mag = nc->js.mag;
		if(mag>0.3)
		{
			nunchuk_is_moving = true;
			nunchuk_yaw = nc->js.ang;
		}
		else
		{
			nunchuk_is_moving = false;
			nunchuk_yaw = 0.0f;
		}
	}
}

DWORD WINAPI WiiMoteThread(LPVOID none)
{
	g_wiimotes =  wiiuse_init(1);

	bool wiiMoteFound = false;
	int wiimotesFound = 0;
	int wiimotesConnected = 0;
	while(1)
	{
		if(!wiiMoteFound)
		{
			wiimotesFound = wiiuse_find(g_wiimotes,1,5);
			if(wiimotesFound)
			{
				wiiMoteFound = true;
				wiimotesConnected = wiiuse_connect(g_wiimotes,1);
				if(wiimotesConnected)
				{
					wiiuse_set_ir_vres(g_wiimotes[0],RESOLUTION_X,RESOLUTION_Y);
					wiiuse_set_leds(g_wiimotes[0],WIIMOTE_LED_1);
					wiiuse_set_ir(g_wiimotes[0],1);
					wiiuse_rumble(g_wiimotes[0],1);
					Sleep(200);
					wiiuse_rumble(g_wiimotes[0],0);
				}
			}
			Sleep(300);
		}
		else
		{
			wiiuse_poll(g_wiimotes,1);
			switch(g_wiimotes[0]->event)
			{
			case WIIUSE_EVENT:
				handle_event(g_wiimotes[0]);
				break;
			}

			if(wiimote_ir_tracking)
			{
				float		left		= WIIMOTE_SCREEN_THRESHOLD_X;
				float		leftLimit	= left - WIIMOTE_SCREEN_THRESHOLD_X*0.5f;
				
				float		right		= RESOLUTION_X - WIIMOTE_SCREEN_THRESHOLD_X;
				float		rightLimit	= right + WIIMOTE_SCREEN_THRESHOLD_X*0.5f;
				
				float		down		= WIIMOTE_SCREEN_THRESHOLD_Y;
				float		downLimit	= down - WIIMOTE_SCREEN_THRESHOLD_Y*0.5f;

				float		up			= RESOLUTION_Y - WIIMOTE_SCREEN_THRESHOLD_Y;
				float		upLimit		= up + WIIMOTE_SCREEN_THRESHOLD_Y*0.5f;

				g_renderer->m_Cursor.SetEnable(true);
				g_renderer->m_Cursor.SetCursorPos((unsigned int)wiimote_ir_screenCoordX,(unsigned int)wiimote_ir_screenCoordY);

				float coordX = wiimote_ir_screenCoordX;
				float coordY = wiimote_ir_screenCoordY;

				LIMITTO(coordX,leftLimit,rightLimit);
				LIMITTO(coordY,downLimit,upLimit);

				if(!ISINSIDE(coordX,left,right) || !ISINSIDE(coordY,down,up))
				{
					float rYaw = 0.0f;
					float rPitch = 0.0f;

					if(coordX<left)
					{	rYaw = -(left-coordX)/(WIIMOTE_SCREEN_THRESHOLD_X*0.5f);}
					else if(coordX>right)
					{	rYaw = (coordX - right)/(WIIMOTE_SCREEN_THRESHOLD_X*0.5f);	}

					if(coordY<down)
					{	rPitch = (down-coordY)/(WIIMOTE_SCREEN_THRESHOLD_Y*0.5f);}
					else if(coordY>up)
					{	rPitch = -(coordY - up)/(WIIMOTE_SCREEN_THRESHOLD_Y*0.5f);	}

					float mulFactor = 1.0f;

					rYaw*=mulFactor;
					rYaw*=mulFactor;
					LIMITTO(rYaw,-mulFactor,mulFactor);
					LIMITTO(rPitch,-mulFactor,mulFactor);

					yaw+=rYaw;
					pitch+=rPitch;
					LIMITTO(pitch,-70.0f,70.0f);

					g_Fw = Vector3::Fw;
					g_Fw.RotateAroundAxis(Vector3::Up,RADIAN(yaw));
					Vector3 FwCUp = g_Fw^(Vector3::Up);
					g_Fw.RotateAroundAxis(FwCUp,RADIAN(pitch));
				}				
			}
			else
			{
				g_renderer->m_Cursor.SetEnable(false);
			}
		}

		g_renderer->SetCamera(g_Pos,g_Fw);

		if(nunchuk_is_moving)
		{
			Vector3 Fw = g_renderer->data.ViewTransformation.GetFw();
			Vector3 Up = g_renderer->data.ViewTransformation.GetUp();
			Vector3 Right = g_renderer->data.ViewTransformation.GetRight();

			Vector3 ToAdd = Fw;
			ToAdd.RotateAroundAxis(Up,RADIAN(nunchuk_yaw));

			g_Pos+=ToAdd;
		}

		g_renderer->SetCamera(g_Pos,g_Fw);
	}

}