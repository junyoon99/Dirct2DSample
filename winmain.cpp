#include <Windows.h>
#include <d2d1.h>

#pragma comment (lib, "d2d1.lib")
const wchar_t CLASS_NAME[]{ L"Direct2DSampleClass" };

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// D2D 오브젝트
ID2D1Factory* gpD2dFactory{};
ID2D1HwndRenderTarget* gpRenderTarget{};

// GDI 연동
ID2D1SolidColorBrush* gpBrush{};
ID2D1RadialGradientBrush* gpRadialBrush{};

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) 
{
	// 1. Factory 생성
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gpD2dFactory);

	if (FAILED(hr)) 
	{
		OutputDebugString(L"Failed to Create Factory!\n");
		return 0;
	}

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = CLASS_NAME;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc)) 
	{
		OutputDebugString(L"Failed RegisterClass");
		return 0;
	}

	RECT wr = { 0,0,1024,768 };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowEx(
		NULL,
		CLASS_NAME,
		L"Direct2D Sample",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		NULL,
		NULL,
		hInstance,
		NULL

	);

	if (!hwnd) 
	{
		OutputDebugString(L"Failed to Create Window!");
		return 0;
	}

	// 2. 렌더타겟 생성
	hr = gpD2dFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(wr.right - wr.left, wr.bottom - wr.top)),
		&gpRenderTarget
	);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"Failed to Create Hwnd Render Target!");
		return 0;
	}

	hr = gpRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Red, 1.0f),
		&gpBrush
	);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"Failed to Crreate solid color brush!");
		return 0;
	}

	ID2D1GradientStopCollection* pGSC{};
	D2D1_GRADIENT_STOP gradientsStops[2]
	{
	{ 0.0f, D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f) },
	{ 1.0f, D2D1::ColorF(D2D1::ColorF::Crimson, 1.0f)}
	};
	hr = gpRenderTarget->CreateGradientStopCollection(
		gradientsStops, 2, &pGSC
	);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"failed to Create Gradient Stop collection!");
		return 0;
	}

	hr = gpRenderTarget->CreateRadialGradientBrush(
		D2D1::RadialGradientBrushProperties(D2D1::Point2F(50.0f,150.0f), D2D1::Point2F(), 50.0f, 50.0f), 
		pGSC,
		&gpRadialBrush
	);
	if (FAILED(hr)) 
	{
		OutputDebugString(L"Failed to Create Radial Gradient Brush!");
		return 0;
	}

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 4. 해제
	if (gpRadialBrush) 
	{
		gpRadialBrush->Release();
		gpRadialBrush = nullptr;
	}
	if (pGSC) 
	{
		pGSC->Release();
		pGSC = nullptr;
	}

	if (gpRenderTarget) 
	{
		gpRenderTarget->Release();
		gpRenderTarget = nullptr;
	}
	if (gpD2dFactory) 
	{
		gpD2dFactory->Release();
		gpD2dFactory = nullptr;
	}

	return (int)msg.wParam;
}

void OnPaint(HWND hwnd) 
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hwnd, &ps);

	// 3. 그리기
	gpRenderTarget->BeginDraw();

	gpRenderTarget->Clear(D2D1::ColorF(0.0f, 0.2f, 0.4f, 1.0f));
	gpRenderTarget->FillRectangle(D2D1::RectF(0.0f,0.0f,100.0f,100.0f), gpBrush);

	gpRenderTarget->FillEllipse(
		D2D1::Ellipse(D2D1::Point2F(50.0f, 150.0f), 50.0f, 50.0f),
		gpRadialBrush
	);

	gpRenderTarget->EndDraw();

	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			OnPaint(hwnd);
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
			break;
	}
	return 0;
}
