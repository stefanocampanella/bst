# Binary Search Tree

The class and methods have been implemented in a header only file (`bst.h`), according to the prescribed signature.

The `Tree` class is a wrapper around `__node`, which model the real recursive data structure (although better encapsulation could have been provided in other ways, the `__node` has been implemented as a struct for convenience). 

An helper class `__iterator` provides the iterator interface.  Indeed, the core of the `Tree` class is the `insert` method, which ensures the consistency of the data model, while the iterator just traverse the tree.

The benchmark (`benchmark.cc`) can be compiled with gcc/clang (using `-std=c++17`) and the executable takes two arguments, the number of elements which are pushed into the `Tree`/`std::map` and the number of keys to search for (the keys are selected at random among the ones in the container).

Even for small trees, `std::map` outperforms by orders of magnitude the proposed implementation. The most probable explanation is that this is due to the `balance` method, which could be fragile or broken. 
