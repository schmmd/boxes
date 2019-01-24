#ifndef _H_APP
#define _H_APP

#include <windows.h>

class CGenericProc;

class CClass
{
public:
    CClass (char const * szClassName, HINSTANCE hInst);

	void SetCursor(int idCursor)
	{
		_class.hCursor = LoadCursor(_class.hInstance, (char*)idCursor);
	}
	void SetIcon(int idIcon)
	{
		_class.hIcon = LoadIcon(_class.hInstance, (char*)idIcon);
	}
	void SetMenu(int idMenu)
	{
		_class.lpszMenuName = (char*)idMenu;
	}
    void Register ()
    {
        ::RegisterClass (&_class);
    }
private:
	WNDCLASS _class;

	static LRESULT CALLBACK Procedure
						(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

class CWindow
{
public:
	CWindow () : _hWnd(0)
	{	}
	CWindow (char const * szCaption, char const * szClass, HINSTANCE hInstance);

	bool Create(CGenericProc* proc);
	bool ProcessMessage();

	void AttachAccelerator(int idAccel);

	void Show (int iShowCmd = SW_SHOW)
	{
		::ShowWindow(_hWnd, iShowCmd);
		::UpdateWindow(_hWnd);
	}
	void SetDimentions (int width, int height, bool bIsMenu = false)
	{
		RECT client = {0, 0, width, height};
		AdjustWindowRect(&client, _style, bIsMenu);
		_width  = client.right  - client.left;
		_height = client.bottom - client.top;
	}

	HWND GetWnd()
	{
		return _hWnd;
	}
protected:
	void AddCreationData (void * data) { _data = data; }

    HWND         _hWnd;

	HACCEL		 _hAccel;

	HINSTANCE    _hInstance;
	char const * _szCaption;
	char const * _szClass;
    DWORD        _exStyle;      // extended window style
    DWORD        _style;        // window style
    int          _x;            // horizontal position of window
    int          _y;            // vertical position of window
    int          _width;        // window width  
    int          _height;       // window height
    HWND         _hWndParent;   // handle to parent or owner window
    HMENU        _hMenu;        // handle to menu, or child-window ID
    void       * _data;         // pointer to window-creation data
};

#endif