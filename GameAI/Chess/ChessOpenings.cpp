#include "ChessOpenings.h"
#include <iostream>
using namespace std;

ChessOpenings* ChessOpenings::mInstance = NULL;


ChessOpenings::ChessOpenings()
{
	switch (rand() % 3)
	{
	case 0:
		//SpanishOpening
		moves.push_back(Move(4, 1, 4, 3));
		moves.push_back(Move(1, 0, 2, 2));
		moves.push_back(Move(5, 0, 1, 4));
		break;
	case 1:
		//Sicilian Defence //e5 //Nc6 //Bc5
		moves.push_back(Move(2, 1, 2, 3));
		moves.push_back(Move(6, 0, 5, 2));
		moves.push_back(Move(6, 1, 6, 2));
	case 2:
		//French Defence
		moves.push_back(Move(4, 1, 4, 2));
		moves.push_back(Move(3, 1, 3, 3));
		moves.push_back(Move(6, 0, 5, 2));
		break;

	default:
		break;
	} 


	currentMoveNumber = 0;
}


ChessOpenings::~ChessOpenings()
{
	mInstance = NULL;
}

ChessOpenings* ChessOpenings::Instance()
{
	if (!mInstance)
		mInstance = new ChessOpenings();

	return mInstance;
}

void ChessOpenings::IncrementCurrentMoveNumber()
{
	cout << "currentMoveNumber++;" << endl;
	currentMoveNumber++;
}
