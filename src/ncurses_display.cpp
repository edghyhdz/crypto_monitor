/*

Based on the script from the Udacity CppND from the system monitor project
github repo -> https://github.com/udacity/CppND-System-Monitor
References https://github.com/flarn2006/MiscPrograms/blob/master/graph.c
Plot graph cool repo from flarn2006
*/

#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm> 
#include <fstream>
#include <math.h>
#include "ncurses_display.h"

using namespace NCursesDisplay; 
using std::string;
using std::to_string;


/*  Start.
    From this line onward code parts of code were taken/modified from 
    flarn2006's cool repository
*/
double NCursesDisplay::scale(double value, double omin, double omax, double nmin, double nmax)
{
	/* Useful function to scale a value in one range to a different range.
	   omin/omax - old range
	   nmin/nmax - new range
	*/
	double x = (value - omin) / (omax - omin);
	return x * (nmax - nmin) + nmin;
}

void NCursesDisplay::getViewStep(WINDOW *win, const viewwin *view, double *xstep, double *ystep)
{
	/* Gets the 'value' of one character on either or both axes. */
	int xm, ym; getmaxyx(win, ym, xm);
	if (xstep) *xstep = (view->xmax - view->xmin) / (xm + 1);
	if (ystep) *ystep = (view->ymax - view->ymin) / (ym + 1);
}


void NCursesDisplay::DrawAxes(WINDOW *window, const viewwin *view) {
  /* This function is what draws the axes on the screen. */

  int xm, ym;
  getmaxyx(window, ym, xm);
  double x0 = scale(0, view->xmin, view->xmax, 0, xm);
  double y0 = scale(0, view->ymin, view->ymax, ym, 0);

  double xstep, ystep;
  getViewStep(window, view, &xstep, &ystep);

  std::string middle = to_string(view->xmax / 2); 
  std::string test = to_string(fabs(fmod(view->ymin + ystep * 6, view->yscl)) < ystep);
  std::string step = to_string(ystep); 

  wattron(window, COLOR_PAIR(2));
  int i;

  for (i = 2; i <= xm - 2; i++) {
    double plotx = view->xmin + xstep * i;
    float abs_val = fabs(fmod(plotx, view->xscl)); 
    int tick = abs_val < xstep;
    mvwaddch(window, ym - 2, i, tick ? ACS_PLUS : ACS_HLINE);
  }
  for (i = 2; i <= ym - 2; i++) {
    double ploty = view->ymin + ystep * i;
    float abs_val = fabs(fmod(ploty, view->yscl));
    int tick = abs_val < xstep;
    mvwaddch(window, i, view->xmin + 2, tick ? ACS_PLUS : ACS_VLINE);
  }
  mvwaddch(window, ym - 4, view->xmin + 2, ACS_PLUS);
  wattroff(window, COLOR_PAIR(2));

}

void NCursesDisplay::DrawGraph(WINDOW *window, const viewwin *view, std::vector<std::vector<std::string>> &plotData) {
  /* Draws a graph on the screen without axes.
     win              - ncurses window for drawing
     view             - view parameters structure
     yfunc            - function to graph (function pointer)
     enableSlopeChars - whether or not to call slopeChar to determine characters
  */
  int xm, ym;
  getmaxyx(window, ym, xm);
  double step;
  getViewStep(window, view, &step, NULL);
  double x;
  char ch = '*';
  for (double i = 2; i < 180; i++) {
    double xpd, ypd;
    xpd = scale(i, 0, xm, 0, xm);
    ypd = scale(i, 0, ym, ym, 0);
    xpd = (int)(xpd + 0.5 - (xpd < 0));
    ypd = (int)(ypd + 0.5 - (ypd < 0));
    mvwaddch(window, ypd, xpd, ch);
  }
}
/*  Finished.
    flarn2006's cool repo
*/

// Updates every given amount of seconds
void NCursesDisplay::DisplayData(WINDOW *window, std::vector<std::vector<std::string>> &plotData){
  int row_data{0};
  int column_data {5}; 
  std::string y_axis = "*";
  std::string x_axis = "*";

  // Start assuming there are no data holes
  for (int i = 0; i < plotData.size(); i++) {
    std::string dataString = plotData[i][0] + ":" + plotData[i][1]; 
    wattron(window, COLOR_PAIR(1));
    mvwprintw(window, ++row_data, column_data, dataString.c_str());
    wattroff(window, COLOR_PAIR(1));
    if (i > 40){
      break; 
    }
  }
}

// Updates every given amount of seconds
void NCursesDisplay::DisplayPlot(WINDOW *window, std::vector<std::vector<std::string>> &plotData){
  int row{1};
  int column{4}; 
  std::string y_axis = "*";
  std::string x_axis = "*";

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
  WINDOW *plot_window = newwin(3 + n, x_max - (x_max * 1 / 6) - 3, system_window->_maxy + 1, 0);
  WINDOW *data_window = newwin(3 + n, x_max - (x_max * 5 / 6), system_window->_maxy + 1, (x_max * 5 / 6) - 1);

  // Start http requests
  Orchestrator orchestrator = Orchestrator();
  orchestrator.runQuery();
  
  // Send keys timeout
  keypad(system_window, true);
  wtimeout(system_window, 10);

  NCursesDisplay::viewwin view;
	view.xmin = XMIN; view.xmax = XMAX;
	view.ymin = YMIN; view.ymax = YMAX;
	view.xscl = XSCL; view.yscl = YSCL;

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
    std::vector<std::vector<std::string>> plotData = orchestrator.getPlotData();

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
    wclear(data_window); 

    box(system_window, 0, 0);
    box(plot_window, 0, 0);
    box(data_window, 0, 0);
    // DisplayHTTPStats(system_window, doorsAreOpen, waitingTime, runSim, waitingArea); 
    // DisplayPlot(plot_window, plotData); 
    DisplayData(data_window, plotData); 
    DrawAxes(plot_window, &view);
    DrawGraph(plot_window, &view, plotData); 
    wrefresh(system_window);
    wrefresh(plot_window);
    wrefresh(data_window); 
    refresh();

    // Speed in which it is refreshed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // for (int i = 0; i < plotData.size();  i++){
    //   std::cout << plotData[i][0] << ":" << plotData[i][1]; 
    // }


  }
  endwin();
}