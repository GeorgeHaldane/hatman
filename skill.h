#pragma once

#include "entity_type.h" // 'Creature' class (ptr to ability parent entity)
#include "timer.h" // 'Timer' class



namespace entity_primitive_types { class Creature; } // forward declare stuff

using Parent = entity_primitive_types::Creature; // convenience typedef



// # Skill #
// - ABSTRACT
// - Base class for all skills
// - Derived skills use _init methods and 'effect()' override to initize themselves
// - Represents an ability with cooldown and animation
// - Skill object has full controll over it's parent entity
class Skill {
public:
	Skill() = default;

	Skill(Parent* parentCreature);

	void update(Milliseconds elapsedTime);
	void draw() const;

	virtual void use(); // some skills have right/left orientations
	bool ready() const;

protected:
	// Module inits
	void _init_sprite(const std::string &imageFileName, const std::initializer_list<std::pair<Rectangle, Milliseconds>> &animationFrames);
	void _init_sprite(); // when skill has no sprite

	// Member inits
	void _init_cooldown(Milliseconds cooldown);
	void _init_alignment(const Vector2 &alignment);

	Parent* parent_creature;

	Vector2 skill_position() const; // returns sprite position

private:
	std::unique_ptr<ControllableSprite> sprite; // has 1 animation that is played once and ends with a transparent picture
	std::unique_ptr<Vector2d> sprite_position;
		// technical position used to align ability sprite relative to entity
		// = parent_creature.position + this->aligment
		// sprite holds a reference to this object => store through pointer to preserve adress during move
	Vector2 alignment;

	Milliseconds cooldown;
	Timer cooldown_timer;
	Timer animation_timer; // turns of drawing when skill animation is finished
};



namespace skills {

	// # Slam #
	// - Starts at entity feet
	// - Deals AOE damage
	struct Slam : public Skill {
		Slam() = default;

		Slam(Parent* parentEntity);

		void use() override;
	};

	// # Slash #
	struct Slash : public Skill {
		Slash() = default;

		Slash(Parent* parentEntity);

		void use() override;
	};

	// # KnockbackAOE #
	struct KnockbackAOE : public Skill {
		KnockbackAOE() = default;

		KnockbackAOE(Parent* parentEntity);

		void use() override;
	};

}