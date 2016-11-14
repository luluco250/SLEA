::SLEA Helper v1.0
@echo off
SetLocal EnableDelayedExpansion
set _PROJECTNAME_=SLEA

echo SLEA Setup Helper v1.0
echo This script will create a SLEA configuration file for you,
echo as well as setup it's environment (such as renaming the launcher).
echo.
echo What operation would you like to do?
echo 1 - Install SLEA
echo 2 - Uninstall SLEA
echo 3 - Reinstall SLEA
echo.
set /p _OPERATION_=Selection: 
echo.
if not [%_OPERATION_%]==[1] goto:uninstall

:install

if [%1]==[] goto:noexe
set _EXENAME_=%~n1
goto:haveexe

:noexe
echo Let's setup the launcher name...
echo Tip: You can skip this step next time you run this script by
echo drag and dropping the launcher executable on it.
echo Tip: Pressing tab can auto-complete names.
echo.
echo If you do not wish to replace the launcher with SLEA 
echo (a backup of it is made) simply press enter.
echo Type the launcher exe name (include the ".exe" at the end):
echo.
set /p _LAUNCHERNAME_=Launcher Name:  
echo.

:haveexe
echo Type the name of the game executable to launch:
echo Tip: Pressing tab can auto-complete names.
echo.
set /p _EXENAME_=Executable Name: 
echo.

echo Type the name of the injector executable to launch (ENB's is named ENBInjector.exe)
echo (optional, just hit enter if you don't need this feature):
echo Tip: Pressing tab can auto-complete names.
echo.
set /p _INJECTORNAME_=Injector Name: 
echo.

echo Do you wish to set a custom command line to launch the game executable with?
echo If so, type it's name here, otherwise leave it blank and just press enter:
echo Tip: Pressing tab can auto-complete names.
echo.
set /p _COMMANDLINE_=Command Line: 
echo.

::Rename files only if a launcher name was specified
if not [%_LAUNCHERNAME_%]==[] (
    ren "%_LAUNCHERNAME_%" "%_LAUNCHERNAME_%.%_PROJECTNAME_%_BAK"
    ren "%_PROJECTNAME_%.exe" "%_LAUNCHERNAME_%"
)

::Write configuration file
(
    echo ;%_PROJECTNAME_% Config File
    echo ;
    echo ;Set ExeName to whatever game executable you want to launch, like "FalloutNV.exe".
    echo ;
    echo ;Set InjectorName to whatever injector you wish to launch, like "ENBInjector.exe".
    echo ;You can leave it blank to disable this feature.
    echo ;
    echo ;Set CommandLine to whatever arguments you wish to pass to the game executable.
    echo ;Leave blank to automatically passthrough arguments given to %_PROJECTNAME_% to the game executable.
    echo ;
    echo ;Should you wish to hear a feedback beep when %_PROJECTNAME_% loads successfully, leave DoBeep at true.
    echo ;Otherwise set to false.
    echo ;
    echo ExeName=%_EXENAME_%
    echo InjectorName=%_INJECTORNAME_%
    echo CommandLine=%_COMMANDLINE_%
) > %_PROJECTNAME_%.ini
echo Installation successful!
pause
exit

:uninstall
::Check if a *.SLEA_BAK file exists (the backup of the launcher executable)
for %%i in (*) do (
    if [%%~xi]==[.%_PROJECTNAME_%_BAK] (
        ren "%%~ni" "%_PROJECTNAME_%.exe"
        ren "%%~nxi" "%%~ni"
    )
)
echo.
echo Uninstallation successful!
echo.
if [%_OPERATION_%]==[3] goto:install
pause
exit