#pragma once

/* Contains all unique types of entities (classes derived from 'Entity' class) */

#include "entity_type.h" // 'entity_types' base classes




namespace entities {

	std::unique_ptr<Entity> make_entity(const std::string &type, const std::string &name, const Vector2d &position);
		// creates entity of a correct class based on type and name and returns ownership


	



	namespace items {

		// # BrassRelic #
		struct BrassRelic : public entity_types::ItemEntity {
			BrassRelic() = delete;

			BrassRelic(const Vector2d &position);
		};



		// # Paper #
		struct Paper : public entity_types::ItemEntity {
			Paper() = delete;

			Paper(const Vector2d &position);
		};
	}


	
	namespace destructibles {

		// # TNT #
		class TNT : public entity_types::Destructible {
		public:
			TNT() = delete;

			TNT(const Vector2d &position);

		private:
			void effect() override;
		};
	}



	namespace enemies {

		// # Ghost #
		// - Fast agressive movement
		// - Fast melee attacks
		// - Ignores terrain
		class Ghost : public entity_types::Enemy {
		public:
			Ghost() = delete;

			Ghost(const Vector2d &position);

		private:
			bool aggroCheck() override;
			bool deaggroCheck() override;

			void wander(Milliseconds elapsedTime) override;

			bool canAttack() override;

			void chase(Milliseconds elapsedTime) override;
			void attack(Milliseconds elapsedTime) override;

			void onDeath() override;
		};



		// # Sludge #
		class Sludge : public entity_types::Enemy {
		public:
			Sludge() = delete;

			Sludge(const Vector2d &position);

		private:
			bool aggroCheck() override;
			bool deaggroCheck() override;

			void wander(Milliseconds elapsedTime) override;

			bool canAttack() override;

			void chase(Milliseconds elapsedTime) override;
			void attack(Milliseconds elapsedTime) override;

			void onDeath() override;

			Timer wander_timer; // used to make random turns while wandering
			bool wander_move; // decides whether sludge will more or stand still for the timer duration
		};
	}
}
