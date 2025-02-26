/*

Based on script from the Udacity CppND from the system monitor project
github repo -> https://github.com/udacity/CppND-System-Monitor

Used snippets of code from flarn2006's cool repoo MiscPrograms graph.c and 
modified it to suit this dashboard needs
References https://github.com/flarn2006/MiscPrograms/blob/master/graph.c
user -> flarn2006
*/

#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include "Orchestrator.h"
#include <curses.h>
#include <iostream>
#include <string>
#include <thread>
#include "Variables.h"


#define XMIN 1
#define XMAX 10
#define YMIN  1
#define YMAX 10
#define XSCL .5
#define YSCL .5
#define PCTX 0.00000001
#define PCTY 0.015
#define COIN_TO_PLOT "CHZUSDT"
#define COIN_TO_PLOT_SECOND "ENJUSDT"
#define WINDOW_RANGE 180	// DATA_RANGE * Fetch interval = total seconds of data i.e. 180 * 10s-interval ->30 min window
#define FIELD_MAX_CHARS 32

namespace NCursesDisplay {void Display(int n);

typedef struct _viewwin viewwin;
typedef struct _khdata khdata;
typedef void (*key_handler)(int key, khdata *data);

struct _viewwin {
	double xmin, xmax;
	double ymin, ymax;
	double xscl, yscl;
	int window_range; 
    std::string first_coin;
	std::string second_coin ;  
	std::string third_coin;
	bool plot_all;  
	bool wallet; 
};

struct _khdata {
	/* Struct for pointers to data that key handlers may need to access */
	viewwin *view;
	double *trace;
};

/*  Start.
    From this line onward code parts of code were taken/modified from 
    flarn2006's cool repository
*/
double scale(double value, double omin, double omax, double nmin, double nmax); 
void getViewStep(WINDOW *win, const viewwin *view, double *xstep, double *ystep);
void DrawAxes(WINDOW *window, const viewwin *view); 
void DrawGraph(WINDOW *window, const viewwin *view, std::vector<std::vector<std::vector<std::string>>> &plotData);
void defaultKeyHandler(int key, khdata *data);
int editViewWindow(viewwin *view); 
/*  Finished.
    flarn2006's cool repo
*/
void DrawSubPlot(WINDOW *window, std::vector<std::vector<std::string>> &plotData, char ch, int color, std::string coin_name, int x_offset); 
void DisplayHTTPStats(WINDOW *window, int requestWeight, bool wallet); 
void DisplayWallet(WINDOW *window, std::map<std::string, double> & coinToQuantity, std::map<std::string, double> &coinToPrice); 
void DisplayData(WINDOW *window, std::vector<std::vector<std::string>> &plotData);

}; // namespace NCursesDisplay

#endif