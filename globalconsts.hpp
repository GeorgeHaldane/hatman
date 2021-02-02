#pragma once



// rendering::
// - Holds hardcoded values related to rendering
namespace rendering {
	const int RENDERING_WIDTH = 640;
	const int RENDERING_HEIGHT = 360;

	const int TILE_SIZE = 32;
	const int ITEM_SIZE = 16;

	
};



/// May be ommited in a future
namespace physics {
	const double GRAVITY_ACCELERATION = 1500; // downwards speed increase per 1 sec

	const double DEFAULT_MASS_PLAYER = 100;
	const double DEFAULT_MASS_ITEMS = 40;
	const double DEFAULT_MASS_CREATURES = 150;

	const double DEFAULT_FRICTION_PLAYER = 0.6;
	const double DEFAULT_FRICTION_ITEMS = 0.3;
	const double DEFAULT_FRICTION_CREATURES = 0.5;
}