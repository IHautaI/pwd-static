#include <memory>
#include <cstddef>
#include <iterator>

template<typename T>
struct stack
{
  template<typename U>
  struct stack_node
  {
    U value;
    std::shared_ptr<stack_node<U>> next;

    stack_node(const U& v)
    : value(v)
    , next(nullptr)
    {}

    stack_node(U&& v)
    : value(std::move(v))
    , next(nullptr)
    {}

    stack_node(U&& v, const std::shared_ptr<stack_node<U>>& n)
    : value(std::move(v))
    , next(n)
    {}

    stack_node(const U& v, const std::shared_ptr<stack_node<U>>& n)
    : value(v)
    , next(n)
    {}

    ~stack_node()
    {
      if( next.get() == this )
      {
        auto t = std::shared_ptr<stack_node<U>>();
        next.swap(t);
      }
    }
  };


  std::shared_ptr<stack_node<T>> head;
  std::shared_ptr<stack_node<T>> nil;


  bool empty()
  {
    return head == nil;
  }


  T pop()
  {
    if( head != nil )
    {
      auto t = std::move(head->value);
      head = head->next;
      return t;
    }

    return std::move(head->value);
  }


  T& front()
  {
    return head->value;
  }


  std::shared_ptr<stack_node<T>> last()
  {
    auto n = head;
    while( n->next != nil )
    {
      n = n->next;
    }
    return n;
  }


  void push(T&& t)
  {
    auto tmp = head;
    head = std::make_shared<stack_node<T>>(std::forward<T>(t));
    head->next = tmp;
  }


  void push(const T& t)
  {
    auto tmp = head;
    head = std::make_shared<stack_node<T>>(t);
    head->next = tmp;
  }


  stack_node<T>* push_back(T&& t)
  {
    if( head == nil )
    {
      push(std::move(t));
      return nullptr;
    } else // splice
    {
      auto n = head;
      while( n->next != nil )
      {
        n = n->next;
      }
      auto tmp = n->next;
      n->next = std::make_shared<stack_node<T>>(std::move(t));
      n->next->next = tmp;
      return n->next.get();
    }
  }


  stack_node<T>* push_back(const T& t)
  {
    if( head == nil )
    {
      push(t);
      return nullptr;
    } else // splice
    {
      auto n = head;
      while( n->next != nil )
      {
        n = n->next;
      }
      auto tmp = n->next;
      n->next = std::make_shared<stack_node<T>>(t);
      n->next->next = tmp;
      return n->next.get();
    }
  }


  void clear()
  {
    head = nil;
  }


  stack<T> fork()
  {
    return stack<T>(head, nil);
  }


  stack(const std::shared_ptr<stack_node<T>>& nil)
  : head(nil)
  , nil(nil)
  {}


  stack(const std::shared_ptr<stack_node<T>>& head,
        const std::shared_ptr<stack_node<T>>& nil)
  : head(head)
  , nil(nil)
  {}


  stack(stack&& o)
  : head(std::move(o.head))
  , nil(std::move(o.nil))
  {}


  stack& operator=(stack&& o)
  {
    head = std::move(o.head);
    nil = std::move(o.nil);
    return *this;
  }

  ~stack(){}

  template<typename U>
  struct s_iter
  {
    using value_type = U;
    using difference_type = std::ptrdiff_t;
    using pointer = U*;
    using reference = U&;
    using iterator_category = std::forward_iterator_tag;

    stack_node<T>* s;

    s_iter() = default;

    s_iter(std::shared_ptr<stack_node<U>>& s)
    : s(s.get())
    {}


    s_iter<U> operator+(int i)
    {
      auto it = s_iter<U>(s);
      for( auto j = 0; j < i; ++j )
      {
        ++it;
      }

      return it;
    }


    s_iter<U>& operator++()
    {
      s = s->next.get();
      return *this;
    }

    s_iter<U> operator++(int)
    {
      auto tmp = *this;
      s = s->next;
      return tmp;
    }

    bool operator==(const s_iter& o)
    {
      return s == o.s;
    }

    bool operator!=(const s_iter& o)
    {
      return s != o.s;
    }

    reference operator*()
    {
      return s->value;
    }

    pointer operator->()
    {
      return &(s->value);
    }

    stack_node<U>* node()
    {
      return s;
    }

    stack_node<U>* next()
    {
      return s->next.get();
    }
  };

  s_iter<T> begin()
  {
    return s_iter<T>(head);
  }

  s_iter<T> end()
  {
    return s_iter<T>(nil);
  }
};

template<typename T>
using s_node = typename stack<T>:: template stack_node<T>;

template<typename T>
using nil_t = std::shared_ptr<s_node<T>>;

template<typename T, typename... Us>
nil_t<T> make_nil(Us&&... us)
{
  return std::make_shared<s_node<T>>(T(us...), std::shared_ptr<s_node<T>>());
}
