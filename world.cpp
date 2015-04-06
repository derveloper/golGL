#include <array>
#include <fstream>
#include <future>
#include <iostream>
#include <random>
#include <vector>
#include <thread>

#include <boost/range/irange.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include "world.hpp"
#include "random.hpp"


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
  std::vector<std::thread> threads;

  auto w_range = boost::irange(0, width);
  auto h_range = boost::irange(0, height);
  auto workers = boost::irange(0, THREADS);
  auto worker_load = cells.size()/THREADS;

  boost::for_each(workers, [this, &threads, &worker_load, w_range, h_range] (int worker) {
      auto start_w = worker*worker_load;
      threads.push_back(std::thread([this, start_w, worker, worker_load, w_range, h_range] {
        std::for_each(w_range.begin()+start_w, w_range.begin()+start_w+worker_load, [this, h_range] (int x) {
            boost::for_each(h_range, [&] (int y) {
                evolution(x, y);
            });
        });
      }));
  });

  boost::for_each(threads, [] (auto &t) { t.join(); });

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

  auto sum = [=](auto a, auto b) { return a + count_neighbours(b, x, y); };
  return std::accumulate(n_arr.begin(), n_arr.end(), 0, sum);
}
