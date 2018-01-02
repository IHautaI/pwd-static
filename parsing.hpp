#include <cstring>
#include <iostream>
#include "stack.hpp"

/*
*  Types:
*  0  EMPTY
*  1  OR
*  2  AND
*  3  STAR
*  4  NONTERMINAL
*  5+ TOKENS
*/

// Just doing recognizer here
// add full parsing later

struct Lang
{
  struct entry_t
  {
    stack<int> stk;
    stack<int> tokens;

    bool empty()
    {
      return stk.empty();
    }

    template<typename T>
    auto push(const T& t)
    {
      stk.push(t);
    }

    template<typename T>
    auto push_back(const T& t)
    {
      stk.push_back(t);
    }

    auto pop()
    {
      return stk.pop();
    }

    entry_t fork()
    {
      auto val = entry_t();
      val.stk = stk.fork();
      val.tokens = tokens.fork();
      return val;
    }

    entry_t(int i)
    : stk()
    , tokens()
    {
      stk.push(i);
    }

    entry_t()
    : stk()
    , tokens()
    {}

    friend std::ostream& operator<<(std::ostream& out, entry_t& e)
    {
      auto n = e.fork();
      auto& t = n.tokens;
      auto& s = n.stk;

      while( !s.empty() )
      {
        std::cout << s.pop() << " -> ";
      }
      std::cout << " | ";

      while( !t.empty() )
      {
        std::cout << t.pop() << " -> ";
      }
      std::cout << std::endl;
      return out;
    }
  };

  std::unique_ptr<int[]> lang;
  std::unique_ptr<bool[]> nullable;
  int size;
  stack<entry_t> queue;

  Lang()
  : lang(nullptr)
  , nullable(nullptr)
  , size(0)
  , queue()
  {
    queue.push(0);
  }

  template<int N>
  Lang(int (&inp)[N])
  : lang(new int[N])
  , nullable(new bool[N])
  , size(N)
  , queue()
  {
    queue.push(0);
    std::memcpy(lang.get(), &inp[0], sizeof(int) * N);
  }


  static int left(int i)
  {
    return 2 * i + 1;
  }

  static int right(int i)
  {
    return 2 * (i + 1);
  }


  auto init()
  {
    for( auto i = 0; i < size; ++i )
    {
      set_nullable(i);
    }
  }


  auto reset()
  {
    queue.clear();
    queue.push(0);
  }


  auto apply(const int t)
  {
    std::cout << "applying " << t << std::endl;
    if( queue.empty() )
    {
      return;
    }

    queue.front().tokens.push_back(t);

    auto s = queue.head->next;
    while( s != nullptr )
    {
      s->value.tokens.push_back(t);
      s = s->next;
    }

    stack<entry_t> out;
    entry_t current;

    while( !queue.empty() )
    {
      current = queue.pop();
      // std::cout << "searching:\n"
      //           << current
      //           << std::endl;
      search(current, out);
    }

    queue = std::move(out);

    auto c = queue.fork();
    // std::cout << "out" << std::endl;
    while( !c.empty() )
    {
      current = c.pop();
      // std::cout << current;
      // std::cout << std::endl;
    }
  }


  void fork_to(entry_t& entry, int to)
  {
    auto second = entry.fork();
    second.push(to);
    queue.push_back(std::move(second));
  }


  // searches from last node in entry until
  // either dead-end, nonterminal reached
  void search(entry_t& entry, stack<entry_t>& out)
  {
    if( entry.empty() )
    {
      if( entry.tokens.empty() ) // add to out, could be exit, else cleaned up next pass
      {
        out.push(std::move(entry));
      }

      return;
    }

    auto index = entry.pop();

    switch( lang[index] )
    {
      case 0: // EMPTY - end? good else dead
        if( entry.tokens.empty() )
        {
          // entry.push(index);
          out.push(std::move(entry));
        }

        return;
        break;

      case 1: // OR - ADD SECOND SIDE TO CURRENT QUEUE, CONTINUE SEARCH WITH THIS ONE
        fork_to(entry, right(index));
        entry.push(left(index));
        search(entry, out);
        break;

      case 2: // AND - ADD RIGHT, SEARCH LEFT, if nullable add right as
              // second search w/o left
        if( nullable[left(index)] )
        {
          fork_to(entry, right(index));
        }

        entry.push(right(index));
        entry.push(left(index));
        search(entry, out);
        break;

      case 3: // STAR - search contained node, then STAR again
        entry.push(index);
        entry.push(left(index));
        search(entry, out);
        break;

      case 4: // NONTERM - add pointed-to index, add to output for next round
        entry.push( lang[left(index)] );
        out.push(std::move(entry));
        break;

      default: // TOKENS - check entry's token, if front
               // matches, pop token, search entry
        if( entry.tokens.empty() ) // waiting for token... put back
        {
          entry.push(index);
          out.push(std::move(entry));
        } else if( entry.tokens.front() == lang[index] )
        {
          entry.tokens.pop();
          search(entry, out);
        }
        break;
    }
  }


  bool nonterm_nullable(int i, stack<std::pair<int, bool>>& prev)
  {
    // std::cout << "nonterm nullable: " << i << std::endl;
    prev.push(std::make_pair(i, false));
    auto* s = prev.head.get();
    s->value.second = set_nullable(left(i), prev);
    // std::cout << "nonterm " << i << " " << s->value.second << std::endl;
    return s->value.second;
  }


  bool set_nullable(int i, stack<std::pair<int, bool>>& prev)
  {
    auto* next = prev.head.get();

    switch( lang[i] )
    {
      case 0: // EMPTY
        return true;
        break;

      case 1: // OR
        return set_nullable(left(i), prev) || set_nullable(right(i), prev);
        break;

      case 2: // AND
        return set_nullable(left(i), prev) && set_nullable(right(i), prev);
        break;

      case 3: // STAR
        return true;
        break;

      case 4: // Nonterminal
        // check if we've visited this nonterm, return value if we have
        while( next != nullptr )
        {
          if( next->value.first == i )
          {
            return next->value.second;
          }
          next = next->next.get();
        }

        // else add as false, then follow it to see if that's true
        return nonterm_nullable(i, prev);
        break;

      default: // tokens
        return false;
        break;
    }
  }


  void set_nullable(int i)
  {
    stack<std::pair<int, bool>> prev;
    nullable[i] = set_nullable(i, prev);
  }


  bool accept(entry_t& e)
  {
    if( e.tokens.empty() )
    {
      while( !e.stk.empty() )
      {
        if( !nullable[e.stk.pop()] )
        {
          return false;
        }
      }
      return true;
    } else
    {
      while( !e.stk.empty() )  // remove nullable entries, try to use tokens ?
      {
        int start = e.stk.pop();
        int s = start;
        while( nullable[s] )
        {
          s = e.stk.pop();
        }

        if( s != start && !nullable[s] )
        {
          e.push(s);
          search(e, queue);
        } else
        {
          break;
        }
      }
      return false;
    }
  }


  bool accepted()
  {
    for( auto i = 0; i < size; ++i )
    {
      // std::cout << "(" << i << "," << nullable[i] << "), ";
    }
    std::cout << std::endl;

    stack<entry_t> out;
    entry_t current;

    while( !queue.empty() )
    {
      current = queue.pop();
      // std::cout << current;

      if( accept(current) )
      {
        return true;
      }
    }

    return false;
  }
};
