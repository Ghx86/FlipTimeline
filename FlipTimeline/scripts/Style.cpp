//----------------------------------------------------------------------------------
//	Style.cpp - Drawing and display
//----------------------------------------------------------------------------------
#include "Read.h"
#include <windows.h>

extern int g_scroll_offset_x;
extern int g_scroll_offset_y;

namespace DrawConfig {
	const int LAYER_HEIGHT = 30;
	const int FRAME_WIDTH = 5;
	const int OFFSET_X = 60;
	const int OFFSET_Y = 50;

	const COLORREF BG_COLOR = RGB(30, 30, 30);
	const COLORREF GRID_COLOR = RGB(60, 60, 60);
	const COLORREF GRID_LINE_COLOR = RGB(120, 120, 120);
	const COLORREF BASE_LINE_COLOR = RGB(0, 0, 0);
	const COLORREF TEXT_COLOR = RGB(200, 200, 200);
	const COLORREF DEBUG_COLOR = RGB(255, 255, 0);
	const COLORREF OBJECT_COLOR = RGB(70, 120, 200);
	const COLORREF OBJECT_BORDER_COLOR = RGB(90, 140, 220);
	const COLORREF SELECTED_OBJECT_COLOR = RGB(100, 170, 255);
	const COLORREF SELECTED_OBJECT_BORDER_COLOR = RGB(130, 200, 255);
	const COLORREF OBJECT_TEXT_COLOR = RGB(255, 255, 255);
	const COLORREF CURRENT_FRAME_COLOR = RGB(255, 0, 0);

	const int FONT_SIZE = 14;
	const int DEBUG_FONT_SIZE = 12;
	const wchar_t* FONT_NAME = L"Yu Gothic UI";
}

void DrawBackground(HDC hdc, RECT rect) {
	HBRUSH bg_brush = CreateSolidBrush(DrawConfig::BG_COLOR);
	FillRect(hdc, &rect, bg_brush);
	DeleteObject(bg_brush);
}

void DrawDebugInfo(HDC hdc, const TimelineData& data) {
	HFONT debug_font = CreateFont(
		DrawConfig::DEBUG_FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		DrawConfig::FONT_NAME);

	HFONT old_font = (HFONT)SelectObject(hdc, debug_font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, DrawConfig::DEBUG_COLOR);

	wchar_t debug_buf[128];
	wsprintf(debug_buf, L"Objects: %d, MaxFrame: %d, MaxLayer: %d",
		(int)data.objects.size(), data.max_frame, data.max_layer);
	TextOut(hdc, 120, 12, debug_buf, lstrlen(debug_buf));

	SelectObject(hdc, old_font);
	DeleteObject(debug_font);
}

void DrawFrameInfo(HDC hdc, const TimelineData& data) {
	HFONT font = CreateFont(
		25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		DrawConfig::FONT_NAME);

	HFONT old_font = (HFONT)SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, DrawConfig::TEXT_COLOR);

	double fps = (double)data.rate / data.scale;
	int total_frames = data.current_frame;
	double total_seconds = total_frames / fps;
	int minutes = (int)(total_seconds / 60);
	int seconds = (int)total_seconds % 60;
	int frames = total_frames % (int)fps;

	wchar_t buf[128];
	wsprintf(buf, L"%02d:%02d:%02d  %d/%d  fps:%d",
		minutes, seconds, frames,
		data.current_frame, data.max_frame,
		data.rate);
	TextOut(hdc, 120, 10, buf, lstrlen(buf));

	SelectObject(hdc, old_font);
	DeleteObject(font);
}

void DrawBaseLine(HDC hdc, RECT rect) {
	HPEN base_pen = CreatePen(PS_SOLID, 2, DrawConfig::BASE_LINE_COLOR);
	HPEN old_pen = (HPEN)SelectObject(hdc, base_pen);

	MoveToEx(hdc, 0, DrawConfig::OFFSET_Y, NULL);
	LineTo(hdc, rect.right, DrawConfig::OFFSET_Y);

	SelectObject(hdc, old_pen);
	DeleteObject(base_pen);
}

void DrawGrid(HDC hdc, RECT rect, const TimelineData& data) {
	HPEN grid_pen = CreatePen(PS_SOLID, 1, DrawConfig::GRID_COLOR);
	HPEN old_pen = (HPEN)SelectObject(hdc, grid_pen);

	for (int i = 0; i <= data.max_layer + 1; i++) {
		int y = i * DrawConfig::LAYER_HEIGHT + DrawConfig::OFFSET_Y + g_scroll_offset_y;
		MoveToEx(hdc, 0, y, NULL);
		LineTo(hdc, rect.right, y);
	}

	SelectObject(hdc, old_pen);
	DeleteObject(grid_pen);
}

void DrawTimeGridLines(HDC hdc, const TimelineData& data) {
	double fps = (double)data.rate / data.scale;
	int half_second_frames = (int)(fps * 0.5);
	int one_second_frames = (int)fps;

	HPEN line_pen = CreatePen(PS_SOLID, 1, DrawConfig::GRID_LINE_COLOR);

	for (int i = 0; i <= data.max_frame; i++) {
		bool is_one_second = (i % one_second_frames == 0);
		bool is_half_second = (i % half_second_frames == 0) && !is_one_second;

		if (is_one_second || is_half_second) {
			int x = i * DrawConfig::FRAME_WIDTH + DrawConfig::OFFSET_X + g_scroll_offset_x;
			int line_height = is_one_second ? 20 : 10;

			SelectObject(hdc, line_pen);
			MoveToEx(hdc, x, DrawConfig::OFFSET_Y, NULL);
			LineTo(hdc, x, DrawConfig::OFFSET_Y - line_height);
		}
	}

	DeleteObject(line_pen);
}

void DrawLayerLabels(HDC hdc, const TimelineData& data) {
	HBRUSH bg_brush = CreateSolidBrush(DrawConfig::BG_COLOR);

	RECT left_area = { 0, DrawConfig::OFFSET_Y, DrawConfig::OFFSET_X, 10000 };
	FillRect(hdc, &left_area, bg_brush);

	DeleteObject(bg_brush);

	HFONT font = CreateFont(
		DrawConfig::FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		DrawConfig::FONT_NAME);

	HFONT old_font = (HFONT)SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, DrawConfig::TEXT_COLOR);

	for (int i = 0; i <= data.max_layer; i++) {
		wchar_t buf[32];
		wsprintf(buf, L"\u30EC\u30A4\u30E4\u30FC %d", i + 1);
		TextOut(hdc, 5, i * DrawConfig::LAYER_HEIGHT + DrawConfig::OFFSET_Y + 5 + g_scroll_offset_y,
			buf, lstrlen(buf));
	}

	SelectObject(hdc, old_font);
	DeleteObject(font);
}

void DrawObjects(HDC hdc, const TimelineData& data) {
	HFONT font = CreateFont(
		DrawConfig::FONT_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		DrawConfig::FONT_NAME);

	HBRUSH obj_brush = CreateSolidBrush(DrawConfig::OBJECT_COLOR);
	HBRUSH sel_brush = CreateSolidBrush(DrawConfig::SELECTED_OBJECT_COLOR);
	HPEN obj_pen = CreatePen(PS_SOLID, 2, DrawConfig::OBJECT_BORDER_COLOR);
	HPEN sel_pen = CreatePen(PS_SOLID, 2, DrawConfig::SELECTED_OBJECT_BORDER_COLOR);

	HFONT old_font = (HFONT)SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, DrawConfig::OBJECT_TEXT_COLOR);

	for (const auto& obj : data.objects) {
		SelectObject(hdc, obj.is_selected ? sel_brush : obj_brush);
		SelectObject(hdc, obj.is_selected ? sel_pen : obj_pen);

		int x1 = obj.start * DrawConfig::FRAME_WIDTH + DrawConfig::OFFSET_X + g_scroll_offset_x;
		int x2 = (obj.end + 1) * DrawConfig::FRAME_WIDTH + DrawConfig::OFFSET_X + g_scroll_offset_x;

		int display_y = (data.max_layer - obj.layer) * DrawConfig::LAYER_HEIGHT + DrawConfig::OFFSET_Y + 2 + g_scroll_offset_y;
		int y1 = display_y;
		int y2 = y1 + DrawConfig::LAYER_HEIGHT - 4;

		Rectangle(hdc, x1, y1, x2, y2);

		if (!obj.name.empty()) {
			TextOut(hdc, x1 + 2, y1 + 5, obj.name.c_str(), (int)obj.name.length());
		}
	}

	SelectObject(hdc, old_font);
	DeleteObject(font);
	DeleteObject(obj_brush);
	DeleteObject(sel_brush);
	DeleteObject(obj_pen);
	DeleteObject(sel_pen);
}

void DrawCurrentFrame(HDC hdc, RECT rect, const TimelineData& data) {
	HPEN frame_pen = CreatePen(PS_SOLID, 2, DrawConfig::CURRENT_FRAME_COLOR);
	HPEN old_pen = (HPEN)SelectObject(hdc, frame_pen);

	int x = data.current_frame * DrawConfig::FRAME_WIDTH + DrawConfig::OFFSET_X + g_scroll_offset_x;
	MoveToEx(hdc, x, DrawConfig::OFFSET_Y, NULL);
	LineTo(hdc, x, rect.bottom);

	SelectObject(hdc, old_pen);
	DeleteObject(frame_pen);
}

void DrawFrameBoundaries(HDC hdc, RECT rect, const TimelineData& data) {
	HPEN boundary_pen = CreatePen(PS_SOLID, 2, DrawConfig::GRID_LINE_COLOR);
	HPEN old_pen = (HPEN)SelectObject(hdc, boundary_pen);

	int x_start = 0 * DrawConfig::FRAME_WIDTH + DrawConfig::OFFSET_X + g_scroll_offset_x;
	MoveToEx(hdc, x_start, DrawConfig::OFFSET_Y, NULL);
	LineTo(hdc, x_start, rect.bottom);

	int x_end = data.max_frame * DrawConfig::FRAME_WIDTH + DrawConfig::OFFSET_X + g_scroll_offset_x;
	MoveToEx(hdc, x_end, DrawConfig::OFFSET_Y, NULL);
	LineTo(hdc, x_end, rect.bottom);

	SelectObject(hdc, old_pen);
	DeleteObject(boundary_pen);
}

void DrawTopFixedArea(HDC hdc, RECT rect) {
	HBRUSH bg_brush = CreateSolidBrush(DrawConfig::BG_COLOR);
	RECT top_area = { 0, 0, rect.right, DrawConfig::OFFSET_Y };
	FillRect(hdc, &top_area, bg_brush);
	DeleteObject(bg_brush);
}

void DrawTimelineWindow(HDC hdc, RECT rect) {
	const TimelineData& data = GetTimelineData();

	DrawBackground(hdc, rect);
	DrawGrid(hdc, rect, data);
	DrawFrameBoundaries(hdc, rect, data);
	DrawObjects(hdc, data);
	DrawLayerLabels(hdc, data);
	DrawBaseLine(hdc, rect);
	DrawTopFixedArea(hdc, rect);
	DrawFrameInfo(hdc, data);
	DrawTimeGridLines(hdc, data);
	DrawCurrentFrame(hdc, rect, data);
}