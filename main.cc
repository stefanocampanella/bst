#include "bst.h"
#include <random>

int main(){
  Tree<int, int> t{};

  auto rand = 
    [rng = std::default_random_engine(), 
    dist = std::uniform_int_distribution<int>(-100,100)]() mutable {
      return dist(rng);
    };

  for(auto n = 0; n < 20; ++n) t.emplace(rand(), 0);
  std::cout << t << std::endl;
  t[90] = 12;

  t.clear();
  std::cout << t << std::endl;
}
