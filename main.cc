#include "bst.h"
#include <exception>
#include <vector>

int main(){
  try {

    std::vector test{1,3,4,6,7,8,10,13,14};
    Tree<int, int> t{};

    for(const auto& x : test) t.emplace(x, 0);
    t.balance();
    std::cout << t << std::endl;

    Tree s{t};
    t.emplace(12, 0);

    std::cout << t << std::endl;

  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "Error: unknown exception" << std::endl;
  }
}
