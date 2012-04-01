#include <iostream>
#include <thread>

int main() {
	auto t = [] {
		std::cout << "Hello World!" << std::endl;
	};
	t();
}
