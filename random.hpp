#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

class random_gen {
    std::random_device rd;
    std::mt19937 e;
    std::uniform_int_distribution<int> uniform_dist;
public:
    random_gen(int from, int to);
    int get();
};

#endif //RANDOM_HPP