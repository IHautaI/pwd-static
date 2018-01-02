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
  // parens lang L = L(L) | e = (or (and L (and '(' (and L ')'))) empty )
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
  l.init();

  // auto s = l.queue.pop();
  // l.fork_to(s, 11);
  // l.queue.push(std::move(s));
  // std::cout << l.queue.head->next->value;

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
