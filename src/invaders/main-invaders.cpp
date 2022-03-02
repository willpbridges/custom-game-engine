#include "base/InputManager.hpp"
#include "base/GameObject.hpp"
#include "base/Level.hpp"
#include "base/GenericComponent.hpp"
#include "base/RemoveOnCollideComponent.hpp"
#include "base/RectRenderComponent.hpp"
#include "base/SpriteRenderComponent.hpp"
#include "base/PhysicsComponent.hpp"
#include "base/PatrolComponent.hpp"
#include "base/PhysicsManager.hpp"
#include "base/SDLGraphicsProgram.hpp"
#include "base/ResourceManager.hpp"
#include "base/LTimer.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#include <Box2D/Box2D.h>
#include <memory>

const float SIZE = 40.0f;
static const int TAG_PLAYER = 1;
static const int TAG_TOP = 7;
static const int TAG_SIDE = 8;
static const int TAG_ENEMY = 4;
static const int TAG_PROJECTILE = 9;
static const int TAG_ENEMY_PROJ = 10;
static const int TAG_SHIELD = 11;
static const int TAG_SPEEDUP = 12;
static const int TAG_HEALTHUP = 13;

//tells the SDLGraphicsProgram which game is being run
static const int GAME_ID = 3;

int channel;

/**
 * A projectile that can be shot by enemies
 */
class EnemyProjectile: public GameObject {
public:
	EnemyProjectile(Level &level, float x, float y, Mix_Chunk *deathSound) :
			GameObject(level, x, y, SIZE * 0.25, SIZE * 0.25, TAG_ENEMY_PROJ) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::DYNAMIC_SOLID));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0xff, 0x00, 0x00));
		physicsComponent()->setVy(300);
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_ENEMY_PROJ, nullptr, channel));
	}
};

/**
 * A projectile that can be shot by the player
 */
class Projectile: public GameObject {
public:
	Projectile(Level &level, float x, float y, Mix_Chunk *deathSound) :
			GameObject(level, x, y, SIZE * 0.25, SIZE * 0.25, TAG_PROJECTILE) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::DYNAMIC_SOLID));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0xff, 0x00, 0x00));
		physicsComponent()->setVy(-600);
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_ENEMY_PROJ, nullptr, channel));
	}
};

/**
 * Component that handles any power up
 */
class PowerUpComponent: public GenericComponent {
public:

	PowerUpComponent(GameObject &gameObject, int powerUpType) :
			GenericComponent(gameObject), mPowerUpType(powerUpType) {
	}

	virtual void collision(Level &level, std::shared_ptr<GameObject> obj)
			override {
		if (obj->tag() == TAG_PLAYER) {

			if (mPowerUpType == 1) {
				level.restoreHealth();
			} else if (mPowerUpType == 2) {
				level.setSpeedUp(true);
			}
		}
	}

private:
	int mPowerUpType;
};

//A health up powerup game object 
class HealthUp: public GameObject {
public:
	HealthUp(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE, SIZE, TAG_HEALTHUP) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::STATIC_SENSOR));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0xFF, 0x69, 0xB4));
		addGenericComponent(std::make_shared < PowerUpComponent > (*this, 1));
	}
};

//A speedup powerup game object
class SpeedUp: public GameObject {
public:
	SpeedUp(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE, SIZE, TAG_SPEEDUP) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::STATIC_SENSOR));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0x2A, 0xBE, 0xBB));
		addGenericComponent(std::make_shared < PowerUpComponent > (*this, 2));
	}
};

/**
 * Component that handles health for various objects
 */
class HealthComponent: public GenericComponent {
public:

	HealthComponent(GameObject &gameObject, int health, int tag) :
			GenericComponent(gameObject), mHealth(health), enemyTag(tag) {
	}

	virtual void collision(Level &level, std::shared_ptr<GameObject> obj)
			override {
		if (obj->tag() == enemyTag) {
			mHealth--;
			if (mHealth == 0) {
				GameObject &gameObject = getGameObject();
				gameObject.setPhysicsComponent(nullptr);
				//level.removeObject(gameObject.shared_from_this());
			}
		}
	}

	void setHealth(int health) {
		mHealth = health;
	}

private:
	int mHealth;
	int enemyTag;
};

/**
 * Component to handle player input.
 */
class InvadersInputComponent: public GenericComponent {
public:
	InvadersInputComponent(GameObject &gameObject, float speed,
			Mix_Chunk *shootSound) :
			GenericComponent(gameObject), mSpeed(speed) {
		sound = shootSound;
		playerTimer.start();
		shootTime = 400;
		nextShot = 0;
		powerUpTime = 0;
	}

	virtual void update(Level &level) override
	{
		bool left = InputManager::getInstance().isKeyDown(SDLK_LEFT);
		bool right = InputManager::getInstance().isKeyDown(SDLK_RIGHT);
		bool space = InputManager::getInstance().isKeyDown(SDLK_SPACE);

		GameObject &gameObject = getGameObject();
		std::shared_ptr<PhysicsComponent> pc = gameObject.physicsComponent();

		if (level.isSpedUp()) {
			powerUpTime = 400;
			level.setSpeedUp(false);
		}

		if (powerUpTime > 0) {
			mSpeed = 1000.0f;
			powerUpTime--;
		}

		if (!(level.isSpedUp()) && powerUpTime == 0) {
			mSpeed = 500.0f;
		}

		if (left && !right) {
			pc->setVx(-mSpeed);
		} else if (!left && right) {
			pc->setVx(mSpeed);
		} else {
			pc->setVx(0.0f);
		}

		if (space) {
			if (playerTimer.getTicks() > nextShot) {
				nextShot = playerTimer.getTicks() + shootTime;
				auto shot =
						std::make_shared < Projectile
								> (level, gameObject.x() + (gameObject.w() / 2)
										- 5, gameObject.y(), sound);
				level.addObject(shot);
			}
		}

		pc->setVy(0.0f);

	}

private:
	Mix_Chunk *sound;
	float mSpeed;
	LTimer playerTimer;
	Uint32 shootTime;
	Uint32 nextShot;
	int powerUpTime;
};

/**
 * Represents the sides of the screen, which the ball bounces off of
 */
class SideWall: public GameObject {
public:
	SideWall(Level &level, float x, float y) :
			GameObject(level, x, y, 1, SIZE * 20, TAG_SIDE) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::STATIC_SOLID));
	}
};

/**
 * Represents the top of the screen, which the ball bounces off of
 */
class TopWall: public GameObject {
public:
	TopWall(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE * 20, 1, TAG_TOP) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::STATIC_SOLID));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_PROJECTILE, nullptr, channel));
	}
};

/**
 * Represents the player
 */
class InvadersPlayer: public GameObject {
public:
	InvadersPlayer(Level &level, float x, float y,
			std::vector<SDL_Texture*> playerTextures, Mix_Chunk *shootSound) :
			GameObject(level, x, y, SIZE * 1.5, SIZE * 1.5, TAG_PLAYER) {
		addGenericComponent(
				std::make_shared < InvadersInputComponent
						> (*this, 500.0f, shootSound));
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::DYNAMIC_SOLID));
		setRenderComponent(
				std::make_shared < SpriteRenderComponent
						> (*this, playerTextures));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_HEALTHUP, nullptr, channel));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_SPEEDUP, nullptr, channel));
		addGenericComponent(
						std::make_shared < HealthComponent
								> (*this, 2, TAG_ENEMY_PROJ));
	}
};

/**
 * Controls enemy shooting and sprite switching
 */
class EnemyControlComponent: public GenericComponent {
public:
	EnemyControlComponent(GameObject &gameObject, Mix_Chunk *shootSound, int id) :
			GenericComponent(gameObject) {
		sound = shootSound;
		enemyTimer.start();
		shootTime = 3000;
		enemyId = id;
		interval = 400;
		nextShot = id * interval;

		sprite = 0;
		spriteTime = 400;
		nextSpriteSwitch = 0;
	}

	virtual void update(Level &level) override
	{
		GameObject &gameObject = getGameObject();
		std::shared_ptr<RenderComponent> rc = gameObject.renderComponent();
		std::shared_ptr<SpriteRenderComponent> spriteComponent =
				std::static_pointer_cast < SpriteRenderComponent > (rc);

		if (enemyTimer.getTicks() > nextShot) {
			nextShot = enemyTimer.getTicks() + shootTime;
			auto shot =
					std::make_shared < EnemyProjectile
							> (level, gameObject.x() + (gameObject.w() / 2) - 5, gameObject.y()
									+ gameObject.h(), sound);
			level.addObject(shot);
		}

		if (enemyTimer.getTicks() > nextSpriteSwitch) {
			nextSpriteSwitch = enemyTimer.getTicks() + spriteTime;
			if (sprite == 0) {
				sprite = 1;
			} else {
				sprite = 0;
			}
			spriteComponent->setSprite(sprite);
		}
	}

private:
	Mix_Chunk *sound;
	LTimer enemyTimer;
	Uint32 shootTime;
	Uint32 nextShot;
	int enemyId;
	int interval;
	int sprite;
	Uint32 spriteTime;
	Uint32 nextSpriteSwitch;
};

//Regular enemy class that has the functionality to shoot rocks and a player
class SpaceEnemy: public GameObject {
public:
	SpaceEnemy(Level &level, float x, float y, float distX, float distY,
			std::vector<SDL_Texture*> enemyTextures, Mix_Chunk *deathSound,
			int enemyId) :
			GameObject(level, x, y, SIZE, SIZE, TAG_ENEMY) {
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_PROJECTILE, deathSound, channel));
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::DYNAMIC_SOLID));
		setRenderComponent(
				std::make_shared < SpriteRenderComponent
						> (*this, enemyTextures));
		addGenericComponent(
				std::make_shared < PatrolComponent
						> (*this, x + distX, y + distY, SIZE * 2));
		addGenericComponent(
				std::make_shared < EnemyControlComponent
						> (*this, nullptr, enemyId));
		addGenericComponent(
				std::make_shared < HealthComponent
						> (*this, 2, TAG_PROJECTILE));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_ENEMY_PROJ, nullptr, channel));
	}
};

// Sheild powerup gameobject. The sheild completely restores health upon collection.
class Shield: public GameObject {
public:
	Shield(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE * 1.5, SIZE * 0.5, TAG_SHIELD) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::STATIC_SOLID));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 0xFF, 0xFF, 0xFF));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_PROJECTILE, nullptr, channel));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_ENEMY_PROJ, nullptr, channel));
		mHealthComponent = std::make_shared < HealthComponent
				> (*this, 3, TAG_ENEMY_PROJ);
		addGenericComponent(mHealthComponent);
	}

	std::shared_ptr<HealthComponent> mHealthComponent;
};

/**
 * Represents a level of Invaders
 */
class InvadersLevel: public Level {
public:
	InvadersLevel(std::vector<std::string> layout,
			std::vector<SDL_Surface*> surfaces, std::vector<Mix_Chunk*> sounds) :
			Level(20 * SIZE, 20 * SIZE, false, GAME_ID) {
		levelLayout = layout;
		levelSounds = sounds;
		levelSurfaces = surfaces;
	}

	//Restore health completely when obtaining a sheild
	void restoreHealth() override
	{
		for (auto shield : shields) {
			shield->mHealthComponent->setHealth(3);
		}

		for (std::shared_ptr<GameObject> shield : shields) {
			std::shared_ptr<RenderComponent> rc = std::make_shared < RectRenderComponent
					> (*shield, 0xFF, 0xFF, 0xFF);
			shield->setRenderComponent(rc);
		}
	}

	void makeObject(int tag, std::pair<int, int> position) override
	{
		switch (tag) {
		case TAG_PLAYER: {
			auto player =
					std::make_shared < InvadersPlayer
							> (*this, position.first * SIZE, position.second
									* SIZE, playerTextures, levelSounds[0]);
			addObject(player);
			break;
		}

		case TAG_ENEMY: {
			auto enemy =
					std::make_shared < SpaceEnemy
							> (*this, position.first * SIZE, position.second
									* SIZE, SIZE, 0, enemyTextures, levelSounds[0], numEnemies);
			addObject(enemy);
			numEnemies++;

			break;
		}

		case TAG_TOP: {
			addObject(
					std::make_shared < TopWall
							> (*this, position.first * SIZE, position.second
									* SIZE));
			break;
		}
		case TAG_SIDE: {
			addObject(
					std::make_shared < SideWall
							> (*this, position.first * SIZE, position.second
									* SIZE));
			break;
		}
		case TAG_SHIELD: {
			auto shield = std::make_shared < Shield
					> (*this, position.first * SIZE, position.second * SIZE);
			addObject(shield);
			shields.push_back(shield);
			break;
		}

		case TAG_SPEEDUP: {
			addObject(
					std::make_shared < SpeedUp
							> (*this, position.first * SIZE, position.second
									* SIZE));
			break;
		}

		case TAG_HEALTHUP: {
			addObject(
					std::make_shared < HealthUp
							> (*this, position.first * SIZE, position.second
									* SIZE));
			break;
		}
		}
	}

	void initialize(SDL_Renderer *renderer) override
	{
		finalize();

		SDL_Texture *player = SDL_CreateTextureFromSurface(renderer,
				levelSurfaces[0]);
		if (player == NULL) {
			SDL_Log("Failed to create texture");
		} else {
			SDL_Log("Loaded texture");
		}
		playerTextures.push_back(player);

		for (int i = 1; i < 3; i++) {
			SDL_Texture *newTexture = SDL_CreateTextureFromSurface(renderer,
					levelSurfaces[i]);
			if (newTexture == NULL) {
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

				if (levelLayout[i][j] == 'P') {
					makeObject(TAG_PLAYER, std::make_pair(xPos, yPos));
				}
				if (levelLayout[i][j] == 'E') {
					makeObject(TAG_ENEMY, std::make_pair(xPos, yPos));
				}
				if (levelLayout[i][j] == 'X') {
					makeObject(TAG_SHIELD, std::make_pair(xPos, yPos));
				}
				if (levelLayout[i][j] == 'S') {
					makeObject(TAG_SPEEDUP, std::make_pair(xPos, yPos));
				}
				if (levelLayout[i][j] == 'H') {
					makeObject(TAG_HEALTHUP, std::make_pair(xPos, yPos));
				}

				xPos++;
			}
			xPos = 0;
			yPos++;
		}

		makeObject(TAG_TOP, std::make_pair(0, 0));
		makeObject(TAG_SIDE, std::make_pair(0, 0));
		makeObject(TAG_SIDE, std::make_pair(20, 0));
	}

private:
	std::vector<std::string> levelLayout;
	std::vector<Mix_Chunk*> levelSounds;
	std::vector<SDL_Surface*> levelSurfaces;
	std::vector<SDL_Texture*> playerTextures;
	std::vector<SDL_Texture*> enemyTextures;
	std::vector<std::shared_ptr<Shield>> shields;
	int numEnemies = 0;
};

int main(int argc, char **argv) {

	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n",
				Mix_GetError());
	}

	std::cout << "Use the arrow keys to move and space to shoot.\n";
	std::cout << "Press R to reload or Q to quit.\n";

	//Resource loading
	ResourceManager::getInstance().startUp();
	ResourceManager::getInstance().loadSurface("Sprites/ship.png");
	ResourceManager::getInstance().loadSurface("Sprites/enemy1.png");
	ResourceManager::getInstance().loadSurface("Sprites/enemy2.png");
	std::vector<SDL_Surface*> surfaces =
			ResourceManager::getInstance().getSurfaces();

	ResourceManager::getInstance().loadLevel("/Levels/Invaders/level1.txt");
	ResourceManager::getInstance().loadLevel("/Levels/Invaders/level2.txt");
	ResourceManager::getInstance().loadLevel("/Levels/Invaders/level3.txt");
	//ResourceManager::getInstance().loadBGM("Sounds/Invaders.wav");
	ResourceManager::getInstance().loadMissEffect("Sounds/Death.wav");
	ResourceManager::getInstance().loadCollisionEffect("Sounds/Bounce.wav");

	std::vector<Mix_Chunk*> soundVector;
	soundVector.push_back(ResourceManager::getInstance().missEff);
	soundVector.push_back(ResourceManager::getInstance().collEff);

	std::vector < std::string > levelOneFile =
			ResourceManager::getInstance().levelVector[0];
	std::vector < std::string > levelTwoFile =
			ResourceManager::getInstance().levelVector[1];
	std::vector < std::string > levelThreeFile =
			ResourceManager::getInstance().levelVector[2];

	std::shared_ptr<InvadersLevel> firstLevel = std::make_shared < InvadersLevel
			> (levelOneFile, surfaces, soundVector);
	std::shared_ptr<InvadersLevel> secondLevel = std::make_shared
			< InvadersLevel > (levelTwoFile, surfaces, soundVector);
	std::shared_ptr<InvadersLevel> thirdLevel = std::make_shared < InvadersLevel
			> (levelThreeFile, surfaces, soundVector);
	std::vector < std::shared_ptr < Level >> levels;
	levels.push_back(firstLevel);
	levels.push_back(secondLevel);
	levels.push_back(thirdLevel);
	SDLGraphicsProgram mySDLGraphicsProgram(levels, GAME_ID);
	mySDLGraphicsProgram.loop();
	ResourceManager::getInstance().shutDown();
	return 0;
}
