// BattleSound.cpp sound effects + music for the battle screen (see BattleSound.h)
#include "BattleSound.h"
#include "Battle.h"
#include <SFML/Audio.hpp>
#include <cstdlib>
#include <string>

//settings
static const float SFX_VOLUME= 70.f;    //0 to 100
static const float MUSIC_VOLUME = 50.f;    //0 to 100
static const float PITCH_VARIATION = 0.08f;   //hits/hurts change pitch upto +-8% so they don't sound identical every time (0 = off)

//which file plays for which event. [have to change the names to match my own files.]
//order matches the SND_ list in Battle.h
static const int TOTAL_SOUNDS = SND_COUNT + 3;      //battle + 3menu sounds
static const char* SOUND_FILES[TOTAL_SOUNDS] ={
    "",                                   //SND_NONE
    "assets/sounds/hit.wav",              //SND_PLAYER_HIT  (you hit the boss)
    "assets/sounds/hurt.wav",             //SND_PLAYER_HURT  (player/viktor takes damage)
    "assets/sounds/block.wav",            //SND_BLOCK  (damage taken while defending)
    "assets/sounds/heal.wav",             //SND_HEAL  (healing potion)
    "assets/sounds/buff.wav",             //SND_BUFF  (buff potion)
    "assets/sounds/laugh.wav",            //SND_SCIENTIST_LAUGH
    "assets/sounds/screech.wav",          //SND_BAT_SCREECH
    "assets/sounds/wail.wav",             //SND_HYDRA_WAIL
    "assets/sounds/victory.wav",          //SND_VICTORY
    "assets/sounds/defeat.wav",           //SND_DEFEAT
    "assets/sounds/ui_move.wav",          //menu cursor moved
    "assets/sounds/ui_select.wav",        //menuchoice made
    "assets/sounds/ui_back.wav"           //cancelled or not allowed
};

//sound effects
static sf::Sound* sounds[TOTAL_SOUNDS];
static bool loaded = false;

static void loadAll()
{
    loaded = true;
    for (int i = 0; i < TOTAL_SOUNDS; i++)
    {
        sounds[i] = 0;
        if (i == SND_NONE) continue;
        sf::SoundBuffer* buffer = new sf::SoundBuffer();
        if (buffer->loadFromFile(SOUND_FILES[i]))
        {
            sounds[i] = new sf::Sound();
            sounds[i]->setBuffer(*buffer);
            sounds[i]->setVolume(SFX_VOLUME);
        }
        else delete buffer;                //if file missing then stay silent
    }
}

static void playNow(int id)
{
    if (!loaded) loadAll();
    if (id <= 0 || id >= TOTAL_SOUNDS || !sounds[id]) return;
    if (id == SND_PLAYER_HIT || id == SND_PLAYER_HURT || id == SND_BLOCK) sounds[id]->setPitch(1.f + PITCH_VARIATION * ((rand() % 201) - 100) / 100.f);
    sounds[id]->play();
}

void updateBattleSounds()
{
    int id = queueSound();              //Battle.cpp keeps a list of "something happened" events
    while (id != SND_NONE)
    {
        playNow(id);
        id = queueSound();
    }
}

void playUiSound(int which) { playNow(SND_COUNT + which); }
void clearBattleSounds() { while (queueSound() != SND_NONE) {} }

//music
static sf::Music* music = 0;

//opens assets/music/<name>.ogg or .wav or .flac. will return false if there is no such file
static bool openMusic(const std::string& name)
{
    const char* extensions[3] = { ".ogg", ".wav", ".flac" };
    for (int i = 0; i < 3; i++) if (music->openFromFile("assets/music/" + name + extensions[i])) return true;
    return false;
}

void startBattleMusic(int bossType)
{
    if (!music) music = new sf::Music();
    music->stop();
    if (!openMusic("battle_" + std::to_string(bossType)) && !openMusic("battle")) return;   //no music (silent)
    music->setLoop(true);
    music->setVolume(MUSIC_VOLUME);
    music->play();
}
void stopBattleMusic() { if (music) music->stop(); }