#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <functional>

template<typename data_t>
struct __node {
  std::unique_ptr<__node> left;
  std::unique_ptr<__node> right;
  data_t data;
  __node * const up;

  // default constructor is not allowed:
  // data and upper node are invariants for the class
  // and must be provided
  __node() = delete;
 
  // move semantics
  __node(__node&&) noexcept = default;
  __node& operator=(__node&& t) noexcept = default;

  // (deep) copy semantics, assignment is implied
  __node(const __node& t) :
    left{nullptr}, right{nullptr}, data{t.data}, up{t.up} {
      std::cout << "copy ctor called" << std::endl;
      if(!t.is_leaf()) {
        left = std::make_unique<__node>(*t.left);
        right = std::make_unique<__node>(*t.right);
      }
    }

  __node(const data_t& x, __node * const n) : 
    left{nullptr}, right{nullptr}, data{x}, up{n} {}
  __node(data_t&& x, __node * const n) : 
    left{nullptr}, right{nullptr}, data{std::move(x)}, up{n} {}

  bool has_left()  const { return left.get() != nullptr; };
  bool has_right() const { return right.get() != nullptr; };
  bool is_leaf()   const { return !(has_left() || has_right()); }
  
  auto leftmost() const {
    if( !has_left() ) return this;
    else              return left->leftmost(); 
  }

  auto leftmost() {
    if( !has_left() ) return this;
    else              return left->leftmost(); 
  }

#ifdef DEBUG 
  void debug() const {
    std::cout << "(" << data.first << ", ";
    
    if( up != nullptr )
      std::cout << up->data.first << ", ";
    else
      std::cout << "null, ";

    if( has_left() )
      std::cout << left->data.first << ", ";
    else
      std::cout<< "null, ";

    if( has_right() )
      std::cout << right->data.first << ")";
    else
      std::cout<< "null) ";

    std::cout << std::endl;
  }
#endif
};

template <typename T, typename U>
class __iterator;

template <typename key_t, typename val_t, typename cmp_t = std::less<key_t>>
class Tree {
  using data_t = std::pair<const key_t, val_t>;
  using node_t = __node<data_t>;
  using iterator = __iterator<data_t, node_t>;
  using const_iterator = __iterator<const data_t, node_t>;
  
  std::unique_ptr<node_t> root;
  const cmp_t cmp = cmp_t{};

  public:

  // Default constructor
  Tree() noexcept = default;

  // Move semantics
  Tree(Tree&&) noexcept = default;
  Tree& operator=(Tree&&) noexcept = default;

  bool is_empty() const { return root.get() == nullptr; }

  iterator begin() { 
    if( is_empty() )
      return iterator{nullptr};
    else {
#ifdef DEBUG
      root->leftmost()->debug();
#endif 
      return iterator{root->leftmost()};
    }
  }
  iterator end() { return iterator{nullptr}; }
  
  const_iterator begin() const { 
    if( is_empty() )
      return const_iterator{nullptr};
    else
      return const_iterator{root->leftmost()};
  }
  const_iterator end() const { return const_iterator{nullptr}; }

  const_iterator cbegin() const { 
    if( is_empty() )
      return iterator{nullptr};
    else
      return const_iterator{root->leftmost()};
  }
  const_iterator cend() const { return const_iterator{nullptr}; }
  
  template<typename T>
  std::pair<iterator, bool> insert(T&& x) {
    if( is_empty() ) {
      root = std::make_unique<node_t>(x, nullptr);
      return {iterator{root.get()}, true};
    } 
    else {
      auto k = x.first;
      auto n = root.get();
      while( true ) {
        auto kp = n->data.first;
        if ( !cmp(k, kp) && !cmp(kp, k) ) 
          return {iterator{n}, false};
        else if( cmp(k, kp) && n->has_left() )
          n = n->left.get();
        else if( cmp(kp, k) && n->has_right() )
          n = n->right.get();
        else
          break;
      }
      if ( cmp(k, n->data.first) ) {
        n->left = std::make_unique<node_t>(x, n);
        return {iterator{n->left.get()}, true};
      }
      else {
        n->right = std::make_unique<node_t>(x, n->up);
        return {iterator{n->right.get()}, true};
      }
    }
  }

  template<typename... Types>
  std::pair<iterator, bool> emplace(Types&&... args) {
    return insert(data_t{std::forward<Types>(args)...});
  }

  void clear() { root.release(); }

  iterator find(const key_t& k) {
    auto itr = begin();
    while(itr != end() && itr->first != k)
      ++itr;
    return itr;
  }
   
  const_iterator find(const key_t& k) const {
    auto itr = cbegin();
    while(itr != cend() && itr->first != k)
      ++itr;
    return itr;
  }

  template<typename T>
  val_t& operator[](T&& k) {
    auto elem = emplace(k, val_t{}).first;
    return elem->second;
  }

  friend std::ostream& operator<<(std::ostream& os, const Tree& t) {
    for(auto& v : t) os << v.first << " ";
    return os;
  }
};

template <typename T, typename U>
class __iterator {
  U* current;

 public:
  explicit __iterator(U* x) noexcept : current{x} {}

  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  using reference = value_type&;
  using pointer = value_type*;

  reference operator*() const noexcept { return current->data; }
  pointer operator->() const noexcept { return &(*(*this)); }

  __iterator& operator++() noexcept {
    if (current->has_right()) {
      current = current->right.get();
      current = current->leftmost();
    }
    else 
      current = current->up;
#ifdef DEBUG 
    if (current != nullptr)
      current->debug();
#endif
    return *this;
  }

  friend bool operator==(const __iterator& a, const __iterator& b) {
    return a.current == b.current;
  }
  friend bool operator!=(const __iterator& a, const __iterator& b) {
    return !(a == b);
  }
};
