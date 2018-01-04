
template<typename T, typename U>
struct iter
{
  using value_type = U;
  using difference_type = std::ptrdiff_t;
  using pointer = U*;
  using reference = U&;
  using iterator_category = std::forward_iterator_tag;

  T* ptr;

  iter() = default;

  iter(T* ptr)
  : ptr(ptr)
  {}

  iter<T,U>& operator++()
  {
    ptr = ptr->next;
    return *this;
  }

  iter<T,U> operator++(int)
  {
    auto tmp = *this;
    ptr = ptr->next;
    return tmp;
  }

  bool operator==(const iter& o)
  {
    return ptr == o.ptr;
  }

  bool operator!=(const iter& o)
  {
    return ptr != o.ptr;
  }

  reference operator*()
  {
    return ptr->value;
  }

  pointer operator->()
  {
    return &(ptr->value);
  }
};
