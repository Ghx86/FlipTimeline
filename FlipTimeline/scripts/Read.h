//----------------------------------------------------------------------------------
//	Read.h - Data acquisition and definition (Header)
//----------------------------------------------------------------------------------
#ifndef READ_H
#define READ_H

#include <windows.h>
#include <vector>
#include <string>

struct EDIT_HANDLE;

//---------------------------------------------------------------------
//	Object information structure
//---------------------------------------------------------------------
struct ObjectInfo {
	int layer;
	int start;
	int end;
	std::wstring name;
	bool is_selected;
	void* handle;

	ObjectInfo() : layer(0), start(0), end(0), is_selected(false), handle(nullptr) {}
};

//---------------------------------------------------------------------
//	Timeline data class
//---------------------------------------------------------------------
class TimelineData {
public:
	std::vector<ObjectInfo> objects;
	int max_frame;
	int max_layer;
	int current_frame;
	int rate;
	int scale;

	TimelineData() : max_frame(100), max_layer(10), current_frame(0), rate(24000), scale(1000) {}

	void Clear() {
		objects.clear();
		max_frame = 100;
		max_layer = 10;
		current_frame = 0;
		rate = 24000;
		scale = 1000;
	}
};

//---------------------------------------------------------------------
//	Function declarations
//---------------------------------------------------------------------
void ReadTimelineData(EDIT_HANDLE* edit_handle);
void UpdateCurrentFrame(EDIT_HANDLE* edit_handle);
void SelectObject(EDIT_HANDLE* edit_handle, void* obj_handle, bool add_selection);
void MoveObject(EDIT_HANDLE* edit_handle, void* obj_handle, int new_layer, int new_start);
void UpdateObjectDisplay(void* obj_handle, int new_layer, int new_start);
void SetCurrentFrame(EDIT_HANDLE* edit_handle, int frame);
const TimelineData& GetTimelineData();

#endif // READ_H