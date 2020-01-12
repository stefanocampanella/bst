#pragma once

#include <iostream>
#include <iterator>
#include <memory>
#include <functional>
#include <algorithm>
#include <set>

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
    return *this;
  }

  friend bool operator==(const __iterator& a, const __iterator& b) {
    return a.current == b.current;
  }
  friend bool operator!=(const __iterator& a, const __iterator& b) {
    return !(a == b);
  }
};

template<typename data_t>
struct __node {
  std::unique_ptr<__node> left;
  std::unique_ptr<__node> right;
  data_t data;
  __node * const up;

  __node() = delete;
 
  __node(__node&&) noexcept = default;
  __node& operator=(__node&& t) noexcept = default;
  
  __node(const __node& t) :
    left{nullptr}, right{nullptr}, data{t.data}, up{t.up} {
      if(t.has_left())  left  = std::make_unique<__node>(*t.left);
      if(t.has_right()) right = std::make_unique<__node>(*t.right);
    }

  __node(const data_t& x, __node * const n) : 
    left{nullptr}, right{nullptr}, data{x}, up{n} {}
  __node(data_t&& x, __node * const n) : 
    left{nullptr}, right{nullptr}, data{std::move(x)}, up{n} {}

  bool has_left()  const noexcept { return left.get() != nullptr; };
  bool has_right() const noexcept { return right.get() != nullptr; };
  
  auto leftmost() const noexcept {
    if( !has_left() ) return this;
    else              return left->leftmost(); 
  }

  auto leftmost() noexcept {
    if( !has_left() ) return this;
    else              return left->leftmost(); 
  }
};

template<typename key_t, typename cmp_t, typename node_t>
node_t* binary_search(const key_t k, const cmp_t lt, node_t* const n) {
  if(lt(k, n->data.first) && n->has_left()) 
    return binary_search<key_t, cmp_t, node_t>(k, lt, n->left.get());
  else if(lt(n->data.first, k) && n->has_right()) 
    return binary_search<key_t, cmp_t, node_t>(k, lt, n->right.get());
  else
    return n;
}

template <typename key_t, typename val_t, typename cmp_t = std::less<key_t>>
class Tree {
  using data_t = std::pair<const key_t, val_t>;
  using node_t = __node<data_t>;
  using iterator = __iterator<data_t, node_t>;
  using const_iterator = __iterator<const data_t, node_t>;
  
  std::unique_ptr<node_t> root;
  const cmp_t lt = cmp_t{};
  
  bool is_empty() const { return root.get() == nullptr; }

  iterator median() {
    auto d = std::distance(begin(), end());
    if(d == 1)
      return begin();
    else {
      auto m = begin();
      for(auto n = 0; n < std:: distance(begin(), end()) / 2; ++n) ++m;
      return m;
    }
  }

  public:

  Tree() noexcept = default;

  Tree(Tree&&) noexcept = default;
  Tree& operator=(Tree&&) noexcept = default;

  Tree(const Tree& t) { root = std::make_unique<node_t>(*t.root); }

  iterator begin() noexcept { 
    if( is_empty() )
      return iterator{nullptr};
    else {
      return iterator{root->leftmost()};
    }
  }
  iterator end() noexcept { return iterator{nullptr}; }
  
  const_iterator begin() const noexcept { 
    if( is_empty() )
      return const_iterator{nullptr};
    else
      return const_iterator{root->leftmost()};
  }
  const_iterator end() const noexcept { return const_iterator{nullptr}; }

  const_iterator cbegin() const noexcept { 
    if( is_empty() )
      return iterator{nullptr};
    else
      return const_iterator{root->leftmost()};
  }
  const_iterator cend() const noexcept { return const_iterator{nullptr}; }
  
  template<typename T>
  std::pair<iterator, bool> insert(T&& x) {
    if( is_empty() ) {
      root = std::make_unique<node_t>(x, nullptr);
      return {iterator{root.get()}, true};
    } 
    else {
      auto n = binary_search(x.first, lt, root.get());
      if ( lt(x.first, n->data.first) ) {
        n->left = std::make_unique<node_t>(x, n);
        return {iterator{n->left.get()}, true};
      }
      else if ( lt(n->data.first, x.first) ) {
        n->right = std::make_unique<node_t>(x, n->up);
        return {iterator{n->right.get()}, true};
      }
      else
        return {iterator{n}, false};
    }
  }

  template<typename... Types>
  std::pair<iterator, bool> emplace(Types&&... args) {
    return insert(data_t{std::forward<Types>(args)...});
  }

  void clear() noexcept { root.release(); }

  iterator find(const key_t& k) noexcept {
    auto itr = begin();
    while(itr != end() && itr->first != k)
      ++itr;
    return itr;
  }
   
  const_iterator find(const key_t& k) const noexcept {
    auto itr = cbegin();
    while(itr != cend() && itr->first != k)
      ++itr;
    return itr;
  }

  void balance() {
    std::set s{begin(), end(), 
      [](data_t x, data_t y){ return x.first < y.first; }};
    Tree t{};
    auto median = [&s]() {
      auto p = s.begin();
      for(auto n = 0; n < std::distance(s.begin(), s.end()) / 2; ++n) ++p;
      return *p;
    };

    while(!s.empty()) {
      auto data = median();
      t.emplace(data);
      s.erase(data);
    }

    root.swap(t.root);
  }

  template<typename T>
  val_t& operator[](T&& k) noexcept {
    auto elem = emplace(k, val_t{}).first;
    return elem->second;
  }

  friend std::ostream& operator<<(std::ostream& os, const Tree& t) {
    for(const auto& v : t) os << v.first << " ";
    return os;
  }
};

