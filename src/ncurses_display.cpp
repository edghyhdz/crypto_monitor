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
#include <form.h>
#include <string.h>
#include <sstream>
#include <iomanip>

using namespace NCursesDisplay; 
using std::string;
using std::to_string;

// Format price displayed in DisplayWallet method
std::string getFormatAmount(double value){
  std::stringstream sst;
  sst.imbue(std::locale(""));
  sst << std::fixed << std::setprecision(2) << value;
  return sst.str();
}

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

  int i;
  for (i = 2; i <= ym - 2; i++) {
    double ploty = view->ymin + ystep * i;
    float abs_val = fabs(fmod(ploty, view->yscl));
    int tick = abs_val < xstep;

    // Add grid in the x direction
    if (tick == 1) {
      wattron(window, COLOR_PAIR(9));
      for (int k = 4; k <= xm - 2; k++) {
        mvwaddch(window, i, k, ACS_HLINE);
      }
      wattroff(window, COLOR_PAIR(9));
    }
    wattron(window, COLOR_PAIR(2));
    mvwaddch(window, i, view->xmin + 2, tick ? ACS_PLUS : ACS_VLINE);
  }

  for (i = 2; i <= xm - 2; i++) {
    double plotx = view->xmin + xstep * i;
    float abs_val = fabs(fmod(plotx, view->xscl)); 
    int tick = abs_val < xstep;

    // Add grid in the y direction
    if (tick == 1) {
      wattron(window, COLOR_PAIR(9));
      for (int k = 4; k <= ym - 2; k++) {
        double ploty = view->ymin + ystep * k;
        float abs_val = fabs(fmod(ploty, view->yscl));
        int tick = abs_val < xstep;

        if (k == 5) {
          mvwaddch(window, k, i, ACS_TTEE);
        } else if (k > 5){
          mvwaddch(window, k, i, tick ? ACS_PLUS : ACS_VLINE);
        }
      }
      wattroff(window, COLOR_PAIR(9));
    }

    wattron(window, COLOR_PAIR(2));
    mvwaddch(window, ym - 2, i, tick ? ACS_PLUS : ACS_HLINE);
    wattroff(window, COLOR_PAIR(2));
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

int NCursesDisplay::editViewWindow(viewwin *view)
{
	/* Figure out where to put the window */
	int wwidth = 21, wheight = 11;
	int ym, xm; getmaxyx(stdscr, ym, xm);
	int wy = ym/2 - wheight/2;
	int wx = xm/2 - wwidth/2;

	/* Create a new window and draw the form */
	WINDOW *fwin = newwin(wheight, wwidth, wy, wx);
	keypad(fwin, TRUE);
	werase(fwin);
	
	wattron(fwin, A_BOLD);
	box(fwin, 0, 0);
	mvwprintw(fwin, 0, 4, " VIEW WINDOW ");
	mvwprintw(fwin, 2, 3, "COIN 1 =");
	mvwprintw(fwin, 3, 3, "COIN 2 =");
	mvwprintw(fwin, 4, 3, "COIN 3 =");
	mvwprintw(fwin, 5, 3, "WINDOW RANGE = ");

	wattroff(fwin, A_BOLD);
	mvwprintw(fwin, 9, 6, "[SPC] OK");
	/* Create the form fields */
	FIELD *fields[5];
	int i; for (i=0; i<4; i++) {
		fields[i] = new_field(1, 8, i, 0, 0, 0);
		set_field_back(fields[i], A_REVERSE | A_UNDERLINE);
		set_field_type(fields[i], i!=3?TYPE_ALNUM:TYPE_NUMERIC, 6, 0.0, 0.0);
		// set_field_type(fields[i], TYPE_NUMERIC, 6, 0.0, 0.0);
		field_opts_off(fields[i], O_AUTOSKIP | O_STATIC);
		set_max_field(fields[i], FIELD_MAX_CHARS);
	}
	fields[4] = NULL;

	/* Fill the form fields with initial values */
	char printbuf[FIELD_MAX_CHARS+1];
	set_field_buffer(fields[0], 0, (view->first_coin).c_str());
	set_field_buffer(fields[1], 0, (view->second_coin).c_str());
	set_field_buffer(fields[2], 0, (view->third_coin).c_str());
  snprintf(printbuf, FIELD_MAX_CHARS+1, "%i", view->window_range);
	set_field_buffer(fields[3], 0, printbuf);

	/* Create a subwindow for the form fields */
	WINDOW *fsub = derwin(fwin, 6, 8, 2, 10);
	keypad(fsub, TRUE);
	
	/* Create the actual form */
	FORM *f = new_form(fields);
	set_form_win(f, fwin);
	set_form_sub(f, fsub);
	post_form(f);
	wrefresh(fwin);
	wrefresh(fsub);
	curs_set(1);

	/* Handle input */
	int savewin = 1;
	int exitloop = 0;
	int ch; while (!exitloop) {
		switch (ch = wgetch(fwin)) {
			case 'j': case '\n': case KEY_DOWN:
				form_driver(f, REQ_NEXT_FIELD);
				break;
			case 'k': case KEY_UP:
				form_driver(f, REQ_PREV_FIELD);
				break;
			case KEY_BACKSPACE:
				form_driver(f, REQ_DEL_PREV);
				break;
			case ' ':
				exitloop = 1;
				break;
			case 'q':
				savewin = 0;
				exitloop = 1;
				break;
			default:
				form_driver(f, ch);
				break;
		}
	}

	if (savewin) {
          form_driver(f, REQ_VALIDATION);
          view->first_coin = field_buffer(fields[0], 0);
          view->second_coin = field_buffer(fields[1], 0);
          view->third_coin = field_buffer(fields[2], 0);
          view->window_range = std::stoi(field_buffer(fields[3], 0));
        }

  /* Clean up */
	curs_set(0);
	unpost_form(f);
	free_form(f);
	for (i=0; i<6; i++) free_field(fields[i]);
	delwin(fsub);
	delwin(fwin);
	refresh();

	return savewin;
}

void NCursesDisplay::DrawSubPlot(WINDOW *window, std::vector<std::vector<std::string>> &plotData, char ch, int color, std::string coin_name, int x_offset){
  int xm, ym;
  getmaxyx(window, ym, xm);
  double max_p_range, min_p_range, max_ts_range, min_ts_range; 
  double max_price, min_price; 
  std::vector<double> price_data;
  std::vector<double> time_stamps; 

  // Remove any trailing spaces from coin_name
  coin_name.erase(remove(coin_name.begin(), coin_name.end(), ' '), coin_name.end());
  
  if (plotData.size()>0){
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

    wattron(window, COLOR_PAIR(color));
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

    wattroff(window, COLOR_PAIR(color));
    wattron(window, COLOR_PAIR(color));
    wattron(window, A_BOLD);
    mvwprintw(window, 1, xm - x_offset, ("COIN          : " + coin_name).c_str());
    mvwprintw(window, 2, xm - x_offset, ("Current price : " + to_string(last_price) + " USDT").c_str());
    wattroff(window, A_BOLD);
    mvwprintw(window, 3, xm - x_offset, ("Max price     : " + to_string(max_price) + " USDT").c_str());
    mvwprintw(window, 4, xm - x_offset, ("Min price     : " + to_string(min_price) + " USDT").c_str());
    wattroff(window, COLOR_PAIR(color));
  }
}

void NCursesDisplay::DrawGraph(WINDOW *window, const viewwin *view, std::vector<std::vector<std::vector<std::string>>> &allPlotData) {
  /* Draws a graph on the screen without axes.
     win              - ncurses window for drawing
     view             - view parameters structure
     yfunc            - function to graph (function pointer)
     enableSlopeChars - whether or not to call slopeChar to determine characters
  */

  // Vector size is limited to the amount of available coins as indicated in [_viewwin]
  if (allPlotData[0].size() > 0) {
    DrawSubPlot(window, allPlotData[0], 'o', 4, view->first_coin, 35);
  }
  if (allPlotData[1].size() > 0) {
    DrawSubPlot(window, allPlotData[1], '*', 5, view->second_coin, 70);
  }
  if (allPlotData[2].size() > 0) {
    DrawSubPlot(window, allPlotData[2], '+', 6, view->third_coin, 105);
  }
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
  wattron(window, A_BOLD);
  wattron(window, COLOR_PAIR(4));
  mvwprintw(window, ++row_data, column_data, column_names.c_str());
  wattroff(window, COLOR_PAIR(4));
  wattroff(window, A_BOLD);

  // Horizontal line to divide col names with price data
  ++row_data;
  for (int k = 5; k < window->_maxx - 5; k++) {
    mvwaddch(window, row_data, k, ACS_HLINE);
  }
  // Start assuming there are no data holes
  for (int i = 0; i < plotData.size(); i++) {
    std::string time_str = timeStampToHReadble(stol(plotData[i][0]) / 1000);
    std::string dataString = time_str + "\t " + plotData[i][1]; 
    wattron(window, COLOR_PAIR(4));
    mvwprintw(window, ++row_data, column_data, dataString.c_str());
    wattroff(window, COLOR_PAIR(4));
    // Vertical line dividing time stamps with price data
    mvwaddch(window, row_data, (window->_maxx / 2) - 2, ACS_VLINE);
    if (i > window->_maxx + 3){
      break; 
    }
  }
}

void NCursesDisplay::DisplayHTTPStats(WINDOW *window, int requestWeight, bool wallet) {
  int row{0};
  int column {5}; 
  wattron(window, A_BOLD);
  wattron(window, COLOR_PAIR(4));
  std::string title = "Current request weight:";
  std::string wallet_enabled = "Wallet enabled: ";  
  mvwprintw(window, 1, window->_maxx - 25, title.c_str());
  mvwprintw(window, 2, window->_maxx - 25, (to_string(requestWeight) + "/1200/min").c_str()); 
  mvwprintw(window, 3, window->_maxx - 25, (wallet_enabled + (wallet?"YES":"NO")).c_str()); 
  wattroff(window, COLOR_PAIR(4));
  wattroff(window, A_BOLD);
}

void NCursesDisplay::DisplayWallet(WINDOW *window, std::map<std::string, double> &coinToQuantity, std::map<std::string, double> &coinToPrice) {

  if (coinToQuantity.size() > 0) {
    
    double total_value; 
    std::string const coin_title{"COIN"};
    std::string const total_units{"Total Units"};  
    std::string const unit_price{"Price"}; 
    std::string const total_price{"Total USD"};
    std::string const title_portfolio{"Total portfolio value: "}; 
    int col_width{45}, counter{1}, column{2}; 

    for (auto &kv : coinToQuantity) {
      wattron(window, A_BOLD);
      if ( counter == 1 ){
        mvwprintw(window, counter, column, coin_title.c_str());
        mvwprintw(window, counter, (18 + column) - total_units.size(), total_units.c_str());
        mvwprintw(window, counter, (28 + column) - unit_price.size(), unit_price.c_str());
        mvwprintw(window, counter, (40 + column) - total_price.size(), total_price.c_str());
      } wattroff(window, A_BOLD);
      wattron(window, COLOR_PAIR(4));
      mvwprintw(window, ++counter, column, (kv.first).c_str());
      wattroff(window, COLOR_PAIR(4));
      std::string units = getFormatAmount(kv.second); 
      mvwprintw(window, counter, (18 + column) - units.size(), units.c_str());

      if ( coinToPrice.find(kv.first + "USDT") == coinToPrice.end() ) {
        std::string n_a = "N/A"; 
        mvwprintw(window, counter, (28 + column) - n_a.size(), n_a.c_str());
      }
      else if ( kv.first == "USDT" ){
        double usdt_price = coinToPrice.at(kv.first + "USDT") * kv.second;
        total_value += usdt_price;
        std::string usdt = getFormatAmount(usdt_price); 
        std::string unit_price = getFormatAmount(coinToPrice.at(kv.first + "USDT")); 
        mvwprintw(window, counter, (28 + column) - unit_price.size(), unit_price.c_str());
        mvwprintw(window, counter, (40 + column) - usdt.size(), usdt.c_str());

      } else {
        double usdt_price = coinToPrice.at(kv.first + "USDT") * kv.second;
        total_value += usdt_price;
        std::string usdt = getFormatAmount(usdt_price); 
        std::string unit_price = getFormatAmount(coinToPrice.at(kv.first + "USDT")); 
        mvwprintw(window, counter, (28 + column) - unit_price.size(), unit_price.c_str());
        mvwprintw(window, counter, (40 + column) - usdt.size(), usdt.c_str());
      }

      if (counter == window->_maxy - 1) { // Rows per column
        counter = 1;
        column += col_width;
        for (int i = 1; i < window->_maxy ; i++) {
          // Vertical line dividing time stamps with price data
          mvwaddch(window, i, column - 2, ACS_VLINE);
        }
      }
    }

    // Total portfolio value
    std::string total_value_str = getFormatAmount(total_value); 
    wattron(window, A_BOLD);
    mvwprintw(window, 6, window->_maxx - 25, (total_value_str + " USD").c_str());
    mvwprintw(window, 5, window->_maxx - 25, title_portfolio.c_str());
    wattroff(window, A_BOLD);
  }
}

void setCoinsVector(std::vector<std::string> *coins_vector, viewwin &view){
  /*  Sets coin_vector with coins given in the editViewWindow
      display

      coins_vector      - vector with coin names to plot
      view              - view parameters structure

  */
  (*coins_vector).clear(); 
  (*coins_vector).push_back(view.first_coin);
  (*coins_vector).push_back(view.second_coin);
  (*coins_vector).push_back(view.third_coin);
}

void NCursesDisplay::Display(int n) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color
  
  int x_max{getmaxx(stdscr)};
  bool walletExists = false; 
  std::map<std::string, double> coinToQuantity; 
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);
  WINDOW *plot_window = newwin(3 + n, x_max - (x_max * 1 / 6) - 3, system_window->_maxy + 1, 0);
  WINDOW *data_window = newwin(3 + n, x_max - (x_max * 5 / 6), system_window->_maxy + 1, (x_max * 5 / 6) - 1);
  
  // Send keys timeout
  keypad(system_window, true);
  wtimeout(system_window, 10);

  NCursesDisplay::viewwin view;
	view.xmin = XMIN; view.xmax = XMAX;
	view.ymin = YMIN; view.ymax = YMAX;
	view.xscl = XSCL; view.yscl = YSCL;
  view.window_range = WINDOW_RANGE; 
  view.first_coin = COIN_TO_PLOT; 
  view.second_coin = COIN_TO_PLOT_SECOND; 
  view.third_coin = ""; 
  view.wallet = WALLET; 

  std::vector<std::string> coins_vector; 
  std::string price; 

  // Start http requests
  Orchestrator orchestrator = Orchestrator(WALLET);

  setCoinsVector(&coins_vector, view); 
  orchestrator.setCoinsToPlot(coins_vector); 
  orchestrator.setWindowRange(view.window_range); 
  orchestrator.setWalletStatus(view.wallet);
  orchestrator.runQuery();

  while (1) {
    switch(wgetch(system_window)){
      case KEY_F0 + 1:
      editViewWindow(&view);
      // orchestrator.setCoinToPlot(view.first_coin);
      setCoinsVector(&coins_vector, view); 
      orchestrator.setCoinsToPlot(coins_vector); 
      orchestrator.setWindowRange(view.window_range); 
      break;
    }

    // Get current request weight from binance api and data to plot
    int requestWeight = orchestrator.getCurrentWeightRequest(); 
    std::vector<std::vector<std::vector<std::string>>> allPlotData = orchestrator.getAllPlotData();
    std::map<std::string, double> coinToPrice; 

    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_GREEN, COLOR_WHITE); 
    init_pair(8, COLOR_WHITE, COLOR_GREEN);
    init_pair(9, COLOR_WHITE, COLOR_BLACK); 
    wclear(system_window); 
    wclear(plot_window);
    wclear(data_window); 

    box(system_window, 0, 0);
    box(plot_window, 0, 0);
    box(data_window, 0, 0);
    DisplayHTTPStats(system_window, requestWeight, view.wallet);
    DrawAxes(plot_window, &view);
    if (allPlotData.size() > 0){ 
      if (!allPlotData[0].empty()){
        DisplayData(data_window, allPlotData[0]); 
        DrawGraph(plot_window, &view, allPlotData);
      }
    }

    // TODO: Fix all these
    // If portfolio display is enabled -> wallet
    if (view.wallet) {
      if (!walletExists) {
        coinToQuantity = orchestrator.getCoinToQuantity();
        if (coinToQuantity.size() > 0) {
          walletExists = true;
        }
      }
      coinToPrice = orchestrator.getCoinToPrice(); 
      DisplayWallet(system_window, coinToQuantity, coinToPrice); 
    }

    wrefresh(system_window);
    wrefresh(plot_window);
    wrefresh(data_window); 
    refresh();

    // Speed in which it is refreshed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  endwin();
}