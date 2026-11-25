//Game.h : this is for the window, the main loop and switching between game states
//these are the state names, everyone should use these same names
#ifndef GAME_H
#define GAME_H

const int STATE_MENU = 0;      //main menu (start, continue, difficulty, screen mode, credits, exit)
const int STATE_EXPLORE = 1;    //walking around the map
const int STATE_BATTLE = 2;    //fighting a boss
const int STATE_INVENTORY = 3;    //inventory
const int STATE_DIALOGUE = 4;    //story text (relics, intro, ending)
const int STATE_GAME_OVER = 5;
const int STATE_CREDITS = 6;    //credits (after the ending, or from the menu)

//opens the window and runs the game until the window is closed
int runGame();
#endif