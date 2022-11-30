#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
#include <process.h>
#include <Tlhelp32.h>

#include <tchar.h>
#include <strsafe.h>
#include <Shlobj.h>

#include <shlwapi.h>
#include <SensorsApi.h>
#include <sensors.h>
#include <SetupAPI.h>
#include <devguid.h>
#include "AmbientLightAwareSensorEvents.h"
#include "AmbientLightAwareSensorManagerEvents.h"

#include <Mmdeviceapi.h>
#include <Endpointvolume.h>

#define CLAMSHELL       0
#define LANDSCAPE_0     1
#define PORTRAIT_90     2
#define LANDSCAPE_180   3
#define PORTRAIT_270    4
#define FLAT            5
#define SIMPLE_ORIENTATION 1
#define CHECK_TABLET

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
#define EXIT_ON_ERROR(hr) if (FAILED(hr)) { goto Exit; }

TCHAR szWindowClass[] = TEXT("FMHookBackgroundClass");

#ifndef TEST_PROGRAM
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

//#define RPC_MONITOR_EVENT
#define DBG_BUF_SIZE   (512)
#define MAX_KEY_LENGTH (512)

HINSTANCE g_hInst;
ISensor * g_pOrientationSensor = NULL;
CAmbientLightAwareSensorManagerEvents* g_pSensorManagerEvents;

int gicount = 0;

int i_Orientation = 0;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TCHAR szOut[256] = {};
    CoInitialize(NULL);

	HRESULT hr;
#ifndef TEST_PROGRAM
	WNDCLASSEX wcex;

	//register window class
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 4;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	ATOM wc = RegisterClassEx(&wcex);

	if (!wc)
	{
		printf("Register error!\n");
		return -1;
	}

	HWND hWnd = CreateWindow(szWindowClass, L"read_g_sensor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, g_hInst, NULL);
	if (!hWnd)
	{
		printf("Create error\n");
		return -1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
#endif

	Sleep(10000);
	g_pSensorManagerEvents = new CAmbientLightAwareSensorManagerEvents;//(this);
	hr = g_pSensorManagerEvents->Initialize();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    if (NULL != g_pSensorManagerEvents)
    {
		g_pSensorManagerEvents->Uninitialize();
        delete g_pSensorManagerEvents;
        g_pSensorManagerEvents = NULL;
    }
	
	Sleep(2000);

#ifndef TEST_PROGRAM
	UnregisterClass(szWindowClass, g_hInst);
#endif

    CoUninitialize();
	return 0;
}

#ifndef TEST_PROGRAM
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR szOut[256];
	switch(uMsg)
	{
        case WM_CREATE:
            SetTimer(hWnd, 100, 1000, NULL);
            break;

		case WM_TIMER:

			if(g_pSensorManagerEvents != NULL)
			{
                if (g_pSensorManagerEvents->m_pSimpleOrientationSensor != NULL)
				{
					CComPtr<ISensorDataReport> spDataReport;
                    HRESULT hr;
                    PROPVARIANT pv;
                    hr = g_pSensorManagerEvents->m_pSimpleOrientationSensor->GetData(&spDataReport);
                    if (SUCCEEDED(hr))
                    {
                        PropVariantInit(&pv);
                        hr = spDataReport->GetSensorValue(SENSOR_DATA_TYPE_SIMPLE_DEVICE_ORIENTATION, &pv);
                        if (SUCCEEDED(hr))
                        {
							i_Orientation = V_UI4(&pv);
							StringCbPrintf(szOut, 256, L"SimpleOrientationSensor GetSensorValue i_Orientation = %d\n", i_Orientation);
							OutputDebugString(szOut);
						}
						else
						{
							i_Orientation = 0;
						}
                    }
					PropVariantClear(&pv);
				}
				else {
					StringCbPrintf(szOut, 256, L"SimpleOrientationSensor == NULL\n");
					OutputDebugString(szOut);
				}
			}
			else {
				StringCbPrintf(szOut, 256, L"g_pSensorManagerEvents == NULL\n");
				OutputDebugString(szOut);
			}
            break;

		case WM_CLOSE:
            KillTimer(hWnd, 100);
            SAFE_RELEASE(g_pOrientationSensor);
            DestroyWindow(hWnd); 
			break;

        case WM_DESTROY: 
            // Post the WM_QUIT message to 
            // quit the application terminate. 
            PostQuitMessage(0); 
            break;

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
#endif