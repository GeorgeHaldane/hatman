#pragma once

#include <unordered_set> // related type

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types



// # SolidFlags #
// - Enum of all flags that apply to solids and determine their behaviour
enum class SolidFlags {
	SOLID_FOR_TILES,
	SOLID_FOR_BORDER,
	AFFECTED_BY_GRAVITY
};



// # SolidRectangle #
// - Represents a rectangle with physics attached to it
// - Behaviour depends on active flags
class SolidRectangle {
public:
	

	SolidRectangle() = delete;

	SolidRectangle(Vector2d &parentPosition, const Vector2 &hitboxSize, const std::initializer_list<SolidFlags> &flags, double mass, double friction);

	void update(Milliseconds elapsedTime);

	Rectangle getHitbox() const;

	Vector2 hitboxSize; // hitbox rectangle with a center in parent_position

	Vector2d &parent_position; // position of the object module is attached to
	Vector2d speed;
	Vector2d acceleration; // acceleration not accounting for constants aka gravity

	void applyForce(const Vector2d &force);
	void applyImpulse(const Vector2d &impulse);

	// Properties
	double mass;
	double friction; // slows down grounded objects horizontaly by its value per second

	bool isGrounded;

private:
	Vector2d total_force;
	
	void apply_Gravity(); // applies gravity force
	void apply_Friction(); // applies friction force
	void apply_TileCollisions();
	void apply_LevelBorderCollisions();

	std::unordered_set<SolidFlags> flags;
};