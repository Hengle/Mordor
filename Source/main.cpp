/*
	Created 	Elinor Townsend 2012
				WndProc function based on code from d3dApp.cpp by Frank Luna 
				(C) 2008 All Rights Reserved.
*/

/*	
	Name		Main
	Brief		Sets up the Windows API window and handles messages sent 
				to the window
*/
//..............................................................................
#include "Scene/Scene.hpp"
#include "Global/Global.hpp"

// Declarations of Windows API functions
void registerWindow(HINSTANCE hInstance);
bool initialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

Scene* App = Scene::instance();
bool running = true;

/*
	Name		WINAPI WinMain
	Syntax		WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, 
							   PSTR cmdLine, int showCmd)
	Param		HINSTANCE hInstance - Handle to an intance
	Param		HINSTANCE prevInstance - No longer used (always 0)
	Param		PSTR cmdLine - command-line arguments as a Unicode string
	Param		int showCmd -  Flag to indicate if the window will be minimized, 
							   maximized, or shown normally
	Return		int - Not used
	Brief		Entry point of programme
*/
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE prevInstance, 
					PSTR cmdLine,
					int showCmd)
{
	MSG msg;
	
	registerWindow(hInstance);

   	if (!initialiseWindow(hInstance, showCmd))
		return false;

	
	
	App->initialise();

	while(running) 
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		    if (msg.message == WM_QUIT)
			{
				running = false;
			}
			if (msg.message == WM_CLOSE)
			{
				running = false;
			}
			TranslateMessage (&msg);							
			DispatchMessage (&msg);
		}
		else
		{
			running = App->runFrame();
		}		
    }

	App->deinitialise();

	return msg.wParam;	
}

/*
	Name		registerWindow
	Syntax		registerWindow(HINSTANCE hInstance)
	Param		HINSTANCE hInstance - handle to an instance
	Return		void
	Brief		Creates and registers an instance of the Windows class
*/
void registerWindow(HINSTANCE hInstance)
{
    WNDCLASSEX  wcex;									

    wcex.cbSize        = sizeof (wcex);				
    wcex.style         = CS_HREDRAW | CS_VREDRAW;		
    wcex.lpfnWndProc   = WndProc;						
    wcex.cbClsExtra    = 0;								
    wcex.cbWndExtra    = 0;								
    wcex.hInstance     = hInstance;						
    wcex.hIcon         = 0; 
    wcex.hCursor       = LoadCursor (NULL, IDC_ARROW);	
															
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
    wcex.lpszMenuName  = NULL;							
    wcex.lpszClassName = "Mordor";				
    wcex.hIconSm       = 0; 

	RegisterClassEx (&wcex);							
}

/*
	Name		initialiseWindow
	Syntax		initialiseWindow(HINSTANCE hInstance, int showCmd)
	Param		HINSTANCE hInstance - handle to an instance
	Param		int showCmd - Flag to indicate if the window will be minimized, 
							  maximized, or shown normally
	Return		bool - Returns false if window is not created
	Brief		Initialises the current instance of the windows class and opens 
				a window
*/
bool initialiseWindow(HINSTANCE hInstance, int showCmd)
{
	HWND hWnd;
	hWnd = CreateWindow ("Mordor",					
						 "Mordor",		  	
						 WS_OVERLAPPEDWINDOW,	
						 CW_USEDEFAULT,			
						 CW_USEDEFAULT,			
						 SCREENWIDTH,			
						 SCREENHEIGHT,			
						 NULL,					
						 NULL,					
						 hInstance,				
						 NULL);								
	if (!hWnd)
	{
		return false;
	}

    ShowWindow(hWnd, showCmd);  //opens the window						
    UpdateWindow(hWnd);
	ghWnd = hWnd;
	return true;
}

/*
	Name		WndProc
	Syntax		WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	Param		HWND hWnd - A handle to the window
	Param		UINT uMsg - The message
	Param		WPARAM wParam - Additional message information
	Param		LPARAM lParam - Additional message information
	Return		LRESULT - The result of the message processing
	Brief		Processes messages sent to the window
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)											
    {														
		case WM_CREATE:	
			break;

		// WM_SIZE is sent when the user resizes the window
		case WM_SIZE:
			// Save the new client area dimensions
			App->setWidth(LOWORD(lParam));
			App->setHeight(HIWORD(lParam));

			if ( wParam == SIZE_MINIMIZED )
			{
				App->setPaused(true);
				App->setMinimised(true);
				App->setMaximised(false);
			}
			else if ( wParam == SIZE_MAXIMIZED )
			{
				App->setPaused(false);
				App->setMinimised(false);
				App->setMaximised(true);
				App->onResize();
			}
			else if ( wParam == SIZE_RESTORED )
			{
				
				// Restoring from minimised state?
				if (App->isMinimised())
				{
					App->setPaused(false);
					App->setMinimised(false);
					App->onResize();
				}

				// Restoring from maximised state?
				else if (App->isMaximised())
				{
					App->setPaused(false);
					App->setMinimised(false);
					App->onResize();
				}
				else if (App->isResizing())
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from 
					// dragging the resize bars.  So instead, we reset after the 
					// user is done resizing the window and releases the resize 
					// bars, which sends a WM_EXITSIZEMOVE message.
				}
				else
				{
					App->onResize();
				}
			}
			return 0;
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars
		case WM_ENTERSIZEMOVE:
			App->setPaused(true);
			App->setResizing(true);
			return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars
		// Here we reset everything based on the new window dimensions
		case WM_EXITSIZEMOVE:
			App->setPaused(false);
			App->setResizing(false);
			App->onResize();
			return 0;

		case WM_KEYDOWN:
			break;

		case WM_KEYUP:
			break;

		case WM_MOUSEMOVE:
			break;

		case WM_LBUTTONDOWN:
			break;
			
		case WM_RBUTTONDOWN:
			break;

		case WM_MBUTTONDOWN:
			break;

		case WM_PAINT:
		    break;		

		case WM_DESTROY:	
			PostQuitMessage(0);						
			break;				
	}													
	return DefWindowProc (hWnd, uMsg, wParam, lParam);																	
}