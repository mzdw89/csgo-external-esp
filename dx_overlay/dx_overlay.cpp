#include "dx_overlay.h"

#include <iostream>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace forceinline {
	/*
		Regarding the not_topmost argument:
		Some games/anti-cheats specifically check if a window has the WS_EX_TOPMOST and WS_VISIBLE flags, or has the game window as its parent.
		By setting not_topmost to true, we will use a trick to make our window act similar to having the WS_EX_TOPMOST style without it actually having it, 
		therefore making that window harder to detect (or easier, it depends on the anti-cheat. Before doing anything you should become familiar with the
		anti-cheat the game has to avoid being banned.
	*/
	dx_overlay::dx_overlay( std::wstring_view target_class, std::wstring_view target_window, bool not_topmost ) {
		m_not_topmost = not_topmost;

		if ( target_window.empty( ) && target_class.empty( ) )
			throw std::invalid_argument( "dx_overlay::dx_overlay: target_class and target_window empty" );

		if ( !FindWindowW( target_class.empty( ) ? NULL : target_class.data( ), target_window.empty( ) ? NULL : target_window.data( ) ) ) {
			std::string target_class_mb( target_class.begin( ), target_class.end( ) );
			std::string target_window_mb( target_window.begin( ), target_window.end( ) );

			throw std::invalid_argument( "dx_overlay::dx_overlay: target window \"" + target_window_mb + "\" with target class \"" + target_class_mb + "\" could not be found" );
		}

		create_overlay( target_class, target_window );
	}

	dx_overlay::~dx_overlay( ) {
		if ( m_overlay_wnd )
			DestroyWindow( m_overlay_wnd );

		if ( m_d3d )
			m_d3d->Release( );

		if ( m_device )
			m_device->Release( );
	}

	dx_renderer dx_overlay::create_renderer( ) {
		return dx_renderer( m_device );	//Return a renderer object
	}

	HWND dx_overlay::get_overlay_wnd( ) {
		return m_overlay_wnd;	//Return our window handle
	}

	bool dx_overlay::is_initialized( ) {
		return m_initialized;	//Is our overlay initialized properly?
	}

	void dx_overlay::create_overlay( std::wstring_view target_class, std::wstring_view target_window ) {
		WNDCLASSEX wc;
		wc.cbSize = sizeof( wc );

		//Create our window class
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = m_wnd_proc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = NULL;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = HBRUSH( RGB( 0, 0, 0 ) );
		wc.lpszMenuName = "";
		wc.lpszClassName = "forceinline::dx_overlay";
		wc.hIconSm = NULL;

		//Register our window class
		if ( !RegisterClassExA( &wc ) )
			throw std::exception( "dx_overlay::create_overlay: failed to register wndclassex" );

		//Find our target window
		m_target_wnd = FindWindowW( target_class.data( ), target_window.data( ) );

		//Get the size of our target window
		GetWindowRect( m_target_wnd, &m_target_wnd_size );

		//Make the window transparent
		DWORD ex_styles = WS_EX_LAYERED | WS_EX_TRANSPARENT;

		//Add WS_EX_TOPMOST if we choose
		if ( !m_not_topmost )
			ex_styles |= WS_EX_TOPMOST;

		//Create our window
		m_overlay_wnd = CreateWindowExA( ex_styles, "forceinline::dx_overlay", "", WS_POPUP | WS_VISIBLE,
										 m_target_wnd_size.left, m_target_wnd_size.top, m_target_wnd_size.width( ), m_target_wnd_size.height( ), NULL, NULL, NULL, NULL );

		if ( !m_overlay_wnd )
			throw std::exception( "dx_overlay::create_overlay: failed to create overlay window" );

		//Let DWM handle our window
		MARGINS m = { m_target_wnd_size.left, m_target_wnd_size.top, m_target_wnd_size.width( ), m_target_wnd_size.height( ) };
		DwmExtendFrameIntoClientArea( m_overlay_wnd, &m );

		//Set window to use alpha channel
		SetLayeredWindowAttributes( m_overlay_wnd, RGB( 0, 0, 0 ), 255, LWA_ALPHA );

		//Show our window
		ShowWindow( m_overlay_wnd, SW_SHOW );

		//Initialize DirectX
		init_dx9( );
	}

	void dx_overlay::init_dx9( ) {
		//Create DirectX object
		m_d3d = Direct3DCreate9( D3D_SDK_VERSION );

		if ( !m_d3d )
			throw std::exception( "dx_overlay::init_dx9: failed to create dx3d9 object" );

		//Create DirectX present parameters struct
		D3DPRESENT_PARAMETERS d3d_pp;
		ZeroMemory( &d3d_pp, sizeof( d3d_pp ) );

		//Set our device parameters
		d3d_pp.Windowed = true;
		d3d_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3d_pp.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3d_pp.BackBufferWidth = m_target_wnd_size.width( );
		d3d_pp.BackBufferHeight = m_target_wnd_size.height( );
		d3d_pp.hDeviceWindow = m_overlay_wnd;
		d3d_pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		//Create DirectX device
		if ( FAILED( m_d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_overlay_wnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d_pp, &m_device ) ) ) {
			m_d3d->Release( );
			throw std::exception( "dx_overlay::init_dx9: failed to create device" );
		}

		//Overlay successfully initialized
		m_initialized = true;
	}

	bool dx_overlay::m_not_topmost = false;
	HWND dx_overlay::m_target_wnd, dx_overlay::m_overlay_wnd;

	LRESULT CALLBACK dx_overlay::m_wnd_proc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam ) {
		//Imitate WS_EX_TOPMOST if specified
		if ( m_not_topmost ) {
			//Grab target window RECT
			wnd_rect_t r;
			GetWindowRect( m_target_wnd, &r );

			//Set the target windows z position to be under our overlay
			SetWindowPos( m_target_wnd, m_overlay_wnd, r.left, r.top, r.width( ), r.height( ), SWP_NOMOVE | SWP_NOSIZE );
		}

		switch ( msg ) {
			case WM_DESTROY:
				exit( 0 );
				break;
			default:
				return DefWindowProc( wnd, msg, wparam, lparam );
		}
	}
}
