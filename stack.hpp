#include <memory>
#include <iterator>

template<typename T>
struct stack
{
  template<typename U>
  struct stack_node
  {
    U value;
    std::shared_ptr<stack_node<U>> next;

    stack_node(U&& v, std::shared_ptr<stack_node<U>> n)
    : value(std::move(v))
    , next(n)
    {}

    stack_node(const U& v, std::shared_ptr<stack_node<U>> n)
    : value(v)
    , next(n)
    {}

    ~stack_node(){}
  };


  std::shared_ptr<stack_node<T>> head;
  std::shared_ptr<stack_node<T>> nil;
  stack_node<T>* last;


  bool empty()
  {
    return head == nil;
  }


  T pop()
  {
    auto t = std::move(head->value);
    if( last == head.get() )
    {
      last = head->next.get();
    }

    head = head->next;
    return t;
  }


  T& front()
  {
    return head->value;
  }


  stack_node<T>* push(T&& t)
  {
    head = std::make_shared<stack_node<T>>(std::move(t), head);
    if( last == nil.get() )
    {
      last = head.get();
    }
    return head.get();
  }


  stack_node<T>* push(const T& t)
  {
    head = std::make_shared<stack_node<T>>(t, head);
    if( last == nil.get() )
    {
      last = head.get();
    }
    return head.get();
  }


  stack_node<T>* push_back(T&& t)
  {
    if( head == nil )
    {
      push(std::move(t));
      last = head.get();
    } else // splice
    {
      last->next = std::make_shared<stack_node<T>>(std::move(t), nil);
      last = last->next.get();
    }
    return last;
  }


  stack_node<T>* push_back(const T& t)
  {
    if( head == nil )
    {
      push(t);
      last = head.get();
    } else // splice
    {
      last->next = std::make_shared<stack_node<T>>(t, nil);
      last = last->next.get();
    }
    return last;
  }


  void clear()
  {
    head = nil;
    last = nil.get();
  }


  stack<T> fork()
  {
    return stack<T>(head, nil, last);
  }


  stack()
  : head(new stack_node<T>(T(), nullptr))
  , nil(head)
  , last(head.get())
  {}


  stack(std::shared_ptr<stack_node<T>>& head,
        std::shared_ptr<stack_node<T>>& nil,
        stack_node<T>* last)
  : head(head)
  , nil(nil)
  , last(last)
  {}


  stack(stack&& o)
  : head(o.head)
  , nil(o.nil)
  , last(o.last)
  {}


  stack& operator=(stack&& o)
  {
    head = o.head;
    nil = o.nil;
    last = o.last;
    return *this;
  }

  ~stack(){}

  template<typename U>
  struct s_iter
  {
    using value_type = U;
    using difference_type = ptrdiff_t;
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
