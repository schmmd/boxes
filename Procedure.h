#ifndef _H_PROC
#define _H_PROC

#include "app.h"
#include "view.h"
#include "world.h"
#include "lib\timer.h"

#include <windows.h>

//
// Override some message handlers
//

class CGenericProc
{
	friend LRESULT CALLBACK CClass::Procedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	void SetWindowHandle (HWND hWnd) { _hWnd = hWnd; }
public:
	virtual void Create (CREATESTRUCT * create) {}
	virtual bool Destroy () { return false; }
	virtual bool SysCommand (int cmd) { return false; }
	virtual bool Command (WORD wNotifyCode, WORD wID, HWND hwndCtrl) { return false; }
	virtual bool Size (int width, int height) { return false; }
	virtual bool Timer (int id, TIMERPROC * proc) { return false; }
	virtual bool MenuCommand (int cmdId, bool isAccel) { return false; }
	virtual bool CtrlCommand (int notifCode, int ctrlId, HWND hwnd) { return false; }
	virtual bool Paint (HDC hdc) { return false; }
	virtual bool KeyDown (int iVirtKey) { return false; }
protected:
	HWND _hWnd;
};

class CProcedure: public CGenericProc
{
public:
	CProcedure () 
		: _world (0)
	{}
	~CProcedure ();

	void Create (CREATESTRUCT * create);
	bool SysCommand (int cmd);
	bool MenuCommand (int cmdId, bool isAccel);
	bool Destroy () { ::PostQuitMessage (0); return true; }
	bool Size (int width, int height);
	bool KeyDown (int iVirtKey);
	bool Paint (HDC hdc);
	bool Timer (int id, TIMERPROC * proc);

	void UpdateScreen(BOOL bErase = 0);
	void UpdateDisplays(BOOL bErase = 0);

	bool RunSettingsDialog();

	void Exit ();
private:
	CWorld*		_world;
	CView		_view;

	CTimer _timer;
	CTimer _displaytimer;
	CTimer _winktimer;
	CTimer _unwinktimer;
};

#endif