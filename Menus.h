//# License
//----------
//The source code in this project is licensed under the MIT license.
//The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
//A copy of that license can be found in the 'Assets' directory.

#pragma once

#include <stdint.h>

typedef struct MENUITEM
{
	char* Name;

	int16_t x;

	int16_t y;

	void(*Action)(void);

} MENUITEM;

typedef struct MENU
{
	char* Name;

	uint8_t SelectedItem;

	uint8_t ItemCount;

	MENUITEM** Items;

} MENU;

void MenuItem_TitleScreen_Resume(void);

void MenuItem_TitleScreen_StartNew(void);

void MenuItem_TitleScreen_Options(void);

void MenuItem_TitleScreen_Exit(void);

void MenuItem_ExitYesNo_Yes(void);

void MenuItem_ExitYesNo_No(void);

// Title screen 

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2) - ((6 * 6) / 2), 100, MenuItem_TitleScreen_Resume };

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 115, MenuItem_TitleScreen_StartNew };

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((7 * 6) / 2), 130, MenuItem_TitleScreen_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH / 2) - ((4 * 6) /2), 145, MenuItem_TitleScreen_Exit };

MENUITEM* gMI_TitleScreenItems[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { "Title Screen Menu", 1, _countof(gMI_TitleScreenItems), gMI_TitleScreenItems };

///

// Exit Yes or No Screen

MENUITEM gMI_ExitYesNo_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, MenuItem_ExitYesNo_Yes };

MENUITEM gMI_ExitYesNo_No = { "No",   (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 115, MenuItem_ExitYesNo_No };

MENUITEM* gMI_ExitYesNoItems[] = { &gMI_ExitYesNo_Yes, &gMI_ExitYesNo_No };

MENU gMenu_ExitYesNo = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItems), gMI_ExitYesNoItems };