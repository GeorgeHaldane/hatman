#pragma once

#include <memory> // types

#include "geometry_utils.h" // types
#include "stats.h" // module 'Health'
#include "sprite.h" // module 'ControllableSprite'
#include "solid.h" // module 'SolidRectangle'



// # Entity #
// - ABSTRACT
// - Base class for all entity types
// - Contains a number of optional modules
class Entity {
public:
	Entity() = delete;

	Entity(const Vector2d &position);

	virtual ~Entity() = default;

	virtual void update(Milliseconds elapsedTime); // updates animation-related values
	virtual void draw() const; // draws a correct frame of current animation

	void mark_for_erase(); // marks entity for erasion
	bool marked_for_erase(); // returns whether entity should be erased


	Vector2d position; // position in a level

	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<SolidRectangle> solid;
	std::unique_ptr<Health> health;

	bool enabled = true; // entity doesn't update/draw if disabled

protected:
	bool erase_mark = false;
};