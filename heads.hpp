#include "iter.hpp"
#include "hydras.hpp"

template<typename T>
struct heads
{
  hydras<T>* children;

  void push(const T& t)
  {
    children = children->push(t);
  }

  T pop()
  {
    auto tmp = children->value;
    children = children->next;
    return tmp;
  }

  auto empty()
  {
    return children->next == nullptr;
  }

  auto value()
  {
    return children->value;
  }

  heads(hydras<T>* n)
  : children(n)
  {}
};
