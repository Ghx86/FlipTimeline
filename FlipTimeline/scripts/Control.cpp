//----------------------------------------------------------------------------------
//	Control.cpp - Control processing
//----------------------------------------------------------------------------------
#include <windows.h>
#include "Read.h"
#include "../plugin2.h"

extern void DrawTimelineWindow(HDC hdc, RECT rect);

#define TimelineWindowName L"Filp Timeline beta"
#define IDC_UPDATE_BUTTON 1001

EDIT_HANDLE* g_edit_handle = nullptr;
HWND g_hwnd = nullptr;

int g_scroll_offset_x = 0;
int g_scroll_offset_y = 0;
static bool g_dragging = false;
static POINT g_drag_start;
static int g_drag_offset_x_start = 0;
static int g_drag_offset_y_start = 0;

static bool g_object_dragging = false;
static void* g_dragged_object = nullptr;
static int g_drag_object_offset_frame = 0;
static int g_drag_object_offset_layer = 0;

void UpdateTimeline() {
	if (!g_edit_handle) return;
	ReadTimelineData(g_edit_handle);
	if (g_hwnd) {
		InvalidateRect(g_hwnd, NULL, FALSE);
	}
}

void ClampScrollOffsets(RECT client_rect) {
	const TimelineData& data = GetTimelineData();

	int window_width = client_rect.right - client_rect.left;
	int window_height = client_rect.bottom - client_rect.top;

	int half_width = window_width / 2;
	int max_scroll_x_left = half_width - 60;
	int max_scroll_x_right = -(data.max_frame * 5 + 60 - half_width);

	if (g_scroll_offset_x > max_scroll_x_left) g_scroll_offset_x = max_scroll_x_left;
	if (g_scroll_offset_x < max_scroll_x_right) g_scroll_offset_x = max_scroll_x_right;

	int max_scroll_y_bottom = -((data.max_layer + 1) * 30 - (window_height - 50));
	if (max_scroll_y_bottom > 0) max_scroll_y_bottom = 0;

	if (g_scroll_offset_y > 0) g_scroll_offset_y = 0;
	if (g_scroll_offset_y < max_scroll_y_bottom) g_scroll_offset_y = max_scroll_y_bottom;
}

LRESULT CALLBACK TimelineWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_CREATE:
		CreateWindowEx(
			0,
			L"BUTTON",
			L"\u66F4\u65B0",
			WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
			10, 10, 100, 30,
			hwnd,
			(HMENU)IDC_UPDATE_BUTTON,
			GetModuleHandle(0),
			nullptr);
		break;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDC_UPDATE_BUTTON) {
			UpdateTimeline();
		}
		break;

	case WM_USER + 1:
		UpdateTimeline();
		break;

	case WM_LBUTTONDOWN: {
		int x = LOWORD(lparam);
		int y = HIWORD(lparam);
		const TimelineData& data = GetTimelineData();

		int frame = (x - 60 - g_scroll_offset_x) / 5;
		int layer_index = (y - 50 - g_scroll_offset_y) / 30;
		int layer = data.max_layer - layer_index;

		bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		void* clicked_obj = nullptr;

		for (const auto& obj : data.objects) {
			if (obj.layer == layer && frame >= obj.start && frame <= obj.end) {
				clicked_obj = obj.handle;
				break;
			}
		}

		if (clicked_obj) {
			SelectObject(g_edit_handle, clicked_obj, ctrl);
			g_object_dragging = true;
			g_dragged_object = clicked_obj;
			g_drag_start.x = x;
			g_drag_start.y = y;

			for (const auto& obj : data.objects) {
				if (obj.handle == clicked_obj) {
					g_drag_object_offset_frame = obj.start;
					g_drag_object_offset_layer = obj.layer;
					break;
				}
			}

			SetCapture(hwnd);
			UpdateTimeline();
		}
		else {
			if (!ctrl && g_edit_handle) {
				SelectObject(g_edit_handle, nullptr, false);
				UpdateTimeline();
			}
		}
		break;
	}

	case WM_LBUTTONUP:
		if (g_object_dragging) {
			g_object_dragging = false;

			int current_x = LOWORD(lparam);
			int current_y = HIWORD(lparam);
			int dx = current_x - g_drag_start.x;
			int dy = current_y - g_drag_start.y;

			const TimelineData& data = GetTimelineData();

			int frame_delta = dx / 5;
			int layer_delta = -dy / 30;

			int new_start = g_drag_object_offset_frame + frame_delta;
			int new_layer = g_drag_object_offset_layer + layer_delta;

			if (new_start < 0) new_start = 0;
			if (new_layer < 0) new_layer = 0;
			if (new_layer > data.max_layer) new_layer = data.max_layer;

			MoveObject(g_edit_handle, g_dragged_object, new_layer, new_start);

			g_dragged_object = nullptr;
			ReleaseCapture();
			UpdateTimeline();
		}
		break;

	case WM_MBUTTONDOWN: {
		g_dragging = true;
		g_drag_start.x = LOWORD(lparam);
		g_drag_start.y = HIWORD(lparam);
		g_drag_offset_x_start = g_scroll_offset_x;
		g_drag_offset_y_start = g_scroll_offset_y;
		SetCapture(hwnd);
		break;
	}

	case WM_MBUTTONUP:
		if (g_dragging) {
			g_dragging = false;
			ReleaseCapture();
		}
		break;

	case WM_MOUSEMOVE:
		if (g_object_dragging && g_dragged_object) {
			int current_x = LOWORD(lparam);
			int current_y = HIWORD(lparam);
			int dx = current_x - g_drag_start.x;
			int dy = current_y - g_drag_start.y;

			const TimelineData& data = GetTimelineData();

			int frame_delta = dx / 5;
			int layer_delta = -dy / 30;

			int new_start = g_drag_object_offset_frame + frame_delta;
			int new_layer = g_drag_object_offset_layer + layer_delta;

			if (new_start < 0) new_start = 0;
			if (new_layer < 0) new_layer = 0;
			if (new_layer > data.max_layer) new_layer = data.max_layer;

			UpdateObjectDisplay(g_dragged_object, new_layer, new_start);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		else if (g_dragging) {
			int current_x = LOWORD(lparam);
			int current_y = HIWORD(lparam);
			int dx = current_x - g_drag_start.x;
			int dy = current_y - g_drag_start.y;

			g_scroll_offset_x = g_drag_offset_x_start + dx;
			g_scroll_offset_y = g_drag_offset_y_start + dy;

			RECT client_rect;
			GetClientRect(hwnd, &client_rect);
			ClampScrollOffsets(client_rect);

			InvalidateRect(hwnd, NULL, FALSE);
		}
		break;

	case WM_MOUSEWHEEL: {
		int delta = GET_WHEEL_DELTA_WPARAM(wparam);
		int scroll_amount = (delta / 120) * 50;

		g_scroll_offset_x += scroll_amount;

		RECT client_rect;
		GetClientRect(hwnd, &client_rect);
		ClampScrollOffsets(client_rect);

		InvalidateRect(hwnd, NULL, FALSE);
		break;
	}

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		if (g_edit_handle) {
			UpdateCurrentFrame(g_edit_handle);
		}

		RECT rect;
		GetClientRect(hwnd, &rect);

		HDC mem_dc = CreateCompatibleDC(hdc);
		HBITMAP mem_bitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
		HBITMAP old_bitmap = (HBITMAP)SelectObject(mem_dc, mem_bitmap);

		DrawTimelineWindow(mem_dc, rect);

		BitBlt(hdc, 0, 0, rect.right, rect.bottom, mem_dc, 0, 0, SRCCOPY);

		SelectObject(mem_dc, old_bitmap);
		DeleteObject(mem_bitmap);
		DeleteDC(mem_dc);

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		g_hwnd = nullptr;
		break;
	}
	return DefWindowProc(hwnd, message, wparam, lparam);
}

HWND CreateTimelineWindow(HOST_APP_TABLE* host) {
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpszClassName = TimelineWindowName;
	wcex.lpfnWndProc = TimelineWindowProc;
	wcex.hInstance = GetModuleHandle(0);
	wcex.hbrBackground = nullptr;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassEx(&wcex)) {
		return nullptr;
	}

	g_hwnd = CreateWindowEx(
		0,
		TimelineWindowName,
		TimelineWindowName,
		WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 400,
		nullptr, nullptr,
		GetModuleHandle(0),
		nullptr);

	if (!g_hwnd) {
		return nullptr;
	}

	host->register_window_client(TimelineWindowName, g_hwnd);

	g_edit_handle = host->create_edit_handle();

	PostMessage(g_hwnd, WM_USER + 1, 0, 0);

	return g_hwnd;
}

EDIT_HANDLE* GetEditHandle() {
	return g_edit_handle;
}

HWND GetTimelineWindowHandle() {
	return g_hwnd;
}