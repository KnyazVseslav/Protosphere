#include "D3DApplication.h"

D3DApplication::D3DApplication(): d3d(new D3D)
{


	ClientWidth  = 1024;
	ClientHeight = 768;

	mAppPaused  = false;
	mMinimized  = false;
	mMaximized  = false;
	mResizing   = false;


	isAppPaused= false;

}

D3DApplication::~D3DApplication()
{

	delete d3d;

}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	static D3DApplication *d3dApp= 0;
	

	switch(msg)
    {
   
		case WM_CREATE:
			{
						
				CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
				d3dApp= (D3DApplication*)cs->lpCreateParams;

				d3dApp->hWnd= hwnd;

				return 0;
			
			}
   
    }

	if (d3dApp) d3dApp->WndProc(msg, wParam, lParam);
	
		else

  return DefWindowProc(hwnd, msg, wParam, lParam);


}

ATOM D3DApplication::RegisterWindowClass()
{
	WNDCLASS wcls;

	wcls.style			= CS_HREDRAW | CS_VREDRAW;
	wcls.lpfnWndProc	= MainWndProc;
	wcls.cbClsExtra		= 0;
	wcls.cbWndExtra		= 0;
	wcls.hInstance		= hInst;
	wcls.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_MY));
	wcls.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcls.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcls.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcls.lpszMenuName	= MAKEINTRESOURCE(IDC_MY);
	wcls.lpszClassName	= szWindowClass;

	return RegisterClass(&wcls);
}

ATOM D3DApplication::RegisterWindowClassEx()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_MY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MY);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


BOOL D3DApplication::Init(HINSTANCE hInstance, int nCmdShow)
{

	hInst = hInstance; // Store instance handle in our global variable


	 // Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);

	if (!RegisterWindowClassEx())
	{
	
		MessageBox(0, L"Failed registering window class", 0, 0);

		return FALSE;
	
	}

	if(!InitInstance(nCmdShow))
	{
	
		return FALSE;
	
	}

	return TRUE;


}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL D3DApplication::InitInstance(int nCmdShow)
{
   HWND hWnd;
   

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInst, this);

   if (!hWnd)
   {

	   MessageBox(0, L"Failed creating window", 0, 0);

      return FALSE;
   }


	//Инициализация Direct3D
   d3d->Initialize(hWnd, d3d->feature_levels_array, TRUE, ClientWidth, ClientHeight);


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the D3DApplicationlication menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT D3DApplication::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;


	switch (message)
	{
	
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
				switch (wmId)
				{
				case IDM_ABOUT:
					DialogBox(getInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;

				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
				}

		  break;

/*
	case WM_KEYDOWN:
		if(wParam == VK_ESCAPE) DestroyWindow(hWnd);
		*/
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any Rendering code here...
		EndPaint(hWnd, &ps);

		break;


	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			mAppPaused = true;
			mTimer.stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.start();
		}
		break;

	case WM_SIZE:

			ClientWidth  = LOWORD(lParam);
			ClientHeight = HIWORD(lParam);


			if( d3d->Device)
			{
				if( wParam == SIZE_MINIMIZED )
				{
					mAppPaused = true;
					mMinimized = true;
					mMaximized = false;
				}
				else if( wParam == SIZE_MAXIMIZED )
				{
					mAppPaused = false;
					mMinimized = false;
					mMaximized = true;
					OnResize();
				}
				else if( wParam == SIZE_RESTORED )
				{
				
					// Restoring from minimized state?
					if( mMinimized )
					{
						mAppPaused = false;
						mMinimized = false;
						OnResize();
					}

					// Restoring from maximized state?
					else if( mMaximized )
					{
						mAppPaused = false;
						mMaximized = false;
						OnResize();
					}
					else if( mResizing )
					{
						// If user is dragging the resize bars, we do not resize 
						// the buffers here because as the user continuously 
						// drags the resize bars, a stream of WM_SIZE messages are
						// sent to the window, and it would be pointless (and slow)
						// to resize for each WM_SIZE message received from dragging
						// the resize bars.  So instead, we reset after the user is 
						// done resizing the window and releases the resize bars, which 
						// sends a WM_EXITSIZEMOVE message.
					}
					else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
					{
						OnResize();
					}
				}
			}
			break;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		case WM_ENTERSIZEMOVE:
			mAppPaused = true;
			mResizing  = true;
			mTimer.stop();
			break;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		case WM_EXITSIZEMOVE:
			mAppPaused = false;
			mResizing  = false;
			mTimer.start();
			OnResize();
		    break;

	case WM_DESTROY:

		PostQuitMessage(0);

		break;

	default:
	// Forward any other messages we did not handle above to the
    // default window procedure. Note that our window procedure
    // must return the return value of DefWindowProc.
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK D3DApplication::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

int D3DApplication::MessageLoop()
{

	MSG msg = {0};
	HACCEL hAccelTable;

	hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_MY));

	
	// Main message loop:

	// Loop until we get a WM_QUIT message. The function
    // GetMessage will only return 0 (false) when a WM_QUIT message
    // is received, which effectively exits the loop. The function
    // returns -1 if there is an error. Also, note that GetMessage
    // puts the D3DApplicationlication thread to sleep until there is a
    // message.

	mTimer.reset();
	
	BOOL r = 1;

	while( msg.message != WM_QUIT )
    {
        
		r = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

		if(r == -1)
        {
            MessageBox(0, L"GetMessage FAILED - Error occured while getting message from the queue", L"Error", MB_OK);
            break;
        }
        else if(r)
        {

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
		else
		{

			mTimer.tick();

			if( !mAppPaused )
				UpdateScene(mTimer.getDeltaTime());	
			else
				Sleep(50);
		
			RenderScene();	



			/*
			//mTimer.tick();

			if( !isAppPaused )
			{	
				mTimer.start();
				mTimer.tick();
			
				 UpdateScene(mTimer.getDeltaTime());	
			    
				   RenderScene();	

			}

			else
			{
			
				if (isAppPaused) mTimer.stop();

			
			//MessageBox(0, L"PAUSED", L"!", MB_OK);
				//Sleep(50);
			}
			//RenderScene();
			*/
		
		}
	}

	return (int) msg.wParam;

}

void D3DApplication::UpdateScene(float dt)
{



}

void D3DApplication::OnResize()
{

	d3d->Resize(ClientWidth, ClientHeight);

}


void D3DApplication::Reset_RTV_DSV_VP(UINT ClientWidth, UINT ClientHeight)
{

	d3d->Reset_RTV_DSV_VP(ClientWidth, ClientHeight);

}


void D3DApplication::RenderScene()
{

	d3d->Render();

}