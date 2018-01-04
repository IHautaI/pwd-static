#include <memory>

template<typename T>
struct heads
{
  std::unique_ptr<std::unique_ptr<T>[]> children;
  int size;
  int filled;

  heads()
  : children()
  , size(0)
  , filled(0)
  {}

  heads(heads&& o)
  : children(std::move(o.children))
  , size(o.size)
  , filled(o.filled)
  {}

  ~heads(){}

  void bigger()
  {
    int old = size;
    size = size ? 2 * size : 1;
    auto n = new std::unique_ptr<T>[size];
    int j = 0;

    for( auto i = 0; i < old; ++i )
    {
      if( children[i] != nullptr )
      {
        n[j] = std::move(children[i]);
        ++j;
      }
    }
    children.reset(n);
  }

  void smaller()
  {
    int old = size;
    size = size == 1 ? 1 : size / 2;
    auto n = new std::unique_ptr<T>[size];
    int j = 0;

    for( auto i = 0; i < old; ++i )
    {
      if( children[i] != nullptr )
      {
        n[j] = std::move(children[i]);
        ++j;
      }
    }

    children.reset(n);
  }

  template<typename... Us>
  T* emplace(Us&&... us)
  {
    ++filled;
    if( filled > size / 2 )
    {
      bigger();

      auto i = size > 1 ? size / 2 : 0;
      for( ; i < size; ++i )
      {
        if( children[i] == nullptr )
        {
          children[i] = std::make_unique<T>(std::forward<Us>(us)...);
          return children[i].get();
        }
      }
    } else
    {
      for(auto i = 0; i < size; ++i )
      {
        if( children[i] == nullptr )
        {
          children[i] = std::make_unique<T>(std::forward<Us>(us)...);
          return children[i].get();
        }
      }
    }
    return nullptr;
  }


  template<typename U>
  T* push(const U& u)
  {
    ++filled;
    if( filled >= size / 2 )
    {
      bigger();

      auto i = size > 1 ? size / 2 : 0;
      for( ; i < size; ++i )
      {
        if( children[i] == nullptr )
        {
          children[i] = std::make_unique<T>(u);
          return children[i].get();
        }
      }
    } else
    {
      for(auto i = 0; i < size; ++i )
      {
        if( children[i] == nullptr )
        {
          children[i] = std::make_unique<T>(u);
          return children[i].get();
        }
      }
    }
    return nullptr;
  }


  void remove(T* ele)
  {
    --filled;
    if( filled < size / 4 )
    {
      smaller();
    }

    for( auto i = 0; i < size; ++i )
    {
      if( children[i].get() == ele )
      {
        children[i].reset(nullptr);
      }
    }
  }
};


template<typename T>
struct hydra
{
  heads<hydra<T>> children;
  hydra<T>* next;
  T value;

  friend hydra<T>* push(hydra<T>* that, const T& w)
  {
    auto ptr = that->children.push(w);
    ptr->next = that;
    return ptr;
  }

  template<typename... Us>
  friend hydra<T>* emplace(hydra<T>* that, Us&&... us)
  {
    auto ptr = that->children.emplace(std::forward<Us>(us)...);
    ptr->next = that;
    return ptr;
  }

  friend hydra* pop(hydra* that)
  {
    auto n = that->next;
    n->children.remove(that);
    return n;
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

  // template<typename... Us>
  // hydra(Us&&... us)
  // : children()
  // , next()
  // , value(std::forward<Us>(us)...)
  // {}


  ~hydra(){}

};
