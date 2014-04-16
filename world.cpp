#include <array>
#include <random>

#include <boost/range/irange.hpp>

#include "world.hpp"

world::world(int width, int height)
  : width(width),
    height(height)
{
  random_gen r(0,3);
  for(auto x : boost::irange(0, width)) {
    cell_vector row;
    for(auto y : boost::irange(0, height)) {
      row.push_back(cell{(r.get() == 1)});
    }
    cells.push_back(row);
  }
}

void world::seed_life() {
  random_gen r(0,3);
  for(auto &col : cells) {
    for(auto &cell : col) {
      cell.alive = (r.get() == 1);
    }
  }
}

void world::next_generation() {
  for(auto x : boost::irange(0, width)) {
    for(auto y : boost::irange(0, height)) {
      int n = neighbours(x,y);
      cell &c = cells[x][y];
      if(c.alive) {
        c.alive = (n == 2 || n == 3);
      }
      else {
        c.alive = (n == 3);
      }
    }
  }
}

int world::neighbours(int &x, int &y) {
  std::array<std::array<int,2>, 8> n_arr{{
    {-1, 1}, {0, 1}, {1, 1},
    {-1, 0},         {1, 0},
    {-1,-1}, {0,-1}, {1,-1}
  }};

  int n = 0;

  for(auto ni : n_arr) {
    int x_off = ni[0];
    int y_off = ni[1];
    int nx = x+x_off;
    int ny = y+y_off;

    if(nx == width) {
      nx = 0;
    }
    else if(nx == -1) {
      nx = width-1;
    }

    if(ny == height) {
      ny = 0;
    }
    else if(ny == -1) {
      ny = height-1;
    }

    if(cells[nx][ny].alive) {
      n++;
    }
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
