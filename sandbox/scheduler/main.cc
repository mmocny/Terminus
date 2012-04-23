#include <iostream>
#include <chrono>
#if 0 // linux
#include <sys/epoll.h>
#include <ncurses.h>
#elif 0 // win
// io completion events
#elif 1 // mac
// kqueue?
#include <curses.h>
#endif
// libev libevent

class Scheduler {
public:
  Scheduler();
};

class View {
};

class Model {
};

/*
 * Animation framework:
 * - Specify animations (SRT) in terms of a 0-100% line
 * - Also provide a timing curve (linear, ease in, ease out, ...)
 * - Then provide a duration
 *
 * Now a timer can tick at uneven intervals and we map that to a % based on start time and duration
 * and timing curve adjusts % and animation finally provides a value.
 *
 * Can have many animation types, including simply transforming integer values
 */

int main()
{
  // We interact with Model
  // Scheduler kicks of view every tick
  // if view is still working, it sets a flag [possibly cancel previous frame, but for now, no]

  cbreak();
  //noecho();
  nodelay(NULL, false);
  for (;;)
    std::cout << getch() << std::endl;
}
