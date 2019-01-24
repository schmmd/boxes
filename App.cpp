#include "App.h"

CClass::CClass(char const * szClassName, HINSTANCE hInst)
{
    _class.style = 0;
    _class.lpfnWndProc = Procedure;		// window procedure: mandatory
    _class.cbClsExtra = 0;
    _class.cbWndExtra = 0;
    _class.hInstance = hInst;			// owner of the class: mandatory
    _class.hIcon = 0;
    _class.hCursor = ::LoadCursor (0, IDC_ARROW);			// optional
    _class.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);		// optional
    _class.lpszMenuName = NULL;
    _class.lpszClassName = szClassName; // mandatory
}

CWindow::CWindow(char const * szCaption, char const * szClass, HINSTANCE hInstance)
: _szCaption(szCaption), _szClass(szClass), _hInstance(hInstance), _hAccel(0)
{
	_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	_x = _y = CW_USEDEFAULT;
	_width = _height = CW_USEDEFAULT;
	_hWndParent = NULL;
	_hMenu		= NULL;
	_data		= NULL;
}

bool CWindow::Create(CGenericProc* proc)
{
	AddCreationData(proc);
    _hWnd = ::CreateWindow (_szClass,		// name of a registered window class
							_szCaption,     // window caption
							_style,			// window style
							_x,				// x position
							_y,				// y position
							_width,			// witdh
							_height,		// height
							_hWndParent,	// handle to parent window
							_hMenu,			// handle to menu
							_hInstance,		// application instance
							_data);			// window creation data

	return _hWnd != NULL;
}

void CWindow::AttachAccelerator(int idAccel)
{
	_hAccel = LoadAccelerators(_hInstance, (char*)idAccel);
}

bool CWindow::ProcessMessage()
{
	MSG msg;

	// If there is a message, retrieve and remove
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
	{
		if (msg.message == WM_QUIT)
			return false;

		if (!TranslateAccelerator(_hWnd, _hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}