#pragma once
#include <vector>
#include "../csgo_sdk/csgo_sdk.h"
#include "../dx_overlay/dx_overlay.h"

namespace esp {
	struct esp_entity_t {
		bool valid = false;

		int health = 0;
		std::string name = "";
		sdk::vec3_t origin, top_origin;
	};

	extern std::vector< esp_entity_t > entities;

	extern void draw( forceinline::memory_manager* memory, forceinline::dx_renderer* renderer );
}