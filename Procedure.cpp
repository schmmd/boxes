#include "procedure.h"
#include "lib\Canvas.h"
#include "constant.h"
#include "dialogsettings.h"
#include "resource.h"

#define WINK_INTERVAL 35000
#define WINK_DURATION 350

BOOL CALLBACK SimpleDlgProc
   (HWND hwnd, UINT message, UINT wParam, LPARAM lParam);

void CProcedure::Create (CREATESTRUCT * create)
{
	_world = new CWorld ();
	_view.Create (create->hInstance, _world);

	_timer.Create       (_hWnd, 1);
	_displaytimer.Create(_hWnd, 2);
	_winktimer.Create   (_hWnd, 3);
	_unwinktimer.Create (_hWnd, 4);

	_world->Size   (create->cx / TILEW, (create->cy - 40) / TILEH); // -20: account for display area
}

CProcedure::~CProcedure ()
{
	delete _world;
}

bool CProcedure::MenuCommand (int cmdID, bool isAccel)
{
	if (cmdID == ID_FILE_NEWGAME)
	{
		_displaytimer.Set(1000);
		_world->NewGame();
		_timer.Set((int)(_world->_iBeastDelay/60.0 * 1000.0));
		_world->Pause();

		UpdateScreen();

		return true;
	}
	else if (cmdID == ID_FILE_NEWARCADE)
	{
		if (RunSettingsDialog())
		{
			_displaytimer.Set(1000);
			_world->NewArcade();
			_timer.Set((int)(_world->_iBeastDelay/60.0 * 1000.0));
			_world->Pause();

			UpdateScreen();
		}

		return true;
	}
	else if (cmdID == ID_FILE_PAUSE)
		_world->Pause();
	else if (cmdID == ID_FILE_EXIT)
		Exit();
	else if (cmdID == ID_HELP_ABOUT)
		DialogBox(GetModuleHandle(NULL), (char*)IDD_ABOUT, _hWnd, DLGPROC(SimpleDlgProc));
	else if (cmdID == ID_HELP_INSTRUCTIONS)
		DialogBox(GetModuleHandle(NULL), (char*)IDD_INSTRUCTIONS, _hWnd, DLGPROC(SimpleDlgProc));
	else if (cmdID == ID_HELP_SCHMITZTECH)
		ShellExecute(_hWnd, "Open", "http://www.schmitztech.com/", "", "", SW_SHOWNORMAL);

	return false;
}

bool CProcedure::SysCommand (int cmd)
{
	// disable screen savers
	if (cmd == SC_SCREENSAVE || cmd == SC_MONITORPOWER)
		return true;
	return false;
}

bool CProcedure::Size (int width, int height)
{
	_view  .Size   (_hWnd, width, height);

	UpdateScreen();

	// force repaint
	InvalidateRect(_hWnd, NULL, TRUE);
	return true;
}

bool CProcedure::Paint (HDC hdc)
{
	// validate painted area
	PaintCanvas canvas (_hWnd);

	_view.Paint (canvas);
	return true;
}

bool CProcedure::KeyDown(int iVirtKey)
{
	if (_world->KeyDown(iVirtKey))
	{
		if (_world->IsPaused())
		{
			// Reset timer
			_timer.Set((int)(_world->_iBeastDelay/60.0 * 1000.0));
			_displaytimer.Set(1000);
			_world->AddSecond();
			// Move the beasts
			_world->RunBeastAI();
			// Unpause
			_world->Unpause();
		}

		// Reset winktimer
		_winktimer.Set(WINK_INTERVAL);

		UpdateScreen();
		return true;
	}
	else if ((_world->Won() || _world->Lost()) && _world->GetState() == GAME)
	{
		if (iVirtKey == VK_RETURN)
		{
			if (_world->Won())
				_world->NewLevel();

			if (_world->Lost())
				_world->NewLife();
			else
				_world->New();

			_world->Pause();

			_displaytimer.Set(1000);
			_timer.Set((int)(_world->_iBeastDelay/60.0 * 1000.0));

			UpdateScreen();
		}
	}
	
	return false;
}

bool CProcedure::Timer(int id, TIMERPROC * proc)
{
	static bool bWinking = false;

	if (id == 1)
	{
		if (!_world->IsPaused() && !_world->Lost() && !_world->Won())
			_world->RunBeastAI();
		else
			_timer.Kill();

		UpdateScreen();
	}
	else if (id == 2)
	{
		if (!_world->IsPaused() && !_world->Lost() && !_world->Won())
		{
			_world->AddSecond();
			UpdateDisplays();
		}
		else
			_displaytimer.Kill();
	}
	else if (id == 3)
	{
		if (!_world->IsPaused() && !_world->Lost() && !_world->Won() && _world->GetState() != INTRO && _world->_iFaceIndex == FACE_NORMAL)
		{
			_world->_iFaceIndex = FACE_WINK;
			bWinking = true;

			UpdateScreen();

			_unwinktimer.Set(WINK_DURATION);
			_winktimer.Kill();
		}
	}
	else if (id == 4)
	{
		_world->_iFaceIndex = FACE_NORMAL;
		bWinking = false;

		UpdateScreen();

		_unwinktimer.Kill();
	}

	return true;
}

void CProcedure::Exit ()
{
    ::DestroyWindow(_hWnd);
}

bool CProcedure::RunSettingsDialog()
{
	return SettingsDialog(_hWnd, _world->_iBeastDelay, _world->_iBeasts, _world->_iBoxDensity);
}

void CProcedure::UpdateScreen(BOOL bErase)
{
	UpdateCanvas canvas(_hWnd);
	_view.Update (canvas);
	_view.UpdateInfo(canvas);

	InvalidateRect(_hWnd, NULL, FALSE);
}

void CProcedure::UpdateDisplays(BOOL bErase)
{
	UpdateCanvas canvas(_hWnd);

	_view.UpdateInfo(canvas);
	InvalidateRect(_hWnd, &_view.display[0], bErase);
	InvalidateRect(_hWnd, &_view.display[1], bErase);
}

BOOL CALLBACK SimpleDlgProc
   (HWND hwnd, UINT message, UINT wParam, LPARAM lParam)
{
	RECT rect, rect2;

    switch (message)
    {
    case WM_INITDIALOG:
		// Center dialog
		GetWindowRect(GetParent(hwnd), &rect);
		GetWindowRect(hwnd , &rect2);
		SetWindowPos (hwnd, HWND_TOP, ((rect.left + rect.right) / 2) - ((rect2.right - rect2.left) / 2), ((rect.top + rect.bottom) / 2) - ((rect2.bottom - rect2.top) / 2), 0, 0, SWP_NOSIZE);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}
