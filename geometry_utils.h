#pragma once

/* Contains classes that deal with geometry */

#include <SDL.h> // 'SDL_Rect' type (.toSDLRect() method)



// # Vector2 #
// - Used for simplicity when working with integer (x, y) pairs
class Vector2 {
public:
	Vector2(); // constructs { 0, 0 } vector
	Vector2(int X, int Y); // constructs { X, Y } vector

	int x, y;

	// Note that +-*/ have the usual priority
	Vector2 operator + (const Vector2 &other) const;
	Vector2 operator - (const Vector2 &other) const;
	Vector2 operator / (double value) const;
	Vector2 operator * (double value) const;
	Vector2& operator += (const Vector2 &other);
	Vector2& operator -= (const Vector2 &other);
	Vector2& operator *= (double value);
	Vector2& operator /= (double value);

	Vector2& set(int x, int y);

	int length2() const; // square of length

	double length() const; // slower than length2()
	/// length_rough() const; // faster than length()

	Vector2 normalized() const; /// IMPLEMENT OPTIMIZATION
};



// # Vector2d #
// - Used for simplicity when working with double (x, y) pairs
// - Like Vector2 but double
class Vector2d {
public:
	Vector2d();
	Vector2d(double X, double Y);

	Vector2d(const Vector2 &other); // allows convenient conversion to more precise type of vector

	double x, y;

	Vector2 toVector2() const; // When explicit convertion to Vector2 is needed

	// Note that +-*/ have the usual priority
	Vector2d operator + (const Vector2d &other) const;
	Vector2d operator - (const Vector2d &other) const;
	Vector2d operator / (double value) const;
	Vector2d operator * (double value) const;
	Vector2d& operator += (const Vector2d &other);
	Vector2d& operator -= (const Vector2d &other);
	Vector2d& operator *= (double value);
	Vector2d& operator /= (double value);

	Vector2d& set(double x, double y);

	double length2() const; // square of length
	int length2_rough() const; //rough result, but faster

	double length() const; // slower than length2()
	/// int length_rough() const; // faster than length()

	Vector2d normalized() const;
	/// Vector2 normalized_rough(); // faster than .normalized()
};



// # Side #
// - Used to represent sides of a rectangle and directions
enum class Side {
	NONE = 0,
	LEFT = -1,
	RIGHT = 1,
	BOTTOM = -2,
	TOP = 2
};



// # Orientation #
// - Like side but only LEFT/RIGHT
enum class Orientation {
	LEFT = -1,
	RIGHT = 1	
};



// helpers::
// - Holds some small helper functions
namespace helpers {
	Side invert(Side side); // returns opposite side
	Orientation invert(Orientation orientation); // returns opposite orientation

	template<typename T>
	int sign(T val) { // standard 'sign()' function (x<0 => -1, x==0 => 0, x>0 => 1)
		return (T(0) < val) - (val < T(0));
	}

	/// OPTIMIZATIONS MIGHT BE NEEDED LATER
	int dice(int min, int max); // random int between min and max
}



// # Rectangle #
// - Holds info about rectangle with int coordinates
// - Handles most rectangle-related calculations
// - Converts to SDL_Rect
class Rectangle {
public:
	Rectangle(const Vector2 &point = Vector2(0, 0), const Vector2 &dimensions = Vector2(0, 0), bool initializeAsCentered = false); // <point> is either a top left corner or a center depending on <initializeAsCentered> value
	Rectangle(int pointX, int pointY, int sizeX, int sizeY, bool initializeAsCentered = false); // same as above
	
	// Move
	void moveTo(const Vector2 &newPoint1); // moves point1 (top-left corner) TO the <newPoint1> without changing dimensions 
	void moveTo(int newPoint1X, int newPoint1Y); // same as above
	void moveCenterTo(const Vector2 &newCenter); // moves center point TO the <newCenter> without changing dimensions	
	void moveBy(const Vector2 &movement); // moves rectangle point BY <movement> without changing dimensions
	void moveBy(int movementX, int movementY); // same as above
	void moveSide(Side side, int newValue); // moves rectangle side to the <newValue> without changing dimensions

	// Scale
	void scaleInPlaceBy(double scale); // scales rectangle without changing center position
	void scaleInPlaceTo(const Vector2 &newDimensions); // sets new dimensions without changing center position
	
	// Getters
	Vector2 getCorner(Side sideY, Side sideX) const; // returns corner based on <sideY> and <sideX> in that order, if there is no such corner returns Vector2(0, 0)
	Vector2 getCorner() const; // return top left-corner
	Vector2 getDimensions() const; // returns dimenions
	Vector2 getCenter() const; // returns center point, if dimensions have odd value center is imprecise
	int getSide(Side side) const; // returns requested side, if side <NONE> is requested returns -1;
	Vector2 getSideMiddle(Side side) const; // returns middle point of given side
	
	bool overlapsWithRect(const Rectangle &other) const; // detects intersection with given rectangle	
	Side getCollisionSide(const Rectangle &other) const; // detects intersection with given line
	bool containsPoint(const Vector2d &point) const;

	SDL_Rect toSDLRect() const; // conversion to SDL_Rect

private:	
	Vector2 point1; // top-left corner
	Vector2 point2; // bottom-right corner
};
