rm Assets.dat
rm CopyAssets.log

# --- BITMAPS ---
./MyMiniz Assets.dat + ./Assets/polepig03.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/lightning01.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/6x7font.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Down_Standing.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Down_Walk1.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Down_Walk2.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Left_Standing.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Left_Walk1.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Left_Walk2.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Right_Standing.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Right_Walk1.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Right_Walk2.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Up_Standing.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Up_Walk1.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hero_Suit0_Up_Walk2.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Maps/Overworld01.bmpx >> CopyAssets.log

# --- BATTLE SCENES ---
./MyMiniz Assets.dat + ./Assets/BattleScenes/Grasslands01.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/BattleScenes/Dungeon01.bmpx >> CopyAssets.log

# --- SOUNDS ---
./MyMiniz Assets.dat + ./Assets/MenuChoose.wav >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/MenuNavigate.wav >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/SplashScreen.wav >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Hit01.wav >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Miss01.wav >> CopyAssets.log

# --- MUSIC ---

./MyMiniz Assets.dat + ./Assets/Overworld01.ogg >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Dungeon01.ogg >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Battle01.ogg >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/BattleIntro01.ogg >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/VictoryIntro.ogg >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/VictoryLoop.ogg >> CopyAssets.log

# --- MONSTERS ---

./MyMiniz Assets.dat + ./Assets/Monsters/Slime001.bmpx >> CopyAssets.log
./MyMiniz Assets.dat + ./Assets/Monsters/Rat001.bmpx >> CopyAssets.log

# --- TILEMAPS ---

./MyMiniz Assets.dat + ./Assets/Maps/Overworld01.tmx >> CopyAssets.log

# copy Assets.dat into whatever directory or directories it needs to be in

cp Assets.dat ./x64/Debug/Assets.dat
cp Assets.dat ./x64/Release/Assets.dat
cp Assets.dat ./build/Assets.dat
