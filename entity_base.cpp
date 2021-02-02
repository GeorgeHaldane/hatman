#include "entity_base.h"



// # Entity #
Entity::Entity(const Vector2d &position) :
	sprite(nullptr),
	solid(nullptr),
	health(nullptr),
	position(position)
{}

void Entity::update(Milliseconds elapsedTime) {
	if (this->enabled) {
		if (this->sprite) { this->sprite->update(elapsedTime); }
		if (this->solid) { this->solid->update(elapsedTime); }
		if (this->health) { this->health->update(elapsedTime); }	
	}
}
void Entity::draw() const {
	if (this->enabled) {
		if (this->sprite) { this->sprite->draw(); }
	}
}

void Entity::mark_for_erase() {
	this->erase_mark = true;
	this->enabled = false;
}
bool Entity::marked_for_erase() {
	return this->erase_mark;
}