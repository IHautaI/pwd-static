#include <memory>

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
  std::shared_ptr<stack_node<T>> end;
  stack_node<T>* last;


  bool empty()
  {
    return head == end;
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


  void push(T&& t)
  {
    head = std::make_shared<stack_node<T>>(std::move(t), head);
  }


  void push(const T& t)
  {
    head = std::make_shared<stack_node<T>>(t, head);
  }


  void push_back(T&& t)
  {
      if( head == end )
      {
        push(std::move(t));
        last = head.get();
      } else // splice
      {
        last->next = std::make_shared<stack_node<T>>(std::move(t), end);
        last = last->next.get();
      }
  }


  void push_back(const T& t)
  {
      if( head == end )
      {
        push(t);
        last = head.get();
      } else // splice
      {
        last->next = std::make_shared<stack_node<T>>(t, end);
        last = last->next.get();
      }
  }


  void clear()
  {
    head = end;
    last = end.get();
  }


  stack<T> fork()
  {
    return stack<T>(head, end, last);
  }


  stack()
  : head(new stack_node<T>(T(), nullptr))
  , end(head)
  , last(end.get())
  {}


  stack(std::shared_ptr<stack_node<T>>& head,
        std::shared_ptr<stack_node<T>>& end,
        stack_node<T>* last)
  : head(head)
  , end(end)
  , last(end.get())
  {}


  stack(stack&& o)
  : head(o.head)
  , end(o.end)
  , last(o.last)
  {}


  stack& operator=(stack&& o)
  {
    head = o.head;
    end = o.end;
    last = o.last;
    return *this;
  }

  ~stack(){}
};
