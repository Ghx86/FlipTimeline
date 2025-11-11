//----------------------------------------------------------------------------------
//	Read.cpp - Data acquisition and definition
//----------------------------------------------------------------------------------
#include <windows.h>
#include "Read.h"
#include "../plugin2.h"

TimelineData g_timeline_data;

//---------------------------------------------------------------------
//	Get timeline information from AviUtl
//---------------------------------------------------------------------
void ReadTimelineData(EDIT_HANDLE* edit_handle) {
	if (!edit_handle) return;

	g_timeline_data.Clear();

	edit_handle->call_edit_section([](EDIT_SECTION* edit) {
		g_timeline_data.max_frame = edit->info->frame_max;
		g_timeline_data.max_layer = edit->info->layer_max;
		g_timeline_data.current_frame = edit->info->frame;
		g_timeline_data.rate = edit->info->rate;
		g_timeline_data.scale = edit->info->scale;

		std::vector<OBJECT_HANDLE> selected_objects;
		int selected_num = edit->get_selected_object_num();
		for (int i = 0; i < selected_num; i++) {
			auto obj = edit->get_selected_object(i);
			if (obj) selected_objects.push_back(obj);
		}

		for (int layer = 0; layer <= edit->info->layer_max; layer++) {
			int frame = 0;
			while (frame <= edit->info->frame_max) {
				auto obj = edit->find_object(layer, frame);
				if (obj) {
					auto lf = edit->get_object_layer_frame(obj);
					auto alias = edit->get_object_alias(obj);

					ObjectInfo info;
					info.layer = lf.layer;
					info.start = lf.start;
					info.end = lf.end;
					info.handle = obj;

					for (auto sel : selected_objects) {
						if (sel == obj) {
							info.is_selected = true;
							break;
						}
					}

					if (alias) {
						std::string str(alias);
						size_t pos = str.find("effect.name=");
						if (pos != std::string::npos) {
							pos += 12;
							size_t end_pos = str.find('\n', pos);
							if (end_pos == std::string::npos) end_pos = str.length();
							std::string name_utf8 = str.substr(pos, end_pos - pos);

							int len = MultiByteToWideChar(CP_UTF8, 0, name_utf8.c_str(), -1, NULL, 0);
							if (len > 0) {
								std::vector<wchar_t> buf(len);
								MultiByteToWideChar(CP_UTF8, 0, name_utf8.c_str(), -1, buf.data(), len);
								info.name = buf.data();
							}
						}
					}

					g_timeline_data.objects.push_back(info);
					frame = lf.end + 1;
				}
				else {
					break;
				}
			}
		}
		});
}

//---------------------------------------------------------------------
//	Update current frame only
//---------------------------------------------------------------------
void UpdateCurrentFrame(EDIT_HANDLE* edit_handle) {
	if (!edit_handle) return;

	edit_handle->call_edit_section([](EDIT_SECTION* edit) {
		g_timeline_data.current_frame = edit->info->frame;
		});
}

//---------------------------------------------------------------------
//	Access function to timeline data
//---------------------------------------------------------------------
const TimelineData& GetTimelineData() {
	return g_timeline_data;
}

//---------------------------------------------------------------------
//	Select object
//---------------------------------------------------------------------
struct SelectObjectData {
	OBJECT_HANDLE obj_handle;
	bool add_selection;
};

static void SelectObjectProc(EDIT_SECTION* edit) {
	SelectObjectData* data = (SelectObjectData*)edit;
}

static SelectObjectData g_select_data;

void SelectObject(EDIT_HANDLE* edit_handle, void* obj_handle, bool add_selection) {
	if (!edit_handle) return;

	g_select_data.obj_handle = (OBJECT_HANDLE)obj_handle;
	g_select_data.add_selection = add_selection;

	edit_handle->call_edit_section([](EDIT_SECTION* edit) {
		if (!g_select_data.add_selection) {
			int num = edit->get_selected_object_num();
			for (int i = num - 1; i >= 0; i--) {
				auto obj = edit->get_selected_object(i);
				if (obj && obj != g_select_data.obj_handle) {
					edit->set_focus_object(nullptr);
				}
			}
		}
		if (g_select_data.obj_handle) {
			edit->set_focus_object(g_select_data.obj_handle);
		}
		});
}

//---------------------------------------------------------------------
//	Move object
//---------------------------------------------------------------------
struct MoveObjectData {
	OBJECT_HANDLE obj_handle;
	int new_layer;
	int new_start;
};

static MoveObjectData g_move_data;

void MoveObject(EDIT_HANDLE* edit_handle, void* obj_handle, int new_layer, int new_start) {
	if (!edit_handle || !obj_handle) return;

	g_move_data.obj_handle = (OBJECT_HANDLE)obj_handle;
	g_move_data.new_layer = new_layer;
	g_move_data.new_start = new_start;

	edit_handle->call_edit_section([](EDIT_SECTION* edit) {
		edit->move_object(g_move_data.obj_handle, g_move_data.new_layer, g_move_data.new_start);
		});

	for (auto& obj : g_timeline_data.objects) {
		if (obj.handle == obj_handle) {
			int duration = obj.end - obj.start;
			obj.layer = new_layer;
			obj.start = new_start;
			obj.end = new_start + duration;
			break;
		}
	}
}

//---------------------------------------------------------------------
//	Update object display only
//---------------------------------------------------------------------
void UpdateObjectDisplay(void* obj_handle, int new_layer, int new_start) {
	for (auto& obj : g_timeline_data.objects) {
		if (obj.handle == obj_handle) {
			int duration = obj.end - obj.start;
			obj.layer = new_layer;
			obj.start = new_start;
			obj.end = new_start + duration;
			break;
		}
	}
}

//---------------------------------------------------------------------
//	Set current frame
//---------------------------------------------------------------------
static int g_set_frame = 0;

void SetCurrentFrame(EDIT_HANDLE* edit_handle, int frame) {
	if (!edit_handle) return;

	g_set_frame = frame;

	edit_handle->call_edit_section([](EDIT_SECTION* edit) {
		if (g_set_frame < 0) g_set_frame = 0;
		if (g_set_frame > edit->info->frame_max) g_set_frame = edit->info->frame_max;
		edit->info->frame = g_set_frame;
		});
}