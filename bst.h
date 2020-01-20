#pragma once

#include <iostream>
#include <iterator>
#include <memory>
#include <functional>
#include <algorithm>

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
      current = current->next;
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
  __node * const next;

  __node() = delete;
 
  __node(__node&&) noexcept = default;
  __node& operator=(__node&& t) noexcept = default;
  
  __node(const __node& t) :
    left{nullptr}, right{nullptr}, data{t.data}, next{nullptr} {
      if(t.has_left())  
        left = std::make_unique<__node>(t.left->data, this);
      if(t.has_right()) 
        right = std::make_unique<__node>(t.right->data, this->next);
    }

  __node(const data_t& x, __node * const n) : 
    left{nullptr}, right{nullptr}, data{x}, next{n} {}
  __node(data_t&& x, __node * const n) : 
    left{nullptr}, right{nullptr}, data{std::move(x)}, next{n} {}

  bool has_left()  const noexcept { return left != nullptr; }
  bool has_right() const noexcept { return right != nullptr; }
  
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
  cmp_t lt;
  
  bool is_empty() const { return root == nullptr; }
  
  template<typename iterator_t> 
  auto __begin() const {
    if( is_empty() )
      return iterator_t{nullptr};
    else {
      return iterator_t{root->leftmost()};
    }
  }
  
  template<typename T>
  auto __insert(T&& x) {
    if( is_empty() ) {
      root = std::make_unique<node_t>(x, nullptr);
      return std::pair{iterator{root.get()}, true};
    } 
    else {
      auto n = binary_search(x.first, lt, root.get());
      if ( lt(x.first, n->data.first) ) {
        n->left = std::make_unique<node_t>(x, n);
        return std::pair{iterator{n->left.get()}, true};
      }
      else if ( lt(n->data.first, x.first) ) {
        n->right = std::make_unique<node_t>(x, n->next);
        return std::pair{iterator{n->right.get()}, true};
      }
      else
        return std::pair{iterator{n}, false};
    }
  }
  
  template<typename T>
  val_t& __subscript(T&& k) {
    auto elem = emplace(k, val_t{}).first;
    return elem->second;
  }

  public:

  Tree() noexcept = default;

  Tree(Tree&&) noexcept = default;
  Tree& operator=(Tree&&) noexcept = default;

  Tree(const Tree& t) { root = std::make_unique<node_t>(*t.root); }

  Tree(cmp_t op) : lt{op} {}

  iterator begin() noexcept { return __begin<iterator>(); }
  iterator end()   noexcept { return iterator{nullptr}; }
  
  const_iterator begin() const noexcept { return __begin<const_iterator>(); }
  const_iterator end()   const noexcept { return const_iterator{nullptr}; }

  const_iterator cbegin() const noexcept { return __begin<const_iterator>(); }
  const_iterator cend()   const noexcept { return const_iterator{nullptr}; }
  

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

  auto insert(const data_t& x) { return __insert<data_t>(x); }
  auto insert(data_t&& x) { return __insert<data_t>(std::move(x)); }
  
  void balance() {
    Tree t{};
    auto m = median(begin(), end());
    //std::cout << "balance begin with " << m->first << std::endl;
    t.emplace(*m);
    balance(begin(), end(), t);
    root.swap(t.root);
  }

  void balance(iterator first, iterator last, Tree& t) {
    if (first != last) {
      auto m = median(first, last);
      
      if(first != m) {
        auto l = median(first, m);
        //std::cout << "insert " << l->first << " at left of " << m->first << std::endl;
        t.emplace(*l);
      }
      if(m != last) {
        auto r = median(m, last);
        //std::cout << "insert " << r->first << " at right of " << m->first << std::endl;
        t.emplace(*r);
      }
      balance(first, m, t);
      balance(++m, last, t);
    }
  }

  val_t& operator[](const key_t& k) { return __subscript(k); }
  val_t& operator[](key_t&& k) { return __subscript(std::move(k)); }

  friend std::ostream& operator<<(std::ostream& os, const Tree& t) {
    for(const auto& v : t) os << v.first << " ";
    return os;
  }
};

template<typename iterator>
iterator median(iterator first, iterator last) {
  auto m = first;
  for(auto n = 2; n < std::distance(first, last); n += 2) ++m;
  return m;
}
