#include "esp.h"

//TODO: Add dynamic way of screen size getting
sdk::vec3_t screen_size( 1920, 1080, 0 );

namespace esp {
	std::vector< esp_entity_t > entities( 64 );

	void draw( forceinline::memory_manager* memory, forceinline::dx_renderer* renderer ) {
		//Create our vectors for the 2D screen position
		sdk::vec3_t screen_bottom, screen_top;

		//Get the module base for client_panorama.dll
		static std::uintptr_t client_dll = memory->get_module_base( "client_panorama.dll" );

		//Grab the view matrix
		sdk::view_matrix_t view_matrix = memory->read< sdk::view_matrix_t >( client_dll + 0x4CF86E4 ); //view_matrix

		//Doesn't work; flickers a LOT for some reason.
		//static std::uintptr_t crender_bytes = memory->find_pattern( "engine.dll", "B9 ? ? ? ? A1 ? ? ? ? FF 60 38" ) + 1;
		//static std::uintptr_t crender_offset = memory->read< std::uintptr_t >( crender_bytes );

		//sdk::view_matrix_t view_matrix = memory->read< sdk::view_matrix_t >( crender_offset + 39 * 4 );

		//Loop our vector containing the entities
		for ( auto& ent : entities ) {
			//If our entity isn't valid, skip it
			if ( !ent.valid )
				continue;
			
			//Convert 3D space coordinates to 2D screen coordinates
			if ( !sdk::world_to_screen( screen_size, ent.origin, screen_bottom, view_matrix ) || !sdk::world_to_screen( screen_size, ent.top_origin, screen_top, view_matrix ) )
				 continue;

			//Box properties
			int box_height = screen_bottom.y - screen_top.y;
			int box_width = box_height / 2;

			//Draw the ESP
			renderer->draw_outlined_rect( screen_top.x - box_width / 2, screen_top.y, box_width, box_height, D3DCOLOR_RGBA( 65, 135, 245, 255 ) );
			renderer->draw_text( ent.name, screen_top.x, screen_top.y - 14, 0xFFFFFFFF );
			renderer->draw_text( std::to_string( ent.health ) + " HP", screen_top.x, screen_bottom.y, 0xFFFFFFFF );
		}
	}
}