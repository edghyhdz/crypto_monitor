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

void getMaxMinValue(std::vector<double> data, double *value_x, double *value_y, double pct){
  /*  Gets max and min values from a vector of doubles either containing
      x or y series of data to plot
  */
  *value_x = *std::max_element(data.begin(), data.end());
  *value_x = *value_x + *value_x * pct;

  *value_y = *std::min_element(data.begin(), data.end());
  *value_y = *value_y - *value_y * pct;
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
  char ch = 'o';
  double max_p_range, min_p_range, max_ts_range, min_ts_range; 
  double max_price, min_price; 
  std::vector<double> price_data;
  std::vector<double> time_stamps; 
  if (plotData.size() > 0) {
    for (int i = 0; i < plotData.size(); i++) {
      price_data.push_back(stod(plotData[i][1]));
      time_stamps.push_back(stol(plotData[i][0])); 
    }
    getMaxMinValue(price_data, &max_p_range, &min_p_range, PCTY); 
    getMaxMinValue(time_stamps, &max_ts_range, &min_ts_range, PCTX); 
    max_price = *std::max_element(price_data.begin(), price_data.end());
    min_price = *std::min_element(price_data.begin(), price_data.end());
  }


  wattron(window, COLOR_PAIR(4));
  std::reverse(plotData.begin(), plotData.end());
  double last_price; 
  for (int i = 0; i < plotData.size(); i++) {
    double xpd, ypd;
    xpd = scale(stod(plotData[i][0]), min_ts_range, max_ts_range, 0, xm);
    ypd = scale(stod(plotData[i][1]), min_p_range, max_p_range, ym, 0);

    xpd = (int)(xpd + 0.5 - (xpd < 0));
    ypd = (int)(ypd + 0.5 - (ypd < 0));
    mvwaddch(window, ypd, xpd, ch);

    // Retrieve current price
    if (i == plotData.size() - 1){
      last_price = stod(plotData[i][1]); 
    }
  }

  wattroff(window, COLOR_PAIR(4));
  wattron(window, COLOR_PAIR(4));
  mvwprintw(window, 1, xm - 35, ("Max price     : " + to_string(max_price) + " USDT").c_str());
  mvwprintw(window, 2, xm - 35, ("Min price     : " + to_string(min_price) + " USDT").c_str());
  mvwprintw(window, 3, xm - 35, ("Current price : " + to_string(last_price) + " USDT").c_str());
  wattroff(window, COLOR_PAIR(4));
}
/*  Finished.
    flarn2006's cool repo
*/

// Reference https://stackoverflow.com/a/21303065/13743493
std::string timeStampToHReadble(const time_t rawtime)
{
    struct tm * dt;
    char buffer [30];
    dt = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", dt);
    return std::string(buffer);
}


// Updates every given amount of seconds
void NCursesDisplay::DisplayData(WINDOW *window, std::vector<std::vector<std::string>> &plotData){
  int row_data{0};
  int column_data {5}; 

  std::string column_names = "Time\t Price [USDT]"; 
  wattron(window, COLOR_PAIR(7));
  mvwprintw(window, ++row_data, column_data, column_names.c_str());
  wattroff(window, COLOR_PAIR(7));


  // Start assuming there are no data holes
  for (int i = 0; i < plotData.size(); i++) {
    std::string time_str = timeStampToHReadble(stol(plotData[i][0]) / 1000);
    std::string dataString = time_str + "\t " + plotData[i][1]; 
    wattron(window, COLOR_PAIR(4));
    mvwprintw(window, ++row_data, column_data, dataString.c_str());
    wattroff(window, COLOR_PAIR(4));
    if (i > 39){
      break; 
    }
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