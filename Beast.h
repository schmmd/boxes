#ifndef _H_BEAST
#define _H_BEAST

#include "worldobjects.h"
#include <windows.h>

#define BEAST_REGULAR 1
#define BEAST_RANDOM  2
#define BEAST_TRACER  3

class CBeast : public CEntity
{
public:
	CBeast() : iTrace(0) {}
	void AttachBoard (CBoard*  board ) { _board  = board; }
	void AttachPlayer(CPlayer* player) { _player = player; }
	void SetType (DWORD type) { _type = type; }

	void RunAI();
protected:
	CBoard*  _board;
	CPlayer* _player;

	int xPre, yPre;
	int cxTrace, cyTrace;
	int iTrace;

	DWORD _type;
};

#endif