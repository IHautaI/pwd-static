#include "hydra.hpp"
#include <iostream>

int main()
{
  auto root = hydra<int>();
  auto* x = push(&root, 5);
  auto* y = push(x, 6);
  auto* z = push(x, 7);

  std::cout << x->value
            << " "
            << y->value
            << " "
            << z->value
            << std::endl;

  std::cout << pop(y) // don't do X, it doesn't have a value (root)
            << " "
            << pop(z) << std::endl;
}
