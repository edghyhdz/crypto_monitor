/*

g++ main.cpp Agent.h Agent.cpp WaitingArea.h WaitingArea.cpp ncurses_display.h ncurses_display.cpp -lcurses -pthread -o display.o
g++ -g src/QueryCrypto.cpp src/QueryCrypto.h src/Binance.h src/Orchestrator.cpp src/Orchestrator.h src/ncurses_display.h src/ncurses_display.cpp src/main.cpp -o run.test -lcurses -lcurl -pthread
*/

#include "ncurses_display.h"
#include <thread>

int main() {
  int n = 40;
  NCursesDisplay::Display(n);
}