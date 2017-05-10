#include "ChessClosings.h"

ChessClosings* ChessClosings::mInstance = NULL;


ChessClosings::ChessClosings()
{
	isActive = false;
}


ChessClosings::~ChessClosings()
{
	mInstance = NULL;
}

ChessClosings* ChessClosings::Instance()
{
	if (!mInstance)
		mInstance = new ChessClosings();

	return mInstance;
}

bool ChessClosings::CheckForFinalPieces(Board board)
{
	int wPawnCount = 0;
	int wKnightCount = 0;
	int wRookCount = 0;
	int wQueenCount = 0;
	int wBishopCount = 0;
	int wKingCount = 0;

	for (int x = 0; x < kBoardDimensions; x++)
	{
		for (int y = 0; y < kBoardDimensions; y++)
		{
			Vector2D piecePosition = Vector2D(x, y);

			//Check for pieces.
			BoardPiece currentPiece = board.currentLayout[x][y];
			if (currentPiece.colour == COLOUR_WHITE  && currentPiece.piece != PIECE_NONE)
			{
				switch (currentPiece.piece)
				{
				case PIECE_PAWN:
					wPawnCount++;
					break;

				case PIECE_KNIGHT:
					wKnightCount++;
					break;

				case PIECE_BISHOP:
					wBishopCount++;
					break;

				case PIECE_ROOK:
					wRookCount++;
					break;

				case PIECE_QUEEN:
					wQueenCount++;
					break;

				case PIECE_KING:
					wKingCount++;
					break;

				case PIECE_NONE:
					break;

				default:
					break;
				}
			}
		}
	}

	int pieceCount = wPawnCount + wKnightCount + wBishopCount + wRookCount + wQueenCount;

	if (pieceCount < 5)
	{
		isActive = true;
		cout << "Endgame is Active" << endl;
		return true;
	}
	else 
		return false;
}
