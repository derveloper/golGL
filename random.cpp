#include <boost/cast.hpp>
#include "random.hpp"

random_gen::random_gen(int from, int to)
        : rd(),
          e(rd()),
          uniform_dist(from,to)
{ }

int random_gen::get() {
    return uniform_dist(e);
}
