// Filename: OptionsScreen.c
// Contains code for the functions that are specific to the options screen.
//
// Project Codename: GameB
// TODO: Come up with a better name later.
// 2020 Joseph Ryan Ries <ryanries09@gmail.com>
// My YouTube series where we program an entire video game from scratch in C.
// Watch it on YouTube:    https://www.youtube.com/watch?v=3zFFrBSdBvA
// Follow along on GitHub: https://github.com/ryanries/GameB
// Find me on Twitter @JosephRyanRies 
// # License
// ----------
// The source code in this project is licensed under the MIT license.
// The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
// A copy of that license can be found in the 'Assets' directory.
// stb_vorbis by Sean Barrett is public domain and a copy of its license can be found in the stb_vorbis.c file.

#include "Main.h"

#include "OptionsScreen.h"

MENUITEM gMI_OptionsScreen_SFXVolume = { "SFX Volume:", (GAME_RES_WIDTH / 2) - ((11 * 6) / 2) - 16, 100, TRUE, MenuItem_OptionsScreen_SFXVolume };

MENUITEM gMI_OptionsScreen_MusicVolume = { "Music Volume:", (GAME_RES_WIDTH / 2) - ((13 * 6) / 2) - 16, 115, TRUE, MenuItem_OptionsScreen_MusicVolume };

MENUITEM gMI_OptionsScreen_ScreenSize = { "Screen Size:", (GAME_RES_WIDTH / 2) - ((12 * 6) / 2) - 16, 130, TRUE, MenuItem_OptionsScreen_ScreenSize };

MENUITEM gMI_OptionsScreen_Back = { "Back", (GAME_RES_WIDTH / 2) - ((4 * 6) / 2) - 16, 145, TRUE, MenuItem_OptionsScreen_Back };

MENUITEM* gMI_OptionsScreenItems[] = { &gMI_OptionsScreen_SFXVolume, &gMI_OptionsScreen_MusicVolume, &gMI_OptionsScreen_ScreenSize, &gMI_OptionsScreen_Back };

MENU gMenu_OptionsScreen = { "Options", 0, _countof(gMI_OptionsScreenItems), gMI_OptionsScreenItems };

void DrawOptionsScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen = 0;

    static int AlphaAdjust = -256;

    char ScreenSizeString[64] = { 0 };

    // If global TotalFramesRendered is greater than LastFrameSeen,
    // that means we have either just entered this gamestate for the first time,
    // or we have left this gamestate previously and have just come back to it.
    // For example we may have gone from the title screen, to the options screen,
    // and then back to the title screen again. In that case, we want to reset all
    // of the "local state," i.e., things that are local to this game state. Such
    // as text animation, selected menu item, etc.
    if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        gMenu_OptionsScreen.SelectedItem = 0;

        AlphaAdjust = -256;

        gInputEnabled = FALSE;
    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

#ifdef SMOOTH_MENU_FADES
    // Here is a smoother fade in that looks nicer, but the original NES was not capable of such smooth gradients and fade
    // effects. We will have to decide which we prefer later - looks better, or is more faithful to the original hardware?

    if (AlphaAdjust < 0)
    {
        AlphaAdjust += 4;
    }

#else
    // Here is an easy, "chunky" fade-in from black in 4 steps, that sort of has a similar feel
    // to the kind of fade-in you might have seen on the classic NES. AlphaAdjust starts at -256 and ends at 0.
    switch (LocalFrameCounter)
    {
        case 15:
        case 30:
        case 45:
        case 60:
        {
            AlphaAdjust += 64;
        }
    }
#endif

    // It doesn't feel very nice to have to wait the full 60 frames for the fade-in to complete in order for 
    // input to be enabled again. We should enable it sooner so the kids with fast reflexes can work the menus quickly.
    if (LocalFrameCounter == REENABLE_INPUT_AFTER_X_FRAMES_DELAY)
    {
        gInputEnabled = TRUE;
    }

    for (int MenuItem = 0; MenuItem < gMenu_OptionsScreen.ItemCount; MenuItem++)
    {
        if (gMenu_OptionsScreen.Items[MenuItem]->Enabled == TRUE)
        {
            BlitStringToBufferEx(
                gMenu_OptionsScreen.Items[MenuItem]->Name,
                &g6x7Font,                
                gMenu_OptionsScreen.Items[MenuItem]->x,
                gMenu_OptionsScreen.Items[MenuItem]->y,
                255,
                255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        }
    }

    for (int Volume = 0; Volume < 10; Volume++)
    {
        if (Volume >= (int)(gSFXVolume * 10))
        {
            BlitStringToBufferEx(
                "\xf2", 
                &g6x7Font,                
                224 + (Volume * 6), 
                gMI_OptionsScreen_SFXVolume.y,
                32,
                32,
                32,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        }
        else
        {
            BlitStringToBufferEx(
                "\xf2", 
                &g6x7Font,                
                224 + (Volume * 6), 
                gMI_OptionsScreen_SFXVolume.y,
                255,
                255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        }
    }

    for (int Volume = 0; Volume < 10; Volume++)
    {
        if (Volume >= (int)(gMusicVolume * 10))
        {
            BlitStringToBufferEx(
                "\xf2", 
                &g6x7Font,                
                224 + (Volume * 6), 
                gMI_OptionsScreen_MusicVolume.y,
                32,
                32,
                32,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);  
        }
        else
        {
            BlitStringToBufferEx(
                "\xf2", 
                &g6x7Font, 
                224 + (Volume * 6), 
                gMI_OptionsScreen_MusicVolume.y,
                255,
                255,
                255,
                AlphaAdjust,
                BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);
        }
    }

    snprintf(ScreenSizeString,
        sizeof(ScreenSizeString),
        "%dx%d",
        GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor,
        GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor);

    BlitStringToBufferEx(
        ScreenSizeString, 
        &g6x7Font, 
        224, 
        gMI_OptionsScreen_ScreenSize.y,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    BlitStringToBufferEx(
        "\xBB",
        &g6x7Font,        
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->x - 6,
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->y,
        255,
        255,
        255,
        AlphaAdjust,
        BLIT_FLAG_ALPHABLEND | BLIT_FLAG_TEXT_SHADOW);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void PPI_OptionsScreen(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_OptionsScreen.SelectedItem == gMenu_OptionsScreen.ItemCount - 1)
        {
            gMenu_OptionsScreen.SelectedItem = 0;
        }
        else
        {
            gMenu_OptionsScreen.SelectedItem++;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_OptionsScreen.SelectedItem == 0)
        {
            gMenu_OptionsScreen.SelectedItem = gMenu_OptionsScreen.ItemCount - 1;            
        }
        else
        {
            gMenu_OptionsScreen.SelectedItem--;
        }

        PlayGameSound(&gSoundMenuNavigate);
    }

    
    if ((gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown))
    {
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
    else if ((gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown) ||        
             (gGameInput.RightKeyIsDown && !gGameInput.RightKeyWasDown))    
    {
        // Allow the left and right buttons to work in the options menu,
        // but not on the back button. Must still use the choose key to go back.
        if (gMenu_OptionsScreen.SelectedItem != gMenu_OptionsScreen.ItemCount - 1)
        {
            gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->Action();

            PlayGameSound(&gSoundMenuChoose);
        }
    }

    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        MenuItem_OptionsScreen_Back();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_OptionsScreen_Back(void)
{
    ASSERT(gCurrentGameState == GAMESTATE_OPTIONSSCREEN, "Invalid game state!");

    gCurrentGameState = gPreviousGameState;

    gPreviousGameState = GAMESTATE_OPTIONSSCREEN;

    LogMessageA(LL_INFO, "[%s] Transitioning from game state %d to %d. Player selected '%s' from '%s' menu.",
        __FUNCTION__,
        gPreviousGameState,
        gCurrentGameState,
        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->Name,
        gMenu_OptionsScreen.Name);

    if (SaveRegistryParameters() != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] SaveRegistryParameters failed!", __FUNCTION__);
    }
}

void MenuItem_OptionsScreen_ScreenSize(void)
{
    if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
    {
        gPerformanceData.CurrentScaleFactor--;        
    }
    else
    {
        gPerformanceData.CurrentScaleFactor++;
    }

    if (gPerformanceData.CurrentScaleFactor < 1)
    {
        gPerformanceData.CurrentScaleFactor = gPerformanceData.MaxScaleFactor;
    }

    if (gPerformanceData.CurrentScaleFactor > gPerformanceData.MaxScaleFactor)
    {
        gPerformanceData.CurrentScaleFactor = 1;
    }

    InvalidateRect(gGameWindow, NULL, TRUE);
}

void MenuItem_OptionsScreen_MusicVolume(void)
{
    if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
    {
        gMusicVolume -= 0.1f;
    }
    else
    {
        gMusicVolume += 0.1f;
    }

    if (gMusicVolume < 0.0f)
    {
        gMusicVolume = 1.0f;
    }  

    if ((uint8_t)(gMusicVolume * 10) > 10)
    {
        gMusicVolume = 0.0f;
    }

    gXAudioMusicSourceVoice->lpVtbl->SetVolume(gXAudioMusicSourceVoice, gMusicVolume, XAUDIO2_COMMIT_NOW);
}

void MenuItem_OptionsScreen_SFXVolume(void)
{
    if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
    {
        gSFXVolume -= 0.1f;
    }
    else
    {
        gSFXVolume += 0.1f;
    }

    if (gSFXVolume < 0.0f)
    {
        gSFXVolume = 1.0f;
    }

    if ((uint8_t)(gSFXVolume * 10) > 10)
    {
        gSFXVolume = 0.0f;
    }

    for (uint8_t Counter = 0; Counter < NUMBER_OF_SFX_SOURCE_VOICES; Counter++)
    {
        gXAudioSFXSourceVoice[Counter]->lpVtbl->SetVolume(gXAudioSFXSourceVoice[Counter], gSFXVolume, XAUDIO2_COMMIT_NOW);
    }
}