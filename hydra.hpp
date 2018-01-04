#include "iter.hpp"
#include "heads.hpp"

template<typename T>
struct hydra
{
  heads<hydra<T>> children;
  hydra<T>* next;
  T value;

  std::unique_ptr<hydra<T>>& child(hydra<T>* that)
  {
    return children.child(that);
  }


  friend hydra<T>* push(hydra<T>* that, const T& t)
  {
    auto ptr = that->children.push(t);
    ptr->next = that;
    return ptr;
  }

  friend hydra<T>* push(hydra<T>& that, const T& t)
  {
    return push(&that, t);
  }

  friend hydra<T>* push_back(hydra<T>* that, const T& t)
  {
    auto* n = that;
    while( n->next->next != nullptr )
    {
      n = n->next;
    }

    auto mid = n->next.push(t);
    mid->push(std::move(n->next.child(n)));
  }

  friend hydra<T>* push_back(hydra<T>& that, const T& t)
  {
    return push_back(&that, t);
  }

  template<typename... Us>
  friend hydra<T>* emplace(hydra<T>* that, Us&&... us)
  {
    auto ptr = that->children.emplace(std::forward<Us>(us)...);
    ptr->next = that;
    return ptr;
  }

  friend T pop(hydra* that)
  {
    auto r = std::move(that->value);
    that->next->children.remove(that);
    return r;
  }

  hydra<T>* last()
  {
    auto n = this;
    while( n->next != nullptr )
    {
      n = n->next;
    }
  }

  hydra()
  : children()
  , next()
  , value()
  {}

  hydra(hydra&& o)
  : children(std::move(o.children))
  , next(o.next)
  , value(std::move(o.value))
  {}

  hydra& operator=(hydra&& o)
  {
    children = std::move(o.children);
    next = o.next;
    value = std::move(o.value);
    return *this;
  }


  hydra(const T& t)
  : children()
  , next()
  , value(t)
  {}

  hydra(T&& t)
  : children()
  , next()
  , value(std::move(t))
  {}

  ~hydra(){}
};
