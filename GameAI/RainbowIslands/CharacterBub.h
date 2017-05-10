//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _CHARACTERBUB_H
#define _CHARACTERBUB_H

#include "Character.h"
#include "CharacterRainbow.h"
#include <iostream>
#include <vector>
using namespace::std;

class CharacterBub : public Character
{

//--------------------------------------------------------------------------------------------------
public:
	CharacterBub(SDL_Renderer* renderer, string imagePath, LevelMap* map, Vector2D startPosition);
	~CharacterBub();

	void AlternateCharacterUpdate(float deltaTime, SDL_Event e);  //Required as it is very similar to original, but with RAINBOW tweaks.
	void Update(float deltaTime, SDL_Event e);
	void Render();

	void MoveLeft(float deltaTime, int yCentreOnImage);
	void MoveRight(float deltaTime, int yCentreOnImage);
	void Jump();

	void SetState(CHARACTER_STATE newState);

	bool OnARainbow()						{ return mOnRainbow; }
	void SetOnARainbow(bool yesNo)			{ mOnRainbow = yesNo; }

	void AddPoints()						{ mPoints += 10; }
	int  GetPoints()						{ return mPoints; }

	int	GetRainbowsAllowed();

	vector<double> GetSurroundings(Vector2D enemyLocation, vector<CharacterRainbow*> _mRainbows);

	vector<Vector2D> surroundingPositions;
	vector<double> surroundings;

//--------------------------------------------------------------------------------------------------
private:
	int		mStartFrame;
	int		mEndFrame;
	int		mCurrentFrame;
	float	mFrameDelay;

	bool	mOnRainbow;
	int		mPoints;
};
#endif //_CHARACTERBUB_H