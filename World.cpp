#include "world.h"

void CWorld::Init()
{
	srand(GetTickCount());
}

void CWorld::New()
{
	int x, y;

	_bWon  = false;
	_bLost = false;

	_dwSeconds = 0;

	_board.Clear();
	_board.PopulateBoxes((float)_iBoxDensity / (float)100.0);

	// Create player
	_board.RandomSpace(x, y);
	_player.SetX(x);
	_player.SetY(y);

	// Create beasts
	_beasts.DeleteItems();
	for (int i = 0; i < _iBeasts; i++)
	{
		do
		{
			_board.RandomSpace(x, y);
		} while (IsBeast(x, y) || (_player.GetX() == x && _player.GetY() == y));

		_beasts.AddItem();
		CBeast * beast = _beasts.GetItem(0);

		beast->SetX(x);
		beast->SetY(y);
		beast->AttachBoard (&_board);
		beast->AttachPlayer(&_player);
		beast->SetType     (BEAST_REGULAR);
	}

	if (BeastsContained())
		Win();
}

void CWorld::NewLevel()
{
	_iBeasts      = 3 + (_iLevel - 1);
	_iBeastDelay  = 60 - 2 * (_iLevel - 1);
	_iBoxDensity  = 35 - (int)(0.5 * (_iLevel - 1));
	/*
	if (_iLevel == 0)
		_iBeasts = 3;
	else
		_iBeasts += int(2.0 * (1.0 / _iLevel));

	_iBeastDelay  = 60 - _iLevel;
	_iBoxDensity  = 35 - (int)(0.4 * _iLevel);
	*/
}

void CWorld::NewGame()
{
	_state = GAME;

	_iLevel = 1;
	_dwTotalSeconds = 0;

	NewLevel();
	NewLife();
}

void CWorld::NewLife()
{
	if (rand() % 50 == 0)
		_iFaceIndex = FACE_COOL;
	else
		_iFaceIndex = FACE_NORMAL;

	New();
}

void CWorld::NewArcade()
{
	_state = ARCADE;
	_dwTotalSeconds = 0;

	if (rand() % 20 == 0)
		_iFaceIndex = FACE_COOL;
	else
		_iFaceIndex = FACE_NORMAL;

	New();
}

void CWorld::Size(int width, int height)
{
	_game.right  = width - 3; // -1 for border, -1 for conversion from width, -1
	_game.bottom = height - 3;

	_board.Size(width - 2, height - 2); // -2 for borders on each side
}

bool CWorld::KeyDown(int iVirtKey)
{
	if (!Won() && !Lost())
	{
		int xOld = _player.GetX(), yOld = _player.GetY();
		bool bMoved = false;
		if (iVirtKey == VK_LEFT)
		{
			_player.Left();
			bMoved = true;
		}
		else if (iVirtKey == VK_RIGHT)
		{
			_player.Right();
			bMoved = true;
		}
		else if (iVirtKey == VK_UP)
		{
			_player.Up();
			bMoved = true;
		}
		else if (iVirtKey == VK_DOWN)
		{
			_player.Down();
			bMoved = true;
		}

		if (bMoved)
		{
			bool bIllegalPosition = false;

			if (_board.Get(_player.GetX(), _player.GetY()) == BOX)
			{
				// Push boxes

				// Determine change of direction
				int cx = _player.GetX() - xOld;
				int cy = _player.GetY() - yOld;

				int  x = _player.GetX();
				int  y = _player.GetY();

				// Find next empty space
				while(_board.Get(x, y) == BOX)
				{
					x += cx;
					y += cy;

					if (x < 0 || y < 0 || x >= _board.Width() || y >= _board.Height())
					{
						bIllegalPosition = true;
						break;
					}
				}

				if (!bIllegalPosition && !IsBeast(x, y))
				{
					// Swap empty space with pushed tile
					_board.Set(x				, y				, BOX);
					_board.Set(_player.GetX()	, _player.GetY(), EMPTY);
				}
				else
					bIllegalPosition = true;
			}
			if (IllegalPosition(_player) || bIllegalPosition)
			{
				_player.SetX(xOld);
				_player.SetY(yOld);
			}

			if (BeastsContained())
				Win();
			else if (IsBeast(_player.GetX(), _player.GetY()))
				Loose();

			return true;
		}
	}

	return false;
}

bool CWorld::IsBeast(int x, int y, CBeast * exclude)
{
	int i = 0;
	CBeast * beast;
	while ((beast = _beasts.GetItem(i++)) != NULL)
	{
		if (beast == exclude)
			continue;

		if (beast->GetX() == x && beast->GetY() == y)
			return true;
	}

	return false;
}

bool CWorld::BeastsContained()
{
	int i = 0;
	CBeast * beast;
	while ((beast = _beasts.GetItem(i++)) != NULL)
	{
		int x = beast->GetX(), y = beast->GetY();

		for (int cx = -1; cx <= 1; cx++)
		{
			for (int cy = -1; cy <= 1; cy++)
			{
				char tile = _board.Get(x + cx, y + cy);
				if (!(cx == 0 && cy == 0) && (tile != BOX && tile != ERROR && !IsBeast(x + cx, y + cy)))
					return false;
			}
		}
	}

	return true;
}

void CWorld::RunBeastAI()
{
	int i = 0;
	CBeast * beast;
	while ((beast = _beasts.GetItem(i++)) != NULL)
	{
		int xOld = beast->GetX(), yOld = beast->GetY();

		beast->RunAI();

		if (IllegalPosition(*beast) || IsBeast(beast->GetX(), beast->GetY(), beast))
		{
			beast->SetX(xOld);
			beast->SetY(yOld);
		}
	}

	if (IsBeast(_player.GetX(), _player.GetY()))
		Loose();
}

bool CWorld::Highscore()
{
	bool bHighscore = false;

	if (_iLevel > _highscore.iLevel)
		bHighscore = true;
	else if (_iLevel == _highscore.iLevel && _dwTotalSeconds < _highscore.dwSeconds)
		bHighscore = true;

	if (bHighscore)
	{
		_highscore.iLevel    = _iLevel;
		_highscore.dwSeconds = _dwTotalSeconds;
		return true;
	}

	return false;
}

void CWorld::Win()
{
	_dwTotalSeconds += _dwSeconds;

	if (_iFaceIndex == FACE_NORMAL)
	{
		if (_dwSeconds < 60)
			_iWinFaceIndex = WIN_LAUGH;
		else if (_dwSeconds < 120)
			_iWinFaceIndex = WIN_CHUCKLE;
		else if (_dwSeconds < 240)
			_iWinFaceIndex = WIN_BIGSMILE;
		else if (_dwSeconds > 480)
			_iWinFaceIndex = WIN_RELIEVED;
		else
			_iWinFaceIndex = WIN_SMILE;
	}
	else if (_iFaceIndex == FACE_COOL)
	{
		if (_dwSeconds < DWORD(60 + _iLevel * 20))
			_iWinFaceIndex = WIN_SMOKER;
		else
			_iWinFaceIndex = WIN_COOL;
	}

	_bWon = true;
	_iLevel++;
}

void CWorld::Loose()
{ 
	_dwTotalSeconds += _dwSeconds;

	_bLost = true; 
}