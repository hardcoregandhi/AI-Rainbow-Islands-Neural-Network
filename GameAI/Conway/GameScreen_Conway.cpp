//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "GameScreen_Conway.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "ConwayConstants.h"
#include "../TinyXML/tinyxml.h"

//--------------------------------------------------------------------------------------------------

GameScreen_Conway::GameScreen_Conway(SDL_Renderer* renderer) : GameScreen(renderer)
{
	mMap = new int*[kConwayScreenWidth/kConwayTileDimensions];
	for(int i =0; i < kConwayScreenWidth/kConwayTileDimensions; i++)
		mMap[i] = new int[kConwayScreenHeight/kConwayTileDimensions];

	//Get all required textures.
	mWhiteTile = new Texture2D(renderer);
	mWhiteTile->LoadFromFile("Images/Conway/WhiteTile.png");
	mBlackTile = new Texture2D(renderer);
	mBlackTile->LoadFromFile("Images/Conway/BlackTile.png");

	//Start map has 50% active cells.
	CreateRandomMap(20);

	//Start game in frozen state.
	mPause = true;

	mMapTemp = new int*[kConwayScreenWidth / kConwayTileDimensions];
	for (int i = 0; i < kConwayScreenWidth / kConwayTileDimensions; i++)
		mMapTemp[i] = new int[kConwayScreenHeight / kConwayTileDimensions];
}

//--------------------------------------------------------------------------------------------------

GameScreen_Conway::~GameScreen_Conway()
{
	delete mWhiteTile;
	mWhiteTile = NULL;

	delete mBlackTile;
	mBlackTile = NULL;

	for(int i =0; i < kConwayScreenWidth/kConwayTileDimensions; i++)
	{
		delete mMap[i];
		mMap[i] = NULL;
	}
	delete mMap;
	mMap = NULL;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::Render()
{
	for(int x = 0; x < kConwayScreenWidth/kConwayTileDimensions; x++)
	{
		for(int y = 0; y < kConwayScreenHeight/kConwayTileDimensions; y++)
		{
			switch(mMap[x][y])
			{
				case 0:
					mBlackTile->Render(Vector2D(x*kConwayTileDimensions,y*kConwayTileDimensions));
				break;
				
				case 1:
					mWhiteTile->Render(Vector2D(x*kConwayTileDimensions,y*kConwayTileDimensions));
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::Update(float deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch(e.type)
	{
		//Deal with mouse click input.
		case SDL_KEYUP:
			switch(e.key.keysym.sym)
			{
				case SDLK_SPACE:
					mPause = !mPause;
				break;

				case SDLK_r:
					mPause = true;
					CreateRandomMap(75);
				break;

				case SDLK_l:
					mPause = true;
					LoadMap("Conway/ConwaySeed.xml");
				break;
			}
		break;
	
		default:
		break;
	}
	
	if(!mPause)
		UpdateMap();
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::UpdateMap()
{
	//Rules
	//1. Any living cell with less than 2 live neighbour dies.
	//2. Any living cell with 2 or 3 live cells lives on to next generation.
	//3. Any live cell with more than 3 live neighbours dies.
	//4. Any dead cell with exactly 3 live neighbours becomes a living cell.

    //TODO: This is where you code the rules.

	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{
			mMapTemp[x][y] = 0;
		}
	}

	int livingCellNeighbours, deadCellNeighbours;

	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{

			livingCellNeighbours = 0;
			deadCellNeighbours = 0;

			//Neighbour Checks
			if (x != 0) //Not against left wall
			{
				if (mMap[x - 1][y] == 1)	//LEFT
					livingCellNeighbours++;

				if (y != 0) //Not against top wall
				{
					if (mMap[x - 1][y - 1] == 1)	//TOP LEFT
						livingCellNeighbours++;
				}
			}

			if (y != 0) //Not against top wall
			{
				if (mMap[x][y - 1] == 1)	//UP
					livingCellNeighbours++;

				if (x + 1 != kConwayScreenHeight / kConwayTileDimensions) //Not against right wall
				{
					if (mMap[x + 1][y - 1] == 1)	//TOP RIGHT
						livingCellNeighbours++;
				}
			}

			if (x + 1 != kConwayScreenHeight / kConwayTileDimensions) //Not against right wall
			{
				if (mMap[x + 1][y] == 1)	//RIGHT
					livingCellNeighbours++;

				if (y + 1 != kConwayScreenHeight / kConwayTileDimensions) //Not against bottom wall
				{
					if (mMap[x + 1][y + 1] == 1)	//BOTTOM RIGHT
						livingCellNeighbours++;
				}
			}

			if (y + 1 != kConwayScreenHeight / kConwayTileDimensions) //Not against bottom wall
			{
				if (mMap[x][y + 1] == 1)	//DOWN
					livingCellNeighbours++;

				if (x != 0) //Not against left wall
				{
					if (mMap[x - 1][y + 1] == 1)	//BOTTOM LEFT
						livingCellNeighbours++;
				}
			}

			// Living Cells Check
			if (mMap[x][y] == 1)
			{
				if (livingCellNeighbours < 2)
				{
					mMapTemp[x][y] = 0;
					//cout << livingCellNeighbours << " : DEAD" << endl;
				}
				else if (livingCellNeighbours == 2 || livingCellNeighbours == 3)
				{
					mMapTemp[x][y] = 1;
					//cout << livingCellNeighbours << " : LIVE" << endl;
				}
				else if (livingCellNeighbours > 3)
				{
					mMapTemp[x][y] = 0;
					//cout << livingCellNeighbours << " : DEAD" << endl;
				}
			}

			// Dead Cells Check
			else if (mMap[x][y] == 0)
			{
				if (livingCellNeighbours == 3)
					mMapTemp[x][y] = 1;
			}

		}
	}

	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{
			mMap[x][y] = mMapTemp[x][y];
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::CreateRandomMap(int percentageInactive)
{
	int randomNumber = 0;

	for(int x = 0; x < kConwayScreenWidth/kConwayTileDimensions; x++)
	{
		for(int y = 0; y < kConwayScreenHeight/kConwayTileDimensions; y++)
		{
			randomNumber = (rand() % 100)+1;

			if(randomNumber > percentageInactive)
				mMap[x][y] = 1;
			else
				mMap[x][y] = 0;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::LoadMap(std::string path)
{
	//Get the whole xml document.
	TiXmlDocument doc;
	if(!doc.LoadFile(path))
	{
		cerr << doc.ErrorDesc() << endl;
	}

	//Now get the root element.
	TiXmlElement* root = doc.FirstChildElement();
	if(!root)
	{
		cerr << "Failed to load file: No root element." << endl;
		doc.Clear();
	}
	else
	{
		//Jump to the first 'objectgroup' element.
		for(TiXmlElement* groupElement = root->FirstChildElement("objectgroup"); groupElement != NULL; groupElement = groupElement->NextSiblingElement())
		{
			string name = groupElement->Attribute("name");
			if(name == "Seed")
			{
				int x = 0;
				int y = 0;

				//Jump to the first 'object' element - within 'objectgroup'
				for(TiXmlElement* objectElement = groupElement->FirstChildElement("object"); objectElement != NULL; objectElement = objectElement->NextSiblingElement())
				{
					string name = objectElement->Attribute("name");
					if(name == "TileTypes")
					{
						//Reset x position of map to 0 at start of each element.
						x = 0;

						//Split up the comma delimited connections.
						stringstream ss(objectElement->Attribute("value"));
						int i;
						while(ss >> i)
						{
							mMap[x][y] = i;

							if(ss.peek() == ',')
								ss.ignore();

							//Increment the x position.
							x++;
						}

						//Move down to the next y position in the map.
						y++;
					}
				}
			}
		}
	}
}