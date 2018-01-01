#include "parsing.hpp"

#include <iostream>
#include <string>

auto map(char c)
{
  if( c == '(' )
  {
    return 5;
  } else if ( c == ')' )
  {
    return 6;
  } else
  {
    exit(1);
  }
}

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


  auto line = std::string();
  while( getline(std::cin, line) )
  {
    for( auto& c : line )
    {
      l.apply(map(c));
    }
    std::cout << "Accepted? \n" << l.accepted() << std::endl;
    l.reset();
  }
}
