//all the sound and music for the battle screen is in BattleSound.cpp

//TO BE ADDED! put files in:
//assets/sounds/sound effects : names are listed at the top of BattleSound.cpp
//assets/music/battle music : battle_0 (scientist), battle_1 (bat), battle_2 (hydra) or one shared "battle"
//NOTE TO SELF: use .ogg, .wav or .flac (SFML 2 cannot play .mp3)
//a file that is missing is just silent so the game runs fine without any of them

#ifndef BATTLE_SOUND_H
#define BATTLE_SOUND_H

const int UI_SOUND_MOVE = 0;      //cursor moved
const int UI_SOUND_SELECT = 1;    //something chosen
const int UI_SOUND_BACK = 2;     // cancelled/not allowed

//called once per frame, plays the sounds that Battle.cpp announced (hits, laughs, etc)
void updateBattleSounds();
void playUiSound(int which);         //played rightaway (menu sound)
void clearBattleSounds();            //if a new battle, it clears the sounds that were waiting to be played

//battle music. stops on like the normal ending to a fight                   (NOTE TO SELF: have to stop it in other cases)
void startBattleMusic(int bossType);      //ENEMY_MAD_SCIENTIST or ENEMY_HUMANOID_BAT or ENEMY_HYDRA
void stopBattleMusic();
#endif