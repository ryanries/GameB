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

typedef enum BATTLESTATE
{
	BATTLESTATE_INTRO,

	BATTLESTATE_PLAYERATTACKING,

	BATTLESTATE_MONSTERATTACKING,

	BATTLESTATE_MONSTERTHINKING,

	BATTLESTATE_PLAYERTHINKING,

	BATTLESTATE_PLAYERDEFENDING,

	BATTLESTATE_MONSTERDEFENDING,

	BATTLESTATE_PLAYERCASTINGSPELL,

	BATTLESTATE_MONSTERCASTINGSPELL,

	BATTLESTATE_PLAYERRUNNINGAWAY,

	BATTLESTATE_MONSTERRUNNINGAWAY,

	BATTLESTATE_MONSTERISDEAD,

	BATTLESTATE_PLAYERISDEAD

} BATTLESTATE;

// Pretty similar stats to the player
typedef struct MONSTER
{
	const char Name[13];

	GAMEBITMAP* Sprite;	

	int MaxHP;			

	int HP;

	int Money;			// The coin that will be awarded to the player once this monster is killed

	int MaxMP;

	int MP;				// Magic points

	int XP;				// The amount of xp that will be awarded to the player once this monster is killed	

	int Strength;		// Increases the physical damage the monster can do

	int Damage;			// Since the monster doesn't have a weapon, we will combine this with strength to determine monster's damage dealt

	int Dexterity;		// Increases chance to hit.

	int Luck;			// Increases miscellaneous odds of things happening.

	int Intelligence;	// Spell damage and chance to hit.

	int Evasion;		// Chance to dodge, and chance to run away successfully

	int Defense;		// Damage mitigation.

						// THESE FOUR MUST ADD UP TO 100
	int AttackChance;	// 95%

	int RunChance;		// 5% (Goes up if low on health)

	int SpellChance;	// 0%

	int DefendChance;	// 0%

	int BaseDamage;

	const char* Emotes[3];

	const SPELL KnownSpells[8];

} MONSTER;

extern const MONSTER gSlime001;

extern const MONSTER gRat001;

extern MONSTER gCurrentMonster;

void GenerateMonster(void);

void PPI_Battle(void);

void DrawBattle(void);

void MenuItem_BattleAction_Attack(void);

void MenuItem_BattleAction_Spell(void);

void MenuItem_BattleAction_Run(void);

void MenuItem_BattleAction_Defend(void);

void MenuItem_BattleAction_Item(void);

void MonsterAttack(void);