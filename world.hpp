#ifndef WORLD_HPP
#define WORLD_HPP

#include <vector>
#include <array>

constexpr const int THREADS = 4;

struct cell {
  bool alive;
};

typedef std::vector<cell> cell_vector;
typedef std::vector<cell_vector> cell_grid;

class world
{
public:
  cell_grid cells;
  cell_grid last_gen;
  int width;
  int height;
  int ratio_w;
  int ratio_h;
  int generation;
  unsigned long last_dump;
  std::string last_dump_str;

public:
  world(const int &width = 100, const int &height = 70);

  void seed_life(const bool random = true);
  void seed_life(cell_grid &seed);
  void next_generation();
  void evolution(const int &x, const int &y);
  void dump_generation();
  void load_generation(std::string filename);
  unsigned long get_timestamp();
  int const neighbours(const int &x, const int &y);
  int const count_neighbours(const std::array<int,2> &ni, const int &x, const int &y);
};

#endif // WORLD_HPP
