#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

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

// Car class
class Car {
public:
  Car(int number, double acceleration);

  int number() const { return number_; }
  int speedInKmphAtTimeInMs(int ms) const;
  int distanceTraveledAtTimeInMs(int ms) const;

private:
  int number_;
  double accelerationInMps2_;
};

Car::Car(int number, double acceleration)
  : number_(number), accelerationInMps2_(acceleration)
{
}

int Car::speedInKmphAtTimeInMs(int ms) const
{
  double s = ms / 1000.0;
  return (accelerationInMps2_ * s * 3.6);
}

int Car::distanceTraveledAtTimeInMs(int ms) const
{
  double s = ms / 1000.0;
  return (accelerationInMps2_ * s * s / 2);
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
  static const int racetrackWidth = 60;

  std::cout << "Time: " << ms/1000 << std::endl;
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  bool raceComplete = false;
  //for (auto it = cars_.begin(); it != cars_.end(); ++it) {
  //  Car const& car = *it;
  for (Car const& car : cars_) {
    int speed = car.speedInKmphAtTimeInMs(ms);
    int distance = car.distanceTraveledAtTimeInMs(ms);
    float asPercent = std::min(1.0f, float(distance)/distanceInM_);
    int leadingSpaces = (racetrackWidth - 1) * asPercent;
    int trailingSpaces = racetrackWidth - leadingSpaces - 1;
    std::cout << std::string(leadingSpaces, ' ') << " \"O=o>" << std::string(trailingSpaces, ' ') << "[" << speed << "]" << std::endl;
    if (distance >= distanceInM_)
      raceComplete = true;
  }
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  if (raceComplete) {
    int currentLeader = -1;
    int currentLeaderDistance = -1;
    for (Car const& car : cars_) {
      int distance = car.distanceTraveledAtTimeInMs(ms);
      if (distance > currentLeaderDistance) {
        currentLeaderDistance = distance;
        currentLeader = car.number();
      }
    }
   std::cout << "FINISHED! Car " << currentLeader << " wins! Average speed: " << ((currentLeaderDistance* 3600) / ms) << " kmph"<< std::endl;
  }

  return raceComplete;
}

////////////////////////////////////////////////////////////////////////////////

void sleep(int ms)
{
#if defined(__clang__)
  usleep(ms * 1000);
#else
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

template< class Func >
void setInterval(Func func, int intervalInMs)
{
  for(int currentTime = 0; ; currentTime += intervalInMs) {
    bool done = func(currentTime);
    if (done)
      return;
    sleep(intervalInMs);
  }
}

////////////////////////////////////////////////////////////////////////////////

double getrandom(double min, double max)
{
  //auto rand = std::bind(std::uniform_int_distribution<int>(2, 10), std::mt19937(std::random_device()()));
  return (((double(rand()) / RAND_MAX) * (max-min))+min);
}

int main() {
  std::srand(time(NULL));
  auto rand = std::bind(&getrandom, 5, 7);

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

  const int targetFrameRate = 12;
  const int paintIntervalInMs = 1000/targetFrameRate;

  setInterval(std::bind(&RaceTrack::paintAtTime, racetrack, std::placeholders::_1), paintIntervalInMs);
}
