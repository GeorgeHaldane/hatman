#pragma once

/* Contains module: 'Health' */

#include "timer.h" // 'Milliseconds' type



using uint = unsigned int; // typedefs for convenient future refactioring
using sint = int;



// # Fraction #
// - Enum that represents entity fraction
// - Any entity with non-'NONE' fraction receives no damage from entities of the same fraction
enum class Fraction {
	// NONE fraction is an enemy to everything including itself
	NONE,
	// All other fractions are enemies everything but themselves
	PLAYER,
	UNDEAD
};



// # Damage #
// - Represents a damage packet
struct Damage {
	Damage() = default;

	Damage(Fraction fraction, double phys = 0, double magic = 0, double dot = 0, double pure = 0);

	Damage& operator+=(const Damage &other);
	Damage operator+(const Damage &other);
	Damage& operator*=(double modifier);
	Damage operator*(double modifier);

	Damage& modify(double modifierPhys, double modifierMagic, double modifierDot, double modifierPure);

	double phys;
	double magic;
	double dot;
	double pure;

	Fraction fraction;
};



// # Health #
// - MODULE
// - Containt health and resists with all their modifiers
// - Calculated and applies damage taken
class Health {
public:
	Health() = delete;

	Health(Fraction fraction, uint maxHp, sint regen = 0, sint physRes = 0, sint magicRes = 0, sint dotRes = 0); // sets base stats

	void update(Milliseconds elapsedTime);

	void setFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes);
	void addFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes);

	void setMulti(double maxHp, double regen, double physRes, double magicRes, double dotRes);
	void addMulti(double maxHp, double regen, double physRes, double magicRes, double dotRes);

	void applyDamage(const Damage &damage);
	void applyHeal(double heal);

	// Getters
	bool dead() const; // true if hp < 0
	double percentage() const; // returns values from 0.0 to 1.0

	double hp; // current hp

	Fraction fraction; // health receives no damage from the same fraction
private:
	// Base values
	uint base_maxHp;
	sint base_regen;
	sint base_physRes;
	sint base_magicRes;
	sint base_dotRes;

	// Flat mods
	uint flat_maxHp = 0;
	sint flat_regen = 0;
	sint flat_physRes = 0;
	sint flat_magicRes = 0;
	sint flat_dotRes = 0;

	// Multiplicative mods
	double multi_maxHp = 0;
	double multi_regen = 0;
	double multi_physRes = 0;
	double multi_magicRes = 0;
	double multi_dotRes = 0;

	// Total values
	uint total_maxHp;
	sint total_regen;
	sint total_physRes;
	sint total_magicRes;
	sint total_dotRes;

	void recalc(); // recalculates total values
};