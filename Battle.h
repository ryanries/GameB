// Filename: Battle.h
// Declarations specific to the battle game state should go here.
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
// miniz by Rich Geldreich <richgel99@gmail.com> is public domain (or possibly MIT licensed) and a copy of its license can be found in the miniz.c file.

#pragma once

typedef struct SPELL
{
	const char Name[13];

	int16_t BaseDamage;

	int16_t MPCost;

} SPELL;

typedef struct MONSTER
{
	const char Name[13];

	GAMEBITMAP* Sprite;	

	int16_t BaseHP;

	int16_t BaseMP;

	int16_t BaseXP;

	int16_t BaseGP;

	int16_t BaseDamage;

	// The sum of these three action percentages should add up to 100(%)

	uint8_t RegularAttackChance;	// e.g. 80

	uint8_t RegularCastSpellChance;	// e.g. 10

	uint8_t RegularRunChance;		// e.g. 10

	// The sum of these three action percentages should add up to 100(%)

	uint8_t LowHPAttackChance;		// e.g. 75

	uint8_t LowHPCastSpellChance;	// e.g. 0

	uint8_t LowHPRunChance;			// e.g. 25

	uint8_t LowHPThresholdPercent;	// e.g. 25 at what point does the monster consider itself to be "low?"

	uint8_t CriticalChance;			// e.g. 5

	float CriticalMultiplier;		// e.g. 1.5f

	uint8_t DodgeChance;			// e.g. 5

	uint8_t Armor;

	uint8_t RunSuccessChance;

	const char* Emotes[3];

	const SPELL* KnownSpells[];

} MONSTER;

extern const MONSTER gSlime001;

extern const MONSTER gRat001;

extern MONSTER gCurrentMonster;

void GenerateMonster(void);

void PPI_Battle(void);

void DrawBattle(void);

void MenuItem_PlayerBattleAction_Attack(void);

void MenuItem_PlayerBattleAction_Spell(void);

void MenuItem_PlayerBattleAction_Run(void);

void MenuItem_PlayerBattleAction_Wait(void);