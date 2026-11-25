//AMINA AHMAD SYED 26L-0760 member 1
//Story.cpp    all the story text: relic stories, intro, ending and credits
//every relic always has the same story, whatever order they are found in (Map.cpp says which tile gives which one)
#include "Story.h"

static const std::string RELIC_TITLES[RELIC_COUNT] = {
    "A Tarnished Locket",
    "A Torn Notebook Page",
    "A Child's Drawing",
    "A Bat-Shaped Brooch",
    "A Water-Stained Ledger",
    "An Empty Key Mould",
    "A Warm Stone"
};

static const std::string RELIC_STORIES[RELIC_COUNT] = {
    "Inside the locket a name has been scratched out. Someone wrote VIKTOR over it, again and again, as if afraid to forget it.",
    "\"The serpent eats its own tail. So must we. Begin again. Begin again.\" The ink is the Scientist's.",
    "Three heads on one body, drawn in crayon. Under it: \"They stay together because they are scared to be alone.\"",
    "The brooch belonged to a nurse. She used to sing to the patients at night, long before the screeching started.",
    "\"Subject One returned. Memory cleared. Subject One returned. Memory cleared.\" The list goes on for pages and pages.",
    "A mould for two keys. A note: \"One opens the chest. One opens nothing. He is never told which.\"",
    "The last relic is warm, like it was just held. \"If you are reading this, you have done this before.\""
};

static const std::string INTRO_LINES[] = {
    "You wake up in the ruins of a place you do not remember.",
    "Everything is quiet. Mist hangs over the ground, hiding what is ahead.",
    "Somewhere here, a treasure chest is hidden. That is all you know.",
    "WASD or the arrow keys to move. E to interact. I for your bag. Esc to pause."
};

static const std::string ENDING_LINES[] = {
    "The key turns. The lid of the chest creaks open.",
    "There is no gold inside. There is only a mirror, and your face in it.",
    "Viktor. The first subject. The only one who ever made it this far.",
    "The relics in your bag grow warm and the whole story comes back at once.",
    "The serpent swallows its tail. The world goes white.",
    "Maybe this time it will end differently."
};

static const std::string CREDITS_LINES[] = {
    "OUROBOROS",
    "",
    "Amina Ahmad Syed - map and exploration",
    "Zarnib Rauf - player, inventory and progression",
    "Dua Nadeem - combat, bosses and the battle screen",
    "",
    "Made with C++ and SFML 2.6.2",
    "",
    "Assets",
    "Pictures, sprites, backgrounds, menu video, music",
    "and most sound effects: AI-generated with ChatGPT and Claude",
    "",
    "Sound effects from other sources",
    "Attack / sword slash swoosh: Mixkit",
    "Hydra wail (monster scream) and pain grunt: UpperBeat",
    "",
    "Font: Crimson Text by The Crimson Text Project Authors",
    "(SIL Open Font License)",
    "",
    "Thank you for playing :)"
};

std::string getRelicTitle(int index)
{
    if (index < 0 || index >= RELIC_COUNT) return "A Relic";
    return RELIC_TITLES[index];
}

std::string getRelicStory(int index)
{
    if (index < 0 || index >= RELIC_COUNT) return "An old relic. It tells you nothing yet.";
    return RELIC_STORIES[index];
}

int getIntroLineCountStory() { return sizeof(INTRO_LINES) / sizeof(INTRO_LINES[0]); }
std::string getIntroLineStory(int index) { return INTRO_LINES[index]; }

int getEndingLineCount() { return sizeof(ENDING_LINES) / sizeof(ENDING_LINES[0]); }
std::string getEndingLine(int index) { return ENDING_LINES[index]; }

int getCreditsLineCount() { return sizeof(CREDITS_LINES) / sizeof(CREDITS_LINES[0]); }
std::string getCreditsLine(int index) { return CREDITS_LINES[index]; }
