#include "parsing.hpp"

#include <iostream>
#include <string>

int map(char c)
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
  // parens lang L = L(L) | e = (or (and L (and '(' (and L ')'))) empty )
  // ->
  // 1 | 2 0 | 4 2 0 0 | 0 0 5 2 0 0 0 0 |
  // 0 0 0 0 0 0 4 6 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 0 0 0 0 0 ...

  Lang<49> l{                         1,
                             2,                       0,
                     4,             2,            0,      0,
                 0,      0,      5,      2,     0,  0,  0,  0,
               0,  0,  0,  0,  0,  0,  4,  6,  0,0,0,0,0,0,0,0,
              0,0,0,0,0,0,0,0,0,0,0,0,0};

  l.init();

  std::string line;
  while( getline(std::cin, line) )
  {
    if( line.empty() )
    {
      break;
    }

    for( auto& c : line )
    {
      l.apply(map(c));
    }
    std::cout << "Accepted? " << l.accepted() << "\n" << std::endl;
    l.reset();
  }
}
