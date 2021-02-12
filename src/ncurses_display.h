/*

Based on script from the Udacity CppND from the system monitor project
github repo -> https://github.com/udacity/CppND-System-Monitor

*/

#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include "Orchestrator.h"
#include <curses.h>
#include <iostream>
#include <string>
#include <thread>

namespace NCursesDisplay {void Display(int n);
// void DisplayStats(WINDOW *window, bool &doorsAreOpen, int &waitingTime,
//                    long &runSim, std::shared_ptr<WaitingArea> waitingArea);
void DisplayPlot(WINDOW *window);
// void DisplayAStarPath(WINDOW *window, int n, std::shared_ptr<WaitingArea> waitingArea, int agentNumber);

}; // namespace NCursesDisplay

#endif