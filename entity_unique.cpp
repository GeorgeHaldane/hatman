#include "entity_unique.h"

#include <functional> // functional types (derived object creation)
#include <unordered_map> // related type (derived object creation)

#include "game.h" // access to game state
#include "controls.h" // access to control keys
#include "globalconsts.hpp"



/* ### CONTROLLERS ### */

typedef std::function<std::unique_ptr<Entity>(const Vector2d&)> make_derived_ptr;

// std::make_unique() wrapper
template<class UniqueEntity>
std::unique_ptr<UniqueEntity> make_derived(const Vector2d &position) {
	return std::make_unique<UniqueEntity>(position);
}

// !!! NAMES !!!
const std::unordered_map<std::string, make_derived_ptr> ENTITY_MAKERS = {
	{"item-brass_relic", make_derived<entities::items::BrassRelic>},
	{"item-paper", make_derived<entities::items::Paper>},
	{"destructible-tnt", make_derived<entities::destructibles::TNT>},
	{"enemy-ghost", make_derived<entities::enemies::Ghost>},
	{"enemy-sludge",make_derived<entities::enemies::Sludge>}
	/// new entities go there
};

std::unique_ptr<Entity> entities::make_entity(const std::string &type, const std::string &name, const Vector2d &position) {
	return ENTITY_MAKERS.at(type + '-' + name)(position); // type and name are joined into a single string
}



/* ### ITEMS ### */

// # BrassRelic #
entities::items::BrassRelic::BrassRelic(const Vector2d &position) :
	ItemEntity(position)
{
	// Init members
	this->name = "brass_relic";

	// Init modules
	const Vector2 spriteSize(14, 10);

	this->_init_sprite("brass_relic.png", {
		std::make_pair(Rectangle(Vector2(0, 0), spriteSize), 3000),
		std::make_pair(Rectangle(Vector2(0, 10), spriteSize), 100),
		std::make_pair(Rectangle(Vector2(14, 10), spriteSize), 100),
		std::make_pair(Rectangle(Vector2(28, 10), spriteSize), 100),
		std::make_pair(Rectangle(Vector2(42, 10), spriteSize), 100)
		});
	
	this->_init_solid(Vector2(14, 10));
}



// # Paper #
entities::items::Paper::Paper(const Vector2d &position) :
	ItemEntity(position)
{
	// Init members
	this->name = "paper";

	// Init modules
	this->_init_sprite("paper.png");

	this->_init_solid(Vector2(14, 14));
}



/* ### DESTRUCTIBLES ### */

// # TNT #
entities::destructibles::TNT::TNT(const Vector2d &position) :
	Destructible(position)
{
	// Init members
	this->_init_delay(300);

	// Init modules
	this->_init_sprite(
		"tnt.png",
		{
			std::make_pair(Rectangle(0, 0, 15, 15), 0)
		},
		{
			std::make_pair(Rectangle(0, 15, 30, 30), 100),
			std::make_pair(Rectangle(30, 15, 30, 30), 100),
			std::make_pair(Rectangle(60, 15, 30, 30), 100)
		});

	this->_init_solid(Vector2(15, 15));

	this->_init_health(Fraction::NONE, 700);
}

void entities::destructibles::TNT::effect() {
	const Rectangle area(this->position.toVector2(), Vector2(50, 50), true);
	const Damage damage(this->health->fraction, 400);
	Game::ACCESS->level.damageInArea(area, damage);
}



/* ### ENEMIES ### */

// # Ghost #
namespace ghost_consts {
	// Physics
	const double MASS = 40;
	const double FRICTION = 0;

	// Stats
	const Fraction FRACTION = Fraction::UNDEAD;
	const uint MAX_HP = 800;
	const sint REGEN = 0;
	const sint PHYS_RES = 50;
	const sint MAGIC_RES = 0;
	const sint DOT_RES = 50;

	// Behaviour
	const int AGGRO_RANGE2 = 150 * 150;
	const int DEAGGRO_RANGE2 = 500 * 500;

	const double MAX_MOVESPEED2 = 150 * 150;
	const double MOVE_FORCE = MASS * 150; // no friction compensation needed

	const Vector2 ATTACK_RANGE(20, 5);
}

entities::enemies::Ghost::Ghost(const Vector2d &position) :
	Enemy(position)
{
	// Init modules
	this->_init_sprite("ghost.png");

	this->_init_solid(
		Vector2(16, 16),
		{ SolidFlags::SOLID_FOR_BORDER },
		ghost_consts::MASS,
		ghost_consts::FRICTION
	);

	this->_init_health(
		ghost_consts::FRACTION,
		ghost_consts::MAX_HP,
		ghost_consts::REGEN,
		ghost_consts::PHYS_RES,
		ghost_consts::MAGIC_RES,
		ghost_consts::DOT_RES
	);

	// Init members
	const Vector2 spriteSize(32, 32);

	this->_init_animation(
		"idle",
		Rectangle(Vector2(0, 0), spriteSize)
		);

	this->_init_animation(
		"slash",
		{
			std::make_pair(Rectangle(Vector2(0, 0), spriteSize), 50),
			std::make_pair(Rectangle(Vector2(32, 0), spriteSize), 100),
			std::make_pair(Rectangle(Vector2(64, 0), spriteSize), 100)
		});

	// Setup skills
	this->skill_map["slash"] = std::make_unique<skills::Slash>(this);

	// Setup unique members
	this->playAnimation("idle");
}

bool entities::enemies::Ghost::aggroCheck() {
	return (this->target_relative_pos.length2_rough() < ghost_consts::AGGRO_RANGE2);
}
bool entities::enemies::Ghost::deaggroCheck() {
	return (this->target_relative_pos.length2_rough() > ghost_consts::DEAGGRO_RANGE2);
}

void entities::enemies::Ghost::wander(Milliseconds elapsedTime) {
	/// !!! IMPLEMENT !!!

	// No special wander animation present
}

bool entities::enemies::Ghost::canAttack() {
	return (
		std::abs(this->target_relative_pos.x) < ghost_consts::ATTACK_RANGE.x &&
		std::abs(this->target_relative_pos.y) < ghost_consts::ATTACK_RANGE.y
		);
}

void entities::enemies::Ghost::chase(Milliseconds elapsedTime) {
	// Calculate new orientation
	if (this->target_relative_pos.x > 0) {
		this->orientation = Orientation::RIGHT;
	}
	else {
		this->orientation = Orientation::LEFT;
	}

	// Move
	if (std::abs(this->solid->speed.length2()) < ghost_consts::MAX_MOVESPEED2) {
		this->solid->applyForce(this->target_relative_pos.normalized() * ghost_consts::MOVE_FORCE);
	} /// MAY NEED TO HANDLE EDGE CASE LATER

	// No special chase animation present
}
void entities::enemies::Ghost::attack(Milliseconds elapsedTime) {
	// Use 'slash'
	if (this->skill_map.at("slash")->ready()) {

		this->skill_map.at("slash")->use();

		this->playAnimation("slash");
		this->animation_lock_timer.start(250);
	}
}

void entities::enemies::Ghost::onDeath() {}



// # Sludge #
namespace sludge_consts {
	// Physics
	const double MASS = 400;
	const double FRICTION = 0.6;

	// Stats
	const Fraction FRACTION = Fraction::UNDEAD;
	const uint MAX_HP = 1500;
	const sint REGEN = 10;
	const sint PHYS_RES = 50;
	const sint MAGIC_RES = 0;
	const sint DOT_RES = 0;

	// Behaviour
	const int AGGRO_RANGE2 = 100 * 100;
	const int DEAGGRO_RANGE2 = 200 * 200;

	const double WANDER_MOVESPEED2 = 10 * 10;
	const int WANDER_MIN_WAIT = 2000;
	const int WANDER_MAX_WAIT = 4000;
	const int WANDER_MIN_MOVE = 800;
	const int WANDER_MAX_MOVE = 2000;

	const double CHASE_MOVESPEED2 = 50 * 50;
	const double MOVE_FORCE =
		MASS * physics::GRAVITY_ACCELERATION * FRICTION // friction compensation
		+ MASS * 40;

	const int ATTACK_RANGE2 = 14 * 14;
}

entities::enemies::Sludge::Sludge(const Vector2d &position) :
	Enemy(position),
	wander_move(false)
{
	// Init modules
	this->_init_sprite("sludge.png");

	this->_init_solid(
		Vector2(16, 16),
		{
			SolidFlags::SOLID_FOR_BORDER,
			SolidFlags::SOLID_FOR_TILES,
			SolidFlags::AFFECTED_BY_GRAVITY
		},
		sludge_consts::MASS,
		sludge_consts::FRICTION
	);

	this->_init_health(
		sludge_consts::FRACTION,
		sludge_consts::MAX_HP,
		sludge_consts::REGEN,
		sludge_consts::PHYS_RES,
		sludge_consts::MAGIC_RES,
		sludge_consts::DOT_RES
	);

	// Init members
	const Vector2 spriteSize(16, 16);

	this->_init_animation(
		"idle",
		{
			std::make_pair(Rectangle(Vector2(0, 0), spriteSize), 250),
			std::make_pair(Rectangle(Vector2(16, 0), spriteSize), 250),
			std::make_pair(Rectangle(Vector2(32, 0), spriteSize), 250),
			std::make_pair(Rectangle(Vector2(48, 0), spriteSize), 250)
		});

	// Setup skills
	this->skill_map["knockbackAOE"] = std::make_unique<skills::KnockbackAOE>(this);

	// Setup unique members
	this->playAnimation("idle");
}

bool entities::enemies::Sludge::aggroCheck() {
	return (this->target_relative_pos.length2_rough() < sludge_consts::AGGRO_RANGE2);
}
bool entities::enemies::Sludge::deaggroCheck() {
	return (this->target_relative_pos.length2_rough() > sludge_consts::DEAGGRO_RANGE2);
}

void entities::enemies::Sludge::wander(Milliseconds elapsedTime) {
	if (this->wander_timer.finished()) {
		this->wander_move = helpers::dice(0, 1);

		if (this->wander_move) {
			this->orientation = helpers::invert(this->orientation);
			this->wander_timer.start(helpers::dice(sludge_consts::WANDER_MIN_MOVE, sludge_consts::WANDER_MAX_MOVE));
		}
		else {
			this->wander_timer.start(helpers::dice(sludge_consts::WANDER_MIN_WAIT, sludge_consts::WANDER_MAX_WAIT));
		}
		
		
	}
	else {
		if (this->wander_move && this->solid->isGrounded) { // only move when grounded and 'wander_move' is true
			if (std::abs(this->solid->speed.length2()) < sludge_consts::WANDER_MOVESPEED2) { // limit speed
				this->solid->applyForce(Vector2d(static_cast<int>(this->orientation) * sludge_consts::MOVE_FORCE, 0));
			}
		}
	}

	// No special wander animation present
}

bool entities::enemies::Sludge::canAttack() {
	return (this->target_relative_pos.length2_rough() < sludge_consts::ATTACK_RANGE2);
}

void entities::enemies::Sludge::chase(Milliseconds elapsedTime) {
	// Calculate new orientation
	if (this->target_relative_pos.x > 0) {
		this->orientation = Orientation::RIGHT;
	}
	else {
		this->orientation = Orientation::LEFT;
	}

	// Move
	if (this->solid->isGrounded) { // only move when grounded
		if (std::abs(this->solid->speed.length2()) < sludge_consts::CHASE_MOVESPEED2) { // limit speed
			this->solid->applyForce(Vector2d(static_cast<int>(this->orientation) * sludge_consts::MOVE_FORCE, 0));
		} /// MAY NEED TO HANDLE EDGE CASE LATER
	}

	// No special chase animation present
}
void entities::enemies::Sludge::attack(Milliseconds elapsedTime) {
	// Use 'knockbackAOE'
	if (this->skill_map.at("knockbackAOE")->ready()) {
		this->skill_map.at("knockbackAOE")->use();
	}

	// No special attack animation present
}

void entities::enemies::Sludge::onDeath() {}