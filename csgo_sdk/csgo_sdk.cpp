#include "csgo_sdk.h"
#include <cmath>

namespace sdk {
	modules_t::modules_t( forceinline::memory_manager* memory ) {
		client_dll = memory->get_module_base( "client_panorama.dll" );
		engine_dll = memory->get_module_base( "engine.dll" );
	}

	vec3_t::vec3_t( ) {
		x = y = z = 0;
	}

	vec3_t::vec3_t( float _x, float _y, float _z ) {
		x = _x;
		y = _y;
		z = _z;
	}

	vec3_t vec3_t::operator+( const vec3_t& other ) {
		return { x + other.x, y + other.y, z + other.z };
	}

	vec3_t vec3_t::operator+=( const vec3_t& other ) {
		x += other.x;
		y += other.y;
		z += other.z;
		
		return *this;
	}

	vec3_t vec3_t::operator-( const vec3_t& other ) {
		return { x - other.x, y - other.y, z - other.z };
	}

	vec3_t vec3_t::operator-=( const vec3_t& other ) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		
		return *this;
	}

	vec3_t vec3_t::operator*( float factor ) {
		return { x * factor, y * factor, z * factor };
	}

	vec3_t vec3_t::operator*( const vec3_t& other ) {
		return { x * other.x, y * other.y, z * other.z };
	}

	vec3_t vec3_t::operator*=( float factor ) {
		x *= factor;
		y *= factor;
		z *= factor;

		return *this;
	}

	vec3_t vec3_t::operator*=( const vec3_t& other ) {
		x *= other.x;
		y *= other.y;
		z *= other.z;

		return *this;
	}

	vec3_t vec3_t::operator/( float factor ) {
		return { x / factor, y / factor, z / factor };
	}

	vec3_t vec3_t::operator/( const vec3_t& other ) {
		return { x / other.x, y / other.y, z / other.z };
	}

	vec3_t vec3_t::operator/=( float factor ) {
		x /= factor;
		y /= factor;
		z /= factor;

		return *this;
	}

	vec3_t vec3_t::operator/=( const vec3_t& other ) {
		x /= other.x;
		y /= other.y;
		z /= other.z;

		return *this;
	}

	bool vec3_t::is_zero( ) {
		return x == 0 && y == 0 && z == 0;
	}

	float vec3_t::dot( const vec3_t& other ) {
		return x * other.x + y * other.y + z * other.z;
	}

	float vec3_t::dist( const vec3_t& other ) {
		return ( *this - other ).length( );
	}

	float vec3_t::length( ) {
		return sqrt( length_sqr( ) );
	}

	float vec3_t::length_sqr( ) {
		return x * x + y * y + z * z;
	}

	bool world_to_screen( const vec3_t& screen_size, const vec3_t& pos, vec3_t& out, sdk::view_matrix_t matrix ) {
		out.x = matrix[ 0 ][ 0 ] * pos.x + matrix[ 0 ][ 1 ] * pos.y + matrix[ 0 ][ 2 ] * pos.z + matrix[ 0 ][ 3 ];
		out.y = matrix[ 1 ][ 0 ] * pos.x + matrix[ 1 ][ 1 ] * pos.y + matrix[ 1 ][ 2 ] * pos.z + matrix[ 1 ][ 3 ];

		float w = matrix[ 3 ][ 0 ] * pos.x + matrix[ 3 ][ 1 ] * pos.y + matrix[ 3 ][ 2 ] * pos.z + matrix[ 3 ][ 3 ];

		if ( w < 0.01f )
			return false;

		float inv_w = 1.f / w;
		out.x *= inv_w;
		out.y *= inv_w;

		float x = screen_size.x * .5f;
		float y = screen_size.y * .5f;

		x += 0.5f * out.x * screen_size.x + 0.5f;
		y -= 0.5f * out.y * screen_size.y + 0.5f;

		out.x = x;
		out.y = y;

		return true;
	}
}