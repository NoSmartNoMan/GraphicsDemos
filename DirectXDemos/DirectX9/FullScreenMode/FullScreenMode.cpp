#include <tchar.h>
#include <d3dx9.h>
#include <mmsystem.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 

//
// ȫ����ʱ��ܷ�CPUʱ�䣬�д����
//
// �������ʹ��D3DFVF_XYZ|D3DFVF_DIFFUSE�����ʽ������һ��Cube�������壩
// �ö����ʽ������գ��Դ���ɫ��Ϣ����������궥�������Ժ�ֻ������һ��
// ��ͼ�����ͶӰ����Ϳ�����Ⱦ��
//
// �ó�����Ӧ�˼�����Ϣ��ʹCube���԰����������᷽����ת
// ����X,Y,Z��ʱ���ֱ�X,Y,Z�᷽����ת
// ���ո��ʱֹͣ��ת
// ��ESC���˳�����
//


LPDIRECT3D9             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; // Buffer to hold vertices
int g_rotAxis = 0; 

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
	FLOAT x, y, z;	// ����λ��
	DWORD color;	// ������ɫ
};

// �Զ��嶥���ʽ����������������������λ�ã�ɢ���
// ʹ�����ֶ����ʽ����Ҫ������Light
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

HRESULT InitD3D( HWND hWnd )
{
	DWORD ScreenW = 0;
	DWORD ScreenH = 0;

	DEVMODE devMode ;
	devMode.dmSize = sizeof(devMode) ;
	DWORD iModeNum = 0 ;
	DWORD r = 1 ;

	while(r != 0)
	{
		r = EnumDisplaySettings(NULL, iModeNum, &devMode) ;
		// Store the maximum resolution currently
		if(devMode.dmPelsWidth >= ScreenW && devMode.dmPelsHeight >= ScreenH)
		{
			ScreenW = devMode.dmPelsWidth ;
			ScreenH = devMode.dmPelsHeight ;
		}

		//OutputModeInfo(iModeNum, devMode) ;
		iModeNum++ ;
	}

	// Create the D3D object.
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	// Set up the structure used to create the D3DDevice
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );

	d3dpp.Windowed               = FALSE;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD; // ������������У������޷�ȫ��
	d3dpp.BackBufferWidth        = ScreenW; 
	d3dpp.BackBufferHeight       = ScreenH;
	d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8; // ������������У������޷�ȫ��


	// Create the D3DDevice
	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}

	// Turn off culling, so we see the front and back of the triangle
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// ���ù���Ч����ǰ��˵���������ʽ���Ѿ���������ɫ��Ϣ
	// ��������ʽ�в�������ɫ��Ϣ����ô����Ҫͨ����������ʾ��ɫ
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING , FALSE );

	return S_OK;
}

HRESULT InitVB()
{
	// �˸�����
	CUSTOMVERTEX A = { 0.0f, 0.0f, -1.0f, 0xffff0000 }; // λ�ã���ɫ
	CUSTOMVERTEX B = { 1.0f, 0.0f, -1.0f, 0x0000ffff };
	CUSTOMVERTEX C = { 1.0f, 0.0f,  0.0f, 0x00ffff00 };
	CUSTOMVERTEX D = { 0.0f, 0.0f,  0.0f, 0xff0000ff };
	CUSTOMVERTEX E = { 0.0f, 1.0f, -1.0f, 0x00ffff00 };
	CUSTOMVERTEX F = { 1.0f, 1.0f, -1.0f, 0xff0000ff };
	CUSTOMVERTEX G = { 1.0f, 1.0f,  0.0f, 0xffff0000 };
	CUSTOMVERTEX H = { 0.0f, 1.0f,  0.0f, 0x0000ffff };

	// ���������һ�������Σ���ʮ���������Σ�
	// �������������һ�������Σ�������������
	// �������������һ��������
	CUSTOMVERTEX vertices[] =
	{
		A, B, C, A, C, D, 
		B, C, F, C, F, G, 
		C, D, H, C, H, G, 
		F, G, H, E, F, H, 
		A, B, E, B, E, F, 
		A, E, D, E, D, H,
	};

	// ����36������Ļ�����
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( 36 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
	{
		return E_FAIL;
	}

	// ���������������������ݿ�����ȥ
	VOID* pVertices;
	if( FAILED( g_pVB->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices, vertices, sizeof(vertices) );
	g_pVB->Unlock();

	return S_OK;
}

VOID Cleanup()
{
	if( g_pVB != NULL )        
		g_pVB->Release();

	if( g_pd3dDevice != NULL ) 
		g_pd3dDevice->Release();

	if( g_pD3D != NULL )       
		g_pD3D->Release();
}

VOID SetupMatrix()
{
	// ���������ƶ���ԭ��
	D3DXMATRIXA16 matTrans ;
	D3DXMatrixTranslation( &matTrans, -0.5f, -0.5f, 0.5f) ;

	// ��ת����ʱ��������ת�Ƕ�
	D3DXMATRIXA16 matRol ;
	UINT  iTime  = timeGetTime() % 1000;
	FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 1000.0f;


	if( g_rotAxis == 4 )	// ��X����ת	
		D3DXMatrixRotationX( &matRol, fAngle );

	else if( g_rotAxis == 2 ) // ��Y����ת
		D3DXMatrixRotationY( &matRol, fAngle );

	else if( g_rotAxis == 1 ) // ��Z����ת
		D3DXMatrixRotationZ( &matRol, fAngle );

	else
		D3DXMatrixIdentity( &matRol ) ; // ���ֲ���

	D3DXMATRIXA16 matWorld = matTrans * matRol ;
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	// ������ͼ����
	D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );		// �����λ��
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );	// �ӵ����ģ���ʲô�ط���
	D3DXVECTOR3 vUpVec( 0.0f, 3.0f, 0.0f );		// �����������ӽǣ���������ˮƽ��ļнǣ�
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// ����ͶӰ����
	D3DXMATRIXA16 matProj;

	// ��Ұ��ò�������������е��ظ�Ŷ��
	// �ݺ�ȣ�1.0f
	// �������棺1.0f
	// Զ�����棺100.0f
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

}

VOID Render()
{
	// Clear the backbuffer to a blue color
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );

	// Begin the scene
	if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		SetupMatrix() ;
		// Draw the triangles in the vertex buffer. This is broken into a few
		// steps. We are passing the vertices down a "stream", so first we need
		// to specify the source of that stream, which is our vertex buffer. Then
		// we need to let D3D know what vertex shader to use. Full, custom vertex
		// shaders are an advanced topic, but in most cases the vertex shader is
		// just the FVF, so that D3D knows what type of vertices we are dealing
		// with. Finally, we call DrawPrimitive() which does the actual rendering
		// of our geometry (in this case, just one triangle).
		g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

		//��12�����������һ��������
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 12 );

		// End the scene
		g_pd3dDevice->EndScene();
	}

	// Present the backbuffer contents to the display
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break ;

			case 'X':
				g_rotAxis = 4 ;
				break ;

			case 'Y':
				g_rotAxis = 2 ;
				break ;

			case 'Z':
				g_rotAxis = 1 ;
				break ;

			case VK_SPACE:
				g_rotAxis = 0 ;
				break ;
			}
		}
		return 0 ;

	case WM_DESTROY:
		Cleanup();
		PostQuitMessage( 0 );
		return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: wWinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	// Register the window class
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"D3D Tutoria", NULL };
	RegisterClassEx( &wc );

	// Create the application's window
	HWND hWnd = CreateWindow( "D3D Tutoria", "D3D Tutorial 02: Vertices",
		WS_OVERLAPPEDWINDOW, 10, 10, 600, 600,
		NULL, NULL, wc.hInstance, NULL );

	// Initialize Direct3D
	if( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		// Create the vertex buffer
		if( SUCCEEDED( InitVB() ) )
		{
			// Show the window
			ShowWindow( hWnd, SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			// Enter the message loop
			MSG msg;
			ZeroMemory( &msg, sizeof(msg) );
			while( msg.message!=WM_QUIT )
			{
				// ����Ϣ�ʹ���
				if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
				// û��Ϣ����Ⱦ
				else
					Render();
			}
		}
	}

	UnregisterClass( "D3D Tutoria", wc.hInstance );
	return 0;
}
