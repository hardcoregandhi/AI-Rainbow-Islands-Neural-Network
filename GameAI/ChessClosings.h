#pragma once
#include "Chess\ChessCommons.h"

class ChessClosings
{
public:
	ChessClosings();
	~ChessClosings();
	static ChessClosings* Instance();
	bool CheckForFinalPieces(Board board);
	bool isActive;

private:
	static ChessClosings* mInstance;

};

