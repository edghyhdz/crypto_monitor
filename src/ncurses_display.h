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

#define XMIN 1
#define XMAX 10
#define YMIN  1
#define YMAX 10
#define XSCL .5
#define YSCL .5
#define PCTX 0.00000001
#define PCTY 0.015
#define COIN_TO_PLOT "CHZUSDTT"
#define FIELD_MAX_CHARS 32

namespace NCursesDisplay {void Display(int n);

typedef struct _viewwin viewwin;
typedef struct _khdata khdata;
typedef void (*key_handler)(int key, khdata *data);

struct _viewwin {
	double xmin, xmax;
	double ymin, ymax;
	double xscl, yscl;
    std::string current_coin = COIN_TO_PLOT; 
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
void DisplayHTTPStats(WINDOW *window, int requestWeight); 
void DrawAxes(WINDOW *window, const viewwin *view); 
void DrawGraph(WINDOW *window, const viewwin *view, std::vector<std::vector<std::string>> &plotData); 
void defaultKeyHandler(int key, khdata *data);
int editViewWindow(viewwin *view); 
/*  Finished.
    flarn2006's cool repo
*/
void DisplayData(WINDOW *window, std::vector<std::vector<std::string>> &plotData);

}; // namespace NCursesDisplay

#endif