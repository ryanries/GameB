@echo off
echo.
echo ==================================
echo Clang x64 Debug Build Batch Script
echo ==================================
echo.
set WIN_SDK_PATH=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64
set OUTPUT_PATH=.\x64\Debug
set OUTPUT_EXE=%OUTPUT_PATH%\GameB_Clang.exe
@echo on
mkdir %OUTPUT_PATH%
clang -g -gcodeview Battle.c CharacterNamingScreen.c ExitYesNoScreen.c GamepadUnplugged.c Main.c ^
miniz.c NewGameAreYouSure.c OpeningSplashScreen.c OptionsScreen.c Overworld.c stb_vorbis.c TitleScreen.c ^
-std=c99 -O1 -Wall -march=core-avx2 ^
-D_DEBUG -DDEBUG -D_WIN64 -D_MBCS -DCLANG ^
-o %OUTPUT_EXE% ^
-l"%WIN_SDK_PATH%\user32.lib" ^
-l"%WIN_SDK_PATH%\AdvApi32.lib" ^
-l"%WIN_SDK_PATH%\Gdi32.lib" ^
-l"%WIN_SDK_PATH%\ole32.lib"
@echo off
if %ERRORLEVEL% NEQ 0 GOTO END
choice /M "Launch executable under debugger?"
if %ERRORLEVEL% EQU 1 (start windbgx %OUTPUT_EXE%)
:END