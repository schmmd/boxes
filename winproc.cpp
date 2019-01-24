#include <windows.h>
#include "app.h"
#include "procedure.h"

template <class T>
inline T GetLong (HWND hwnd, int which = GWL_USERDATA)
{
	return reinterpret_cast<T> (::GetWindowLong (hwnd, which));
}

template <class T>
inline void SetLong (HWND hwnd, T value, int which = GWL_USERDATA)
{
	::SetWindowLong (hwnd, which, reinterpret_cast<long> (value));
}


//-------------------------------------
// Copyright (c) Reliable Software 1998
//-------------------------------------

//
// Window Procedure common to all windows
//

LRESULT CALLBACK CClass::Procedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CProcedure * pCtrl = GetLong<CProcedure *> (hwnd);
	switch (message)
	{
	case WM_GETMINMAXINFO:
		// This is the first message sent to a window
		// We can't do anything about it.
		break;
	case WM_NCCREATE:
		{
			// This is the second message sent to a window
			CREATESTRUCT * create = reinterpret_cast<CREATESTRUCT *> (lParam);
			pCtrl = static_cast<CProcedure *> (create->lpCreateParams);
			pCtrl->SetWindowHandle (hwnd);
			SetLong<CProcedure *> (hwnd, pCtrl);
		}
		break;
	case WM_CREATE:
		try
		{
			CREATESTRUCT * create = reinterpret_cast<CREATESTRUCT *> (lParam);
			pCtrl->Create (create);
		}
		catch (char const * msg)
		{
			::MessageBox (0, msg, "Error", MB_OK | MB_ICONERROR);
			return -1;
		}
		catch (...)
		{
			return -1;
		}
		return 0;
	case WM_SYSCOMMAND:
		if (pCtrl->SysCommand (wParam & 0xfff0))
			return 0;
		break;
	case WM_SIZE:
		if (pCtrl->Size (LOWORD(lParam), HIWORD(lParam)))
			return 0;
		break;
	case WM_TIMER:
		if (pCtrl->Timer (wParam, reinterpret_cast<TIMERPROC *> (lParam)))
			return 0;
		break;
	case WM_COMMAND:
		{
			int notifyCode = HIWORD (wParam);
			if (notifyCode == 0 || notifyCode == 1)
			{
				if (pCtrl->MenuCommand (LOWORD (wParam), notifyCode == 1))
					return 0;
			}
			else
			{
				HWND hwndChild = reinterpret_cast<HWND> (lParam);
				if (pCtrl->CtrlCommand (LOWORD (wParam), notifyCode, hwndChild))
					return 0;
			}
		}
		break;
	case WM_PAINT:
		if (pCtrl->Paint (reinterpret_cast<HDC> (wParam)))
			return 0;
		break;
	case WM_KEYDOWN:
		if (pCtrl->KeyDown((int)wParam))
			return 0;
		break;
	case WM_DESTROY:
		// We're no longer on screen
		pCtrl->Destroy ();
		SetLong<CProcedure *> (hwnd, 0);
		return 0;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}