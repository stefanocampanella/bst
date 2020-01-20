#include "bst.h"
#include <iostream>
#include <random>
#include <sstream>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <exception>
#include <chrono>

int main(int argc, char* argv[]) {
  try {
    if (argc != 2)
      throw std::invalid_argument{"Error: bad arguments"};
    
    size_t size;
    {
      std::stringstream sarg{argv[1]};
      sarg >> size;
      if (sarg.fail())
        throw std::invalid_argument("Error: bad arguments");
    }
   
    /*
    size_t tests;
    {
      std::stringstream sarg{argv[2]};
      sarg >> tests;
      if (sarg.fail())
        throw std::invalid_argument("Error: bad arguments");
    }
    */
    
    Tree<int, int> t;
    std::map<int, int> m;
    auto rand = 
      [rng = std::default_random_engine(std::random_device{}()), 
      dist = std::uniform_int_distribution<int>()]() mutable {
        return dist(rng);
      };

    for(size_t n = 0; n != size; ++n) {
      t.emplace(rand(), 0);
      m.emplace(rand(), 0);
    }
    t.balance();

    for(size_t n = 0; n != size; ++n) 
      if(t[n] != m[n])
          throw std::runtime_error("Element not found");

    auto max = std::max_element(m.begin(), m.end())->first;
    auto benchmark = [&max] (const auto& container) {
      auto t1 = std::chrono::high_resolution_clock::now();
      container.find(max + 1);
      auto t2 = std::chrono::high_resolution_clock::now();
      auto elapsed = 
        std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
      return elapsed.count();
    };
    
    /*
    std::vector<std::pair<const int, int>> test_cases{};
    std::sample(
        m.begin(),
        m.end(),
        std::back_inserter(test_cases),
        tests,
        std::default_random_engine{std::random_device{}()});

    auto benchmark = [&test_cases](const auto& container) {
      std::vector<std::chrono::nanoseconds::rep> times;
      for(const auto& x : test_cases) {
        auto t1 = std::chrono::high_resolution_clock::now();
        //if (container.find(x.first) == container.end())
        //  throw std::runtime_error("Element not found");
        container.find(x.first);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto elapsed = 
          std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
        times.push_back(elapsed.count());
      }
      double avg = 
        std::accumulate(times.begin(), times.end(), 0.0) / times.size();
      return avg; 
    };
    */

    std::cout << benchmark(m) << '\t' << benchmark(t) << std::endl;
    
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Error: unknown exception" << std::endl;
  }
}
