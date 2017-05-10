#include "ChessPlayerAI.h"
#include <thread>
#include <algorithm>

//https://en.wikibooks.org/wiki/C%2B%2B_Programming/Classes/Inheritance
ChessPlayerAI::ChessPlayerAI(SDL_Renderer * renderer, COLOUR colour, Board * board, vector<Vector2D>* highlights, Vector2D * selectedPiecePosition, Move * lastMove, int* _searchDepth) 
	:ChessPlayer(renderer, colour, board, highlights, selectedPiecePosition, lastMove)
{
	searchDepth = _searchDepth;
	futureWhoseMove = COLOUR_BLACK;
	currentMoveNumber = 0;
	
	/*TODO
	*/
}

ChessPlayerAI::~ChessPlayerAI()
{
}

void ChessPlayerAI::createTree(TreeNode* _node)
{
	COLOUR teamToSearchFor;
	if (_node->teamColour == COLOUR_WHITE)
		teamToSearchFor = COLOUR_BLACK;
	else if (_node->teamColour == COLOUR_BLACK)
		teamToSearchFor = COLOUR_WHITE;

	vector<Move> *possibleMoves = new vector<Move>;
	GetAllMoveOptions(_node->board, teamToSearchFor, possibleMoves);
	_node->board = CalcBoardFitness(_node->board);// = _node->possibleMoves->size();
	Board tempCurrentBoard;
	tempCurrentBoard.score = -1;
	//Create new boards for each possible move - Layer 1
	for each (Move move in *possibleMoves)
	{
		tempCurrentBoard = _node->board;
		Board newBoard = createBoardFromMove(tempCurrentBoard, move);
		//if (move.from_X == 1 &&
		//	move.from_Y == 4 && 
		//	move.to_X == 2 && 
		//	move.to_Y == 5)
		//{
		//	cout << endl;
		//}
		newBoard = CalcBoardFitness(newBoard);
		TreeNode* newNode = new TreeNode(_node, newBoard);
		newNode->layer = _node->layer + 1;
		newNode->actualMoveTaken = move;
		newNode->teamColour = teamToSearchFor;
		//cout << "NEW NODE :: " << MoveManager::Instance()->ConvertBoardPositionIntToLetter(newNode->actualMoveTaken.from_X) << newNode->actualMoveTaken.from_Y
		//	<< MoveManager::Instance()->ConvertBoardPositionIntToLetter(newNode->actualMoveTaken.to_X) << newNode->actualMoveTaken.to_Y << endl;
		//cout << "NODE SCORE :: " << newNode->board.score;

		_node->childTurns.push_back(newNode);
		if (newNode->layer < *searchDepth)
		{
			createTree(newNode);
		}
	}
	delete possibleMoves;
}

Board ChessPlayerAI::createBoardFromMove(Board _board, Move _move)
{
	/*
	COPIED FROM ChessPlayer.cpp Ln 209

	//Move the piece into new position.
	mChessBoard->currentLayout[(int)mSelectedPiecePosition->x][(int)mSelectedPiecePosition->y].hasMoved = true;
	mChessBoard->currentLayout[(int)boardPosition.x][(int)boardPosition.y] = mChessBoard->currentLayout[(int)mSelectedPiecePosition->x][(int)mSelectedPiecePosition->y];
	mChessBoard->currentLayout[(int)mSelectedPiecePosition->x][(int)mSelectedPiecePosition->y] = BoardPiece();
	*/

	Board tempBoard = _board;
	tempBoard.currentLayout[(int)_move.to_X][(int)_move.to_Y] = tempBoard.currentLayout[(int)_move.from_X][(int)_move.from_Y];
	tempBoard.currentLayout[(int)_move.from_X][(int)_move.from_Y] = BoardPiece();

	return tempBoard;
}

int ChessPlayerAI::minimax(TreeNode* nextTurn, int depth)
{
	//REMINDER need to create the tree first
	int alpha = 0;
	if (depth == 0)
	{
		//if (nextTurn->board.score > 0)
		//	cout << "FOUND ONE, SCORE OF " << nextTurn->board.score << endl;
		return nextTurn->board.score;
	}
	//if this is opponents move
	if(futureWhoseMove == COLOUR_WHITE)
	{
		futureWhoseMove = COLOUR_BLACK;
		// Return value of minimum-valued child node       
		alpha = MaxInt;
		for each (TreeNode* nextNextTurn in nextTurn->childTurns)
		{
			int minimaxReturn = minimax(nextNextTurn, depth - 1);
			if (alpha > minimaxReturn)
			{
				alpha = minimaxReturn;
			}
		}
	}
	//else if this is our move
	else if(futureWhoseMove == COLOUR_BLACK)
	{
		futureWhoseMove = COLOUR_WHITE;
		// Return value of maximum-valued child node  
		alpha = -MaxInt;
		for each (TreeNode* nextNextTurn in nextTurn->childTurns)
		{
			int minimaxReturn = minimax(nextNextTurn, depth - 1);
			if (alpha < minimaxReturn)
			{
				alpha = minimaxReturn;
				if (depth == *searchDepth)
					bestMoveNode = nextNextTurn;
			}
		}
	}

	return alpha;
}

int ChessPlayerAI::minimaxAlphaBeta(Board nextTurn, int depth, int parentHigh, int parentLow)
{
	int alpha = 0;
	int beta = 0;
	if (depth == 0)
	{
		//if (nextTurn->board.score > 0)
		//	cout << "FOUND ONE, SCORE OF " << nextTurn->board.score << endl;
		return nextTurn.score;
	}
	//if this is opponents move
	if (futureWhoseMove == COLOUR_WHITE)
	{
		futureWhoseMove = COLOUR_BLACK;
		vector<Move> *possibleMoves = new vector<Move>;
		GetAllMoveOptions(nextTurn, COLOUR_WHITE, possibleMoves);
		// Return value of minimum-valued child node       
		int a = MaxInt;
		for each (Move nextNextTurn in *possibleMoves)
		{
			Board newBoard;
			newBoard = createBoardFromMove(newBoard, nextNextTurn);
			newBoard = CalcBoardFitness(newBoard);
			int minimaxReturn = minimaxAlphaBeta(newBoard, depth - 1, MaxInt, a);
			if (a > minimaxReturn)
			{
				a = minimaxReturn;

				if (depth == *searchDepth)
					bestMove = nextNextTurn;
			}
			if (a < alpha)
				return a;
		}
	}
	//else if this is our move
	else if (futureWhoseMove == COLOUR_BLACK)
	{
		futureWhoseMove = COLOUR_WHITE;
		vector<Move> *possibleMoves = new vector<Move>;
		GetAllMoveOptions(nextTurn, COLOUR_BLACK, possibleMoves);
		// Return value of maximum-valued child node       
		int a = -MaxInt;
		for each (Move nextNextTurn in *possibleMoves)
		{
			Board newBoard = createBoardFromMove(nextTurn, nextNextTurn);
			newBoard = CalcBoardFitness(newBoard);
			if (nextNextTurn.from_X == 1 &&
				nextNextTurn.from_Y == 4 &&
				nextNextTurn.to_X == 2 &&
				nextNextTurn.to_Y == 5)
			{
				cout << endl;
			}
			int minimaxReturn = minimaxAlphaBeta(newBoard, depth - 1, a, -MaxInt);
			if (a < minimaxReturn)
			{
				a = minimaxReturn;

				if (depth == *searchDepth)
					bestMove = nextNextTurn;
			}
			if (a > beta)
				return a;
		}
	}

	return alpha;
}

int ChessPlayerAI::maximum(Board nextTurn, int depth, int alpha, int beta)
{
	//REMINDER need to create the tree first
	if (depth == 0)
	{
		return nextTurn.score;
	}
	vector<Move> *possibleMoves = new vector<Move>;
	GetAllMoveOptions(nextTurn, COLOUR_BLACK, possibleMoves);
	for each(Move mv in *possibleMoves)
	{
		Board newBoard = createBoardFromMove(nextTurn, mv);
		newBoard = CalcBoardFitness(newBoard);
		int oldAlpha = alpha;
		alpha = max(alpha, minimum(newBoard, depth - 1, alpha, beta));
		if (oldAlpha != alpha)
		{
			if (depth == *searchDepth)
			{
				bestMove = mv;
			}
		}
		if (alpha >= beta)
		{
			//cout << "Shortcut Found" << endl;
			return alpha;
		}
	}
	return beta;
}

int ChessPlayerAI::minimum(Board nextTurn, int depth, int alpha, int beta)
{
	//REMINDER need to create the tree first
	if (depth == 0)
	{
		return nextTurn.score;
	}
	vector<Move> *possibleMoves = new vector<Move>;
	GetAllMoveOptions(nextTurn, COLOUR_WHITE, possibleMoves);
	for each(Move mv in *possibleMoves)
	{
		Board newBoard = createBoardFromMove(nextTurn, mv);
		newBoard = CalcBoardFitness(newBoard);
		beta = min(beta, maximum(newBoard, depth - 1, alpha, beta));
		if (beta <= alpha)
		{
			//cout << "Shortcut Found" << endl;
			return alpha;
		}
	}
	return beta;
}

Board ChessPlayerAI::CalcBoardFitness(Board board)
{
	int scoreBlack = 0;
	int scoreWhite = 0;

	int bPawnCount = 0;
	int bKnightCount = 0;
	int bRookCount = 0;
	int bQueenCount = 0;
	int bBishopCount = 0;
	int bKingCount = 0;

	int wPawnCount = 0;
	int wKnightCount = 0;
	int wRookCount = 0;
	int wQueenCount = 0;
	int wBishopCount = 0;
	int wKingCount = 0;
	
	//for (int x = 3; x < 5; x++)
	//{
	//	for (int y = 3; y < 5; y++)
	//	{
	//		//Check for pieces.
	//		BoardPiece currentPiece = board.currentLayout[x][y];
	//		if (currentPiece.colour == mTeamColour && currentPiece.piece != PIECE_NONE)
	//		{
	//			switch (currentPiece.piece)
	//			{
	//			case PIECE_PAWN:
	//				scoreBlack += kPieceWorthPawn * kCenterControlWeight;
	//				break;

	//			case PIECE_KNIGHT:
	//				scoreBlack += kPieceWorthKnight * kKnightCenterControlWeight;
	//				break;

	//			case PIECE_BISHOP:
	//				scoreBlack += kPieceWorthBishop * kCenterControlWeight;
	//				break;

	//			case PIECE_ROOK:
	//				scoreBlack += kPieceWorthRook * kCenterControlWeight;
	//				break;

	//			case PIECE_QUEEN:
	//				scoreBlack += kPieceWorthQueen * kCenterControlWeight;
	//				break;

	//			case PIECE_KING:
	//				scoreBlack += kPieceWorthKing * kCenterControlWeight;
	//				break;

	//			case PIECE_NONE:
	//				break;

	//			default:
	//				break;
	//			}
	//		}
	//		else if (currentPiece.colour == !mTeamColour && currentPiece.piece != PIECE_NONE)
	//		{
	//			switch (currentPiece.piece)
	//			{
	//			case PIECE_PAWN:
	//				scoreWhite += kPieceWorthPawn * kCenterControlWeight;
	//				break;

	//			case PIECE_KNIGHT:
	//				scoreWhite += kPieceWorthKnight * kKnightCenterControlWeight;
	//				break;

	//			case PIECE_BISHOP:
	//				scoreWhite += kPieceWorthBishop * kCenterControlWeight;
	//				break;

	//			case PIECE_ROOK:
	//				scoreWhite += kPieceWorthRook * kCenterControlWeight;
	//				break;

	//			case PIECE_QUEEN:
	//				scoreWhite += kPieceWorthQueen * kCenterControlWeight;
	//				break;

	//			case PIECE_KING:
	//				scoreWhite += kPieceWorthKing * kCenterControlWeight;
	//				break;

	//			case PIECE_NONE:
	//				break;

	//			default:
	//				break;
	//			}
	//		}
	//	}
	//}
	//vector<Move> *possibleBlackBishopMoves = new vector<Move>;
	//vector<Move> *possibleBlackQueenMoves = new vector<Move>;
	//vector<Move> *possibleWhiteBishopMoves = new vector<Move>;
	//vector<Move> *possibleWhiteQueenMoves = new vector<Move>;

	for (int x = 0; x < kBoardDimensions; x++)
	{
		for (int y = 0; y < kBoardDimensions; y++)
		{
			Vector2D piecePosition = Vector2D(x, y);

			//Check for pieces.
			BoardPiece currentPiece = board.currentLayout[x][y];
			if (currentPiece.colour == mTeamColour && currentPiece.piece != PIECE_NONE)
			{
				switch (currentPiece.piece)
				{
				case PIECE_PAWN:
					scoreBlack += kPieceWorthPawn * kMaterialWeight;
					if (inClosingStage && ChessClosings::Instance()->isActive)
					{
						scoreBlack += x * 2;
					}
					bPawnCount++;
					break;

				case PIECE_KNIGHT:
					scoreBlack += kPieceWorthKnight * kMaterialWeight;
					bKnightCount++;
					break;

				case PIECE_BISHOP:
					//GetDiagonalMoveOptions(piecePosition, currentPiece, board, possibleBlackBishopMoves);
					//scoreBlack += kBishopCoverage * possibleBlackBishopMoves->size();
					scoreBlack += kPieceWorthBishop * kMaterialWeight;
					bBishopCount++;
					break;

				case PIECE_ROOK:
					scoreBlack += kPieceWorthRook * kMaterialWeight;
					bRookCount++;
					break;

				case PIECE_QUEEN:
					//GetDiagonalMoveOptions(piecePosition, currentPiece, board, possibleBlackQueenMoves);
					//GetHorizontalAndVerticalMoveOptions(piecePosition, currentPiece, board, possibleBlackQueenMoves);
					//scoreBlack += kQueenCoverage * possibleBlackQueenMoves->size();
					scoreBlack += kPieceWorthQueen * kMaterialWeight;
					bQueenCount++;
					break;

				case PIECE_KING:
					scoreBlack += kPieceWorthKing * kMaterialWeight;
					bKingCount++;
					break;

				case PIECE_NONE:
					break;

				default:
					break;
				}
			}
			else if (currentPiece.colour != mTeamColour && currentPiece.piece != PIECE_NONE)
			{
				switch (currentPiece.piece)
				{
				case PIECE_PAWN:
					scoreWhite += kPieceWorthPawn * kMaterialWeight;
					wPawnCount++;
					break;

				case PIECE_KNIGHT:
					scoreWhite += kPieceWorthKnight * kMaterialWeight;
					wKnightCount++;
					break;

				case PIECE_BISHOP:
					/*GetDiagonalMoveOptions(piecePosition, currentPiece, board, possibleWhiteBishopMoves);
					scoreWhite += kBishopCoverage * possibleWhiteBishopMoves->size();*/
					scoreWhite += kPieceWorthBishop * kMaterialWeight;
					wBishopCount++;
					break;

				case PIECE_ROOK:
					scoreWhite += kPieceWorthRook * kMaterialWeight;
					wRookCount++;
					break;

				case PIECE_QUEEN:
					/*GetDiagonalMoveOptions(piecePosition, currentPiece, board, possibleWhiteQueenMoves);
					GetHorizontalAndVerticalMoveOptions(piecePosition, currentPiece, board, possibleWhiteQueenMoves);
					scoreWhite += kQueenCoverage * possibleWhiteQueenMoves->size();*/
					scoreWhite += kPieceWorthQueen * kMaterialWeight;
					wQueenCount++;
					break;

				case PIECE_KING:
					scoreWhite += kPieceWorthKing * kMaterialWeight;
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

	/*possibleWhiteBishopMoves->clear();
	possibleWhiteQueenMoves->clear();
	possibleBlackBishopMoves->clear();
	possibleBlackQueenMoves->clear();*/

	/*delete possibleWhiteBishopMoves;
	delete possibleWhiteQueenMoves;
	delete possibleBlackBishopMoves;
	delete possibleBlackQueenMoves;*/

	if (CheckForCheck(board, COLOUR_WHITE))
		scoreBlack += 5;

	if (CheckForCheckmate(board, COLOUR_WHITE))
		scoreBlack += 1000000;

	if (CheckForCheck(board, COLOUR_BLACK))
		scoreWhite += 5;

	if (CheckForCheckmate(board, COLOUR_BLACK))
		scoreWhite += 1000000;

	//vector<Move>* moves = new vector<Move>;
	//GetAllMoveOptions(board, COLOUR_BLACK, moves);
	//scoreBlack = scoreBlack + (0.1 * moves->size());

	//moves->clear();
	//GetAllMoveOptions(board, COLOUR_WHITE, moves);
	//scoreWhite = scoreWhite + (0.1 * moves->size());

	//cout << "BlackCount : " << endl;
	//cout << "	bPawnCount : " << bPawnCount << endl;
	//cout << "	bKnightCount : " << bKnightCount << endl;
	//cout << "	bBishopCount : " << bBishopCount << endl;
	//cout << "	bRookCount : " << bRookCount << endl;
	//cout << "	bKingCount : " << bKingCount << endl;
	//cout << "	bQueenCount : " << bQueenCount << endl;
	//cout << "WhiteCount : " << endl;
	//cout << "	wPawnCount : " << wPawnCount << endl;
	//cout << "	wKnightCount : " << wKnightCount << endl;
	//cout << "	wBishopCount : " << wBishopCount << endl;
	//cout << "	wRookCount : " << wRookCount << endl;
	//cout << "	wKingCount : " << wKingCount << endl;
	//cout << "	wQueenCount : " << wQueenCount << endl;

	board.score = scoreBlack - scoreWhite;
	//cout << board.score << endl;
	return board;
	
}

GAMESTATE ChessPlayerAI::PreTurn()
{
	//Check whether this player is in CHECK, CHECKMATE or STALEMATE.
	mInCheck = CheckForCheck(*mChessBoard, mTeamColour);

	//Remove any highlight position as we have yet to select a piece.
	mSelectedPiecePosition->x = -1;
	mSelectedPiecePosition->y = -1;

	//If we are in CHECK, can we actually make a move to get us out of it?
	if (mInCheck)
	{
		if (CheckForCheckmate(*mChessBoard, mTeamColour))
			return GAMESTATE_CHECKMATE;
		else
			return GAMESTATE_CHECK;
	}
	else
	{
		if (CheckForStalemate(*mChessBoard, mTeamColour))
			return GAMESTATE_STALEMATE;
		else
			return GAMESTATE_NORMAL;
	}

	//Return normal if none of the above conditions have been met.
	return GAMESTATE_NORMAL;
}

bool ChessPlayerAI::TakeATurn(SDL_Event e)
{
	futureWhoseMove = COLOUR_BLACK;

	if (inOpeningStage)
	{
		MakeAMove(ChessOpenings::Instance()->moves[currentMoveNumber]);
		currentMoveNumber++;
		if (currentMoveNumber >= ChessOpenings::Instance()->moves.size())
		{
			inOpeningStage = false;
		}
		return true;
	}

	//AlphaBetaPruning another attempt
	Board temp = *mChessBoard;

	bool inClosingStage = false;
	if(inClosingStage == false)
		inClosingStage = ChessClosings::Instance()->CheckForFinalPieces(temp);

	bestMove = Move();
	int alpha = MaximumAlphaBetaPruning(temp, *searchDepth, -MaxInt);
	//cout << alpha << endl;
	if (alpha == -2147483647)
	{
		cout << "Possible Move available" << endl;
		vector<Move> moves;
		GetAllMoveOptions(temp, COLOUR_BLACK, &moves);
		bestMove = moves[0];
	}
	MakeAMove(bestMove);

	DebugOutput();
	return true;
}

int ChessPlayerAI::MinimumAlphaBetaPruning(Board node, int depth, int parentHigh)
{
	if (depth == 0)
	{
		node = CalcBoardFitness(node);
		return node.score;
	}
	int alpha = MaxInt;
	vector<Move> moves;
	GetAllMoveOptions(node, COLOUR_WHITE, &moves);
	for each (Move child in moves)
	{
		if (alpha < parentHigh)
		{
			return alpha;
		}
		else
		{
			Board newBoard = createBoardFromMove(node, child);
			alpha = min(alpha, MaximumAlphaBetaPruning(newBoard, depth - 1, alpha));
			if (alpha < parentHigh)
			{
				parentHigh = alpha;
			}
		}
	}
	return alpha;
}

int ChessPlayerAI::MaximumAlphaBetaPruning(Board node, int depth, int parentLow)
{
	if (depth == 0)
	{
		node = CalcBoardFitness(node);
		return node.score;
	}
	int alpha = -MaxInt;
	vector<Move> moves;
	GetAllMoveOptions(node, COLOUR_BLACK, &moves);
	for each (Move child in moves)
	{
		if(alpha > parentLow)
		{
			return alpha;
		}
		else
		{
			Board newBoard = createBoardFromMove(node, child);
			alpha = max(alpha, MinimumAlphaBetaPruning(newBoard, depth - 1, alpha));
			//alpha = MinimumAlphaBetaPruning(newBoard, depth - 1, alpha);
			if (alpha > parentLow)
			{
				parentLow = alpha;

				if (depth == *searchDepth)
					bestMove = child;
			}
		}
	}
	return alpha;
}

void ChessPlayerAI::DebugOutput()
{
	cout << "Chess AI Debug Information - Search Depth: " << *searchDepth << endl;

	vector<Move> moves;
	GetAllMoveOptions(*mChessBoard, COLOUR_BLACK, &moves);

	cout << "PossibleMoves for ROOT board " << " = " << moves.size() << endl;

	Board currentHighest;
	currentHighest.score = -MaxInt;

	cout << bestMove.score;

	//cout << "The score was : " << node->board.score << endl;
	cout << "The best move found was: " << MoveManager::Instance()->ConvertBoardPositionIntToLetter(bestMove.from_X) << bestMove.from_Y
		<< MoveManager::Instance()->ConvertBoardPositionIntToLetter(bestMove.to_X) << bestMove.to_Y << endl;

	//CalcBoardFitness(node->board);

	int scoreBlack = 0;
	int scoreWhite = 0;

	cout << "Black Pieces : " << endl;
	for (int x = 0; x < kBoardDimensions; x++)
	{
		for (int y = 0; y < kBoardDimensions; y++)
		{
			//Check for pieces.
			BoardPiece currentPiece = mChessBoard->currentLayout[x][y];
			if (currentPiece.colour == mTeamColour && currentPiece.piece != PIECE_NONE)
				switch (currentPiece.piece)
				{
				case PIECE_PAWN:
					scoreBlack += kPieceWorthPawn;
					cout << "	PIECE_PAWN Found! Score = " << scoreBlack << endl;
					break;

				case PIECE_KNIGHT:
					scoreBlack += kPieceWorthKnight;
					cout << "	PIECE_KNIGHT Found! Score = " << scoreBlack << endl;
					break;

				case PIECE_BISHOP:
					scoreBlack += kPieceWorthBishop;
					cout << "	PIECE_BISHOP Found! Score = " << scoreBlack << endl;
					break;

				case PIECE_ROOK:
					scoreBlack += kPieceWorthRook;
					cout << "	PIECE_ROOK Found! Score = " << scoreBlack << endl;
					break;

				case PIECE_QUEEN:
					scoreBlack += kPieceWorthQueen;
					cout << "	PIECE_QUEEN Found! Score = " << scoreBlack << endl;
					break;

				case PIECE_KING:
					scoreBlack += kPieceWorthKing;
					cout << "	PIECE_KING Found! Score = " << scoreBlack << endl;
					break;

				case PIECE_NONE:
					break;

				default:
					break;
				}
		}
	}

	cout << "White Pieces : " << endl;
	for (int x = 0; x < kBoardDimensions; x++)
	{
		for (int y = 0; y < kBoardDimensions; y++)
		{
			//Check for pieces.
			BoardPiece currentPiece = mChessBoard->currentLayout[x][y];
			if (currentPiece.colour != mTeamColour && currentPiece.piece != PIECE_NONE)
				switch (currentPiece.piece)
				{
				case PIECE_PAWN:
					scoreWhite += kPieceWorthPawn;
					cout << "	PIECE_PAWN Found! Score = " << scoreWhite << endl;
					break;

				case PIECE_KNIGHT:
					scoreWhite += kPieceWorthKnight;
					cout << "	PIECE_KNIGHT Found! Score = " << scoreWhite << endl;
					break;

				case PIECE_BISHOP:
					scoreWhite += kPieceWorthBishop;
					cout << "	PIECE_BISHOP Found! Score = " << scoreWhite << endl;
					break;

				case PIECE_ROOK:
					scoreWhite += kPieceWorthRook;
					cout << "	PIECE_ROOK Found! Score = " << scoreWhite << endl;
					break;

				case PIECE_QUEEN:
					scoreWhite += kPieceWorthQueen;
					cout << "	PIECE_QUEEN Found! Score = " << scoreWhite << endl;
					break;

				case PIECE_KING:
					scoreWhite += kPieceWorthKing;
					cout << "	PIECE_KING Found! Score = " << scoreWhite << endl;
					break;

				case PIECE_NONE:
					break;

				default:
					break;
				}
		}
	}

	cout << "White Board Score = " << scoreWhite << endl;
	cout << "Black Board Score = " << scoreBlack << endl;
	cout << "Our Score = " << scoreWhite - scoreBlack << endl;
	cout << "Their Score = " << scoreBlack - scoreWhite << endl;

	
	cout << "Board Score = " << mChessBoard->score << endl;
}

void ChessPlayerAI::EndTurn()
{
	//Change move type.
	mCurrentMove = SELECT_A_PIECE;

	//Remove highlights.
	mHighlightPositions->clear();

	//Make all current player's pawns unavailable for en'passant. Opponent had their chance.
	ClearEnPassant();

}

void ChessPlayerAI::deleteTree(TreeNode* _node)
{
	for each (TreeNode* node in _node->childTurns)
	{
		deleteTree(node);
	}
	delete _node;
}

Move ChessPlayerAI::getMove(int minimaxReturn)
{


	//float moveScore = 0;
	//Move highestScoringMove;

	//for each (TreeNode* nextTurn in root->childTurns)
	//{
	//	if (nextTurn->board.score > moveScore)
	//	{
	//		moveScore = nextTurn->board.score;
	//		highestScoringMove = nextTurn->actualMoveTaken;
	//	}
	//}

	////CATCHALL
	//if (moveScore = 0)
	//{
	//	highestScoringMove = root->childTurns[0]->actualMoveTaken;
	//}

	return bestMove;
}

Move ChessPlayerAI::treeGetMove()
{
	while (bestMoveNode->parentNode != root)
		bestMoveNode = bestMoveNode->parentNode;

	return bestMoveNode->actualMoveTaken;
}

bool ChessPlayerAI::MakeAMove(Move _move)
{
	//cout << _move.from_X << _move.from_Y << "     " << _move.to_X << _move.to_Y << endl;
	//MoveManager::Instance()->OutputAIMove(_move);
	mChessBoard->currentLayout[(int)_move.to_X][(int)_move.to_Y] = mChessBoard->currentLayout[(int)_move.from_X][(int)_move.from_Y];
	mChessBoard->currentLayout[(int)_move.from_X][(int)_move.from_Y] = BoardPiece();
	
	MoveManager::Instance()->StoreMove(_move);
	//PAWN -> QUEEN
	if (mChessBoard->currentLayout[(int)_move.to_X][(int)_move.to_Y].piece == PIECE_PAWN &&
		((int)_move.to_Y == 0 || (int)_move.to_Y == 7))
	{
		//Get the new PIECE type, by getting a 0-1 value using its relative x position, and then multiplying by 4 pieces.
		//Add 1 to move the scale from KNIGHT to QUEEN.
		PIECE newPieceType = (PIECE)(PIECE_QUEEN);

		//Change the PAWN into the selected piece.
		mChessBoard->currentLayout[(int)_move.to_X][(int)_move.to_Y].piece = newPieceType;
	}
	
	return true;
}
