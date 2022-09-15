// Filename: Overworld.h
// Declarations specific to the "overworld" screen should go here.
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

#pragma once

// Could be a door, or a gateway, or a staircase, or a cave entrace, etc...
typedef struct PORTAL
{
	// Where is this portal located, in world coordinates?
	POINT WorldPos;

	// Where should the portal take the player, in world coordinates?
	POINT WorldDestination;

	// Where should the player be located, in screen coordinates?
	POINT ScreenDestination;

	// Where should the camera be located, in world coordinates?
	POINT CameraPos;

	// Which area does the portal take you to?
	GAMEAREA DestinationArea;

} PORTAL;



extern PORTAL gPortal001;

extern PORTAL gPortal002;

extern PORTAL gPortals[2];

extern GAMEAREA gCurrentArea;

extern GAMEAREA gOverworldArea;

extern GAMEAREA gHomeGameArea;

void PPI_Overworld(void);

void DrawOverworld(void);

void PortalHandler(void);

void RandomMonsterEncounter(void);

__forceinline int InventoryItemCount(void);