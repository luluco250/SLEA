#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "ini.h"

static const char* str_cfg = "SLEA.ini";
static const char* str_defcfg = 
    "#SLEA configurations\n"
    "#\n"
    "#1-ExecutableName:\n"
    "#\tName of the game executable (like game.exe) that you want to launch.\n"
    "#\n"
    "#2-CommandLine:\n"
    "#\tCommand line parameters you wish to pass to the game executable.\n"
    "#\tLeave empty if unsure. Will be overriden if you pass parameters to SLEA's executable.\n"
    "#\n"
    "#3-InjectorName:\n"
    "#\tName of an optional ENBInjector (or really any other exe) that you want to launch.\n"
    "#\tWill run in the background (there'll be no Window visible).\n"
    "#\tSLEA will close it automatically after the game quits.\n"
    "#\n"
    "#SLEA configurations\n\n"
    "ExecutableName=\n"
    "CommandLine=\n"
    "InjectorName=\n"
;

typedef struct {
    const char* exe_name;
    const char* cmd_line;
    const char* inj_name;
} config;

static int config_handler(void*, const char*, const char*, const char*);
bool fileexists(const char*);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    //load configuration ini file
    config cfg;
    if (ini_parse(str_cfg, config_handler, &cfg) < 0) {
        //if the configuration file couldn't be loaded
        //we create a new one and exit SLEA
        FILE* file_cfg = fopen(str_cfg, "w");
        fprintf(file_cfg, str_defcfg);
        if (fclose(file_cfg) == 0)
            MessageBox(0, "No SLEA.ini found, so one was created!", "SLEA", MB_OK | MB_ICONWARNING);
        else
            MessageBox(0, "Unable to create file!\nTry running SLEA as admin or create the configuration file (SLEA.ini) manually.", "SLEA", MB_OK | MB_ICONERROR);
        return 1;
    }

    //if an executable name is specified
    if (strcmp(cfg.exe_name, "") != 0) {
        if (!fileexists(cfg.exe_name)) {
            MessageBox(0, "Unable to launch executable!\nCheck if the file specified in SLEA.ini exists.", "SLEA", MB_OK | MB_ICONERROR);
        }
        else {
            SHELLEXECUTEINFO inj_sei;
            if (strcmp(cfg.inj_name, "") != 0) {
                if (!fileexists(cfg.inj_name))
                    MessageBox(0, "Unable to launch injector!\nCheck if the file specified in SLEA.ini exists.", "SLEA", MB_OK | MB_ICONERROR);
                else {
                    ZeroMemory(&inj_sei, sizeof(SHELLEXECUTEINFO));
                    inj_sei.cbSize = sizeof(SHELLEXECUTEINFO);
                    inj_sei.fMask = SEE_MASK_NOCLOSEPROCESS;
                    inj_sei.lpFile = cfg.inj_name;
                    ShellExecuteEx(&inj_sei);
                }    
            }
            
            SHELLEXECUTEINFO exe_sei;
            ZeroMemory(&exe_sei, sizeof(SHELLEXECUTEINFO));
            exe_sei.cbSize = sizeof(SHELLEXECUTEINFO);
            exe_sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            exe_sei.lpFile = cfg.exe_name;
            //override command line parameters if any are passed
            exe_sei.lpParameters = strcmp(lpCmdLine, "") == 0 ? cfg.cmd_line : lpCmdLine;
            
            ShellExecuteEx(&exe_sei);
            WaitForSingleObject(exe_sei.hProcess, INFINITE);

            if (inj_sei.hProcess)
                TerminateProcess(inj_sei.hProcess, 0);
        }
    }
    

    return 0;
}

static int config_handler(void* user, const char* section, const char* name, const char* value) {
    config* config_p = (config*)user;

    if (strcmp("ExecutableName", name) == 0) {
        config_p->exe_name = strdup(value);
    } else if (strcmp("CommandLine", name) == 0) {
        config_p->cmd_line = strdup(value);
    } else if (strcmp("InjectorName", name) == 0) {
        config_p->inj_name = strdup(value);
    } else {
        return 0; //unhandled values
    }

    return 1;
}

bool fileexists(const char* filepath) {
    DWORD attribs = GetFileAttributes(filepath);
    return (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
}
