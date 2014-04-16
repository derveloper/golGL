#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <random>

struct cell {
  bool alive;
};

typedef std::vector<cell> cell_vector;
typedef std::vector<cell_vector> cell_grid;

class random_gen {
  std::random_device rd;
  std::mt19937 e;
  std::uniform_int_distribution<int> uniform_dist;
public:
  random_gen(int from, int to);
  int get();
};

class world
{
public:
  cell_grid cells;
  int width;
  int height;

public:
  world(int width = 100, int height = 70);

  void seed_life();
  void next_generation();
  int neighbours(int &x, int &y);
};

#endif // WORLD_HPP
