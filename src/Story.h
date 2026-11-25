//AMINA AHMAD SYED 26L-0760 member 1
//Story.h : all the story text in one place
//the text itself is in Story.cpp
#ifndef STORY_H
#define STORY_H
#include <string>
const int RELIC_COUNT = 7;                //how many relics are on the map

std::string getRelicTitle(int index);     //index 0 to RELIC_COUNT - 1
std::string getRelicStory(int index);

int getIntroLineCountStory();             //shown when a new game starts
std::string getIntroLineStory(int index);

int getEndingLineCount();                 //shown after the chest opens
std::string getEndingLine(int index);

int getCreditsLineCount();
std::string getCreditsLine(int index);
#endif