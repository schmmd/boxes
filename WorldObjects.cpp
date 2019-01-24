#include "WorldObjects.h"

void CBoard::PopulateBoxes(float flDensity)
{
	int iBoxes = (int)(_width * _height * flDensity);

	for (int i = 0; i < iBoxes; i++)
	{
		while (true)
		{
			int x, y;
			x = rand() % _width;
			y = rand() % _height;

			if (Get(x, y) == EMPTY)
			{
				Set(x, y, BOX);
				break;
			}
		}
	}
}

void CBoard::RandomSpace(int & x, int & y)
{
	while (true)
	{
		x = rand() % _width;
		y = rand() % _height;

		if (Get(x, y) == EMPTY)
			break;
	}
}

void CBoard::Size(int width, int height)
{
	_width  = width;
	_height = height;
	_board = new char[width * height];
	memset(_board, EMPTY, sizeof(char) * width * height);
}