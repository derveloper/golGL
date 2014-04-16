#include <iostream>

#include "world.hpp"

using namespace std;

void print_generation(world &w) {
  for(auto col : w.cells) {
    for(auto cell : col) {
      std::cout << (cell.alive ? 'A' : '#');
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int main()
{
  world w(10, 10);

  while(std::cin.get()) {
    print_generation(w);
    w.next_generation();
  }
  return 0;
}

