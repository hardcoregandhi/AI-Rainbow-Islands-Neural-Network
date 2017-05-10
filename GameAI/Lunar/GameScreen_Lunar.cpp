//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "GameScreen_Lunar.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "LunarConstants.h"
#include "LunarLander.h"
#include <time.h>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <random>


//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::GameScreen_Lunar(SDL_Renderer* renderer) : GameScreen(renderer)
{
	srand(static_cast <unsigned> (time(0)));

	//Set up the platform to land on.
	mPlatform = new Texture2D(renderer);
	mPlatform->LoadFromFile("Images/Lunar/Platform.png");

	//Create a lunar lander to use for tests.
	mPlatformPosition = Vector2D(198,430);
	mLunarLander = new LunarLander(renderer, Vector2D(400,125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));

	//-------------------------------------
	//AI Controlled Lunar Lander setup.
	mGeneration = 0;
	GenerateRandomChromosomes();
	for(int i = 0; i < kNumberOfAILanders; i++)
		mAILanders[i] = new LunarLander(renderer, Vector2D(400,125), Rect2D(mPlatformPosition.x, mPlatformPosition.y, mPlatform->GetWidth(), mPlatform->GetHeight()));
	RestartGA();
	mAllowMutation = true;
	//-------------------------------------

	//Start game in frozen state.
	mPause = true;

	//Start game in player control.
	mAIOn = false;

	//My Constructor
	mGenerationDeltaTime = 0;
	mCurrentAction = 0;
	mAIOn = true;
	mPause = false;
	myMutationRate = 500;
}

//--------------------------------------------------------------------------------------------------

GameScreen_Lunar::~GameScreen_Lunar()
{
	delete mPlatform;
	mPlatform = NULL;

	delete mLunarLander;
	mLunarLander = NULL;

	//-------------------------------------
	//TODO: Take out for students version.
	for(int i = 0; i < kNumberOfAILanders; i++)
		delete *mAILanders;
	//-------------------------------------
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Render()
{
	mPlatform->Render(mPlatformPosition);

	if(mAIOn)
	{
		for(int i = 0; i < kNumberOfAILanders; i++)
			mAILanders[i]->Render();
	}
	else
		mLunarLander->Render();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Update(float deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch (e.type)
	{
		//Deal with mouse click input.
	case SDL_KEYUP:
		switch (e.key.keysym.sym)
		{
		case SDLK_SPACE:
			mPause = !mPause;
			if (mPause)
				cout << "Paused" << endl;
			else
				cout << "Unpaused" << endl;
			break;

		case SDLK_a:
			mAIOn = !mAIOn;
			if (mAIOn)
				cout << "AI on" << endl;
			else
				cout << "AI off" << endl;
			break;

		case SDLK_LEFT:
			if (!mAIOn)
				mLunarLander->TurnLeft(deltaTime);
			break;

		case SDLK_RIGHT:
			if (!mAIOn)
				mLunarLander->TurnRight(deltaTime);
			break;

		case SDLK_UP:
			if (!mAIOn)
				mLunarLander->Thrust();
			break;

		case SDLK_r:
			if (!mAIOn)
				mLunarLander->Reset();
			break;
		}
		break;

	default:
		break;
	}

	if (!mPause)
	{
		if (!mAIOn)
		{
			mLunarLander->Update(deltaTime, e);

			double speed = mLunarLander->GetSpeed();
			double RotFitness = 0;
			double DistFitness = 0;
			double SurvFitness = 0;
			double LandedBonus = 0;
			double fitness = 0;

			DistFitness = ((kLunarScreenWidth - fabs(mPlatformPosition.x - mLunarLander->GetPosition().x)) +
				(kLunarScreenHeight - (mPlatformPosition.y - mLunarLander->GetPosition().y)));

			RotFitness = mLunarLander->GetRotationAngle();

			SurvFitness = mLunarLander->GetSurvivalTime() / (mLunarLander->GetSpeed() + 1);

			/*		if (mLunarLander->HasLanded())
			fitness = (MaxDouble / 2) + DistFitness + (400 * RotFitness) + (4 * SurvFitness);//half allows for improvement in the successful landers
			else
			fitness = DistFitness + (400 * RotFitness) + (4 * SurvFitness);*/

			if (mLunarLander->HasLanded())
				fitness = kLandingBonus;
			else
				fitness = (kDistWeight*DistFitness) + (RotFitness*kRotWeight) + (SurvFitness*kAirTimeWeight);


			//cout << "mPlatformPosition.x = " << mPlatformPosition.x << endl;
			//cout << "mLunarLander->GetPosition().x = " << mLunarLander->GetPosition().x << endl;
			//cout << "fabs(mPlatformPosition.x - mLunarLander->GetPosition().x) = " << fabs(mPlatformPosition.x - mLunarLander->GetPosition().x) << endl;
			//cout << "kLunarScreenWidth - (mPlatformPosition.x - mLunarLander->GetPosition().x) = " << kLunarScreenWidth - fabs(mPlatformPosition.x - mLunarLander->GetPosition().x) << endl;

			cout << kDistWeight * DistFitness << "	 ";
			cout << kRotWeight * RotFitness << "   	";
			cout << kAirTimeWeight * SurvFitness << endl;
			cout << fitness << endl << endl;
		}
		else
		if (mAIOn)
			UpdateAILanders(deltaTime, e);
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::GenerateRandomChromosomes()
{
	int randomNumber;

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			randomNumber = (rand() % 4);

			//mChromosomes[i][j] = static_cast<LunarAction>(randomNumber);

			switch (randomNumber)
			{
				case 0:
					mChromosomes[i][j] = LunarAction_None;
					break;
				case 1:
					mChromosomes[i][j] = LunarAction_LunarThrust;
					break;
				case 2:
					mChromosomes[i][j] = LunarAction_LunarLeft;
					break;
				case 3:
					mChromosomes[i][j] = LunarAction_LunarRight;
					break;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::ClearChromosomes()
{
	delete mChromosomes;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::UpdateAILanders(float deltaTime, SDL_Event e)
{
	mAccumulatedDeltaTime += deltaTime;

	bool newAction = false;
	if (mAccumulatedDeltaTime >= 200.0f)
	{
		mAccumulatedDeltaTime = 0.0f;
		
		//cout << " " << mCurrentAction << endl;
		newAction = true;

		//Start the for loop
		for (int i = 0; i < kNumberOfAILanders; i++)
		{
			if (mAILanders[i]->IsAlive() && !mAILanders[i]->HasLanded())
			{
				switch (mChromosomes[i][mCurrentAction])
				{
				case LunarAction_LunarThrust:
					mAILanders[i]->Thrust();
					break;
				case LunarAction_LunarLeft:
					mAILanders[i]->TurnLeft(deltaTime);
					break;
				case LunarAction_LunarRight:
					mAILanders[i]->TurnRight(deltaTime);
					break;
				case LunarAction_None:
					break;
				}
			}
		}

		mCurrentAction++;
	}

	//But still always update the ships
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		mAILanders[i]->Update(deltaTime, e);
	}

	//DEATH CHECKS
	bool AllLandersDeadOrLanded = true;

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		if (mAILanders[i]->IsAlive() && !mAILanders[i]->HasLanded())
		{

			AllLandersDeadOrLanded = false;
			break;
		}
	}

	if (AllLandersDeadOrLanded)
	{
		for (int i = 0; i < kNumberOfAILanders; i++)
		{
			if (mAILanders[i]->HasLanded())
			{
				landedAICount++;
			}
		}

		cout << landedAICount << endl;

		if (landedAICount > 15)
		{
			myMutationRate = 0;
		}
		else if (landedAICount > 5)
		{
			myMutationRate /= 50;
		}


		if (landedAICount == 100)
		{
			for (int i = 0; i < kNumberOfAILanders; i++)
			{
				mAILanders[i]->Update(deltaTime, e);
			}
			for (int i = 0; i < kNumberOfAILanders; i++)
			{
				mAILanders[i]->Render();
			}
			SaveSolution();
			mPause = true;
		}
		else
		{
			mAccumulatedDeltaTime = 0;
			landedAICount = 0;
			CalculateFitness();
			Selection();
			Crossover();
			if (mAllowMutation)
			{
				Mutation();
			}
			//TransferChromosomes();

			RestartGA();
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::CalculateFitness()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		double speed = mAILanders[i]->GetSpeed();
		double RotFitness = 0;
		double DistFitness = 0;
		double SurvFitness = 0;
		double LandedBonus = 0;
		double fitness = 0;

		DistFitness = ((kLunarScreenWidth - fabs(mPlatformPosition.x - mAILanders[i]->GetPosition().x)) +
				       (kLunarScreenHeight - (mPlatformPosition.y - mAILanders[i]->GetPosition().y)));

		RotFitness = mAILanders[i]->GetRotationAngle();

		SurvFitness = mAILanders[i]->GetSurvivalTime() / (mAILanders[i]->GetSpeed() + 1);

/*		if (mAILanders[i]->HasLanded())
			fitness = (MaxDouble / 2) + DistFitness + (400 * RotFitness) + (4 * SurvFitness);//half allows for improvement in the successful landers
		else
			fitness = DistFitness + (400 * RotFitness) + (4 * SurvFitness);*/

		if (mAILanders[i]->HasLanded())
			fitness = kLandingBonus;
		else
			fitness = (kDistWeight*DistFitness) + (RotFitness*kRotWeight) +(SurvFitness*kAirTimeWeight);
		
		//cout << "Index: " << i << endl;
		//cout << "DistFitness = " << kDistWeight*DistFitness << endl;
		//cout << "RotFitness = " << RotFitness*kRotWeight << endl;
		//cout << "SurvFitness = " << SurvFitness*kAirTimeWeight << endl;
		//cout << "Total = " << fitness << endl << endl;

		mFitnessValues[i] = fitness;
	}

//	for (int i = 0; i < kNumberOfAILanders; i++)
//		cout << i << " Fitnes: " << mFitnessValues[i] << endl;

}
//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Selection()
{
	/*
	//Elitism
	SelectedAIChromosomeCount = 0;
	int Ehighest = 0;
	int ElitistRate = 2;

	for (int e = 0; e < ElitistRate; e++)
	{
		for (int current = 0; current < kNumberOfAILanders; current++)
		{
			mFitnessValues[Ehighest] >  mFitnessValues[current] ? Ehighest = Ehighest : Ehighest = current;
		}
		cout << "Selected: " << Ehighest << endl;

		for (int i = 0; i < kNumberOfChromosomeElements; i++)
		{
			mSelectedAIChromosomes[SelectedAIChromosomeCount][i] = mChromosomes[Ehighest][i];
		}
		SelectedAIChromosomeCount++;
	}
	*/

	
	//cout << "Tournament Selection: " << endl;
	//Tournament Selection.
	
	int current = 0;
	int randomIndex = 0;
	int highest = 0;
	int tournamentPool = 30; //<<<<<< Pool Size 

	for (int current = 0; current < kNumberOfAILanders; current++)
	{
		highest = rand() % kNumberOfChromosomeElements;
		for (int i = 0; i < tournamentPool; i++)
		{
			randomIndex = rand() % kNumberOfChromosomeElements;
			if (mFitnessValues[highest] < mFitnessValues[randomIndex])
				highest = randomIndex;
		}
		//cout << "Selected: " << highest << endl;

		for (int i = 0; i < kNumberOfChromosomeElements; i++)
		{
			mSelectedAIChromosomes[current][i] = mChromosomes[highest][i];
		}
	}
	

	/*
	int index1, index2;

	//Tournament Selection.
	for (int current = 0; current < kNumberOfAILanders; current++)
	{
		index1 = rand() % kNumberOfChromosomeElements;
		index2 = rand() % kNumberOfChromosomeElements;

		if (mFitnessValues[index1] >= mFitnessValues[index2])
		{
			for (int i = 0; i < kNumberOfChromosomeElements; i++)
				mSelectedAIChromosomes[current][i] = mChromosomes[index1][i];
		}
		else
		{
			for (int i = 0; i < kNumberOfChromosomeElements; i++)
				mSelectedAIChromosomes[current][i] = mChromosomes[index2][i];
		}
	}
	*/

	/*
	//Roulette

	int fitnessTotalMultiplier = 0;
	long int fitnessTotal = 0;
	long int randomIndex;
	int currentCount;

	for (int current = 0; current < kNumberOfAILanders; current++)
	{
		fitnessTotal += mFitnessValues[current];
	}

	std::srand(std::time(0));
	std::random_device rd;
	std::default_random_engine eng{ rd() };
	std::uniform_int_distribution<> dist(0, fitnessTotal);

	for (int current = 0; current < kNumberOfAILanders; current++)
	{
		randomIndex = dist(eng);
		for (int fitnessValIt = 0; fitnessValIt < kNumberOfAILanders; fitnessValIt++)
		{
			randomIndex -= mFitnessValues[fitnessValIt];

			if (randomIndex < 0)
			{
				for (int gene = 0; gene < kNumberOfChromosomeElements; gene++)
				{
					mSelectedAIChromosomes[current][gene] = mChromosomes[fitnessValIt][gene];
				}
				break;
			}
		}
	}
	*/


	/*
	//https://en.wikipedia.org/wiki/Stochastic_universal_sampling
	//https://books.google.co.uk/books?id=MYJ_AAAAQBAJ&lpg=PA14&ots=XvtLur4yGx&dq=Reducing%20Bias%20and%20Inefficiency%20in%20the%20Selection%20Algorithm&lr&pg=PA16#v=onepage&q=Reducing%20Bias%20and%20Inefficiency%20in%20the%20Selection%20Algorithm&f=false
	//
	//Stochastic
	//////////////////////////////////
	//stepSize = rand < fitnessTotal/kNumberOfLanders;
	//for each lander
	//	while( stepSize < landerFitnessScore)
	//		add it to the selectedAI
	//		step size+= step
	//		
	//////////////////////////////////

	double fitnessTotal = 0;

	for (int current = 0; current < kNumberOfAILanders; current++)
	{
		fitnessTotal += mFitnessValues[current];
	}

	//Large Random Number Generator
	std::srand(std::time(0));
	std::random_device rd;
	std::default_random_engine eng{ rd() };
	std::uniform_int_distribution<> dist(0, fitnessTotal/ kNumberOfAILanders);

	double stepSize = dist(eng);

	double selector = stepSize;
	int ChromosomeCount = 0;
	double fitnessSum = mFitnessValues[0];

	for (int selectedCount = 0; selectedCount < kNumberOfAILanders; selectedCount++)
	{
		if (fitnessSum < selector)
		{
			//cout << ChromosomeCount << " :- Not High Enough, needs: " << selector << "		Current value: " << fitnessSum << endl;

			while (selector > fitnessSum)
			{
				ChromosomeCount++;
				//cout << "fitnessSum : " << fitnessSum << " + " << mFitnessValues[ChromosomeCount] << " = " << fitnessSum+mFitnessValues[ChromosomeCount] << endl;
				fitnessSum += mFitnessValues[ChromosomeCount];
				//system("pause");
			}
		}

		//cout << fitnessSum << "is more than " << selector << endl;
		for (int gene = 0; gene < kNumberOfChromosomeElements; gene++)
		{
			mSelectedAIChromosomes[selectedCount][gene] = mChromosomes[ChromosomeCount][gene];
		}

		//cout << "Selected: " << ChromosomeCount << endl;
		selector += stepSize;
	}
	*/
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Crossover()
{
	// 1 Point Crossover
	
	for (int i = 0; i < kNumberOfAILanders; i+=2)
	{
		//if (rand() % 10000 < kCrossoverRate)
		//{
			for (int j = 0; j < kHalfNumberOfAILanders; j++)
			{
				mChromosomes[i][j] = mSelectedAIChromosomes[i][j];
				mChromosomes[i + 1][j] = mSelectedAIChromosomes[i + 1][j];
			}

			for (int j = kHalfNumberOfAILanders; j < kNumberOfAILanders; j++)
			{
				mChromosomes[i][j] = mSelectedAIChromosomes[i + 1][j];
				mChromosomes[i + 1][j] = mSelectedAIChromosomes[i][j];
			}
		//}
		//else
		//{
		//	for (int j = 0; j < kNumberOfChromosomeElements; j++)
		//	{
		//		mChromosomes[i][j] = mSelectedAIChromosomes[i][j];
		//	}
		//}
	}
	
	// N Point Crossover
	//LunarAction temp[kNumberOfChromosomeElements];
	////for (size_t i = 0; i < kNumberOfChromosomeElements; i++)
	////{
	////	temp[i] = LunarAction::LunarAction_None;

	////}

	//for (int i = 0; i < kNumberOfAILanders; i++)
	//{
	//	for (int j = 25; j < kNumberOfChromosomeElements; j++)
	//	{
	//		temp[j] = mSelectedAIChromosomes[i][j];
	//		mSelectedAIChromosomes[i][j] = mSelectedAIChromosomes[i + 1][j];
	//		mSelectedAIChromosomes[i + 1][j] = temp[j];
	//	}
	//	for (int j = 50; j < kNumberOfChromosomeElements; j++)
	//	{
	//		temp[j] = mSelectedAIChromosomes[i][j];
	//		mSelectedAIChromosomes[i][j] = mSelectedAIChromosomes[i + 1][j];
	//		mSelectedAIChromosomes[i + 1][j] = temp[j];
	//	}
	//	for (int j = 75; j < kNumberOfChromosomeElements; j++)
	//	{
	//		temp[j] = mSelectedAIChromosomes[i][j];
	//		mSelectedAIChromosomes[i][j] = mSelectedAIChromosomes[i + 1][j];
	//		mSelectedAIChromosomes[i + 1][j] = temp[j];
	//	}
	//	i++;
	//

	//MultiPointCrossover
/*	for (int lander = SelectedAIChromosomeCount; lander < kNumberOfChromosomeElements - 1; lander++)
	{
		for (int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			if (rand() % 10000 < kCrossoverRate)
			{
				LunarAction temp = mSelectedAIChromosomes[lander][action];
				mSelectedAIChromosomes[lander][action] = mSelectedAIChromosomes[lander + 1][action];
				mSelectedAIChromosomes[lander + 1][action] = temp;
			}
			//mSelectedAIChromosomes[99][action] = LunarAction::LunarAction_None;
		}
	}*/


}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::Mutation()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		//cout << rand() % 10000 << endl;
		for (int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			if (rand() % 10000 < myMutationRate)
			{
				//int randomNumber = rand() % kNumberOfChromosomeElements;
				int randAction = (rand() % 4);

				switch (randAction)
				{
				case 0:
					mChromosomes[i][action] = LunarAction_None;
					//cout << "Mutated : " << i << " Action : " << action << " to " << LunarAction_None << endl;
					break;
				case 1:
					mChromosomes[i][action] = LunarAction_LunarThrust;
					//cout << "Mutated : " << i << " Action : " << action << " to " << LunarAction_LunarThrust << endl;
					break;
				case 2:
					mChromosomes[i][action] = LunarAction_LunarLeft;
					//cout << "Mutated : " << i << " Action : " << action << " to " << LunarAction_LunarLeft << endl;
					break;
				case 3:
					mChromosomes[i][action] = LunarAction_LunarRight;
					//cout << "Mutated : " << i << " Action : " << action << " to " << LunarAction_LunarRight << endl;
					break;
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::RestartGA()
{
	if (mGeneration >= 500)
	{
		GenerateRandomChromosomes();
		mGeneration = 0;
		mAllowMutation = true;
		cout << endl << "---FAILED (Restart)---" << endl << endl;
	}

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		mFitnessValues[i] = 0.0f;
		mAILanders[i]->Reset();
	}

	mAccumulatedDeltaTime = 0.0f;
	mCurrentAction = 0;

	mGeneration++;
	cout << "---GENERATION " << mGeneration << "---" << endl;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Lunar::SaveSolution()
{
	//Output the solution of the first chromosome - They should all match at the point we output so it
	//shouldn't matter which chromosome is used for the solution.
	ofstream outFile("Solution.txt");
	if(outFile.is_open())
	{
		for(int action = 0; action < kNumberOfChromosomeElements; action++)
		{
			switch(mChromosomes[0][action])
			{
				case LunarAction_None:
					outFile << "0";
				break;
			
				case LunarAction_LunarThrust:
					outFile << "1";
				break;

				case LunarAction_LunarLeft:
					outFile << "2";
				break;

				case LunarAction_LunarRight:
					outFile << "3";
				break;
			}
		
			outFile << ",";
		}

		outFile << endl << "Generations taken: " << mGeneration;
		outFile << endl << "Platform start (" << mPlatformPosition.x << "," << mPlatformPosition.y << ")";
		outFile << endl << "Ship start (" << mAILanders[0]->GetStartPosition().x << "," << mAILanders[0]->GetStartPosition().y << ")";

		outFile.close();
	}
}

void GameScreen_Lunar::TransferChromosomes()
{
	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			mChromosomes[i][j] = LunarAction::LunarAction_None;
		}
	}

	for (int i = 0; i < kNumberOfAILanders; i++)
	{
		for (int j = 0; j < kNumberOfChromosomeElements; j++)
		{
			mChromosomes[i][j] = mSelectedAIChromosomes[i][j];
			mSelectedAIChromosomes[i][j] = LunarAction::LunarAction_None;
		}
	}
}

//--------------------------------------------------------------------------------------------------
