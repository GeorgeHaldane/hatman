#include "geometry_utils.h"

#include <cmath> // 'sqrt()' function (Vector2 length calculation)
#include <cstdlib> // 'abs()' for int, 'rand()'
#include <time.h> // used to generate seed for random



// # Vector2 #
Vector2::Vector2() : x(0), y(0) {}
Vector2::Vector2(int X, int Y) : x(X), y(Y) {}

Vector2 Vector2::operator + (const Vector2 &other) const {
	return Vector2(
		this->x + other.x,
		this->y + other.y
	);
}
Vector2 Vector2::operator - (const Vector2 &other) const {
	return Vector2(
		this->x - other.x, 
		this->y - other.y
	);
}
Vector2 Vector2::operator * (double value) const {
	return Vector2(
		static_cast<int>(this->x * value),
		static_cast<int>(this->y * value)
	);
}
Vector2 Vector2::operator / (double value) const {
	return Vector2(
		static_cast<int>(static_cast<double>(this->x) / value),
		static_cast<int>(static_cast<double>(this->y) / value)
	);
}
Vector2& Vector2::operator += (const Vector2 &other) {
	this->x += other.x;
	this->y += other.y;
	return *this;
}
Vector2& Vector2::operator -= (const Vector2 &other) {
	this->x -= other.x;
	this->y -= other.y;
	return *this;
}

#pragma warning( push ) // disable warning for 'double' to 'int' conversion for next 2 operators
#pragma warning( disable : 4244 )

Vector2& Vector2::operator *= (double value) {
	this->x *= value;
	this->y *= value;
	return *this;
}
Vector2& Vector2::operator /= (double value) {
	this->x /= value;
	this->y /= value;
	return *this;
}

#pragma warning( pop )

Vector2& Vector2::set(int x, int y) {
	this->x = x;
	this->y = y;
	return *this;
}

int Vector2::length2() const {
	return this->x * this->x + this->y * this->y;
}

double Vector2::length() const {
	return std::sqrt(this->x * this->x + this->y * this->y);
}

Vector2 Vector2::normalized() const {
	return (*this / this->length());
}



// # Vector2d #
Vector2d::Vector2d() : x(0), y(0) {}
Vector2d::Vector2d(double X, double Y) : x(X), y(Y) {}
Vector2d::Vector2d(const Vector2 &vector) :	x(vector.x), y(vector.y) {}

Vector2 Vector2d::toVector2() const {
	return Vector2(
		static_cast<int>(this->x),
		static_cast<int>(this->y)
	);
}

Vector2d Vector2d::operator + (const Vector2d &other) const {
	return Vector2d(
		this->x + other.x,
		this->y + other.y
	);
}
Vector2d Vector2d::operator - (const Vector2d &other) const {
	return Vector2d(
		this->x - other.x,
		this->y - other.y
	);
}
Vector2d Vector2d::operator * (double value) const {
	return Vector2d(
		this->x * value,
		this->y * value
	);
}
Vector2d Vector2d::operator / (double value) const {
	return Vector2d(
		this->x / value,
		this->y / value
	);
}
Vector2d& Vector2d::operator += (const Vector2d &other) {
	this->x += other.x;
	this->y += other.y;
	return *this;
}
Vector2d& Vector2d::operator -= (const Vector2d &other) {
	this->x -= other.x;
	this->y -= other.y;
	return *this;
}
Vector2d& Vector2d::operator *= (double value) {
	this->x *= value;
	this->y *= value;
	return *this;
}
Vector2d& Vector2d::operator /= (double value) {
	this->x /= value;
	this->y /= value;
	return *this;
}

Vector2d& Vector2d::set(double x, double y) {
	this->x = x;
	this->y = y;
	return *this;
}

double Vector2d::length2() const {
	return this->x * this->x + this->y * this->y;
}

int Vector2d::length2_rough() const {
	return (int)this->x * (int)this->x + (int)this->y * (int)this->y;
}

double Vector2d::length() const {
	return std::sqrt(this->x * this->x + this->y * this->y);
}

Vector2d Vector2d::normalized() const {
	return (*this / this->length());
}



// helpers::
Side helpers::invert(Side side) {
	return static_cast<Side>(-static_cast<int>(side));
}

Orientation helpers::invert(Orientation orientation) {
	return static_cast<Orientation>(-static_cast<int>(orientation));
}

int helpers::dice(int min, int max) {
	static bool firstExecution = true;

	if (firstExecution) {
		srand(time(nullptr));
		firstExecution = false;
	}

	return min + rand() % (max - min + 1);
}



// # Rectangle #
Rectangle::Rectangle(const Vector2 &point, const Vector2 &dimensions, const bool initializeAsCentered) {
	if (initializeAsCentered) {
		this->point1 = point - dimensions / 2;
		this->point2 = point + dimensions / 2;
	}
	else {
		this->point1 = point;
		this->point2 = point + dimensions;
	}
}
Rectangle::Rectangle(int pointX, int pointY, int sizeX, int sizeY, bool initializeAsCentered) :
	Rectangle(Vector2(pointX, pointY), Vector2(sizeX, sizeY), initializeAsCentered)
{}


void Rectangle::moveTo(const Vector2 &newPoint1) {
	const Vector2 dimensions = this->getDimensions();
	this->point1 = newPoint1;
	this->point2 = newPoint1 + dimensions;
}
void Rectangle::moveTo(int newPoint1X, int newPoint1Y) { this->moveTo(Vector2(newPoint1X, newPoint1Y)); }
void Rectangle::moveCenterTo(const Vector2 &newCenter) {
	const Vector2 dimensions = this->getDimensions();
	this->point1 = newCenter - dimensions / 2;
	this->point2 = newCenter + dimensions / 2;
}
void Rectangle::moveBy(const Vector2 &movement) {
	this->point1 += movement;
	this->point2 += movement;
}
void Rectangle::moveBy(int movementX, int movementY) {
	this->moveBy(Vector2(movementX, movementY));
}
void Rectangle::moveSide(const Side side, const int newValue) {
	const Vector2 dimensions = this->getDimensions();
	switch (side) {
	case Side::LEFT:
		this->point1.x = newValue;
		this->point2.x = newValue + dimensions.x;
		break;
	case Side::RIGHT:
		this->point2.x = newValue;
		this->point1.x = newValue - dimensions.x;
		break;
	case Side::TOP:
		this->point1.y = newValue;
		this->point2.y = newValue + dimensions.y;
		break;
	case Side::BOTTOM:
		this->point2.y = newValue;
		this->point1.y = newValue - dimensions.y;
		break;
	case Side::NONE:
		break;
	}
}


void Rectangle::scaleInPlaceBy(double scale) {
	const Vector2 center = this->getCenter();
	const Vector2 dimensions = this->getDimensions();

	*this = Rectangle(center, dimensions * scale, true);
}
void Rectangle::scaleInPlaceTo(const Vector2 &newDimensions) {
	const Vector2 center = this->getCenter();

	*this = Rectangle(center, newDimensions, true);
}


Vector2 Rectangle::getCorner(const Side sideY, const Side sideX) const {
	if (sideY == Side::TOP && sideX == Side::LEFT) {
		return this->point1;
	}
	else if (sideY == Side::TOP && sideX == Side::RIGHT) {
		return Vector2(point2.x, point1.y);
	}
	else if (sideY == Side::BOTTOM && sideX == Side::LEFT) {
		return Vector2(point1.x, point2.y);
	}
	else if (sideY == Side::BOTTOM && sideX == Side::RIGHT) {
		return point2;
	}
	else {
		return Vector2(0, 0);
	}
}
Vector2 Rectangle::getCorner() const {
	return this->point1;
}
Vector2 Rectangle::getCenter() const {
	return Vector2((this->point1.x + this->point2.x) / 2, (this->point1.y + this->point2.y) / 2);
}
Vector2 Rectangle::getDimensions() const {
	return point2 - point1;
}
int Rectangle::getSide(const Side side) const {
	return
		side == Side::LEFT ? this->point1.x :
		side == Side::RIGHT ? this->point2.x :
		side == Side::TOP ? this->point1.y :
		side == Side::BOTTOM ? this->point2.y :
		-1;
}
Vector2 Rectangle::getSideMiddle(Side side) const {
	switch (side) {
	case Side::BOTTOM:
		return Vector2((this->point1.x + this->point2.x) / 2, this->point2.y);
	case Side::TOP:
		return Vector2((this->point1.x + this->point2.x) / 2, this->point1.y);
	case Side::LEFT:
		return Vector2(this->point1.x, (this->point1.y + this->point2.y) / 2);
	case Side::RIGHT:
		return Vector2(this->point2.x, (this->point1.y + this->point2.y) / 2);
	default:
		return Vector2();
	}
}


bool Rectangle::overlapsWithRect(const Rectangle &other) const {
	return
		other.getSide(Side::LEFT) <= this->getSide(Side::RIGHT) &&
		other.getSide(Side::RIGHT) >= this->getSide(Side::LEFT) &&
		other.getSide(Side::TOP) <= this->getSide(Side::BOTTOM) &&
		other.getSide(Side::BOTTOM) >= this->getSide(Side::TOP);
}

Side Rectangle::getCollisionSide(const Rectangle &other) const {
	// Count the difference between opposite sides, minimal one marks the collision side
	const int differenceLeft = std::abs(this->getSide(Side::LEFT) - other.getSide(Side::RIGHT));
	const int differenceRight = std::abs(this->getSide(Side::RIGHT) - other.getSide(Side::LEFT));
	const int differenceTop = std::abs(this->getSide(Side::TOP) - other.getSide(Side::BOTTOM));
	const int differenceBottom = std::abs(this->getSide(Side::BOTTOM) - other.getSide(Side::TOP));
	const int values[4] = { differenceLeft, differenceRight, differenceTop, differenceBottom };
	int minIndex = 0;
	for (int i = 1; i < 4; ++i) {
		if (values[i] <= values[minIndex]) {
			minIndex = i;
		}
	}
	return
		minIndex == 0 ? Side::LEFT :
		minIndex == 1 ? Side::RIGHT :
		minIndex == 2 ? Side::TOP :
		minIndex == 3 ? Side::BOTTOM :
		Side::NONE;
}
bool Rectangle::containsPoint(const Vector2d &point) const {
	return
		(this->getSide(Side::LEFT) < point.x) &&
		(this->getSide(Side::RIGHT) > point.x) &&
		(this->getSide(Side::TOP) < point.y) &&
		(this->getSide(Side::BOTTOM) > point.y);
}

SDL_Rect Rectangle::toSDLRect() const {	
	return SDL_Rect{ this->point1.x, this->point1.y, this->getDimensions().x, this->getDimensions().y };
}