//----------------------------------------------------------------------------------
//	Simple Timeline Window Plugin for AviUtl ExEdit2
//	Main file
//----------------------------------------------------------------------------------
#include <windows.h>
#include "plugin2.h"

// Function declaration in scripts folder
extern HWND CreateTimelineWindow(HOST_APP_TABLE* host);

//---------------------------------------------------------------------
//	Plugin DLL initialization
//---------------------------------------------------------------------
EXTERN_C __declspec(dllexport) bool InitializePlugin(DWORD version) {
	return true;
}

//---------------------------------------------------------------------
//	Plugin DLL cleanup
//---------------------------------------------------------------------
EXTERN_C __declspec(dllexport) void UninitializePlugin() {
}

//---------------------------------------------------------------------
//	Plugin registration
//---------------------------------------------------------------------
EXTERN_C __declspec(dllexport) void RegisterPlugin(HOST_APP_TABLE* host) {
	host->set_plugin_information(L"Filp Timeline beta_v0.1 by hato");

	// Create timeline window
	CreateTimelineWindow(host);
}