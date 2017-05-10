#pragma once
#include "Chess\ChessPlayer.h"
#include "Texture2D.h"
#include"Chess\ChessOpenings.h"
#include "Chess\ChessMoveManager.h"
#include "GameScreenManager.h"
#include "ChessClosings.h"

struct TreeNode
{
	int layer;
	COLOUR teamColour;
	Board board;
	vector<TreeNode*> childTurns;
	Move actualMoveTaken; //<< To pass to MakeAMove();
	TreeNode* parentNode; //<< For finding the original move in minimax
	TreeNode() {};

	TreeNode(TreeNode* _parent,  Board _board)
	{
		board = _board;
		parentNode = _parent;
	}
};

struct killMe
{
	inline bool operator() (const TreeNode* struct1, const TreeNode* struct2)
	{
		return (struct1->board.score < struct2->board.score);
	}
};

class ChessPlayerAI :
	public ChessPlayer
{
public:
	ChessPlayerAI(SDL_Renderer* renderer, COLOUR colour, Board* board, vector<Vector2D>* highlights, Vector2D* selectedPiecePosition, Move* lastMove, int* searchDepth);
	~ChessPlayerAI();

	bool inOpeningStage;
	bool inClosingStage;
	int currentMoveNumber;

	int* searchDepth;
	int currentSearchDepth; //<< NOT USED - TO DELETE
	TreeNode* root;
	Move bestMove;
	TreeNode* bestMoveNode;
	vector<Move> *bestmoves = new vector<Move>;

	//MiniMaxAlphaBeta
	Board bestMoveBoard;
	int AllTimeHigh;
	int AllTimeLow;

	
	void createTree(TreeNode * board);
	Board createBoardFromMove(Board _board, Move _move);
	int minimax(TreeNode* nextTurn, int depth);
	int minimaxAlphaBeta(Board nextTurn, int depth, int high, int low);
	int maximum(Board nextTurn, int depth, int alpha, int beta);
	int minimum(Board nextTurn, int depth, int alpha, int beta);
	COLOUR futureWhoseMove;
	Board CalcBoardFitness(Board board);

	int MinimumAlphaBetaPruning(Board node, int depth, int parentHigh);
	int MaximumAlphaBetaPruning(Board node, int depth, int parentLow);

	bool comp(TreeNode* tn1, TreeNode* tn2) { return tn1->board.score > tn2->board.score ? true : false; }

	void DebugOutput();
	bool TakeATurn(SDL_Event e);
	//bool MakeAMove(Vector2D boardPosition);
	void EndTurn();
	void deleteTree(TreeNode * _node);
	Move getMove(int minimaxReturn);
	Move treeGetMove();
	bool MakeAMove(Move _move);
	GAMESTATE PreTurn();

	//Virtual Methods that I need from chessPlayer.cpp
	//virtual GAMESTATE	PreTurn();
	//virtual bool		TakeATurn(SDL_Event e);
	//virtual void		EndTurn();
	//virtual bool MakeAMove(Vector2D boardPosition);

};

