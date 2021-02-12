/*

Based on script from the Udacity CppND from the system monitor project
github repo -> https://github.com/udacity/CppND-System-Monitor

*/

#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm> 
#include <fstream>
#include "ncurses_display.h"


using std::string;
using std::to_string;

// Updates every given amount of seconds
void NCursesDisplay::DisplayPlot(WINDOW *window){
  int row{1};
  int column{4}; 
  std::string y_axis = "*";
  std::string x_axis = "*";

  // Create grid?


  for (int i = 0; i < 40; i++) {
    wattron(window, COLOR_PAIR(2));
    mvwprintw(window, ++row, column, y_axis.c_str());
    wattroff(window, COLOR_PAIR(2));
  }

  for (int i = 0; i < 65; i++) {
    wattron(window, COLOR_PAIR(2));
    mvwprintw(window, row, ++++column, x_axis.c_str());
    wattroff(window, COLOR_PAIR(2));
  }
}

void NCursesDisplay::Display(int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color
  
  int x_max{getmaxx(stdscr)};
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);
  WINDOW *plot_window =
      newwin(3 + n, x_max - (x_max * 1 / 3) - 3, system_window->_maxy + 1, 0);
  WINDOW *graph_window = newwin(3 + n, x_max - (x_max * 2 / 3),
                                system_window->_maxy + 1, (x_max * 2 / 3) - 1);

  // Start http requests
  Orchestrator orchestrator = Orchestrator();
  orchestrator.runQuery();
  
  // Send keys timeout
  keypad(system_window, true);
  wtimeout(system_window, 10);

  while (1) {
    // Be able to change agent selection -> right hand side window
    // Displays agents calculated AStar path
    // switch (wgetch(system_window)) {
    // case KEY_F0 + 1:
    //   agentNumber += (agentNumber < (maxAgentNumber - 1)) ? 1 : -agentNumber;
    //   break;
    // case KEY_F0 + 2:
    //   agentNumber -= (agentNumber > 0) ? 1 : -(maxAgentNumber - 1);
    //   break;
    // }

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_WHITE); 
    init_pair(8, COLOR_WHITE, COLOR_GREEN); 
    wclear(system_window); 
    wclear(plot_window);
    wclear(graph_window); 

    box(system_window, 0, 0);
    box(plot_window, 0, 0);
    box(graph_window, 0, 0);
    // DisplayHTTPStats(system_window, doorsAreOpen, waitingTime, runSim, waitingArea); 
    DisplayPlot(plot_window); 
    // DisplayAStarPath(graph_window, n, waitingArea, agentNumber); 
    wrefresh(system_window);
    wrefresh(plot_window);
    wrefresh(graph_window); 
    refresh();
    // Speed in which it is refreshed
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
  endwin();
}