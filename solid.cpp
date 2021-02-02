#include "solid.h"

#include "game.h" // access to timescale and game state
#include "globalconsts.hpp" // contains tile size (used in tile collision detection)



// # Entity::Physics #
SolidRectangle::SolidRectangle(Vector2d &parentPosition, const Vector2 &hitboxSize, const std::initializer_list<SolidFlags> &flags, double mass = 1, double friction = 0) :
	parent_position(parentPosition),
	hitboxSize(hitboxSize),
	flags(flags),
	mass(mass),
	friction(friction),
	isGrounded(false),
	total_force(0, 0)
{}

void SolidRectangle::update(Milliseconds elapsedTime) {
	// Apply forces
	if (this->flags.count(SolidFlags::AFFECTED_BY_GRAVITY)) { this->apply_Gravity(); }
	if (this->isGrounded) { this->apply_Friction(); }

	// Apply position change
	this->acceleration = this->total_force / this->mass;
	this->total_force = Vector2d(0, 0);

	this->speed += this->acceleration * per_second(elapsedTime);
	this->parent_position += this->speed * per_second(elapsedTime);	

	// Apply interaction with objects
	if (this->flags.count(SolidFlags::SOLID_FOR_TILES)) { this->apply_TileCollisions(); }
	if (this->flags.count(SolidFlags::SOLID_FOR_BORDER)) { this->apply_LevelBorderCollisions(); }
}

Rectangle SolidRectangle::getHitbox() const {
	return Rectangle(parent_position.toVector2(), this->hitboxSize, true);
}

void SolidRectangle::applyForce(const Vector2d &force) {
	this->total_force += force;
}
void SolidRectangle::applyImpulse(const Vector2d &impulse) {
	this->speed += impulse / this->mass;
}

void SolidRectangle::apply_Gravity() {
	//this->speed.y += per_second(physics::GRAVITY_ACCELERATION) * elapsedTime;
	this->applyForce(Vector2d(0, this->mass * physics::GRAVITY_ACCELERATION));
}
void SolidRectangle::apply_Friction() {
	if (std::abs(this->speed.x) < 2) this->speed.x = 0; // ensures speed converges at 0 due to friction

	this->applyForce(Vector2d(
		-helpers::sign(this->speed.x) * this->mass * physics::GRAVITY_ACCELERATION * this->friction,
		0
	));
}
void SolidRectangle::apply_TileCollisions() {
	bool collidedAtBottom = false;

	for (const auto &tile : Game::ACCESS->level.tiles) {
		if (tile.hitbox) { // no need to handle if tile has no hitbox
			const Rectangle entityHitbox = this->getHitbox();

			for (const auto& hitboxRect : tile.hitbox->rectangles) {
				if (entityHitbox.overlapsWithRect(hitboxRect)) {
					const Side collisionSide = entityHitbox.getCollisionSide(hitboxRect);
		 			
					if (collisionSide == Side::BOTTOM) { // This case goes first as the most likely
						parent_position.y = (double)hitboxRect.getSide(Side::TOP) - ((double)entityHitbox.getDimensions().y / 2.0);
						collidedAtBottom = true;
						this->speed.y = 0.0;
					}
					else if (collisionSide == Side::TOP) {
						parent_position.y = (double)hitboxRect.getSide(Side::BOTTOM) + ((double)entityHitbox.getDimensions().y / 2.0) + 1.0;
						this->speed.y = 0.0;
					}
					else if (collisionSide == Side::LEFT) {
						parent_position.x = (double)hitboxRect.getSide(Side::RIGHT) + ((double)entityHitbox.getDimensions().x / 2.0) + 1.0;
						this->speed.x = 0.0;
					}
					else if (collisionSide == Side::RIGHT) {
						parent_position.x = (double)hitboxRect.getSide(Side::LEFT) - ((double)entityHitbox.getDimensions().x / 2.0) - 1.0;
						this->speed.x = 0.0;
					}
				}
			}
		}
	}

	this->isGrounded = collidedAtBottom;
}
void SolidRectangle::apply_LevelBorderCollisions() {
	const Rectangle entityHitbox = this->getHitbox();
	const int levelWidth = Game::ACCESS->level.getSize().x * rendering::TILE_SIZE; // Assumes rendering size is equal to physica;
	const int levelHeight = Game::ACCESS->level.getSize().y * rendering::TILE_SIZE;

	if (entityHitbox.getSide(Side::LEFT) < 0) {
		parent_position.x = entityHitbox.getDimensions().x / 2.0;
	}
	else if (entityHitbox.getSide(Side::RIGHT) > levelWidth) {
		parent_position.x = levelWidth - entityHitbox.getDimensions().x / 2.0;
	}

	if (entityHitbox.getSide(Side::TOP) < 0) {
		parent_position.y = entityHitbox.getDimensions().y / 2.0;
	}
	else if (entityHitbox.getSide(Side::BOTTOM) > levelHeight) {
		parent_position.y = levelHeight - entityHitbox.getDimensions().y / 2.0;
	}
}