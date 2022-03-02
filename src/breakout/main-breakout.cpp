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
#include <SDL.h>
#include <SDL_mixer.h>
#include <Box2D/Box2D.h>
#include <memory>

static const int TAG_PLAYER = 1;
static const int TAG_BLOCK = 3;
static const int TAG_BALL = 6;
static const int TAG_TOP = 7;
static const int TAG_SIDE = 8;
static const int GAME_ID = 2;
float ballSpeed = 350;

int channel;

/**
 * Component to handle player input.
 */
class BreakoutInputComponent: public GenericComponent {
public:

	BreakoutInputComponent(GameObject &gameObject, float speed) :
			GenericComponent(gameObject), mSpeed(speed) {
	}

	virtual void update(Level &level) override
	{
		bool left = InputManager::getInstance().isKeyDown(SDLK_LEFT);
		bool right = InputManager::getInstance().isKeyDown(SDLK_RIGHT);

		GameObject &gameObject = getGameObject();
		std::shared_ptr<PhysicsComponent> pc = gameObject.physicsComponent();

		if (left && !right) {
			pc->setVx(-mSpeed);
		} else if (!left && right) {
			pc->setVx(mSpeed);
		} else {
			pc->setVx(0.0f);
		}

		pc->setVy(0.0f);
	}

private:

	float mSpeed;
};

const float SIZE = 40.0f;

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
	}
};

/**
 * Represents the player
 */
class BreakoutPlayer: public GameObject {
public:
	BreakoutPlayer(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE * 3, SIZE * 0.5, TAG_PLAYER) {
		addGenericComponent(
				std::make_shared < BreakoutInputComponent > (*this, 800.0f));
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::DYNAMIC_SOLID));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 255, 255, 255));
	}
};

/**
 * Component that controls how the ball bounces off the paddle
 */
class BallBounceComponent: public GenericComponent {
public:

	BallBounceComponent(GameObject &gameObject, Mix_Chunk* collide) :
			GenericComponent(gameObject) {
		collideSound = collide;
	}

	virtual void collision(Level &level, std::shared_ptr<GameObject> obj)
			override {
		channel = Mix_PlayChannel(-1, collideSound, 0);
		if (obj->tag() == 1) {
			float xPos = obj->x() + (obj->w() / 2);
			float yPos = obj->y() + obj->h();
			GameObject &gameObject = getGameObject();
			std::shared_ptr<PhysicsComponent> pc =
					gameObject.physicsComponent();
			b2Body *body = pc->getBody();
			b2Vec2 bodyPos = body->GetPosition();
			float xDir = xPos - bodyPos.x;
			float yDir = yPos - bodyPos.y;
			pc->setVx(xDir * 0.75);
			pc->setVy(-yDir * 0.75);
		}
	}

private:
	float mSpeed;
	Mix_Chunk* collideSound;
};

/**
 * Represents the ball
 */
class Ball: public GameObject {
public:
	Ball(Level &level, float x, float y, std::vector<Mix_Chunk*> sounds) :
			GameObject(level, x, y, SIZE * 0.25, SIZE * 0.25, TAG_BALL) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::DYNAMIC_SOLID));
		setRenderComponent(
				std::make_shared < RectRenderComponent
						> (*this, 255, 255, 255));
		addGenericComponent(std::make_shared < BallBounceComponent > (*this, sounds[1]));
		addGenericComponent(
				std::make_shared < RemoveOnCollideComponent
						> (*this, TAG_BLOCK, sounds[1], channel));
	}

};

/**
 * Represents the breakable blocks
 */
class Block: public GameObject {
public:
	Block(Level &level, float x, float y) :
			GameObject(level, x, y, SIZE * 0.9, SIZE * 0.7, TAG_BLOCK) {
		setPhysicsComponent(
				std::make_shared < PhysicsComponent
						> (*this, PhysicsComponent::Type::STATIC_SOLID));
		setRenderComponent(
				std::make_shared < RectRenderComponent > (*this, 0, 0, 255));
	}
};

/**
 * Represents a level of breakout
 */
class BreakoutLevel: public Level {
public:
	BreakoutLevel(std::vector<std::string> layout,
			std::vector<Mix_Chunk*> sounds) :
			Level(20 * SIZE, 20 * SIZE, false, GAME_ID) {
		levelLayout = layout;
		levelSounds = sounds;
	}

	void restoreHealth() override
	{
		return;
	}

	void makeObject(int tag, std::pair<int, int> position) override {
		switch (tag) {
		case TAG_PLAYER: {
			auto player = std::make_shared < BreakoutPlayer
					> (*this, position.first * SIZE, position.second * SIZE);
			addObject(player);
		}
			break;

		case TAG_BLOCK: {
			addObject(
					std::make_shared < Block
							> (*this, position.first * SIZE, position.second
									* SIZE));
			break;
		}

		case TAG_BALL: {
			auto ball =
					std::make_shared < Ball
							> (*this, position.first * SIZE, position.second
									* SIZE, levelSounds);
			std::shared_ptr<PhysicsComponent> pc = ball->physicsComponent();
			pc->setVx(ballSpeed);
			pc->setVy(ballSpeed);
			addObject(ball);
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
		}
		}
	}

	void initialize(SDL_Renderer *renderer) override
	{
		finalize();

		float xPos = 0;
		float yPos = 0;
		for (std::size_t i = 0; i < levelLayout.size(); i++) {
			for (std::size_t j = 0; j < levelLayout[i].size(); j++) {

				if (levelLayout[i][j] == 'O') {
					makeObject(TAG_BLOCK, std::make_pair(xPos, yPos));
				}

				xPos++;
			}
			xPos = 0;
			yPos++;
		}

		makeObject(TAG_PLAYER, std::make_pair(10, 15));
		makeObject(TAG_BALL, std::make_pair(10, 14));
		makeObject(TAG_TOP, std::make_pair(0, 0));
		makeObject(TAG_SIDE, std::make_pair(0, 0));
		makeObject(TAG_SIDE, std::make_pair(20, 0));
	}

private:
	std::vector<std::string> levelLayout;
	std::vector<Mix_Chunk*> levelSounds;
}
;

int main(int argc, char **argv) {

	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n",
				Mix_GetError());
	}

	ResourceManager::getInstance().startUp();
	ResourceManager::getInstance().loadLevel("/Levels/Breakout/level1.txt");
	ResourceManager::getInstance().loadLevel("/Levels/Breakout/level2.txt");
	ResourceManager::getInstance().loadLevel("/Levels/Breakout/level3.txt");

	ResourceManager::getInstance().loadMissEffect("Sounds/Death.wav");
	ResourceManager::getInstance().loadBGM("Sounds/Breakout.wav");
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

	std::shared_ptr<BreakoutLevel> firstLevel = std::make_shared < BreakoutLevel
			> (levelOneFile, soundVector);

	std::shared_ptr<BreakoutLevel> secondLevel = std::make_shared
			< BreakoutLevel > (levelTwoFile, soundVector);
	std::shared_ptr<BreakoutLevel> thirdLevel = std::make_shared < BreakoutLevel
			> (levelThreeFile, soundVector);

	std::vector < std::shared_ptr < Level >> levels;
	levels.push_back(firstLevel);
	levels.push_back(secondLevel);
	levels.push_back(thirdLevel);
	SDLGraphicsProgram mySDLGraphicsProgram(levels, GAME_ID);
	mySDLGraphicsProgram.loop();
	ResourceManager::getInstance().shutDown();
	return 0;
}
