#define _CRT_SECURE_NO_WARNINGS
#include "memory_manager/memory_manager.h"
#include "dx_overlay/dx_overlay.h"
#include "csgo_sdk/csgo_sdk.h"
#include "esp/esp.h"

#include <mutex>
#include <iostream>
#include <cassert>

//Custom logging functions
void log( std::string_view message ) {
	std::cout << "[+] " << message << std::endl;
}

void log_and_exit( std::string_view message ) {
	log( message );
	std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
	exit( -1 );
}

int APIENTRY wWinMain( _In_ HINSTANCE instance, _In_opt_ HINSTANCE prev_instance, _In_ LPWSTR p_cmd_line, _In_ int cmd_show ) {
	AllocConsole( );
	freopen( "conout$", "w", stdout );

	try {
		//Create overlay and memory objects
		forceinline::memory_manager memory( "csgo.exe" );
		forceinline::dx_overlay overlay( L"Valve001", L"Counter-Strike: Global Offensive", false );

		//If our constructor didn't throw, these will have to be true
		assert( memory.is_attached( ) );
		assert( overlay.is_initialized( ) );

		log( "Attached to process" );
		log( "Created overlay" );

		//Create a mutex so we can multithread safely
		std::mutex ent_mtx;

		//Grab the base of the modules
		sdk::modules_t modules( &memory );

		//Create a thread to read info so we don't slow down our rendering part
		std::thread read_ent_info( [ & ]( ) -> void {
			std::vector< sdk::ent_info_t > ent_info( 64 );

			while ( 1 ) {
				std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

				//Lock our mutex so our ESP thread doesn't do dumb shit
				std::unique_lock lock( ent_mtx );

				//Invalidate all ESP entities as we're updating them
				for ( auto& esp_ent : esp::entities )
					esp_ent.valid = false;

				//Get clientstate for the entitylist
				std::uintptr_t client_state = memory.read< std::uintptr_t >( modules.engine_dll + 0x590D8C ); //m_dwClientState

				//Read the whole entity list at once
				memory.read_ex< sdk::ent_info_t >( ent_info.data( ), modules.client_dll + 0x4D06CB4, ent_info.size( ) ); //m_dwEntityList

				//Get our local player ptr
				int local_player_idx = memory.read< int >( client_state + 0x180 ); //m_dwClientState_GetLocalPlayer
				std::uintptr_t local_ptr = ent_info[ local_player_idx ].entity_ptr;

				//Is our local player ptr valid?
				if ( !local_ptr )
					continue;

				//Get our local player
				sdk::entity_t local( &memory, &modules, local_ptr );

				//Gather entity information for our ESP
				for ( std::size_t i = 0; i < ent_info.size( ); i++ ) {
					std::uintptr_t ent_ptr = ent_info[ i ].entity_ptr;

					//Entity is invalid, don't draw on ESP
					if ( !ent_ptr )
						continue;

					//Create an entity object so we can get information the easy way
					sdk::entity_t entity( &memory, &modules, ent_ptr );

					//Continue if entity is dormant or dead
					if ( entity.dormant( ) || !entity.is_alive( ) )
						continue;

					//We don't want to draw ESP on our team
					if ( entity.team( ) == local.team( ) )
						continue;

					//We have a valid entity, get a reference to it for ease of use
					esp::esp_entity_t& esp_entity = esp::entities[ i ];

					//Get entity information for our ESP
					esp_entity.health = entity.health( );
					entity.get_name( esp_entity.name );
					esp_entity.origin = entity.origin( );
					esp_entity.top_origin = esp_entity.origin + sdk::vec3_t( 0.f, 0.f, 75.f );

					//Our ESP entity is now valid to draw
					esp_entity.valid = true;
				}
			}
		} );

		log( "Started reading thread, starting rendering" );

		//MSG struct for WndProc
		MSG m;
		ZeroMemory( &m, sizeof m );

		//Get our overlay renderer
		forceinline::dx_renderer renderer = overlay.create_renderer( );

		//Message and rendering loop
		do {
			if ( PeekMessage( &m, overlay.get_overlay_wnd( ), NULL, NULL, PM_REMOVE ) ) {
				TranslateMessage( &m );
				DispatchMessage( &m );
			}
			
			//Lock the mutex so we don't fuck shit up
			std::unique_lock lock( ent_mtx );

			//Render our ESP
			renderer.begin_rendering( );
			renderer.draw_text( std::to_string( renderer.get_fps( ) ), 2, 2, 0xFFFFFFFF, false );
			esp::draw( &memory, &renderer );
			renderer.end_rendering( );

			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		} while ( m.message != WM_QUIT );

		log( "Exiting..." );
		std::this_thread::sleep_for( std::chrono::seconds( 3 ) );

		return 0;
	} catch ( const std::exception& e ) {
		//Catch and log any exceptions
		log_and_exit( e.what( ) );
	}
}