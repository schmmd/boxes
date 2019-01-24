#ifndef _H_VIEW
#define _H_VIEW

#include <windows.h>
#include "lib\bitmap.h"

class CWorld;

class CView
{
public:
	CView ();
	~CView ();
	void Create		(HINSTANCE hInstance, CWorld * world);
	void Size		(HWND hWnd, int width, int height);
	void Update		(Canvas & canvas);
	void UpdateInfo (Canvas & canvas);
	void Paint		(Canvas & canvas);

	void BlitBoard    (Canvas & canvas);
	void OutlineScreen(Canvas & canvas);

	RECT client;
	RECT display[2];
private:
	Bitmap _bmpBack;
	Bitmap _bmpInfo[2];

	Bitmap _bmpBox;
	Bitmap _bmpNormal[3];
	// Bitmap _bmpGlasses;
	Bitmap _bmpWin[7];
	// Bitmap _bmpGlassesWin[2];
	Bitmap _bmpFacemask;
	Bitmap _bmpBeast;

	CWorld* _world;

	int _width;
	int _height;
};

#endif