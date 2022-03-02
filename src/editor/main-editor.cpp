#include "base/InputManager.hpp"
#include "base/GameObject.hpp"
#include "base/Level.hpp"
#include "base/GenericComponent.hpp"
#include "base/PatrolComponent.hpp"
#include "base/RemoveOnCollideComponent.hpp"
#include "base/RectRenderComponent.hpp"
#include "base/SDLGraphicsProgram.hpp"
#include "base/ResourceManager.hpp"
#include "base/SpriteRenderComponent.hpp"
#include <memory>
#include <ctype.h>
#include <stdio.h>
#include "SDL.h"
#include <iostream>
#include <string>

static const int TAG_PLAYER = 1;
static const int TAG_GOAL = 2;
static const int TAG_BLOCK = 3;
static const int TAG_ENEMY = 4;
static const int TAG_COLLECTIBLE = 5;
static const int TAG_SHIELD = 6;

int editorGameId;
std::string filename;
const float SIZE = 40.0f;

/**
 * Input component to manipulate movement in two directions
 * 
 * @param GameObject gameObject - gameObject to be manipulated by input
 * @param float speed - speed at which the gameObject moves through input
 */
class EditorInputComponent: public GenericComponent {
public:
	EditorInputComponent(GameObject &gameObject) :
			GenericComponent(gameObject) {
	}

	virtual void update(Level &level) override
	{
		//GameObject & gameObject = getGameObject();

		bool playerKey = InputManager::getInstance().isKeyDown(SDLK_p);
		bool enemyKey = InputManager::getInstance().isKeyDown(SDLK_e);
		bool goalKey = InputManager::getInstance().isKeyDown(SDLK_g);
		bool blockKey = InputManager::getInstance().isKeyDown(SDLK_o);
		bool deleteKey = InputManager::getInstance().isKeyDown(SDLK_SPACE);
		bool collectibleKey = InputManager::getInstance().isKeyDown(SDLK_c);
		bool saveKey = InputManager::getInstance().isKeyDown(SDLK_s);
		bool shieldKey = InputManager::getInstance().isKeyDown(SDLK_x);

		std::pair<int, int> mousePosn =
				InputManager::getInstance().getMouseGridPosition(SIZE);
		if (editorGameId == 1) {
			if (playerKey) {
				level.makeObject(TAG_PLAYER, mousePosn);
			} else if (enemyKey) {
				level.makeObject(TAG_ENEMY, mousePosn);
			} else if (goalKey) {
				level.makeObject(TAG_GOAL, mousePosn);
			} else if (blockKey) {
				level.makeObject(TAG_BLOCK, mousePosn);
			} else if (collectibleKey) {
				level.makeObject(TAG_COLLECTIBLE, mousePosn);
			}
		} else if (editorGameId == 2) {
			if (blockKey) {
				if (mousePosn.second < 14) {
					level.makeObject(TAG_BLOCK, mousePosn);
				}
			}
		} else if (editorGameId == 3) {
			if (playerKey) {
				level.makeObject(TAG_PLAYER, mousePosn);
			} else if (enemyKey) {
				level.makeObject(TAG_ENEMY, mousePosn);
			} else if (shieldKey) {
				level.makeObject(TAG_SHIELD, mousePosn);
			}
		}

		if (saveKey) {
			std::string output = level.exportLevel(SIZE, filename);
			ResourceManager::getInstance().saveLevel(filename, output);
		}
		if (deleteKey) {
			level.removeObjectAtMouse(mousePosn, SIZE);
		}
	}

private:
};

class EditorShield: public GameObject {
public:
	EditorShield(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE * 2, SIZE * 0.5, TAG_SHIELD) {
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0xFF, 0xFF, 0xFF));
	}
};

/**
 * Player object with dual axis input. Includes an AdvInput component.
 * @param float x - initial position x
 * @param float y - initial position y
 * 
 */
class EditorPlayer: public GameObject {
public:
	EditorPlayer(Level &level, float x, float y,
			std::vector<SDL_Texture*> playerTextures) :
			GameObject(level, x, y, SIZE, SIZE, TAG_PLAYER) {
		setRenderComponent(
				std::make_shared < SpriteRenderComponent
						> (*this, playerTextures));
	}
};

/**
 * Represents the player in space invaders
 */
class EditorInvadersPlayer: public GameObject {
public:
	EditorInvadersPlayer(Level &level, float x, float y,
			std::vector<SDL_Texture*> playerTextures) :
			GameObject(level, x, y, SIZE * 1.5, SIZE * 1.5, TAG_PLAYER) {
		std::vector<SDL_Texture*> tex;
		tex.push_back(playerTextures[4]);
		setRenderComponent(
				std::make_shared < SpriteRenderComponent > (*this, tex));
	}
};

/**
 * Goal Object within the Adventure game mode, position which the character goes through a win state
 * 
 * @param float x - initial position x
 * @param float y - initial position y
 */
class EditorGoal: public GameObject {
public:
	EditorGoal(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE, SIZE, TAG_GOAL) {
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0xff, 0xff, 0x00));
	}
};

//Block allows the component to have collision
class EditorBlock: public GameObject {
public:
	EditorBlock(Level &level, float x, float y,
			std::vector<SDL_Texture*> blockTextures) :
			GameObject(level, x, y, SIZE, SIZE, TAG_BLOCK) {
		setRenderComponent(
				std::make_shared < SpriteRenderComponent
						> (*this, blockTextures));
	}
};

//Block allows the component to have collision
class EditorBreakoutBlock: public GameObject {
public:
	EditorBreakoutBlock(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE * 0.9, SIZE * 0.7, TAG_BLOCK) {
		setRenderComponent(
				std::make_shared < RectRenderComponent > (*this, 0, 0, 255));
	}
};

/**
 * Enemy object with dual axis movement
 * @param float x - initial position x
 * @param float y - initial position y
 */
class EditorEnemy: public GameObject {
public:
	EditorEnemy(Level &level, float x, float y,
			std::vector<SDL_Texture*> enemyTextures) :
			GameObject(level, x, y, SIZE, SIZE, TAG_ENEMY) {
		if (editorGameId == 3) {
			setRenderComponent(
					std::make_shared < SpriteRenderComponent
							> (*this, enemyTextures));
		} else {
			setRenderComponent(
					std::make_shared < RectRenderComponent
							> (*this, 0xff, 0x00, 0x00));
		}
	}
};

/**
 * Collectible object 
 * @param float x - initial position x
 * @param float y - initial position y
 */
class EditorCollectible: public GameObject {
public:
	EditorCollectible(Level &level, float x, float y,
			std::vector<SDL_Texture*> collectibleTextures) :
			GameObject(level, x, y, SIZE, SIZE, TAG_COLLECTIBLE) {
		setRenderComponent(
				std::make_shared < SpriteRenderComponent
						> (*this, collectibleTextures));
	}
};

/**
 * Generic editor object from a GameObject with mouse control input 
 * @param level - Current level of the object
 */
class EditorObject: public GameObject {
public:
	EditorObject(Level &level) :
			GameObject(level, 0.0f, 0.0f, 0.0f, 0.0f, 0) {
		addGenericComponent(std::make_shared < EditorInputComponent > (*this));
	}
};

class EditorLevel: public Level {
public:
	EditorLevel(std::vector<std::string> layout,
			std::vector<SDL_Surface*> surfaces) :
			Level(20 * SIZE, 20 * SIZE, true, editorGameId) {
		levelLayout = layout;
		levelSurfaces = surfaces;
	}

	void restoreHealth() override
	{
		return;
	}

	/**
	 * Creates an object within the current Editor level and adds it to a vector of items to be constructed in the scene
	 *
	 * @param tag The type of object to be added to the scene
	 * @param position The current position of the object to write to.
	 */
	void makeObject(int tag, std::pair<int, int> position) override
	{
		if (getObjectAtPosition(std::make_pair(position.first, position.second),
				SIZE) != 0) {
			return;
		}

		switch (tag) {
		case TAG_PLAYER: // A player object to be created in the scene
		{
			if (mPlayer) {
				movePlayer(position, SIZE);
			} else {
				std::shared_ptr<GameObject> player;

				if (editorGameId == 1) {
					player = std::make_shared < EditorPlayer
							> (*this, position.first * SIZE, position.second
									* SIZE, playerTextures);
				} else if (editorGameId == 3) {
					player = std::make_shared < EditorInvadersPlayer
							> (*this, position.first * SIZE, position.second
									* SIZE, playerTextures);
				}
				setPlayer(player);
				addObject(player);
			}
			break;
		}

		case TAG_GOAL: // A goal object to be created in the scene
		{
			if (mGoal) {
				moveGoal(position, SIZE);
			} else {
				auto goal =
						std::make_shared < EditorGoal
								> (*this, position.first * SIZE, position.second
										* SIZE);
				setGoal(goal);
				addObject(goal);
			}
			break;
		}

		case TAG_BLOCK: // A block object to be created in the scene
		{
			if (editorGameId == 1) {
				addObject(
						std::make_shared < EditorBlock
								> (*this, position.first * SIZE, position.second
										* SIZE, blockTextures));
			} else if (editorGameId == 2) {
				addObject(
						std::make_shared < EditorBreakoutBlock
								> (*this, position.first * SIZE, position.second
										* SIZE));
			}
			break;
		}

		case TAG_ENEMY: // An enemy object to be created in the scene
		{
			addObject(
					std::make_shared < EditorEnemy
							> (*this, position.first * SIZE, position.second
									* SIZE, enemyTextures));
			break;
		}
		case TAG_COLLECTIBLE: // A collectible object to be created in the scene
		{
			addObject(
					std::make_shared < EditorCollectible
							> (*this, position.first * SIZE, position.second
									* SIZE, collectibleTextures));
			break;
		}
		case TAG_SHIELD:
		{
			addObject(
					std::make_shared < EditorShield
							> (*this, position.first * SIZE, position.second
									* SIZE));
			break;
		}
		}
	}

	void initialize(SDL_Renderer *renderer) override
	{
		finalize();
		for (size_t i = 0; i < 5; i++) {
			SDL_Texture *newTexture = SDL_CreateTextureFromSurface(renderer,
					levelSurfaces[i]);
			if (newTexture == NULL) {
				SDL_Log("Failed to create texture");
			} else {
				SDL_Log("Loaded texture");
			}
			//Get rid of old loaded surface
			//SDL_FreeSurface(levelSurfaces[i]);
			playerTextures.push_back(newTexture);
		}

		SDL_Texture *block = SDL_CreateTextureFromSurface(renderer,
				levelSurfaces[5]);
		if (block == NULL) {
			SDL_Log("Failed to create texture");
		} else {
			SDL_Log("Loaded texture");
		}
		//Get rid of old loaded surface
		//SDL_FreeSurface(levelSurfaces[4]);
		blockTextures.push_back(block);

		SDL_Texture *collectibleTexture = SDL_CreateTextureFromSurface(renderer,
				levelSurfaces[6]);
		if (block == NULL) {
			SDL_Log("Failed to create texture");
		} else {
			SDL_Log("Loaded texture");
		}
		collectibleTextures.push_back(collectibleTexture);

		for (int i = 7; i < 9; i++) {
			SDL_Texture *newTexture = SDL_CreateTextureFromSurface(renderer,
					levelSurfaces[i]);
			if (block == NULL) {
				SDL_Log("Failed to create texture");
			} else {
				SDL_Log("Loaded texture");
			}
			enemyTextures.push_back(newTexture);
		}

		float xPos = 0;
		float yPos = 0;
		for (std::size_t i = 0; i < levelLayout.size(); i++) {
			for (std::size_t j = 0; j < levelLayout[i].size(); j++) {

				if (editorGameId == 1) {
					if (levelLayout[i][j] == 'O') {
						makeObject(TAG_BLOCK, std::make_pair(xPos, yPos));
					}
					if (levelLayout[i][j] == 'P') {
						makeObject(TAG_PLAYER, std::make_pair(xPos, yPos));
					}
					if (levelLayout[i][j] == 'G') {
						makeObject(TAG_GOAL, std::make_pair(xPos, yPos));
					}
					if (levelLayout[i][j] == 'E') {
						makeObject(TAG_ENEMY, std::make_pair(xPos, yPos));
					}
					if (levelLayout[i][j] == 'C') {
						makeObject(TAG_COLLECTIBLE, std::make_pair(xPos, yPos));
					}

				} else if (editorGameId == 2) {
					if (levelLayout[i][j] == 'O') {
						makeObject(TAG_BLOCK, std::make_pair(xPos, yPos));
					}
				} else if (editorGameId == 3) {
					if (levelLayout[i][j] == 'P') {
						makeObject(TAG_PLAYER, std::make_pair(xPos, yPos));
					}
					if (levelLayout[i][j] == 'E') {
						makeObject(TAG_ENEMY, std::make_pair(xPos, yPos));
					}
					if (levelLayout[i][j] == 'X') {
						makeObject(TAG_SHIELD, std::make_pair(xPos, yPos));
					}
				}

				addObject(std::make_shared < EditorObject > (*this));
				xPos++;
			}
			xPos = 0;
			yPos++;
		}
	}

private:
	std::vector<std::string> levelLayout;
	std::vector<SDL_Surface*> levelSurfaces;
	std::vector<SDL_Texture*> playerTextures;
	std::vector<SDL_Texture*> blockTextures;
	std::vector<SDL_Texture*> collectibleTextures;
	std::vector<SDL_Texture*> enemyTextures;
};

void loadResources() {
	ResourceManager::getInstance().startUp();
	ResourceManager::getInstance().loadLevel(filename);
	ResourceManager::getInstance().loadSurface("Sprites/slime.png");
	ResourceManager::getInstance().loadSurface("Sprites/slimeleft.png");
	ResourceManager::getInstance().loadSurface("Sprites/slimejump.png");
	ResourceManager::getInstance().loadSurface("Sprites/slimejumpleft.png");
	ResourceManager::getInstance().loadSurface("Sprites/ship.png");
	ResourceManager::getInstance().loadSurface("Sprites/tile.png");
	ResourceManager::getInstance().loadSurface("Sprites/collectible.png");
	ResourceManager::getInstance().loadSurface("Sprites/enemy1.png");
	ResourceManager::getInstance().loadSurface("Sprites/enemy2.png");
}

int main(int argc, char **argv) {
	std::cout << "Welcome to the level editor! \n";
	std::cout << "Which game would you like to edit?\n" << "(1)Platformer\n"
			<< "(2)Breakout\n" << "(3)Space Invaders\n";
	int gameChoice;
	std::cin >> gameChoice;
	if (gameChoice == 1) {
		editorGameId = 1;
		std::cout << "Platformer selected.\n";
		filename = "/Levels/";
	} else if (gameChoice == 2) {
		editorGameId = 2;
		std::cout << "Breakout selected.\n";
		filename = "/Levels/Breakout/";
	} else if (gameChoice == 3) {
		editorGameId = 3;
		std::cout << "Space Invaders selected.\n";
		filename = "/Levels/Invaders/";
	}

	std::cout << "Which level would you like to edit?\n" << "(1) Level 1\n"
			<< "(2) Level 2\n" << "(3) Level 3\n";

	int levelChoice;
	std::cin >> levelChoice;
	if (levelChoice == 1) {
		filename += "level1.txt";
	} else if (levelChoice == 2) {
		filename += "level2.txt";
	} else if (levelChoice == 3) {
		filename += "level3.txt";
	}

	if (editorGameId == 1) {
		std::cout
				<< "Move your mouse cursor over a tile and press one of the following keys to make changes.\n"
				<< "Press SPACE to delete an object.\n"
				<< "Press P to place the player.\n"
				<< "Press E to place an enemy.\n"
				<< "Press G to place the goal.\n"
				<< "Press O to place a platform.\n"
				<< "Press C to place a collectible.\n"
				<< "Press S to save your level.\n";
	} else if (editorGameId == 2) {
		std::cout
				<< "Move your mouse cursor over a tile and press one of the following keys to make changes.\n"
				<< "Press SPACE to delete an object.\n"
				<< "Press O to place a block.\n"
				<< "NOTE: You are unable to place blocks below a certain point,\n"
				<< "otherwise you would interfere with the ball and paddle.";
	} else if (editorGameId == 3) {
		std::cout
				<< "Move your mouse cursor over a tile and press one of the following keys to make changes.\n"
				<< "Press SPACE to delete an object.\n"
				<< "Press P to place the player.\n"
				<< "Press E to place an enemy.\n"
				<< "Press S to save your level.\n";
	}

	loadResources();
	std::vector<SDL_Surface*> surfaces =
			ResourceManager::getInstance().getSurfaces();
	std::vector < std::string > levelOneFile =
			ResourceManager::getInstance().levelVector[0];
	std::shared_ptr<EditorLevel> firstLevel = std::make_shared < EditorLevel
			> (levelOneFile, surfaces);
	std::vector < std::shared_ptr < Level >> levels;
	levels.push_back(firstLevel);
	SDLGraphicsProgram mySDLGraphicsProgram(levels, editorGameId);
	mySDLGraphicsProgram.loop();
	ResourceManager::getInstance().shutDown();
	return 0;
}
