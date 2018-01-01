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
  stack_node<T>* end;

  bool empty()
  {
    return head == nullptr;
  }

  T pop()
  {
    auto t = std::move(head->value);
    head = head->next;
    return t;
  }

  T&
   front()
  {
    return head->value;
  }


  void push(T&& t)
  {
    auto n = std::make_shared<stack_node<T>>(std::move(t), head);
    head = n;
    if( head->next == nullptr )
    {
      end = head.get();
    }
  }


  void push_back(T&& t)
  {
      if( head == nullptr )
      {
        push(std::move(t));
      } else
      {
        end->next = std::make_shared<stack_node<T>>(std::move(t), nullptr);
        end = end->next.get();
      }
  }


  void push(const T& t)
  {
    auto n = std::make_shared<stack_node<T>>(t, head);
    head = n;
    if( head->next == nullptr )
    {
      end = head.get();
    }
  }


  void push_back(const T& t)
  {
      if( head == nullptr )
      {
        push(t);
      } else
      {
        end->next = std::make_shared<stack_node<T>>(t, nullptr);
        end = end->next.get();
      }
  }


  stack<T> fork()
  {
    return stack<T>(head, end);
  }

  stack()
  : head(nullptr)
  , end(nullptr)
  {}

  stack(std::shared_ptr<stack_node<T>>& head, stack_node<T>* end)
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
