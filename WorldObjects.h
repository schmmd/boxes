#ifndef _H_WORLDOBJECTS
#define _H_WORLDOBJECTS

#include <windows.h>

class CEntity
{
public:
	long GetX(		) { return _point.x; }
	long GetY(		) { return _point.y; }
	void SetX(long x) { _point.x = x; }
	void SetY(long y) { _point.y = y; }

	void Left () { _point.x -= 1; }
	void Right() { _point.x += 1; }
	void Up   () { _point.y -= 1; }
	void Down () { _point.y += 1; }

	POINT const * GetPointReference() { return &_point; }

private:
	POINT _point;
};

class CPlayer : public CEntity
{

};

#define ERROR 0
#define EMPTY 1
#define BOX   2

class CBoard
{
public:
	CBoard() : _board(NULL), _width(0), _height(0) { }

	~CBoard()
	{
		delete _board;
	}

	char Get (int x, int y)			  { if (x >= _width || x < 0 || y >= _height || y < 0) return ERROR; else return _board[x + _width * y]; }
	void Set (int x, int y, char val) { _board[x + _width * y] = val;  }

	int  Width () { return _width;  }
	int  Height() { return _height; }

	void RandomSpace(int & x, int & y);

	void Size(int width, int height);

	void PopulateBoxes(float flDensity);
	void Clear()
	{
		for (int i = 0; i < _width; i++)
			for (int n = 0; n < _height; n++)
				Set(i, n, EMPTY);
	}

private:
	char * _board;
	int _width;
	int _height;
};

#endif