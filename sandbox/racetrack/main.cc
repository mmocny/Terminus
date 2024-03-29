#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <stdexcept>

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
//    BONUS: AFTER you write the above, add a special conversion function which
//    translates horsepower, etc into a single acceleration constant
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
  Car(Car const&) = delete;
  Car& operator=(Car const&) = delete;

  Car(Car&&) = default;
  Car& operator=(Car&&) = default;

public:
  Car(int number, double acceleration);

  int number() const { return number_; }
  int speedInKmphAtTimeInMs(int ms) const;
  unsigned int distanceTraveledAtTimeInMs(int ms) const;

  void boost() { ++boosts_; };

private:
  int number_;
  double accelerationInMps2_;
  int boosts_ = 0;
};

Car::Car(int number, double acceleration)
  : number_(number), accelerationInMps2_(acceleration)
{
  assert(number > 0);
  assert(acceleration > 0);
}

int Car::speedInKmphAtTimeInMs(int ms) const
{
  double s = ms / 1000.0;
  return (accelerationInMps2_ * s * 3.6);
}

unsigned int Car::distanceTraveledAtTimeInMs(int ms) const
{
  double s = ms / 1000.0;
  return (accelerationInMps2_ * s * s / 2) + boosts_ * 50;
}

////////////////////////////////////////////////////////////////////////////////

class RaceTrack {
public:
  RaceTrack(unsigned int distance, std::vector<Car> cars);

  bool paintAtTime(int ms);

  Car& getCar(int num);

private:
  unsigned int distanceInM_;
  std::vector<Car> cars_;
};

RaceTrack::RaceTrack(unsigned int distance, std::vector<Car> cars)
  : distanceInM_(distance), cars_(std::move(cars))
{
}

bool RaceTrack::paintAtTime(int ms) {
  ms *= 10;
  static const int racetrackWidth = 60;

  std::cout << "Time: " << ms/1000 << std::endl;
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  bool raceComplete = false;
  //for (auto it = cars_.begin(); it != cars_.end(); ++it) {
  //  Car const& car = *it;
  for (Car const& car : cars_) {
    int speed = car.speedInKmphAtTimeInMs(ms);
    unsigned int distance = car.distanceTraveledAtTimeInMs(ms);
    float asPercent = std::min(1.0f, float(distance)/distanceInM_);
    int leadingSpaces = (racetrackWidth - 1) * asPercent;
    int trailingSpaces = racetrackWidth - leadingSpaces - 1;
    std::cout << std::string(leadingSpaces, ' ') << " \"O=o>" << std::string(trailingSpaces, ' ') << "[" << speed << "]" << std::endl;
    if (distance >= distanceInM_)
      raceComplete = true;
  }
  std::cout << std::string(racetrackWidth, '*') << std::endl;

  if (raceComplete) {
    int currentLeader = 0;
    unsigned int currentLeaderDistance = 0;
    for (Car const& car : cars_) {
      unsigned int distance = car.distanceTraveledAtTimeInMs(ms);
      if (distance > currentLeaderDistance) {
        currentLeaderDistance = distance;
        currentLeader = car.number();
      }
    }
   std::cout << "FINISHED! Car " << currentLeader << " wins! Average speed: " << ((currentLeaderDistance* 3600) / ms) << " kmph"<< std::endl;
  }

  return raceComplete;
}

Car& RaceTrack::getCar(int num)
{
  for (Car& car : cars_) {
    if (car.number() == num)
      return car;
  }
  throw std::out_of_range("Car not found.");
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

void getinput(RaceTrack& racetrack)
{
  int num;
  while (std::cin >> num) {
    try {
      Car& car = racetrack.getCar(num);
      car.boost();
    } catch (std::exception const& e) {
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

double getrandom(double min, double max)
{
  assert(min >= max);
  //auto rand = std::bind(std::uniform_int_distribution<int>(2, 10), std::mt19937(std::random_device()()));
  return (((double(rand()) / RAND_MAX) * (max-min))+min);
}

int main() {
  std::srand(time(NULL));
  auto rand = std::bind(&getrandom, 4, 10);

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

  std::thread timer(
      [&] {
        setInterval(std::bind(&RaceTrack::paintAtTime, &racetrack, std::placeholders::_1), paintIntervalInMs);
      });
  
  //getinput(racetrack);

  timer.join();
}
