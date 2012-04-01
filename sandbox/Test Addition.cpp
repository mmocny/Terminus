// Test Addition.cpp : main project file.

#include <iostream>


int add(int a, int b)
{
	return a+b;
}

int main()
{
	std::cout << "lolol";

#if defined(_WIN32)
	char c;
	std::cin.get(c); 
#endif
}
