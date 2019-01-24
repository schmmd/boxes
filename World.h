#ifndef _H_WORLD
#define _H_WORLD

#include "worldobjects.h"
#include "linkedlist.h"
#include "beast.h"

#include <windows.h>
#include <fstream.h>

enum state
{
	INTRO,GAME,ARCADE
};

struct score
{
	DWORD dwSeconds;
	int iLevel;
};

class CWorld
{
public:
	CWorld () : _iBeasts(4), _iBeastDelay(45), _iBoxDensity(25), 
		_bPaused(false), _bWon(false), _bLost(false), _iFaceIndex(0), 
		_iWinFaceIndex(0), _dwSeconds(0), _state(INTRO)
	{ 
		memset(&_game, 0, sizeof(RECT)); 
		_highscore.iLevel = 0;
		_highscore.dwSeconds = 0;

		ifstream fin("settings.dat", ios::in | ios::binary | ios::nocreate);

		if (fin.fd() == -1)
		{
			_highscore.iLevel = 1;
			_highscore.dwSeconds = 0;
		}
		else
		{
			fin.setmode(filebuf::binary);
			fin.read((char*)&_highscore, sizeof(score));
		}
		fin.close();
	}

	~CWorld ()
	{
		ofstream fout("settings.dat", ios::out | ios::binary);
		fout.setmode(filebuf::binary);
		fout.write((char*)&_highscore, sizeof(score));
		fout.close();
	}

	void Init();
	void New();

	void NewGame();
	void NewLife();
	void NewArcade();

	void NewLevel();

	bool KeyDown(int iVirtKey);
	void RunBeastAI();

	void Size(int width, int height);

	bool IsBeast(int x, int y, CBeast * exclude = NULL);
	bool BeastsContained();

	int GetWinFaceIndex() { return _iWinFaceIndex; }
	int GetFaceIndex()    { return _iFaceIndex; }

	bool IsPaused() { return _bPaused; }
	void Pause   () { _bPaused = true; }
	void Unpause () { _bPaused = false; }

	void Win ();
	bool Won () { return _bWon; }
	void Loose();
	bool Lost () { return _bLost; }

	DWORD GetSeconds() { return _dwSeconds; }
	void  AddSecond () { _dwSeconds++; }

	int GetBeastNumber() { return _iBeasts; }
	int GetBeastDelay () { return _iBeastDelay; }
	int GetBoxDensity () { return _iBoxDensity; }
	int GetLevel      () { return _iLevel; }

	state GetState() { return _state; }

	bool Highscore();

	bool OutOfBounds(CEntity & entity)
	{
		if (entity.GetX() < _game.left || entity.GetY() < _game.top || entity.GetX() > _game.right || entity.GetY() > _game.bottom)
			return true;

		return false;
	}

	bool IllegalPosition(CEntity & entity)
	{
		if (OutOfBounds(entity) || _board.Get(entity.GetX(), entity.GetY()) == BOX)
			return true;

		return false;
	}

	CPlayer _player;
	CBoard  _board;
	CLinkedList<CBeast> _beasts;

	int _iBeasts, _iBeastDelay, _iBoxDensity;
	int _iFaceIndex, _iWinFaceIndex;

	score _highscore;

private:

	int _iLevel;

	DWORD _dwSeconds;
	DWORD _dwTotalSeconds;

	state _state;

	RECT _game;
	bool _bPaused;
	bool _bWon;
	bool _bLost;
};

#define FACE_NORMAL	0
#define FACE_COOL	1
#define FACE_WINK   2

#define WIN_SMILE		0
#define WIN_LAUGH		1
#define WIN_CHUCKLE		2
#define WIN_BIGSMILE	3
#define WIN_RELIEVED	4
#define WIN_COOL		5
#define WIN_SMOKER		6

#endif