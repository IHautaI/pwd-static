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


  bool empty()
  {
    return head == end;
  }


  T pop()
  {
    auto t = std::move(head->value);
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
      } else // splice
      {
        auto n = head;
        while( n->next != end )
        {
          n = n->next;
        }
        n->next = std::make_shared<stack_node<T>>(std::move(t), end);
      }
  }


  void push_back(const T& t)
  {
      if( head == end )
      {
        push(t);
      } else // splice
      {
        auto n = head;
        while( n->next != end )
        {
          n = n->next;
        }
        n->next = std::make_shared<stack_node<T>>(t, end);
      }
  }


  void clear()
  {
    head = end;
  }


  stack<T> fork()
  {
    return stack<T>(head, end);
  }


  stack()
  : head(new stack_node<T>(T(), nullptr))
  , end(head)
  {}


  stack(std::shared_ptr<stack_node<T>>& head, std::shared_ptr<stack_node<T>>& end)
  : head(head)
  , end(end)
  {}


  stack(stack&& o)
  : head(o.head)
  , end(o.end)
  {}


  stack& operator=(stack&& o)
  {
    head = o.head;
    end = o.end;
    return *this;
  }

  ~stack(){}
};
