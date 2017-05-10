#pragma once
#include <vector>
#include "ChessCommons.h"

//http://chess.about.com/od/openings/tp/TopOpenings.htm
class ChessOpenings
{
	enum Openings {
		SpanishOpening, //e5 //Nc6 //Bb5
		//SicilianDefense //c5
		//ItalianGame //e5 //Nc6 //Bc5
	};

public:
	ChessOpenings();
	~ChessOpenings();
	static ChessOpenings* Instance();
	void IncrementCurrentMoveNumber();

	vector<Move> moves;
	int currentMoveNumber;

private:
	static ChessOpenings* mInstance;

};

