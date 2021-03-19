@echo off

if not exist build mkdir build
set CompilerRelease=-nologo -O2 -Oi -MT
set CompilerDebug=-nologo -Zi -MTd
set Linker=-nologo -link -subsystem:windows -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib advapi32.lib ole32.lib

pushd build
cl %CompilerDebug% -LD  ../GameCode/Main.c  %Linker% -EXPORT:TestFunc01 -OUT:GameCode.dll
cl %CompilerDebug% ../Main.c ../Battle.c ../CharacterNamingScreen.c ../ExitYesNoScreen.c  ../GamepadUnplugged.c ../miniz.c ../NewGameAreYouSure.c ../OpeningSplashScreen.c ../OptionsScreen.c ../Overworld.c ../stb_vorbis.c ../TitleScreen.c  %Linker%
del *.obj
popd
CopyAssets.bat