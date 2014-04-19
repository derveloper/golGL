#include <array>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <vector>

#include <boost/range/irange.hpp>

#include "world.hpp"


world::world(const int &width, const int &height)
  : width(width),
    height(height),
    generation(0)
{
  random_gen r(0,5);
  for(auto x : boost::irange(0, width)) {
    cell_vector row;
    for(auto y : boost::irange(0, height)) {
      row.push_back(cell{(r.get() == 1)});
    }
    cells.push_back(row);
  }
  last_gen = cells;
}

void world::seed_life(const bool random) {
  if(random) {
    random_gen r(0,5);
    for(auto &col : cells) {
      for(auto &cell : col) {
        cell.alive = (r.get() == 1);
      }
    }
  }
  else {
    for(auto &col : cells) {
      for(auto &cell : col) {
        cell.alive = false;
      }
    }
  }
  last_gen = cells;
}

void world::seed_life(cell_grid &seed) {
  for(auto x : boost::irange(0, width)) {
    for(auto y : boost::irange(0, height)) {
      cells[x][y].alive = seed[x][y].alive;
    }
  }
}

void world::next_generation() {
  last_gen = cells;
  for(auto x : boost::irange(0, width)) {
    for(auto y : boost::irange(0, height)) {
      evolution(x, y);
    }
  }
  generation++;
}

void world::evolution(const int &x, const int &y) {
  int n = neighbours(x,y);
  cell &c = cells[x][y];
  if(c.alive) {
    c.alive = (n == 2 || n == 3);
  }
  else {
    c.alive = (n == 3);
  }
}

void world::dump_generation() {
  last_dump = get_timestamp();
  random_gen r(1000000,9999999);
  last_dump_str = std::to_string(r.get())+"_"+std::to_string(last_dump);
  std::ofstream dump("dump_"+last_dump_str+".gol", std::ios::binary );
  for(auto &col : cells) {
    for(auto &cell : col) {
      dump.write(reinterpret_cast<const char*>(&cell.alive), sizeof(cell.alive));
    }
  }
}

void world::load_generation(std::string filename) {
  std::ifstream dump(filename, std::ios::binary );
  if(dump) {
    std::string alive_str;
    bool alive;
    for(auto &col : cells) {
      for(auto &cell : col) {
        dump.read(&*alive_str.begin(), sizeof(alive));
        alive = static_cast<bool>(alive_str[0]);
        cell.alive = alive;
      }
    }
  }
  last_gen = cells;
}

unsigned long world::get_timestamp() {
  return
      std::chrono::system_clock::now().time_since_epoch() /
      std::chrono::milliseconds(1);
}

int const world::count_neighbours(const std::array<int,2> &ni, const int &x, const int &y) {
  int x_off = ni[0];
  int y_off = ni[1];
  int nx = x+x_off;
  int ny = y+y_off;

  if(nx == width) {
    nx = 0;
    return 0;
  }
  if(nx == -1) {
    nx = width-1;
    return 0;
  }

  if(ny == height) {
    ny = 0;
    return 0;
  }
  if(ny == -1) {
    ny = height-1;
    return 0;
  }

  if(last_gen[nx][ny].alive) {
    return 1;
  }

  return 0;
}

const int world::neighbours(const int &x, const int &y) {
  const std::array<std::array<int,2>, 8> n_arr{{
    {-1, 1}, {0, 1}, {1, 1},
    {-1, 0},         {1, 0},
    {-1,-1}, {0,-1}, {1,-1}
  }};

  int n = 0;

  for(auto &ni : n_arr) {
    n += count_neighbours(ni, x, y);
  }

  return n;
}

random_gen::random_gen(int from, int to)
  : rd(),
    e(rd()),
    uniform_dist(from,to)
{ }

int random_gen::get() {
  return uniform_dist(e);
}
