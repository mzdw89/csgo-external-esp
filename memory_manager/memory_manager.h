#pragma once
#include <string>

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

namespace forceinline {
	class memory_manager {
	public:
		memory_manager( ) { }
		memory_manager( std::string_view process );
		~memory_manager( );

		MODULEENTRY32 get_module_entry( std::string_view module );
		std::uintptr_t get_module_base( std::string_view module );

		std::uintptr_t find_pattern( std::string_view module, std::string_view pattern );
		std::uintptr_t find_pattern( std::uintptr_t module_begin, std::size_t module_size, std::string_view pattern );

		bool is_attached( );

		template < typename T >
		T read( std::uintptr_t address ) {
			T temp_val;
			ReadProcessMemory( m_proc_handle, reinterpret_cast< LPCVOID >( address ), &temp_val, sizeof T, nullptr );

			return temp_val;
		}

		/*
			Reads custom length.
			Example: you want to read a float[ 64 ].
			you'd use this as follows:
			read_ex< float >( float_arr_ptr, 0xDEADBEEF, 64 )
		*/
		template < typename T >
		void read_ex( T* out_object_ptr, std::uintptr_t address, std::size_t object_count ) {
			ReadProcessMemory( m_proc_handle, reinterpret_cast< LPCVOID >( address ), out_object_ptr, sizeof T * object_count, nullptr );
		}

		template < typename T >
		bool write( std::uintptr_t address, T value ) {
			SIZE_T bytes_written;
			WriteProcessMemory( m_proc_handle, reinterpret_cast< LPCVOID >( address ), &value, sizeof T, &bytes_written );

			return bytes_written == sizeof T;
		}

		//See read_ex
		template < typename T >
		bool write_ex( T* object_ptr, std::uintptr_t address, std::size_t object_count ) {
			SIZE_T bytes_written;
			WriteProcessMemory( m_proc_handle, reinterpret_cast< LPCVOID >( address ), object_ptr, sizeof T * object_count, &bytes_written );

			return bytes_written == sizeof T * object_count;
		}

		std::uintptr_t operator[ ]( std::string_view module );

	private:
		void attach_to_process( std::string_view process );

		HANDLE m_proc_handle = nullptr;
		std::uintptr_t m_proc_id = 0;
	};
}