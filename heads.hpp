#include "hydras.hpp"

template<size_t N, typename T>
struct heads
{
  hydras<N,T>* children;

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

  auto mark()
  {
    children->mark();
  }

  auto unmark()
  {
    children->unmark();
  }

  auto clear_unmarked()
  {
    children->clear_unmarked();
  }

  heads(hydras<N,T>* n)
  : children(n)
  {}
};
