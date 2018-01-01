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
  };

  std::unique_ptr<int[]> lang;
  int size;
  stack<entry_t> queue;

  Lang()
  : lang(nullptr)
  , size(0)
  , queue()
  {
    queue.push(0);
  }

  template<int N>
  Lang(int (&inp)[N])
  : lang(new int[N])
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


  auto apply(const int t)
  {
    if( queue.empty() )
    {
      return;
    }

    queue.front().tokens.push_back(t);

    stack<entry_t> out;
    entry_t current;

    while( !queue.empty() )
    {
      current = queue.pop();
      search(current, out);
    }
    queue = std::move(out);
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

    std::cout << "searching " << index << " type " << lang[index] << std::endl;

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
        entry.push(right(index));
        if( nullable(left(index)) )
        {
          queue.push(entry.fork());
        }

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

      default: // TOKENS - check entry's fork of the tokens, if front
               // matches, pop tokens, search entry
               // else dead
        if( !entry.tokens.empty() && entry.tokens.front() == lang[index] )
        {
          entry.tokens.pop();
          search(entry, out);
        } else
        {
          return;
        }
        break;
    }
  }


  bool nonterm_nullable(int i, stack<std::pair<int, bool>>& prev)
  {
    prev.push(std::make_pair(i, false));
    auto* s = prev.head.get();
    s->value.second = nullable(lang[left(i)], prev);
    return s->value.second;
  }


  // can be precomputed...
  bool nullable(int i, stack<std::pair<int, bool>>& prev)
  {
    auto* next = prev.head.get();

    switch( lang[i] )
    {
      case 0: // EMPTY
        return true;
        break;

      case 1: // OR
        return nullable(left(i), prev) || nullable(right(i), prev);
        break;

      case 2: // AND
        return nullable(left(i), prev) && nullable(right(i), prev);
        break;

      case 3: // STAR
        return true;
        break;

      case 4: // Nonterminal
        while( next != nullptr )
        {
          if( next->value.first == i )
          {
            return next->value.second;
          }
          next = next->next.get();
        }

        return nonterm_nullable(i, prev);
        break;

      default:
        return false;
        break;
    }
  }


  bool nullable(int i)
  {
    stack<std::pair<int, bool>> prev;
    return nullable(i, prev);
  }


  bool accept(entry_t& entry)
  {
    return entry.tokens.empty() && ( entry.stk.empty() || nullable(entry.stk.front() ) );
  }


  bool accepted()
  {
    stack<entry_t> out;
    entry_t current;

    while( !queue.empty() )
    {
      std::cout << "queue pop" << std::endl;
      current = queue.pop();

      if( accept(current) )
      {
        return true;
      }

      search(current, out);
    }

    while( !out.empty() )
    {
      std::cout << "out pop" << std::endl;
      current = out.pop();

      if( accept(current) )
      {
        return true;
      }
    }

    return false;
  }
};
