#include "Main.h"

_declspec(dllexport) void RandomMonsterEncounter(_In_ GAMESTATE* PreviousGameState, _Inout_ GAMESTATE* CurrentGameState)
{
    PreviousGameState = CurrentGameState;

    *CurrentGameState = GAMESTATE_BATTLE;
}