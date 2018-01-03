#include "parsing.hpp"

#include <iostream>
#include <string>

int map(char c)
{
  if( c == '1' )
  {
    return 5;
  } else if ( c == '+' )
  {
    return 6;
  } else
  {
    exit(1);
  }
}

int main(int argc, char* argv[])
{
  // plus lang S = S + S | 1 = (or (and S (and '+' S)) '1' )
  // ->
  // 1 | 2 5 | 4 2 0 0 | 0 0 6 4 0 0 0 0 | 0 0 0 0 0 0 0

  Lang<22> l{                                1,
                             2,                       5,
                     4,             2,            0,      0,
                 0,      0,      6,      4,     0,  0,  0,  0,
               0,  0,  0,  0,  0,  0,  0};

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
    l.print(std::cout);
    std::cout << "Accepted? " << l.accepted() << "\n" << std::endl;
    l.reset();
  }
}
