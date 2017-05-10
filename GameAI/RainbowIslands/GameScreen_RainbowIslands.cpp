//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#include "GameScreen_RainbowIslands.h"
#include <iostream>
#include <algorithm>
#include "LevelMap.h"
#include "Character.h"
#include "CharacterBub.h"
#include "CharacterCaterpillar.h"
#include "CharacterRainbow.h"
#include "CharacterFruit.h"
#include "CharacterChest.h"
#include "Commons_RainbowIslands.h"
#include "Constants_RainbowIslands.h"
#include "../Texture2D.h"
#include "../Collisions.h"
#include "VirtualJoypad.h"

#include <fstream>

using namespace::std;

int RainbowOffsets[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,22,22,23,23,
						 23,22,22,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0, };
//--------------------------------------------------------------------------------------------------

GameScreen_RainbowIslands::GameScreen_RainbowIslands(SDL_Renderer* renderer) : GameScreen(renderer)
{
	srand(NULL);
	mLevelMap = NULL;
	SetUpLevel();
	mNeuralNetwork = new NeuralNetwork();

	mGenAlgUpdateTimer = 0;
	currentPopulationCount = 0;
	mGenerationCount = 0;

	//get the total number of weights used in the sweepers
	//NN so we can initialise the GA
	int m_NumWeightsInNN = mNeuralNetwork->GetNumberOfWeights();

	//initialize the Genetic Algorithm class
	mGeneticAlgorithm = new GeneticAlgorithm(kPopSize, kRainbowMutationRate, kRainbowCrossoverRate, m_NumWeightsInNN);

	//Get the weights from the GA and insert into Bub's brains
	m_vecSetOfWeights = mGeneticAlgorithm->GetChromos();
	mNeuralNetwork->PutWeights(mGeneticAlgorithm->GetChromos()[currentPopulationCount].vWeights);

	//int maxScore = mFruit.size() * 10 + mEnemies.size() * 10;
	//cout << "MAX: " << maxScore << endl;

	cout << "Current Weight Set: " << currentPopulationCount << endl;
	//for each (double var in m_vecSetOfWeights[currentPopulationCount].vWeights)
	//{
	//	cout << var << " | ";
	//}
	//cout << endl;


	//SaveWeightsToFile();
	if (!ReadWeightsFromFile()) {
		cout << "ERROR OCCURED LOADING WEIGHTS" << endl;
		mNeuralNetwork->PutWeights(m_vecSetOfWeights[currentPopulationCount].vWeights);
	}



}

//--------------------------------------------------------------------------------------------------

GameScreen_RainbowIslands::~GameScreen_RainbowIslands()
{
	//Background image.
	delete mBackgroundTexture;
	mBackgroundTexture = NULL;

	//Player character.
	delete mBubCharacter;
	mBubCharacter = NULL;

	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Treasure chest.
	delete mChest;
	mChest = NULL;

	//Enemies.
	mEnemies.clear();

	//Fruit.
	mFruit.clear();

	//Rainbows.
	mRainbows.clear();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::Render()
{
	//Draw the background.
	RenderBackground();

	//Draw the chest.
	if (mChest != NULL)
		mChest->Render();

	//Draw the Fruit.
	for (unsigned int i = 0; i < mFruit.size(); i++)
	{
		mFruit[i]->Render();
		if(closestFruit != nullptr)
			DrawDebugCircle(closestFruit->GetCentralPosition(), 25, 255, 0, 0);
	}

	//Draw the Enemies.
	for(unsigned int i = 0; i < mEnemies.size(); i++)
	{
		mEnemies[i]->Render();
		if (closestEnemy != nullptr)
			DrawDebugCircle(closestEnemy->GetCentralPosition(), 10, 255, 0, 0);

		DrawDebugCircle(mEnemies[i]->GetCentralPosition(), mEnemies[i]->GetCollisionRadius(), 255, 0, 0);
	}

	//Draw the Rainbows.
	for (unsigned int i = 0; i < mRainbows.size(); i++)
	{
		mRainbows[i]->Render();
		//if(mRainbows[i]->CanKill())
		if(mRainbows[i]->GetCentralPosition().Distance(mBubCharacter->GetCentralPosition()) < 28) 
			DrawDebugCircle(Vector2D(mRainbows[i]->GetCentralPosition().x, mRainbows[i]->GetCentralPosition().y + 12), mRainbows[i]->GetCollisionRadius() + 20, 255, 1, 1);
		else
			DrawDebugCircle(Vector2D(mRainbows[i]->GetCentralPosition().x, mRainbows[i]->GetCentralPosition().y + 12), mRainbows[i]->GetCollisionRadius() + 20, 1, 1, 255);
	}

	//Draw the player.
	mBubCharacter->Render();
	DrawDebugCircle(mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius(), 0, 255, 0);

	if (mBubCharacter->surroundings.size() > 35)
		cout << "ERROR OCCURED: INCORRECT SURROUNDINGS" << endl;

	for (unsigned int i = 0; i < mBubCharacter->surroundingPositions.size(); i++)
	{

		if (mBubCharacter->surroundings[i] == 0) 
		{
			DrawDebugCircle(mBubCharacter->surroundingPositions[i], 10, 0, 255, 0);
		} 
		else 
		{
			DrawDebugCircle(mBubCharacter->surroundingPositions[i], 10, 255, 0, 0);
		}

		if (closestEnemy != nullptr)
		{
			if (closestEnemy->GetCentralPosition().Distance(mBubCharacter->surroundingPositions[i]) < 20)
			{
				DrawDebugCircle(mBubCharacter->surroundingPositions[i], 10, 255, 50, 50);
			}
		}

	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::RenderBackground()
{
	if (mLevelMap != NULL)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			for (int y = 0; y < MAP_HEIGHT; y++)
			{
				//Get the portion of the spritesheet you want to draw.
				int num = mLevelMap->GetBackgroundTileAt(y, x);
				int w = TILE_WIDTH*(num % kTileSpriteSheetWidth);
				int h = TILE_HEIGHT*(num / kTileSpriteSheetWidth);
				//								{XPos, YPos, WidthOfSingleSprite, HeightOfSingleSprite}
				SDL_Rect portionOfSpritesheet = { w, h, TILE_WIDTH, TILE_HEIGHT };

				//Determine where you want it drawn.
				SDL_Rect destRect = { x*TILE_WIDTH, kRainbowIslandsScreenHeight-((MAP_HEIGHT-y)*TILE_HEIGHT), TILE_WIDTH, TILE_HEIGHT };

				//Then draw it.
				mBackgroundTexture->Render(portionOfSpritesheet, destRect, SDL_FLIP_NONE);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::Update(float deltaTime, SDL_Event e)
{
	mGenAlgUpdateTimer += deltaTime;

	NeuraliseTheNetworkOfNeural();
	
	//Score Bedug
	//float pointMult = 1, timeMult = 0.001, heightMult = 1000;
	//float points = mBubCharacter->GetPoints() * pointMult;
	//float time = mGenAlgUpdateTimer * timeMult;
	//float height = (1 - (mBubCharacter->GetCentralPosition().y / kRainbowIslandsScreenHeight)) * heightMult;
	//cout << "Score : " << points + time + height << endl;

	//--------------------------------------------------------------------------------------------------
	//Update the Virtual Joypad.
	//--------------------------------------------------------------------------------------------------
	VirtualJoypad::Instance()->SetJoypadState(e);

	//--------------------------------------------------------------------------------------------------
	//Update the level time.
	//--------------------------------------------------------------------------------------------------
	mTimeToCompleteLevel -= (int)deltaTime;
	if (mTimeToCompleteLevel < TRIGGER_ANGRY_TIME && !mTriggeredAnger)
	{
		MakeEnemiesAngry();
		if (mTimeToCompleteLevel <= 0)
		{
			//Moved to Neural
			//NeuraliseTheNetworkOfNeural();
			//RestartLevel();
		}
	}

	//--------------------------------------------------------------------------------------------------
	//Update the player.
	//--------------------------------------------------------------------------------------------------
	mBubCharacter->Update(deltaTime, e);
	if (!mBubCharacter->GetAlive())
		RestartLevel();
	else if (mBubCharacter->GetPosition().y < 60)
		CreateChest(Vector2D(kRainbowIslandsScreenWidth*0.25f, -50.0f));


	//--------------------------------------------------------------------------------------------------
	//Update the game objects.
	//--------------------------------------------------------------------------------------------------
	UpdateFruit(deltaTime, e);
	UpdateEnemies(deltaTime, e);
	UpdateRainbows(deltaTime, e);

	//We have a chest, but is it open yet.
	if (mChest != NULL)
	{
		mChest->Update(deltaTime, e);
		if (mChest->IsChestOpen() && !mTriggeredChestSpawns)
		{
			TriggerChestSpawns();
		}
	}

	//--------------------------------------------------------------------------------------------------
	//Check if we need to create a new rainbow.
	//--------------------------------------------------------------------------------------------------
	if (VirtualJoypad::Instance()->DownArrow && mCanSpawnRainbow)
	{
		bool collidingWithRainbow = false;
		
		for (unsigned int i = 0; i < mRainbows.size(); i++)
		{
			if (Collisions::Instance()->Box(mBubCharacter->GetCollisionBox(), mRainbows[i]->GetCollisionBox()))
			{
				collidingWithRainbow = true;
				break;
			}
		}

		if (!collidingWithRainbow)
		{
			Vector2D pos = mBubCharacter->GetPosition();
			pos.x += 10;
			if (mBubCharacter->GetFacing() == FACING_RIGHT)
				pos.x += mBubCharacter->GetCollisionBox().width-15;
			pos.y -= mBubCharacter->GetCollisionBox().height*0.3f;
			CreateRainbow(pos, mBubCharacter->GetRainbowsAllowed());

			mCanSpawnRainbow = false;
		}
	}
	else if (!VirtualJoypad::Instance()->DownArrow)
	{
		mCanSpawnRainbow = true;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::MakeEnemiesAngry()
{
	if (!mEnemies.empty())
	{
		int enemyIndexToDelete = -1;
		for (unsigned int i = 0; i < mEnemies.size(); i++)
		{
			mEnemies[i]->MakeAngry();
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateEnemies(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Enemies.
	//--------------------------------------------------------------------------------------------------
	if(!mEnemies.empty())
	{
		int enemyIndexToDelete = -1;
		for(unsigned int i = 0; i < mEnemies.size(); i++)
		{
			mEnemies[i]->Update(deltaTime, e);

			//Check to see if the enemy collides with the player.
			if (Collisions::Instance()->Circle(mEnemies[i]->GetCentralPosition(), mEnemies[i]->GetCollisionRadius(), mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius()))
			{
				mBubCharacter->SetState(CHARACTERSTATE_PLAYER_DEATH);
			}

			//If the enemy is no longer alive, then schedule it for deletion.
			if(!mEnemies[i]->GetAlive())
			{
				enemyIndexToDelete = i;
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead enemies - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if(enemyIndexToDelete != -1)
		{
			Character* toDelete = mEnemies[enemyIndexToDelete];

			//We need a pickup to be generated.
			CreateFruit(toDelete->GetPosition(), true);

			mEnemies.erase(mEnemies.begin()+enemyIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateFruit(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Fruit.
	//--------------------------------------------------------------------------------------------------
	if(!mFruit.empty())
	{
		int fruitIndexToDelete = -1;
		for(unsigned int i = 0; i < mFruit.size(); i++)
		{
			//Update the fruit.
			mFruit[i]->Update(deltaTime, e);

			//check if the player has collided with it.
			if (Collisions::Instance()->Circle(mFruit[i]->GetCentralPosition(), mFruit[i]->GetCollisionRadius(), mBubCharacter->GetCentralPosition(), mBubCharacter->GetCollisionRadius()))
			{
				mFruit[i]->SetAlive(false);
				fruitIndexToDelete = i;
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead fruit - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if(fruitIndexToDelete != -1)
		{
			mBubCharacter->AddPoints();

			Character* toDelete = mFruit[fruitIndexToDelete];
			mFruit.erase(mFruit.begin() + fruitIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::UpdateRainbows(float deltaTime, SDL_Event e)
{
	//--------------------------------------------------------------------------------------------------
	//Update the Rainbows.
	//--------------------------------------------------------------------------------------------------
	
	//Need to turn flag off each frame incase the rainbow disapated.
	mBubCharacter->SetOnARainbow(false);

	if (!mRainbows.empty())
	{
		int rainbowIndexToDelete = -1;

		for (unsigned int i = 0; i < mRainbows.size(); i++)
		{
			int xPosition = (int)mBubCharacter->GetPosition().x + (int)(mBubCharacter->GetCollisionBox().width*0.5f);
			int footPosition = (int)(mBubCharacter->GetPosition().y + mBubCharacter->GetCollisionBox().height);

			//Update the rainbow.
			mRainbows[i]->Update(deltaTime, e);

			if (!mRainbows[i]->GetAlive())
				rainbowIndexToDelete = i;
			else
			{
				//check if the player has collided with it.
				if (!mBubCharacter->IsJumping())
				{
					if (Collisions::Instance()->PointInBox(Vector2D(xPosition, footPosition), mRainbows[i]->GetCollisionBox()))
					{
						mBubCharacter->SetState(CHARACTERSTATE_WALK);
						mBubCharacter->SetOnARainbow(true);
						int xPointOfCollision = (int)(mRainbows[i]->GetPosition().x + mRainbows[i]->GetCollisionBox().width - xPosition);
						if (mBubCharacter->GetFacing() == FACING_RIGHT)
							xPointOfCollision = (int)(xPosition - mRainbows[i]->GetPosition().x);

						//We don't want to pop between walking on different rainbows. Ensure the switch between rainbows looks 'realistic'
						double distanceBetweenPoints = footPosition - (mRainbows[i]->GetPosition().y - RainbowOffsets[xPointOfCollision]);
						if(distanceBetweenPoints < 40.0)
							mBubCharacter->SetPosition(Vector2D(mBubCharacter->GetPosition().x, mRainbows[i]->GetPosition().y - RainbowOffsets[xPointOfCollision]));
					}
				}

				//Check for collisions with enemies.
				for (unsigned int j = 0; j < mEnemies.size(); j++)
				{
					if (mRainbows[i]->CanKill())
					{
						if (Collisions::Instance()->Circle(mRainbows[i]->GetStrikePosition(), mRainbows[i]->GetCollisionRadius(), mEnemies[j]->GetPosition(), mEnemies[j]->GetCollisionRadius()))
						{
							mEnemies[j]->SetAlive(false);
						}
					}
				}
			}
		}

		//--------------------------------------------------------------------------------------------------
		//Remove a dead rainbow - 1 each update.
		//--------------------------------------------------------------------------------------------------
		if (rainbowIndexToDelete != -1)
		{
			Character* toDelete = mRainbows[rainbowIndexToDelete];
			mRainbows.erase(mRainbows.begin() + rainbowIndexToDelete);
			delete toDelete;
			toDelete = NULL;
		}
	}
}

//--------------------------------------------------------------------------------------------------

bool GameScreen_RainbowIslands::SetUpLevel()
{
	//Load the background texture.
	mBackgroundTexture = new Texture2D(mRenderer);
	if( !mBackgroundTexture->LoadFromFile("Images/RainbowIslands/Tiles.png"))
	{
		cout << "Failed to load background texture!";
		return false;
	}

	//Create the level map.
	SetLevelMap();

	CreateStartingCharacters();

	mTimeToCompleteLevel = LEVEL_TIME;
	mTriggeredAnger		 = false;

	return true;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::RestartLevel()
{
	//TRAIN NN
	float pointMult = 1, timeMult = 0.001, heightMult = 1000;
	float points = mBubCharacter->GetPoints() * pointMult;
	float time = mGenAlgUpdateTimer * timeMult;
	float height = min((1 - (mBubCharacter->GetCentralPosition().y / kRainbowIslandsScreenHeight)) * heightMult, 1000);
	m_vecSetOfWeights[currentPopulationCount].dFitness = points + time + height - rainbowsFired;
	cout << "Score : " << m_vecSetOfWeights[currentPopulationCount].dFitness << endl;

	currentPopulationCount++;
	mGenAlgUpdateTimer = 0;
	rainbowsFired = 0;
	//BACK PROPOGATION


	if (currentPopulationCount == (int)kPopSize)
	{
		mGenerationCount++;

		cout << kPopSize << " Sets of Chromosomes have been created. Selection/Crossover/Mutation begins." << endl;
		cout << "Current Generation : " << mGenerationCount << endl;

		currentPopulationCount = 0;

		m_vecSetOfWeights = mGeneticAlgorithm->Epoch(m_vecSetOfWeights);

		mNeuralNetwork->PutWeights(mGeneticAlgorithm->GetChromos()[currentPopulationCount].vWeights);

		//update the stats to be used in our stat window -- CAN BE REMOVED
		m_vecAvFitness.push_back(mGeneticAlgorithm->AverageFitness());
		m_vecBestFitness.push_back(mGeneticAlgorithm->BestFitness());

		//incr the gens

		//reset cycles

		//run the GA to create a new population

		//insert the new (hopefully)improved brains back into the sweepers
		//and reset their positions etc

		//save to file
		SaveWeightsToFile();
	}

	mNeuralNetwork->PutWeights(m_vecSetOfWeights[currentPopulationCount].vWeights);

	cout << "Current Weight Set: " << currentPopulationCount << endl;
	//
	//for each (double var in m_vecSetOfWeights[currentPopulationCount].vWeights)
	//{
	//	cout << var << " | ";
	//}
	//cout << endl;

	//Clean up current characters.
	//Player character.
	mBubCharacter->~CharacterBub();
	delete mBubCharacter;
	mBubCharacter = NULL;

	//Level map.
	delete mLevelMap;
	mLevelMap = NULL;

	//Enemies.
	for each (Character* var in mEnemies)
	{
		var->~Character();
		delete var;
	}
	mEnemies.clear();

	//Fruit.
	for each (Character* var in mFruit)
	{
		var->~Character();
		delete var;
	}
	mFruit.clear();

	//Rainbows.
	mRainbows.clear();

	//Treasure chest.
	if (mChest != NULL)
	{
		delete mChest;
		mChest = NULL;
	}

	//Create the level map.
	SetLevelMap();

	//Respawn characters and map.
	CreateStartingCharacters();

	mTimeToCompleteLevel = LEVEL_TIME;
	mTriggeredAnger		 = false;

	closestFruit = nullptr;
	closestEnemy = nullptr;
	
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateStartingCharacters()
{
	//Set up the player character.
	mBubCharacter = new CharacterBub(mRenderer, "Images/RainbowIslands/bub.png", mLevelMap, Vector2D(100, 570));
	mCanSpawnRainbow = true;

	//Set up the bad guys.
	CreateCaterpillar(Vector2D(150, 500), FACING_LEFT);
	CreateCaterpillar(Vector2D(130, 403), FACING_LEFT);
	CreateCaterpillar(Vector2D(270, 450), FACING_LEFT);
	CreateCaterpillar(Vector2D(30, 350), FACING_RIGHT);
	CreateCaterpillar(Vector2D(30, 160), FACING_RIGHT);
	CreateCaterpillar(Vector2D(270, 160), FACING_LEFT);
	CreateCaterpillar(Vector2D(250, 227), FACING_RIGHT);

	//Setup starting pickups.
	CreateFruit(Vector2D(26, 575), false);
	CreateFruit(Vector2D(46, 575), false);
	CreateFruit(Vector2D(256, 575), false);
	CreateFruit(Vector2D(276, 575), false);
	CreateFruit(Vector2D(26, 544), false);
	CreateFruit(Vector2D(256, 544), false);
	CreateFruit(Vector2D(300, 544), false);
	CreateFruit(Vector2D(100, 496), false);
	CreateFruit(Vector2D(200, 496), false);
	CreateFruit(Vector2D(256, 450), false);
	CreateFruit(Vector2D(246, 227), false);
	CreateFruit(Vector2D(276, 227), false);
	CreateFruit(Vector2D(8, 160), false);
	CreateFruit(Vector2D(32, 160), false);
	CreateFruit(Vector2D(56, 160), false);
	CreateFruit(Vector2D(270, 160), false);
	CreateFruit(Vector2D(140, 177), false);
	CreateFruit(Vector2D(83, 112), false);
	CreateFruit(Vector2D(103, 112), false);
	CreateFruit(Vector2D(123, 112), false);

	mChest = NULL;
	mTriggeredChestSpawns = false;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::SetLevelMap()
{
	int backgroundMap[MAP_HEIGHT][MAP_WIDTH] = {
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,30,38,31},
	{7,7,7,7,8,5,5,6,7,7,7,7,7,7,7,8,5,5,5,5},
	{12,12,12,12,13,4,4,11,12,12,12,12,12,12,12,13,4,4,4,4},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,27,28,29,5,5,5,5,5,5,5,5,5,5,25,26,5,5,5},
	{5,5,32,33,34,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,25,26,5,5,5,5,5,5,5},
	{22,5,14,14,5,5,5,5,5,5,5,5,5,5,5,5,14,5,21,22},
	{4,4,4,4,4,5,14,14,14,14,5,5,5,5,5,5,4,4,4,4},
	{5,5,5,5,5,5,4,4,4,4,4,5,5,5,5,5,5,5,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,27,28,29,5,30,37,31},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,32,33,34,14,5,14,14},
	{4,4,4,5,5,14,14,25,26,5,5,5,5,5,5,4,4,4,4,4},
	{5,5,5,5,5,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{5,25,26,5,5,5,5,14,14,14,14,14,6,7,7,7,7,8,5,5},
	{5,5,5,5,4,4,4,4,4,4,4,4,11,12,12,12,12,13,4,4},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{4,4,4,5,5,5,19,5,19,5,25,26,5,5,5,5,5,5,5,25},
	{5,5,5,5,5,17,18,17,18,5,5,5,5,5,5,5,5,5,5,5},
	{21,22,21,22,5,15,16,15,16,5,5,5,5,19,5,5,5,30,36,31},
	{0,0,0,1,0,0,1,0,0,0,0,1,0,0,5,5,5,19,5,5},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0},
	{26,5,5,5,5,5,14,14,14,14,14,14,14,14,14,5,5,5,5,5},
	{5,5,5,19,0,0,0,0,0,0,0,0,0,0,0,5,5,5,5,19},
	{5,5,17,18,5,5,5,5,5,5,5,5,5,5,5,5,5,5,17,18},
	{14,14,15,16,5,5,5,5,5,5,5,5,27,28,29,5,14,14,15,16},
	{0,1,0,1,5,5,25,26,5,5,5,5,32,33,34,5,1,0,1,0},
	{5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5},
	{28,29,5,5,5,14,14,5,5,14,14,5,5,14,14,5,5,5,5,25},
	{33,34,19,5,5,0,0,1,0,0,0,0,1,0,0,5,5,5,5,5},
	{5,17,18,5,5,5,5,5,5,5,5,5,5,5,5,5,5,30,35,31},
	{14,15,16,5,27,28,29,5,19,5,5,5,19,5,5,5,5,5,5,5},
	{2,1,0,0,32,33,34,17,18,5,5,17,18,5,5,5,0,0,1,23},
	{24,24,24,9,9,9,5,15,16,5,5,15,16,5,9,9,9,24,24,24},
	{2,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,0,0,23},
	{20,20,20,20,20,20,10,10,10,20,20,10,10,10,20,20,20,20,20,20}};

	int collisionMap[MAP_HEIGHT][MAP_WIDTH] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1 },
		{ 0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 } };

	//Clear up any old map.
	if(mLevelMap != NULL)
	{
		delete mLevelMap;
	}

	//Set the new one.
	mLevelMap = new LevelMap(collisionMap, backgroundMap);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateCaterpillar(Vector2D position, FACING direction)
{
	CharacterCaterpillar* caterpillarCharacter = new CharacterCaterpillar(mRenderer, "Images/RainbowIslands/Caterpillar.png", mLevelMap, position, direction);
	Character* tempCharacter = (Character*)caterpillarCharacter;
	mEnemies.push_back(tempCharacter);
	caterpillarCharacter = NULL;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateRainbow(Vector2D position, int numberOfRainbows)
{
	rainbowsFired++;
	double xOffset   = 0.0;
	float  spwnDelay = 0.0f;
	do
	{
		if(mBubCharacter->GetFacing() == FACING_LEFT)
			position.x -= xOffset;
		else
			position.x += xOffset;
		
		CharacterRainbow* rainbowCharacter = new CharacterRainbow(mRenderer, "Images/RainbowIslands/Rainbow.png", position, mBubCharacter->GetFacing(), spwnDelay);
		if (rainbowCharacter->GetFacing() == FACING_LEFT)
			rainbowCharacter->SetPosition(Vector2D(rainbowCharacter->GetPosition().x - rainbowCharacter->GetCollisionBox().width, rainbowCharacter->GetPosition().y));
		
		mRainbows.push_back(rainbowCharacter);
		numberOfRainbows--;
		xOffset = rainbowCharacter->GetCollisionBox().width-10.0;
		spwnDelay += 200;
	} while (numberOfRainbows > 0);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateFruit(Vector2D position, bool bounce)
{
	CharacterFruit* fruitCharacter = new CharacterFruit(mRenderer, "Images/RainbowIslands/Pickups.png", mLevelMap, position, (FRUIT_TYPE)(rand()%FRUIT_MAX));
	mFruit.push_back(fruitCharacter);

	if (bounce)
		fruitCharacter->Jump();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::CreateChest(Vector2D position)
{
	if (mChest == NULL)
	{
		mChest = new CharacterChest(mRenderer, "Images/RainbowIslands/Chest.png", mLevelMap, position);
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::TriggerChestSpawns()
{
	Vector2D pos		= mChest->GetCentralPosition();
	int xRange			= (int)mChest->GetCollisionBox().width;
	int xDeductRange	= (int)mChest->GetCollisionBox().width/2;

	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);
	CreateFruit(Vector2D(pos.x + (rand() % xRange) - xDeductRange, pos.y), true);

	mTriggeredChestSpawns = true;
}

//--------------------------------------------------------------------------------------------------
//Debug output - Use to determine where you are checking
//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::DrawDebugCircle(Vector2D centrePoint, double radius, int red, int green, int blue)
{
	Vector2D polarVec(0.0f, radius);

	float stepSize = 0.02f;
	float _360DegAsRads = (float)DegsToRads(360.0f);

	while (polarVec.x < _360DegAsRads)
	{
		Vector2D polarAsCart(polarVec.y * cos(polarVec.x), polarVec.y * sin(polarVec.x));
		Vector2D drawPoint(centrePoint.x + polarAsCart.x, centrePoint.y + polarAsCart.y);

		SDL_SetRenderDrawColor(mRenderer, red, green, blue, 255);
		SDL_RenderDrawPoint(mRenderer, (int)drawPoint.x, (int)drawPoint.y);

		polarVec.x += stepSize;
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::DrawDebugLine(Vector2D startPoint, Vector2D endPoint, int red, int green, int blue)
{
	SDL_SetRenderDrawColor(mRenderer, red, green, blue, 255);
	SDL_RenderDrawLine(mRenderer, (int)startPoint.x, (int)startPoint.y, (int)endPoint.x, (int)endPoint.y);
}

//--------------------------------------------------------------------------------------------------

void GameScreen_RainbowIslands::NeuraliseTheNetworkOfNeural()
{
	/*
	 genrations++;
	 reset tickTimer;
	 Run GA->Epoch(theData)
	 
	 for each
		put weights
		reset
	*/
	vector<double> inputs;

	closestFruit = FindClosestFruit(); //pointer
	closestEnemy = FindClosestEnemy(); //pointer

	// turns into a percentage of the screen height
	double toTheNearestFruit = (mBubCharacter->GetCentralPosition() - closestFruit->GetCentralPosition()).Length();
	toTheNearestFruit /= kRainbowIslandsScreenHeight;
	toTheNearestFruit = 1 - toTheNearestFruit;

	//go left
	if (closestFruit->GetCentralPosition().x < mBubCharacter->GetCentralPosition().x)
		inputs.push_back(1);
	else
		inputs.push_back(0);

	//go right
	if (closestFruit->GetCentralPosition().x > mBubCharacter->GetCentralPosition().x)
		inputs.push_back(1);
	else
		inputs.push_back(0);

	// turns into a percentage of the screen height
	double toTheNearestEnemy = (mBubCharacter->GetCentralPosition() - closestEnemy->GetCentralPosition()).Length();
	toTheNearestEnemy /= kRainbowIslandsScreenHeight; 

	//////////
	/*
	Inputs

	Left?
	Right?
	Distance from the top
	Distance to the nearest fruit
	Distance to the nearest enemy
	Surrounding tiles (5*7=35)
	onRainbow

	= 41
	*/
	//////////

	inputs.push_back(1 - (mBubCharacter->GetCentralPosition().y / kRainbowIslandsScreenHeight));

	inputs.push_back(toTheNearestFruit);
	inputs.push_back(toTheNearestEnemy);

	vector<double> surroundings = mBubCharacter->GetSurroundings(closestEnemy->GetCentralPosition(), mRainbows);
	inputs.insert(inputs.end(), surroundings.begin(), surroundings.end());

	inputs.push_back(mBubCharacter->OnARainbow());
	vector<double> inputsBackup = inputs;

	vector<double> output = mNeuralNetwork->Update(inputs);

	//////////
	/*
	Fitness
	1. Score
	
	Back Propogation
	1. Error = Max Score - Score
	2. Use the formula
	*/
	//////////


	///////////
	// Debug //
	///////////
	/*
	system("cls");
	cout << "Current Weight Set: " << currentPopulationCount << endl;
	for each (double var in m_vecSetOfWeights[currentPopulationCount].vWeights)
	{
		cout << var << " | ";
	}
	cout << endl;
	*/

	//////////
	/*
	Flow

	1.Run the game for 60 seconds using the neural network for t seconds,
	2.Run this x times, altering the weights with back propgation after each run
	3.After x times, run the GA using the fitness alg to select, crossover and mutate the x sets of weights, and create x new weights
	4.Rinse and repeat, lowering the mutatation value as usual with GAs
	*/
	//////////

	if (output.size() < kNumOutputs)
	{
		cout << "Error: Incorrect number of outputs" << endl;
	}

	if (output[0] > 0.80)
		VirtualJoypad::Instance()->LeftArrow = true;
	else
		VirtualJoypad::Instance()->LeftArrow = false;
	if (output[1] > 0.85)
		VirtualJoypad::Instance()->DownArrow = true;
	else
		VirtualJoypad::Instance()->DownArrow = false;
	if (output[2] > 0.80)
		VirtualJoypad::Instance()->RightArrow = true;
	else
		VirtualJoypad::Instance()->RightArrow = false;
	if (output[3] > 0.80)
		VirtualJoypad::Instance()->UpArrow = true;
	else
		VirtualJoypad::Instance()->UpArrow = false;


	//cout << "UP:" << VirtualJoypad::Instance()->UpArrow << endl;
	//cout << "DOWN:" << VirtualJoypad::Instance()->DownArrow << endl;
	//cout << "LEFT:" << VirtualJoypad::Instance()->LeftArrow << endl;
	//cout << "RIGHT:" << VirtualJoypad::Instance()->RightArrow << endl;


	if (VirtualJoypad::Instance()->ForceRestartLevel == true)
	{
		VirtualJoypad::Instance()->ForceRestartLevel = false;
		//mNeuralNetwork->NetworkTrainingEpoch(inputsBackup, output, mBubCharacter->GetPoints(), 410);
		RestartLevel();
	}

	if (mTimeToCompleteLevel <= 0 || mGenAlgUpdateTimer > mGenAlgUpdateTime)
	{
		//Back Prop
		//mNeuralNetwork->NetworkTrainingEpoch(inputsBackup, output, mBubCharacter->GetPoints(), 410);
		RestartLevel();
	}

}

CharacterFruit* GameScreen_RainbowIslands::FindClosestFruit()
{
	Vector2D charPos = mBubCharacter->GetCentralPosition();
	double closestFruitDist = MaxDouble;
	double current = 0;
	CharacterFruit* closestFruit;

	for each (CharacterFruit* var in mFruit)
	{
		//if (var->GetCentralPosition().y < mBubCharacter->GetCentralPosition().y + kTileSpriteSheetHeight)
		{
			current = Vec2DDistance(var->GetCentralPosition(), charPos);
			if (current < closestFruitDist)
			{
				closestFruitDist = current;
				closestFruit = var;
			}
		}
	}

	return closestFruit;
}

Character* GameScreen_RainbowIslands::FindClosestEnemy()
{
	Vector2D charPos = mBubCharacter->GetCentralPosition();
	double closestEnemyDist = MaxDouble;
	double current = 0;
	Character* closestEnemy;

	for each (Character* var in mEnemies)
	{
		current = Vec2DDistance(var->GetCentralPosition(), charPos);
		if (current < closestEnemyDist)
		{
			closestEnemyDist = current;
			closestEnemy = var;
		}
	}

	return closestEnemy;
}

void GameScreen_RainbowIslands::SaveWeightsToFile()
{
	std::fstream fs;
	fs.open("Weights.txt", std::fstream::in | std::fstream::out);

	for (int i = 0; i < kPopSize; i++)
	{
		for each (double vecOfWeights in m_vecSetOfWeights[i].vWeights)
		{
			fs << vecOfWeights << endl;
		}
	}

	fs.close();
}

bool GameScreen_RainbowIslands::ReadWeightsFromFile()
{
	std::fstream fs;
	fs.open("Weights.txt", std::fstream::in | std::fstream::out);

	for (int i = 0; i < kPopSize; i++)
	{
		m_vecSetOfWeights[i].vWeights;
		vector<double> loadedWeights;
		for (size_t j = 0; j < mNeuralNetwork->GetNumberOfWeights(); j++)
		{
			string input;
			fs >> input;
			loadedWeights.push_back(stod(input));
		}
		m_vecSetOfWeights[i].vWeights = loadedWeights;
	}

	fs.close();
	return true;
}


