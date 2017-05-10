//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _GAMESCREEN_RAINBOW_H   
#define _GAMESCREEN_RAINBOW_H

#include "../GameScreen.h"
#include "Commons_RainbowIslands.h"
#include "../Commons.h"
#include <SDL.h>
#include <vector>
#include "../NeuralNetwork.h"
#include "../GeneticAlgorithm.h"
using namespace::std;

class Texture2D;
class Character;
class CharacterBub;
class CharacterFruit;
class CharacterRainbow;
class CharacterChest;
class LevelMap;


class GameScreen_RainbowIslands : GameScreen
{
//--------------------------------------------------------------------------------------------------
public:
	GameScreen_RainbowIslands(SDL_Renderer* renderer);
	~GameScreen_RainbowIslands();

	bool SetUpLevel();
	void Render();
	void RenderBackground();
	void Update(float deltaTime, SDL_Event e);

//--------------------------------------------------------------------------------------------------
private:
	void SetLevelMap();
	void RestartLevel();
	void CreateStartingCharacters();

	void MakeEnemiesAngry();
	void UpdateEnemies(float deltaTime, SDL_Event e);
	void CreateCaterpillar(Vector2D position, FACING direction);

	void UpdateRainbows(float deltaTime, SDL_Event e);
	void CreateRainbow(Vector2D position, int numberOfRainbows);

	void UpdateFruit(float deltaTime, SDL_Event e);
	void CreateFruit(Vector2D position, bool bounce);

	void CreateChest(Vector2D position);
	void TriggerChestSpawns();

	void DrawDebugCircle(Vector2D centrePoint, double radius, int red, int green, int blue);
	void DrawDebugLine(Vector2D startPoint, Vector2D endPoint, int red, int green, int blue);

	void NeuraliseTheNetworkOfNeural();
	CharacterFruit * FindClosestFruit();
	Character * FindClosestEnemy();
	void SaveWeightsToFile();
	bool ReadWeightsFromFile();

//--------------------------------------------------------------------------------------------------
private:
	Texture2D*				  mBackgroundTexture;
	CharacterBub*			  mBubCharacter;
	bool					  mCanSpawnRainbow;
	vector<Character*>		  mEnemies;
	vector<CharacterFruit*>	  mFruit;
	vector<CharacterRainbow*> mRainbows;
	CharacterChest*			  mChest;
	LevelMap*				  mLevelMap;

	int						  mTimeToCompleteLevel;
	bool					  mTriggeredAnger;

	bool					  mTriggeredChestSpawns;

	NeuralNetwork*			  mNeuralNetwork;
	GeneticAlgorithm*		  mGeneticAlgorithm;

	vector<Genome>	          m_vecSetOfWeights;
	int						  currentPopulationCount;
	int						  mGenAlgUpdateTimer;
	const int				  mGenAlgUpdateTime = 60000; //20 seconds
	//stores the average fitness per generation for use 
	//in graphing.
	vector<double>		   m_vecAvFitness;
	//stores the best fitness per generation
	vector<double>		   m_vecBestFitness;
	int						  mGenerationCount;

	CharacterFruit* closestFruit = nullptr;
	Character* closestEnemy = nullptr;
	int rainbowsFired = 0;


};


#endif //_GAMESCREEN_RAINBOW_H