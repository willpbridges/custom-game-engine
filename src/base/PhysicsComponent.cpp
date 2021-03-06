#include "base/PhysicsComponent.hpp"
#include "base/PhysicsManager.hpp"
#include "base/GameObject.hpp"
#include "base/Level.hpp"

PhysicsComponent::PhysicsComponent(GameObject &gameObject, Type type) :
		Component(gameObject) {
	b2BodyDef bodyDef;
	int tag = gameObject.tag();
	bodyDef.type =
			(type == Type::DYNAMIC_SOLID ? b2_dynamicBody : b2_staticBody);
	bodyDef.position.x = (gameObject.x() + 0.5f * gameObject.w())
			* PhysicsManager::GAME_TO_PHYSICS_SCALE;
	bodyDef.position.y = (gameObject.y() + 0.5f * gameObject.h())
			* PhysicsManager::GAME_TO_PHYSICS_SCALE;
	if (tag == 6 || tag == 9 || tag == 10) {
		bodyDef.linearDamping = 0.0f;
	} else {
		bodyDef.linearDamping = 2.0f;
	}

	bodyDef.fixedRotation = true;
	mBody = PhysicsManager::getInstance().getWorld()->CreateBody(&bodyDef);

	mBody->SetUserData(&gameObject);

	b2PolygonShape shape;
	shape.SetAsBox(
			0.5f * gameObject.w() * PhysicsManager::GAME_TO_PHYSICS_SCALE,
			0.495f * gameObject.h() * PhysicsManager::GAME_TO_PHYSICS_SCALE);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = (type == Type::DYNAMIC_SOLID ? 1.0 : 0.0);
	fixtureDef.friction = 0.0f;
	fixtureDef.isSensor = type == Type::STATIC_SENSOR;
	if (tag == 6){
		fixtureDef.restitution = 1.0f;
	}

	mBody->CreateFixture(&fixtureDef);
}

PhysicsComponent::~PhysicsComponent() {
	PhysicsManager::getInstance().getWorld()->DestroyBody(mBody);
	mBody = nullptr;
}

void PhysicsComponent::setVx(float vx) {
	mBody->SetLinearVelocity(
			b2Vec2(vx * PhysicsManager::GAME_TO_PHYSICS_SCALE,
					mBody->GetLinearVelocity().y));
}

void PhysicsComponent::setVy(float vy) {
	mBody->SetLinearVelocity(
			b2Vec2(mBody->GetLinearVelocity().x,
					vy * PhysicsManager::GAME_TO_PHYSICS_SCALE));
}

void PhysicsComponent::addFx(float fx) {
	mBody->ApplyForce(b2Vec2(fx * PhysicsManager::GAME_TO_PHYSICS_SCALE, 0.0f),
			mBody->GetPosition(), true);
}

void PhysicsComponent::addFy(float fy) {
	mBody->ApplyForce(b2Vec2(0.0f, fy * PhysicsManager::GAME_TO_PHYSICS_SCALE),
			mBody->GetPosition(), true);
}

void PhysicsComponent::postStep() {
	b2Vec2 position = mBody->GetPosition();

	GameObject &gameObject = getGameObject();
	gameObject.setX(
			position.x / PhysicsManager::GAME_TO_PHYSICS_SCALE
					- 0.5f * gameObject.w());
	gameObject.setY(
			position.y / PhysicsManager::GAME_TO_PHYSICS_SCALE
					- 0.5f * gameObject.h());
}

b2Body*
PhysicsComponent::getBody() {
	return mBody;
}
