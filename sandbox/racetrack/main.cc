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
class Car {
public:
  Car(int number, int acceleration);

  int number() const { return number_; }
  int speedInKmphAtTimeInMs(int ms) const;
  int distanceTraveledAtTimeInMs(int ms) const;

private:
  int number_;
  int acceleration_; // m/s^2
};

Car::Car(int number, int acceleration)
  : number_(number), acceleration_(acceleration)
{
}

int Car::speedInKmphAtTimeInMs(int ms) const
{
  return acceleration_ * (double(ms) / 1000);
}

int Car::distanceTraveledAtTimeInMs(int ms) const
{
  return 0.5 * acceleration_ * std::pow(double(ms)/1000, 2);
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

bool RaceTrack::paintAtTime(int ms) {
  const int racetrackWidth = 60;
  int winner = -1;

  std::cout << "Time: " << ms/1000 << std::endl;
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  for (Car const& car : cars_) {
    int speed = car.speedInKmphAtTimeInMs(ms);
    int distance = car.distanceTraveledAtTimeInMs(ms);
    float asPercent = std::min(1.0f, float(distance)/distanceInM_);
    int leadingSpaces = (racetrackWidth - 1) * asPercent;
    int trailingSpaces = racetrackWidth - leadingSpaces - 1;
    std::cout << std::string(leadingSpaces, ' ') << " \"O=o>" << std::string(trailingSpaces, ' ') << "[" << speed << "]" << std::endl;

    if (distance >= distanceInM_)
      winner = car.number();
  }
  
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  bool finished = winner != -1;
  if (finished)
    std::cout << "FINISHED! Car " << winner << " wins! Average speed: " << ((distanceInM_ * 3600) / ms) << " kmph"<< std::endl;
  return finished;
}

////////////////////////////////////////////////////////////////////////////////

class Timer {
public:
  Timer(int interval, std::function<bool(int)> callback);

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
  for(int currentTime = 0; ; currentTime += intervalInMs_) {
    bool done = callback_(currentTime);
    if (done)
      return;
    sleep(intervalInMs_);
  }
}

////////////////////////////////////////////////////////////////////////////////

int main() {
  auto rand = std::bind(std::uniform_int_distribution<int>(2, 10), std::mt19937(std::random_device()()));

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

  RaceTrack racetrack(402, std::move(cars)); // 402m is a quarter mile

  const int targetFrameRate = 60;
  const int paintIntervalInMs = 1000/targetFrameRate;

  Timer timer(paintIntervalInMs, std::bind(&RaceTrack::paintAtTime, racetrack, std::placeholders::_1));
  timer.startSynchronous();
}
