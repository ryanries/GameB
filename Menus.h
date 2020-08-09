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

	BOOL Enabled;

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

void MenuItem_OptionsScreen_SFXVolume(void);

void MenuItem_OptionsScreen_MusicVolume(void);

void MenuItem_OptionsScreen_ScreenSize(void);

void MenuItem_OptionsScreen_Back(void);

void MenuItem_CharacterNaming_Add(void);

void MenuItem_CharacterNaming_Back(void);

void MenuItem_CharacterNaming_OK(void);


// Title screen 

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2) - ((6 * 6) / 2), 100, FALSE, MenuItem_TitleScreen_Resume };

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 115, TRUE, MenuItem_TitleScreen_StartNew };

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((7 * 6) / 2), 130, TRUE, MenuItem_TitleScreen_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH / 2) - ((4 * 6) /2), 145, TRUE, MenuItem_TitleScreen_Exit };

MENUITEM* gMI_TitleScreenItems[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { "Title Screen Menu", 1, _countof(gMI_TitleScreenItems), gMI_TitleScreenItems };

///

// Exit Yes or No Screen

MENUITEM gMI_ExitYesNo_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, TRUE, MenuItem_ExitYesNo_Yes };

MENUITEM gMI_ExitYesNo_No = { "No",   (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 115, TRUE, MenuItem_ExitYesNo_No };

MENUITEM* gMI_ExitYesNoItems[] = { &gMI_ExitYesNo_Yes, &gMI_ExitYesNo_No };

MENU gMenu_ExitYesNo = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItems), gMI_ExitYesNoItems };

// Options Screen

MENUITEM gMI_OptionsScreen_SFXVolume = { "SFX Volume:", (GAME_RES_WIDTH / 2) - ((11 * 6) / 2) - 16, 100, TRUE, MenuItem_OptionsScreen_SFXVolume };

MENUITEM gMI_OptionsScreen_MusicVolume = { "Music Volume:", (GAME_RES_WIDTH / 2) - ((13 * 6) / 2) - 16, 115, TRUE, MenuItem_OptionsScreen_MusicVolume };

MENUITEM gMI_OptionsScreen_ScreenSize = { "Screen Size:", (GAME_RES_WIDTH / 2) - ((12 * 6) / 2) - 16, 130, TRUE, MenuItem_OptionsScreen_ScreenSize };

MENUITEM gMI_OptionsScreen_Back = { "Back", (GAME_RES_WIDTH / 2) - ((4 * 6) / 2) - 16, 145, TRUE, MenuItem_OptionsScreen_Back };

MENUITEM* gMI_OptionsScreenItems[] = { &gMI_OptionsScreen_SFXVolume, &gMI_OptionsScreen_MusicVolume, &gMI_OptionsScreen_ScreenSize, &gMI_OptionsScreen_Back };

MENU gMenu_OptionsScreen = { "Options", 0, _countof(gMI_OptionsScreenItems), gMI_OptionsScreenItems };

// Character Naming Menu

// A B C D E F G H I J K L M 
// N O P Q R S T U V W X Y Z
// a b c d e f g h i j k l m
// n o p q r s t u v w x y z
// Back                   OK

                                 // (12 * 8) + (13 * 6) 12 spaces at 8 pixels each, plus 13 letters at 6 pixels wide each
MENUITEM gMI_CharacterNaming_A = { "A", 120, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_B = { "B", 132, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_C = { "C", 144, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_D = { "D", 156, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_E = { "E", 168, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_F = { "F", 180, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_G = { "G", 192, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_H = { "H", 204, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_I = { "I", 216, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_J = { "J", 228, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_K = { "K", 240, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_L = { "L", 252, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_M = { "M", 264, 100, TRUE, MenuItem_CharacterNaming_Add };



MENUITEM gMI_CharacterNaming_N = { "N", 120, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_O = { "O", 132, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_P = { "P", 144, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_Q = { "Q", 156, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_R = { "R", 168, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_S = { "S", 180, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_T = { "T", 192, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_U = { "U", 204, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_V = { "V", 216, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_W = { "W", 228, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_X = { "X", 240, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_Y = { "Y", 252, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_Z = { "Z", 264, 110, TRUE, MenuItem_CharacterNaming_Add };


MENUITEM gMI_CharacterNaming_a = { "a", 120, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_b = { "b", 132, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_c = { "c", 144, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_d = { "d", 156, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_e = { "e", 168, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_f = { "f", 180, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_g = { "g", 192, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_h = { "h", 204, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_i = { "i", 216, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_j = { "j", 228, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_k = { "k", 240, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_l = { "l", 252, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_m = { "m", 264, 120, TRUE, MenuItem_CharacterNaming_Add };


MENUITEM gMI_CharacterNaming_n = { "n", 120, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_o = { "o", 132, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_p = { "p", 144, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_q = { "q", 156, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_r = { "r", 168, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_s = { "s", 180, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_t = { "t", 192, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_u = { "u", 204, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_v = { "v", 216, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_w = { "w", 228, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_x = { "x", 240, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_y = { "y", 252, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNaming_z = { "z", 264, 130, TRUE, MenuItem_CharacterNaming_Add };



MENUITEM gMI_CharacterNaming_Back = { "Back", 120, 140, TRUE, MenuItem_CharacterNaming_Back };

MENUITEM gMI_CharacterNaming_OK = { "OK", 258, 140, TRUE, MenuItem_CharacterNaming_OK };


MENUITEM* gMI_CharacterNamingItems[] = { 
	&gMI_CharacterNaming_A, &gMI_CharacterNaming_B, &gMI_CharacterNaming_C, &gMI_CharacterNaming_D, &gMI_CharacterNaming_E, &gMI_CharacterNaming_F, 
	&gMI_CharacterNaming_G, &gMI_CharacterNaming_H, &gMI_CharacterNaming_I, &gMI_CharacterNaming_J, &gMI_CharacterNaming_K, &gMI_CharacterNaming_L, 
	&gMI_CharacterNaming_M, &gMI_CharacterNaming_N, &gMI_CharacterNaming_O, &gMI_CharacterNaming_P, &gMI_CharacterNaming_Q, &gMI_CharacterNaming_R,
	&gMI_CharacterNaming_S, &gMI_CharacterNaming_T, &gMI_CharacterNaming_U, &gMI_CharacterNaming_V, &gMI_CharacterNaming_W, &gMI_CharacterNaming_X,
	&gMI_CharacterNaming_Y, &gMI_CharacterNaming_Z,
	&gMI_CharacterNaming_a, &gMI_CharacterNaming_b, &gMI_CharacterNaming_c, &gMI_CharacterNaming_d, &gMI_CharacterNaming_e, &gMI_CharacterNaming_f,
	&gMI_CharacterNaming_g, &gMI_CharacterNaming_h, &gMI_CharacterNaming_i, &gMI_CharacterNaming_j, &gMI_CharacterNaming_k, &gMI_CharacterNaming_l,
	&gMI_CharacterNaming_m, &gMI_CharacterNaming_n, &gMI_CharacterNaming_o, &gMI_CharacterNaming_p, &gMI_CharacterNaming_q, &gMI_CharacterNaming_r,
	&gMI_CharacterNaming_s, &gMI_CharacterNaming_t, &gMI_CharacterNaming_u, &gMI_CharacterNaming_v, &gMI_CharacterNaming_w, &gMI_CharacterNaming_x,
	&gMI_CharacterNaming_y, &gMI_CharacterNaming_z, &gMI_CharacterNaming_Back, &gMI_CharacterNaming_OK };

MENU gMenu_CharacterNaming = { "What's your name, hero?", 0, _countof(gMI_CharacterNamingItems), gMI_CharacterNamingItems };