#include "stats.h"



// # Fraction #
bool areEnemies(Fraction fraction1, Fraction fraction2) { // returns if two fractions are enemies
	if (fraction1 != fraction2) {
		return true;
	}
	else {
		return (fraction1 == Fraction::NONE || fraction2 == Fraction::NONE);
	}
} 



// # Damage #
Damage::Damage(Fraction fraction, double phys, double magic, double dot, double pure) :
	fraction(fraction),
	phys(phys),
	magic(magic),
	dot(dot),
	pure(pure)
{}

Damage& Damage::operator+=(const Damage &other) {
	this->phys += other.phys;
	this->magic += other.magic;
	this->dot += other.dot;
	this->pure += other.pure;

	return *this;
}
Damage Damage::operator+(const Damage &other) {
	return Damage(
		this->fraction,
		this->phys + other.phys,
		this->magic + other.magic,
		this->dot + other.dot,
		this->pure + other.pure
	);
}
Damage& Damage::operator*=(double modifier) {
	this->phys *= modifier;
	this->magic *= modifier;
	this->dot *= modifier;
	this->pure *= modifier;

	return *this;
}
Damage Damage::operator*(double modifier) {
	return Damage(
		this->fraction,
		this->phys * modifier,
		this->magic * modifier,
		this->dot * modifier,
		this->pure * modifier
	);
}

Damage& Damage::modify(double modifierPhys, double modifierMagic, double modifierDot, double modifierPure) {
	this->phys *= modifierPhys;
	this->magic *= modifierMagic;
	this->dot *= modifierDot;
	this->pure *= modifierPure;

	return *this;
}



// # Health #
const sint MAX_RESISTANCE = 90;

Health::Health(Fraction fraction, uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes) :
	fraction(fraction),
	base_maxHp(maxHp),
	base_regen(regen),
	base_physRes(physRes),
	base_magicRes(magicRes),
	base_dotRes(dotRes)
{
	this->recalc();

	this->hp = this->total_maxHp;
}

void Health::update(Milliseconds elapsedTime) {
	this->applyHeal(static_cast<double>(total_regen) / 1000.0 * elapsedTime); // account for time elapsed
}

void Health::setFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes) {
	this->flat_maxHp = maxHp;
	this->flat_regen = regen;
	this->flat_physRes = physRes;
	this->flat_magicRes = magicRes;
	this->flat_dotRes = dotRes;

	this->recalc();
}
void Health::addFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes) {
	this->flat_maxHp += maxHp;
	this->flat_regen += regen;
	this->flat_physRes += physRes;
	this->flat_magicRes += magicRes;
	this->flat_dotRes += dotRes;

	this->recalc();
}

void Health::setMulti(double maxHp, double regen, double physRes, double magicRes, double dotRes) {
	this->multi_maxHp = maxHp;
	this->multi_regen = regen;
	this->multi_physRes = physRes;
	this->multi_magicRes = magicRes;
	this->multi_dotRes = dotRes;

	this->recalc();
}
void Health::addMulti(double maxHp, double regen, double physRes, double magicRes, double dotRes) {
	this->multi_maxHp += maxHp;
	this->multi_regen += regen;
	this->multi_physRes += physRes;
	this->multi_magicRes += magicRes;
	this->multi_dotRes += dotRes;

	this->recalc();
}

void Health::recalc() {
	const double currentHpPercentage = this->hp / this->total_maxHp;

	this->total_maxHp = static_cast<uint>((this->base_maxHp + this->flat_maxHp) * (1.0 + this->multi_maxHp));
	this->total_regen = static_cast<sint>((this->base_regen + this->flat_regen) * (1.0 + this->multi_regen));
	this->total_physRes = static_cast<sint>((this->base_physRes + this->flat_physRes) * (1.0 + this->multi_physRes));
	this->total_magicRes = static_cast<sint>((this->base_magicRes + this->flat_magicRes) * (1.0 + this->multi_magicRes));
	this->total_dotRes = static_cast<sint>((this->base_dotRes + this->flat_dotRes) * (1.0 + this->multi_dotRes));

	this->hp = this->total_maxHp * currentHpPercentage;

	// Cap resistances
	if (this->total_physRes > MAX_RESISTANCE) { this->total_physRes = MAX_RESISTANCE; }
	if (this->total_magicRes > MAX_RESISTANCE) { this->total_magicRes = MAX_RESISTANCE; }
	if (this->total_dotRes > MAX_RESISTANCE) { this->total_dotRes = MAX_RESISTANCE; }
}

void Health::applyDamage(const Damage &damage) {
	if (areEnemies(this->fraction, damage.fraction)) {
		this->hp -= damage.phys * (1.0 - static_cast<double>(total_physRes) / 100.0); // phys part
		this->hp -= damage.magic * (1.0 - static_cast<double>(total_magicRes) / 100.0); // magic part
		this->hp -= damage.dot * (1.0 - static_cast<double>(total_dotRes) / 100.0); // dot part
		this->hp -= damage.pure; // pure part
	}
}

void Health::applyHeal(double heal) {
	

	if (this->hp < this->total_maxHp) {
		this->hp += heal;

		// Doesn't let hp overcap
		if (this->hp > this->total_maxHp) {
			this->hp = this->total_maxHp;
		}
	}
}

bool Health::dead() const {
	return (this->hp <= 0);
}
double Health::percentage() const {
	return this->hp / this->total_maxHp;
}