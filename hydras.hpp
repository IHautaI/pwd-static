#include <memory>

/*
*  requires default-constructible value type T
*/
template<size_t N, typename T>
struct hydras
{
  using hydra = hydras<N, T>;

  std::unique_ptr<std::unique_ptr<hydra>[]> children;
  hydra* next;
  int size;
  int filled;
  T value;
  bool marked;

  hydras()
  : children(new std::unique_ptr<hydra>[N])
  , next(nullptr)
  , size(N)
  , filled(N)
  , value()
  , marked(false)
  {}

  hydras(const T& t)
  : children(new std::unique_ptr<hydra>[N])
  , next(nullptr)
  , size(N)
  , filled(N)
  , value(t)
  , marked(false)
  {}

  ~hydras(){}


  // marks node and all below
  void mark()
  {
    if( !marked )
    {
      marked = true;
      if( next != nullptr )
      {
        next->mark();
      }
    }
  }

  // unmarks node and any below
  void unmark()
  {
    if( marked )
    {
      this->marked = false;
      if( next != nullptr )
      {
        next->unmark();
      }
    }
  }

  // only clears immediate children and internal unmarked nodes
  void clear_unmarked()
  {
    if( !empty() )
    {
      for( auto i = 0; i < size; ++i )
      {
        if( children[i] != nullptr )
        {
          if( !children[i]->marked )
          {
            children[i].reset(nullptr);
          } // else   // clear internal unmarked nodes?  might not need
          //  {
          //    children[i]->clear_unmarked();
          //  }
        }
      }
    }
  }

  void bigger()
  {
    int old = size;
    size = size ? 2 * size : 1;
    auto n = new std::unique_ptr<hydra>[size];
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
    auto n = new std::unique_ptr<hydra>[size];
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


  template<typename U>
  hydra* push(const U& u)
  {
    auto* n = child(u);
    if( n != nullptr )
    {
      return n;
    } else
    {
      auto ptr = std::make_unique<hydra>(u);
      return push(ptr);
    }
  }


  hydra* push(std::unique_ptr<hydra>& n)
  {
    n->next = this;

    ++filled;
    if( filled >= size / 2 )
    {
      bigger();

      auto i = size > 1 ? size / 2 : 0;
      for( ; i < size; ++i )
      {
        if( children[i] == nullptr )
        {
          children[i] = std::move(n);
          return children[i].get();
        }
      }
    } else
    {
      for(auto i = 0; i < size; ++i )
      {
        if( children[i] == nullptr )
        {
          children[i] = std::move(n);
          return children[i].get();
        }
      }
    }

    return nullptr;
  }


  bool empty()
  {
    return size == 0 || filled == 0;
  }


  void remove(hydra* ele)
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


  /*
  * search children by ptr
  */
  hydra* child(hydra* that)
  {
    for( auto i = 0; i < size; ++i )
    {
      if( children[i] != nullptr && children[i] == that )
      {
        return children[i];
      }
    }

    return nullptr;
  }


  /*
  *  search children by value held
  */
  hydra* child(const T& val)
  {
    if( size > 0 )
    {
      for( auto i = 0; i < size; ++i )
      {
        if( children[i] != nullptr && children[i]->value == val )
        {
          return children[i].get();
        }
      }
    }
    return nullptr;
  }


  void splice(const T& t)
  {
    if( size == 0 )
    {
      return;
    }

    for( auto i = 0; i < size; ++i )
    {
      if( children[i] != nullptr )
      {
        auto tmp = std::move(children[i]);

        children[i] = std::make_unique<hydra>(t);
        children[i]->next = this;

        children[i]->push(tmp);
      }
    }
  }

  auto full_size()
  {
    if( size == 0 || filled == 0 )
    {
      return 0;
    }

    auto s = filled;
    for( auto i = 0; i < size; ++i )
    {
      if( children[i] != nullptr )
      {
        s += children[i]->full_size();
      }
    }
    return s;
  }
};
