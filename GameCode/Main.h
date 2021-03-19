#pragma once

// Temporarily reduce warning level while including header files over which we have no control.
#pragma warning(push, 3)	
// The Windows API.
#include <Windows.h>

// Restore warning level to /Wall.
#pragma warning(pop)

// The game must always be in 1 and only 1 state at a time.
// The game starts in the "opening splash screen" state, then
// transitions to the title screen state, etc. Some 
// transitions are valid and others are not. E.g., we should not
// be able to transition from the battle gamestate directly to the
// splash screen gamestate or vice versa. We track this with the 
// gCurrentGameState and gPreviousGameState variables.
typedef enum GAMESTATE
{
	GAMESTATE_OPENINGSPLASHSCREEN,

	GAMESTATE_TITLESCREEN,

	GAMESTATE_CHARACTERNAMING,

	GAMESTATE_OVERWORLD,

	GAMESTATE_BATTLE,

	GAMESTATE_OPTIONSSCREEN,

	GAMESTATE_EXITYESNOSCREEN,

	GAMESTATE_GAMEPADUNPLUGGED,

	GAMESTATE_NEWGAMEAREYOUSURE

} GAMESTATE;