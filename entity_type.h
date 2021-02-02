#pragma once

/* Contantains all entity types (items, enemies, NPC, etc) */

#include "entity_base.h" // 'Entity' base class
#include "skill.h" // 'Skill' class and derived classes
#include "inventory.h" // 'Inventory' module (player forms)



class Skill; // forward declare stuff



namespace entity_primitive_types {

	// # Creature #
	// - Entity with conrollable sprite, solid and health
	// - Has a LEFT/RIGHT orientation
	class Creature : public Entity {
	public:
		Creature() = delete;

		Creature(const Vector2d &position);

		virtual void update(Milliseconds elapsedTime);
		void draw() const override; // also draws skills

		Orientation orientation = Orientation::RIGHT; // flips sprite automatically

	protected:
		// Module inits
		void _init_sprite(const std::string &imageFileName);
			// inits controllable sprite, animations are added manualy in derived instances
		void _init_solid(const Vector2 &hitboxSize, const std::initializer_list<SolidFlags> &flags, double mass, double friction);
		void _init_health(Fraction fraction, uint maxHp, sint regen, sint physRes = 0, sint magicRes = 0, sint dotRes = 0);

		// Member inits
		void _init_animation(const std::string &name, const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames);
		void _init_animation(const std::string &name, const Rectangle &frame);
			// multiple calls may be needed to setup all animations

		void playAnimation(const std::string &name, bool once = false); // respects animation_lock
		void setOrientation(Orientation orientation); // respects animation_lock

		// Behaviour
		virtual void onDeath(); // some sort of INSTANT behaviour triggered upon death
			/// IMPLEMENT PARTICLES ON DEATH

		std::unordered_map<std::string, std::unique_ptr<Skill>> skill_map;

		Timer animation_lock_timer; // some actions lock animation change for a while

	private:
		ControllableSprite* _sprite; // casted version of 'sprite', used to access methods in 'ControllableSprite'	
	};



	// # Particle #
	// - Uses static OR animated sprite
	// - Solid is optional
	// - Has no health
	// - Has a lifetime (limited or not), erases itself at the end of it
	class Particle : public Entity {
	public:
		Particle() = delete;

		Particle(const Vector2d &position); // creates particle with infinite lifetime
		Particle(const Vector2d &position, int lifetime); // creates particle with finite lifetime

		virtual void update(Milliseconds elapsedTime);

	protected:
		// Module inits
		virtual void _init_sprite(const std::string &imageFileName);
		virtual void _init_sprite(const std::string &imageFileName, const std::initializer_list< std::pair<Rectangle, Milliseconds> > &frames);
			// inits either static or animated sprite

		virtual void _optinit_solid(const Vector2 &hitboxSize, const std::initializer_list<SolidFlags> &flags, double mass, double friction);
			// optional, inits solid

	private:
		bool lifetime_is_limited;
		int lifetime_left;
	};
}



namespace entity_types {

	// # ItemEntity #
	// - Entity used to represent items outside of inventories
	// - Can be picked up by the player
	class ItemEntity : public Entity {
	public:
		ItemEntity() = delete;

		ItemEntity(const Vector2d &position);

		virtual ~ItemEntity() = default;

		void update(Milliseconds elapsedTime) override;

	protected:
		std::string name;

		virtual bool checkActivation() const;  // checks if item should be activated
		virtual bool checkTrigger() const; // checks if trigger condition is true (item must be ativated)
		virtual void activate(); // triggers the item active state
		virtual void trigger(); // triggers item effect (pickup by default)

		// Module inits
		void _init_sprite(const std::string &imageFileName, const std::initializer_list<std::pair<Rectangle, Milliseconds>> &animationFrames);
			// inits animated sprite
		void _init_sprite(const std::string &imageFileName);
			// inits static sprite (whole image is used as a source rect)
		void _init_solid(const Vector2 &hitboxSize);
			// inits solid with standard flags

		// Member inits
		void _init_name(const std::string &name);
	};



	// # Destructible #
	// - Entity with lifebar that triggers some effect upon death
	class Destructible : public Entity { /// NEEDS TWEAKING
	public:
		Destructible() = delete;

		Destructible(const Vector2d &position);

		virtual ~Destructible() = default;

		void update(Milliseconds elapsedTime) override;

	protected:
		virtual void effect(); // any effect that is triggered upon entity death

		bool effect_triggered = false;
		Milliseconds erasion_delay; // delay before entity is erased after its death (ie time for death animation) 
		Timer timer; // handles erasion delay

		// Module inits
		void _init_sprite(
			const std::string &imageFileName,
			const std::initializer_list<std::pair<Rectangle, Milliseconds>> &defaulAnimationFrames,
			const std::initializer_list<std::pair<Rectangle, Milliseconds>> &deathAnimationFrames);
			// inits controllable sprite
		void _init_solid(const Vector2 &hitboxSize);
			// inits solid with standard flags
		void _init_health(Fraction fraction, uint maxHp, sint regen = 0, sint physRes = 0, sint magicRes = 0, sint dotRes = 0);

		// Member inits
		void _init_delay(int erasionDelay);

	private:
		ControllableSprite* _sprite; // casted version of 'sprite', used to access methods in 'ControllableSprite'
	};



	// # Enemy #
	// - Agressive creature that only attacks player
	class Enemy : public entity_primitive_types::Creature {
	public:
		Enemy() = delete;

		Enemy(const Vector2d &position);

		void update(Milliseconds elapsedTime);

	protected:
		// Behaviour
		virtual bool aggroCheck(); // returns whether enemy should get aggro'ed
		virtual bool deaggroCheck(); // returns whether enemy should get deaggro'ed

		virtual void wander(Milliseconds elapsedTime); // behaviour in deaggro'ed state
		
		virtual bool canAttack(); // returns whether target can be attacked

		virtual void chase(Milliseconds elapsedTime);
		virtual void attack(Milliseconds elapsedTime);
		
		virtual void onDeath(); // some sort of INSTANT behaviour triggered upon death

		bool aggroed;
		Creature* target; // always a player
		Vector2d target_relative_pos;
	};
}