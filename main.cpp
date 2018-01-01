#include "parsing.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
  // parens lang L = L(L) | e
  // ->
  // 1 | 2 0 | 4 2 0 0 | 0 0 5 2 0 0 0 0 |
  // 0 0 0 0 0 0 4 6 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 ...

  int parens[45] = {                       1,
                             2,                       0,
                     4,             2,            0,      0,
                 0,      0,      5,      2,     0,  0,  0,  0,
               0,  0,  0,  0,  0,  0,  4,  6,  0,0,0,0,0,0,0,0,
              0,0,0,0,0,0,0,0,0,0,0,0,0};

  auto l = Lang(parens);

  l.apply(5);
  l.apply(6);

  std::cout << "Accepted? \n" << l.accepted() << std::endl;
}
