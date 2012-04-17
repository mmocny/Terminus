#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <functional>

#if defined(__clang__)
#include <unistd.h>
#else
#include <thread>
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Quarter Mile Race!
//
// * Cars have numbers and a constants speed
// * RaceTracks have a distance(ie, quarter mile) and vector of cars
// * Timer is used to track flow of time, and thus position of cars on racetrack
//
// I suggest starting to read this program from the bottom.
// Read main() to see what the program does.  Compile it, run it, etc.
// Then you can read the classes and member functions to see what they do.
//
////////////////////////////////////////////////////////////////////////////////
//
// Assignments:
//
// 1. There is a bug (that I know of) where we may announce the wrong winner
//    if two cars have similar speed and both cross the finish line within the
//    same timer tick. (decrease frame rate to 1 fps to test).
//    Fix it!
//    Be elegant, or I'll make you do it again :)
//
// 2. Replace Car speed with Car acceleration.
//    A single constant speed is boring.  Use your old physics formulas to
//    calculate distance using acceleration and time, and replace initial speed
//    with initial acceleration.
//    BONUS: AFTER you write the above, add a special conversion function which
//    translates horsepower, gear ratio, etc into a single acceleration constant
//    then create cars using car specs.  Try to be realistic!
//
// 3. Lets add gears and engine rpm.
//    When creating a car, give it gear ratio information and shift rpm.
//    Then, for any speed, calculate the current gear and engine rpm and print
//    that as part of the Racetrack paint method.
//
// 4. Give each gear a different acceleration number, so that you don't know which
//    car will win a race ahead of time!
//
// 5. Allow placing bets before the race starts.
//    Bonus: start the user off with with some balance and calculate a running
//    total.  Keep starting races until the user runs out of money.
//
// 6. Allow placing bets during the race!
//    NOTE: You will need to do a few things:
//    - wrap the synchronous timer call with a std::async so that your timer
//      continues to run while you wait for input from user on main thread.
//    - since two threads will be reading/writing the betting information, you
//      will need to use a std::atomic<int>
//
// 7. Placing bets during a race was a bad idea, lets replace it with a TURBO
//    button (fuck yeah).
//    - during the race, when you press 1-9 turbo activates and does something
//      epic.
//
// 8. ???
// 9. Profit
//
////////////////////////////////////////////////////////////////////////////////

void sleep(int ms)
{
#if defined(__clang__)
  usleep(ms * 1000);
#else
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

////////////////////////////////////////////////////////////////////////////////

// Car class
// used to track information about cars, currently very trivial
class Car {
public:
  // this is a constructor, defined later
  Car(int number, int speed);

  // these are just member functions, defined inline
  int number() const { return number_; }
  int speedInKmph() const { return speedInKmph_; }

private:
  // these are member variables aka fields
  int number_;
  int speedInKmph_;
};

// this is the constructor definition, define outside
Car::Car(int number, int speed)
  : number_(number), speedInKmph_(speed) // <-- this is field initialization syntax, optional, but prefered way
{
}

////////////////////////////////////////////////////////////////////////////////

class RaceTrack {
public:
  RaceTrack(int distance, std::vector<Car> cars);

  bool paintAtTime(int ms);

private:
  int distanceInM_;
  std::vector<Car> cars_;
};

RaceTrack::RaceTrack(int distance, std::vector<Car> cars)
  : distanceInM_(distance), cars_(cars)
{

}

// This is the only complicated function in this program
// It will print the state of the racetrack at some given time
// If you call this function with a fixed frequency and constant time increments, you will see a nice animation
bool RaceTrack::paintAtTime(int ms) {
  const int racetrackWidth = 60;
  int winner = -1;

  // Setting animation
  std::cout << "Time: " << ms/1000 << std::endl;
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  for (Car const& car : cars_) {
    int speed = car.speedInKmph();
    int distance = (((speed * 1000) * ms) / 3600000);
    float asPercent = std::min(1.0f, float(distance)/distanceInM_);
    int leadingSpaces = (racetrackWidth - 1) * asPercent;
    int trailingSpaces = racetrackWidth - leadingSpaces - 1;
    std::cout << std::string(leadingSpaces, ' ') << " \"O=o>" << std::string(trailingSpaces, ' ') << "[" << speed << "]" << std::endl;

    if (distance >= distanceInM_)
      winner = car.number();
  }
  
  // Finish animation
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  // Check if we have a winner
  bool finished = winner != -1;
  if (finished)
    std::cout << "FINISHED! Car " << winner << " wins! Average speed: " << ((distanceInM_ * 3600) / ms) << " kmph"<< std::endl;
  return finished;
}

////////////////////////////////////////////////////////////////////////////////

class Timer {
public:
  // std::function<> is a new feature that lets you store a reference to a function
  // so that you can pass it around and call it later.
  // in this case, we want to call some function every time the timer ticks.
  // the function is a bool(int) because accepts an int (time in ms) and returns a bool (finished)
  // when callback returns that it is finished (returns true), timer should stop ticking
  Timer(int interval, std::function<bool(int)> callback);

  // this is synchronous, which means your program will not return out of this function until it finishes.
  // if your callback never returns true, this will never return.
  void startSynchronous();

private:
  int intervalInMs_;
  std::function<bool(int)> callback_;
};

Timer::Timer(int interval, std::function<bool(int)> callback)
  : intervalInMs_(interval), callback_(callback)
{

}

void Timer::startSynchronous()
{
  // always start time at 0
  // time values are in ms, and it ticks in intervals provided in the constructor
  for(int currentTime = 0; ; currentTime += intervalInMs_) {
    bool done = callback_(currentTime);
    if (done)
      return;
    sleep(intervalInMs_);
  }
}

////////////////////////////////////////////////////////////////////////////////

int main() {
  // This is a very confusing way of creating a random number generator between 80 and 180.
  // Just ignore it, I can explain later.
  auto rand = std::bind(std::uniform_int_distribution<int>(80, 180), std::mt19937(std::random_device()()));

  // Create 9 cars with random speeds
  std::vector<Car> cars;
  cars.push_back(Car(1, rand()));
  cars.push_back(Car(2, rand()));
  cars.push_back(Car(3, rand()));
  cars.push_back(Car(4, rand()));
  cars.push_back(Car(5, rand()));
  cars.push_back(Car(6, rand()));
  cars.push_back(Car(7, rand()));
  cars.push_back(Car(8, rand()));
  cars.push_back(Car(9, rand()));

  // Create racetrack
  RaceTrack racetrack(402, std::move(cars)); // 402m is a quarter mile

  // Figure out the timer tick interval
  // This can be anything you want, yet the speed of the cars will remain constant
  // However, if its too slow it wont feel like an animation
  // and if its too fast, the terminal will do silly stuff
  // so just settle on 60 "frames per second"
  const int targetFrameRate = 60;
  const int paintIntervalInMs = 1000/targetFrameRate;

  // Start a timer with the chosen time interval and callback.
  // Using std::bind in the second argument is just a fancy way to say "hey timer, use this function for your callback"
  // For now, just dont worry about it, I can explain in person
  Timer timer(paintIntervalInMs, std::bind(&RaceTrack::paintAtTime, racetrack, std::placeholders::_1));
  timer.startSynchronous();
}
