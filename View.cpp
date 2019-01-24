//-------------------------------------
// Copyright Reliable Software (c) 1999
//-------------------------------------
#include "View.h"
#include "World.h"
#include "lib\Canvas.h"
#include "resource.h"
#include "constant.h"

#include <stdio.h>

CView::CView () 
: _world (0)
{ memset(&client, 0, sizeof(RECT)); }

CView::~CView()
{
	int i;

	_bmpBox.Release();
	for (i = 0; i < 3; i++)
		_bmpNormal[i].Release();
	for (i = 0; i < 7; i++)
		_bmpWin[i].Release();
	_bmpBeast.Release();
	_bmpBack.Release();
}

void CView::Create (HINSTANCE hInstance, CWorld* world)
{
	_world = world;

	_bmpBox      .Load(hInstance, IDB_BOX);
	_bmpNormal[FACE_NORMAL]	.Load(hInstance, IDB_FACENORMAL);
	_bmpNormal[FACE_COOL]	.Load(hInstance, IDB_FACEGLASS);
	_bmpNormal[FACE_WINK]	.Load(hInstance, IDB_FACEWINK);
	_bmpWin[WIN_SMILE]		.Load(hInstance, IDB_WINSMILE);
	_bmpWin[WIN_BIGSMILE]	.Load(hInstance, IDB_WINBIGSMILE);
	_bmpWin[WIN_CHUCKLE]	.Load(hInstance, IDB_WINCHUCKLE);
	_bmpWin[WIN_LAUGH]		.Load(hInstance, IDB_WINLAUGH);
	_bmpWin[WIN_RELIEVED]   .Load(hInstance, IDB_WINRELIEVED);
	_bmpWin[WIN_COOL]		.Load(hInstance, IDB_WINCOOL);
	_bmpWin[WIN_SMOKER]		.Load(hInstance, IDB_WINSMOKE);
	_bmpBeast.Load(hInstance, IDB_BEAST);

    DesktopCanvas canvas;
    _bmpBack.CreateCompatible (canvas, 1, 1);

	_world->Init();
}

void CView::Paint (Canvas& canvas)
{
	if (_width == 0 || _height == 0)
		return;

    Blitter blitback (_bmpBack);
	Blitter blittop  (_bmpInfo[0]);
	Blitter blitbot  (_bmpInfo[1]);
	blitback.SetDest (client.left, client.top);
	blittop .SetDest (display[0].left, display[0].top);
	blitbot .SetDest (display[1].left, display[1].top);
    blitback.BlitTo  (canvas);
	blittop .BlitTo  (canvas);
	blitbot .BlitTo  (canvas);
}

void CView::Size (HWND h, int width, int height)
{
	client.top    = 20;
	client.right  = width;
	client.bottom = height - 20;
	_width  = width;
	_height = height - 40;

	display[0].left   = client.left;
	display[0].top    = 0;
	display[0].right  = client.right;
	display[0].bottom = client.top;

	display[1].left   = client.left;
	display[1].top    = client.bottom + 1;
	display[1].right  = client.right;
	display[1].bottom = height - 1;
}

void CView::Update (Canvas & canvas)
{
	if (_width == 0 || _height == 0)
		return;

    Bitmap bmp (canvas, _width, _height);
    BitmapCanvas bmpCanvas (canvas, bmp);

	bmpCanvas.WhiteWash(client);

	OutlineScreen(bmpCanvas);
	BlitBoard    (bmpCanvas);

	if (_world->GetState() == INTRO)
	{
		int cx, cy, y = 0;
		bmpCanvas.GetTextSize(cx, cy);

		char szBuffer[128];
		sprintf(szBuffer, "Boxes");

		y = ((client.bottom - client.top) - 2 * cy) / 2;
		bmpCanvas.Text((client.right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));
		y += cy;
		sprintf(szBuffer, "©2002 Michael Schmitz");
		bmpCanvas.Text((client.right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));

	}
	else
	{
		int i = 0;
		CBeast * beast;
		Blitter bltBeast(_bmpBeast);
		while((beast = _world->_beasts.GetItem(i++)) != NULL)
		{
			bltBeast.SetDest((beast->GetX() + 1) * 20, (beast->GetY() + 1) * 20);
			bltBeast.BlitTo(bmpCanvas);
		}
		
		if (!_world->Lost())
		{
			CPlayer * player = &_world->_player;
			Bitmap  * facebmp;
			if (_world->Won())
				facebmp = &_bmpWin[_world->GetWinFaceIndex()];
			else
				facebmp = &_bmpNormal[_world->GetFaceIndex()];

			Blitter bltNorm(*facebmp);
			bltNorm.SetDest((player->GetX() + 1) * TILEW, (player->GetY() + 1) * TILEH);
			bltNorm.SetMode(SRCAND);
			bltNorm.BlitTo (bmpCanvas);
		}
	}

	_bmpBack = bmp;
}

void CView::UpdateInfo (Canvas & canvas)
{
	if (_width == 0 || _height == 0)
		return;

	// Create bitmaps
    Bitmap bmp1 (canvas, _width, 20);
	Bitmap bmp2 (canvas, _width, 20);
    BitmapCanvas bmpTitle (canvas, bmp1);
	BitmapCanvas bmpLower (canvas, bmp2);

	// Wash with white
	RECT rect = { 0, 0, client.right, 20 };
	bmpTitle.WhiteWash(rect);
	bmpLower.WhiteWash(rect);

	// Setup constants and destinations
	int cx, cy, x = 0, y = 0;
	bmpTitle.GetTextSize(cx, cy);

	char szBuffer[128];

	if (_world->GetState() == INTRO)
	{
		/*
		sprintf(szBuffer, "Welcome To #####");
		bmpTitle.Text((display[1].right - strlen(szBuffer) * cx) / 2, 0, szBuffer, strlen(szBuffer));

		sprintf(szBuffer, "By Michael Schmitz");
		bmpLower.Text((display[1].right - strlen(szBuffer) * cx) / 2, 0, szBuffer, strlen(szBuffer));
		*/
	}
	else if (_world->GetState() == GAME)
	{
		static bool bHighscore = false;
		if (_world->Won())
		{
			if (_world->Highscore())
				bHighscore = true;

			if (bHighscore)
			{
				sprintf(szBuffer, "New Highscore!  Press ENTER to continue.", _world->GetLevel(), _world->GetSeconds());

				bmpLower.Text((display[1].right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));
			}
			else
			{
				sprintf(szBuffer, "Level %i reached in %u seconds.  Press ENTER to continue.", _world->GetLevel(), _world->GetSeconds());

				bmpLower.Text((display[1].right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));
			}
		}
		else if (_world->Lost())
		{
			if (_world->Highscore())
				bHighscore = true;

			if (bHighscore)
			{
				sprintf(szBuffer, "New Highscore!  Press ENTER to retry level.", _world->GetLevel(), _world->GetSeconds());

				bmpLower.Text((display[1].right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));
			}
			else
			{
				sprintf(szBuffer, "Died on level %i in %u seconds.  Press ENTER to retry level.", _world->GetLevel(), _world->GetSeconds());

				bmpLower.Text((display[1].right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));
			}
		}
		else
		{
			bHighscore = false;

			sprintf(szBuffer, " Beasts: %i", _world->GetBeastNumber());
			sprintf(szBuffer, "%s        Beast Delay: %i", szBuffer, _world->GetBeastDelay());
			sprintf(szBuffer, "%s        Box Density: %i/100", szBuffer, _world->GetBoxDensity());
			sprintf(szBuffer, "%s        Level: %i", szBuffer, _world->GetLevel());
			sprintf(szBuffer, "%s        Time: %us", szBuffer, _world->GetSeconds());

			bmpLower.Text(x, y, szBuffer, strlen(szBuffer));
		}

		sprintf(szBuffer, "Highscore:  Level %i in %u seconds", _world->_highscore.iLevel, _world->_highscore.dwSeconds);
		bmpTitle.Text((display[1].right - strlen(szBuffer) * cx) / 2, y, szBuffer, strlen(szBuffer));
	}
	else if (_world->GetState() == ARCADE)
	{
		sprintf(szBuffer, " Beasts: %i", _world->GetBeastNumber());
		sprintf(szBuffer, "%s         Beast Delay: %i", szBuffer, _world->GetBeastDelay());
		sprintf(szBuffer, "%s         Box Density: %i/100", szBuffer, _world->GetBoxDensity());
		sprintf(szBuffer, "%s         Time: %us", szBuffer, _world->GetSeconds());

		bmpLower.Text(x, y, szBuffer, strlen(szBuffer));
	}

	_bmpInfo[0] = bmp1;
	_bmpInfo[1] = bmp2;
}

void CView::BlitBoard(Canvas & canvas)
{
	Blitter blt(_bmpBox);
	blt.SetArea(20, 20);

	for (int x = 0; x < _world->_board.Width(); x++)
	{
		for (int y = 0; y < _world->_board.Height(); y++)
		{
			if (_world->_board.Get(x, y) == BOX)
			{
				blt.SetDest((x + 1) * TILEW, (y + 1) * TILEH);
				blt.BlitTo(canvas);
			}
		}
	}
}

void CView::OutlineScreen(Canvas & canvas)
{
	int x, y;
	int width, height;

	_bmpBox.GetSize(width, height);

	Blitter blt(_bmpBox);
	blt.SetArea(20, 20);

	y = 0;
	for (x = 0; x < _width; x += width)
	{
		blt.SetDest(x, y);
		blt.BlitTo(canvas);
	}
	y = _height - height;
	for (x = 0; x < _width; x += width)
	{
		blt.SetDest(x, y);
		blt.BlitTo(canvas);
	}
	x = 0;
	for (y = 0; y < _height; y += height)
	{
		blt.SetDest(x, y);
		blt.BlitTo(canvas);
	}
	x = _width - width;
	for (y = 0; y < _height; y += height)
	{
		blt.SetDest(x, y);
		blt.BlitTo(canvas);
	}
}