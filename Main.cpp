#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>
#include "INIReader.h"
#pragma comment(lib, "Winmm.lib") //requires this library for PlaySound() to work
using namespace std;

//preprocessor
#define _PROJECTNAME_ "SLEA"
#define _DEFAULTEXE_ "FalloutNV.exe"

//globals
wstring dir, cmdline;

//struct for the enumwindows callback
struct EnumWindowsInfo {
	DWORD id;
	vector<HWND> hwnds; //vector for holding all windows found with the same process id
	EnumWindowsInfo(DWORD const ProcessID) : id(ProcessID) {}
};

//prototypes
wstring GetDir();
wstring s2ws(const string& s);
void CreateINI();
BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam);

BOOL WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow) {
	cmdline = pCmdLine; //get command line
	dir = GetDir(); //get directory

	INIReader ini(_PROJECTNAME_ + string(".ini"));
	if (ini.ParseError() < 0) {	//if the configuration file does not exist...
		PlaySound(L"SystemExclamation", NULL, SND_ASYNC); //play error sound asynchronously
		CreateINI();			//...then let's create a new one!
		return true;
	}

	string _cmdline = ini.Get("", "CommandLine", "");
	cmdline = _cmdline != "" ? s2ws(_cmdline) : cmdline; //if the configuration file's command line isn't empty, replace the program's command line with it

	wstring exePath = dir + s2ws(ini.Get("", "ExeName", ""));
	

	SHELLEXECUTEINFO sei_exe;
	ZeroMemory(&sei_exe, sizeof(SHELLEXECUTEINFO));
	sei_exe.cbSize = sizeof(SHELLEXECUTEINFO);
	sei_exe.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei_exe.lpVerb = L"open";
	sei_exe.lpFile = exePath.c_str();
	sei_exe.lpParameters = cmdline.c_str();
	sei_exe.nShow = SW_SHOWNORMAL;

	string injectorName = ini.Get("", "InjectorName", "");

	if (injectorName != "") { //if an injector is specified
		wstring injectorPath = dir + s2ws(injectorName);
		SHELLEXECUTEINFO sei_inj;
		ZeroMemory(&sei_inj, sizeof(SHELLEXECUTEINFO));
		sei_inj.cbSize = sizeof(SHELLEXECUTEINFO);
		sei_inj.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei_inj.lpVerb = L"open";
		sei_inj.lpFile = injectorPath.c_str();
		sei_inj.nShow = SW_SHOWNORMAL;
		if (ShellExecuteEx(&sei_inj)) { //run injector
			WaitForInputIdle(sei_inj.hProcess, INFINITE); //wait for injector to stabilize
			EnumWindowsInfo winf_inj = EnumWindowsInfo(GetProcessId(sei_inj.hProcess));
			EnumWindows(&EnumWindowsCallback, reinterpret_cast<LPARAM>(&winf_inj));
			//now that the injector is all setup, let's open the game executable!
			if (ShellExecuteEx(&sei_exe)) {
				WaitForSingleObject(sei_exe.hProcess, INFINITE); //wait for game process to stop
			}
			for (unsigned int i = 0; i < winf_inj.hwnds.size(); ++i)	//for each window related to the injector...
				PostMessage(winf_inj.hwnds[i], WM_CLOSE, NULL, NULL);	//...send a close message to it!
		}
	}
	else ShellExecuteEx(&sei_exe); //if no injector is specified, simply run the game executable 
	return false; //end program
}

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
	EnumWindowsInfo *info = reinterpret_cast<EnumWindowsInfo*>(lParam);
	DWORD id;
	GetWindowThreadProcessId(hwnd, &id);
	if (info->id == id) { //if window id equals the specified process id
		info->hwnds.push_back(hwnd); //add window to list of windows related to that process id
		return FALSE; //stop callback
	}
	else return TRUE; //repeat callback
}

void CreateINI() {
	ofstream ini(_PROJECTNAME_ + string(".ini"));
	ini << ";" << _PROJECTNAME_ << " Config File\n"
		<< ";\n"
		<< ";Set ExeName to whatever game executable you want to launch, like \"FalloutNV.exe\".\n"
		<< ";\n"
		<< ";Set InjectorName to whatever injector you wish to launch, like \"ENBInjector.exe\".\n"
		<< ";You can leave it blank to disable this feature.\n"
		<< ";\n"
		<< ";Set CommandLine to whatever arguments you wish to pass to the game executable.\n"
		<< ";Leave blank to automatically passthrough arguments given to " << _PROJECTNAME_ << " to the game executable.\n"
		<< ";\n"
		<< "ExeName=\n"
		<< "InjectorName=\n"
		<< "CommandLine=\n"
		<< flush; //force the file to be written before closing the program
	ini.close();
	MessageBox(
		NULL,
		L"No configuration file found, so a new one was created!",
		TEXT(_PROJECTNAME_),
		MB_OK
	);
	ShellExecute(
		NULL,
		L"open",
		(TEXT(_PROJECTNAME_) + wstring(L".ini")).c_str(),
		NULL,
		NULL,
		SW_SHOWNORMAL
	);
}

wstring GetDir() {
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	return wstring(buffer) + wstring(L"\\"); //return the absolute location and append a "\" at the end
}

wstring s2ws(const string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, NULL, s.c_str(), slength, NULL, NULL);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, NULL, s.c_str(), slength, buf, len);
	wstring r(buf);
	delete[] buf;
	return r;
}
