#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
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

template<size_t size>
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
    void push(const T& t)
    {
      stk.push(t);
    }

    template<typename T>
    void push_back(const T& t)
    {
      stk.push_back(t);
    }

    int pop()
    {
      return stk.pop();
    }

    entry_t fork()
    {
      return entry_t(stk.fork(), tokens.fork());
    }

    entry_t fork_to(int i)
    {
      entry_t val(std::move(stk.fork()), std::move(tokens.fork()));
      val.stk.push(i);
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

    entry_t(stack<int>&& stk, stack<int>&& tokens)
    : stk(std::move(stk))
    , tokens(std::move(tokens))
    {}

    entry_t(entry_t&& o)
    : stk(std::move(o.stk))
    , tokens(std::move(o.tokens))
    {}

    entry_t& operator=(entry_t&& o)
    {
      stk = std::move(o.stk);
      tokens = std::move(o.tokens);
      return *this;
    }

    friend std::ostream& operator<<(std::ostream& out, entry_t& e)
    {
      for( auto it = e.stk.begin(); it != e.stk.end(); ++it )
      {
        std::cout << *it << " -> ";
      }
      std::cout << " | ";

      for( auto it = e.tokens.begin(); it != e.tokens.end(); ++it )
      {
        std::cout << *it << " -> ";
      }
      std::cout << std::endl;
      return out;
    }
  };

  int lang[size];
  bool nullable[size];
  stack<entry_t> queue;


  static int left(int i)
  {
    return 2 * i + 1;
  }

  static int right(int i)
  {
    return 2 * (i + 1);
  }


  void init()
  {
    queue.push(0);
    print(std::cout);

    auto prev = stack<std::pair<int, bool>>();
    for( auto i = 0; i < size; ++i )
    {
      nullable[i] = set_nullable(i, prev);
    }

    for( auto it = prev.begin(); it != prev.end(); ++it )
    {
      if( lang[it->first] == 4 )
      {
        nullable[it->first] = nullable[left(it->first)];
      }
    }
  }


  void reset()
  {
    queue.clear();
    queue.push(0);
  }


  void add_to_tails(const int t)
  {
    using s_ptr = stack<int>::stack_node<int>*;

    auto hits = std::vector<s_ptr>();
    std::vector<s_ptr>::iterator it;
    stack<int>::s_iter<int> i;

    for( auto n = queue.begin(); n != queue.end(); ++n )
    {
      if( n->tokens.empty() )
      {
        auto b = n->tokens.push(t);
        hits.push_back(b);
      } else
      {
        i = n->tokens.begin();
        while( i.next() != n->tokens.nil.get() )
        {
          ++i;
        }

        it = std::find(hits.begin(), hits.end(), i.node());
        if( it == hits.end() )
        {
          auto b = n->tokens.push_back(t);
          hits.push_back(b);
        }
      }
    }
  }


  std::ostream& print(std::ostream& out)
  {
    for( auto ele = queue.begin(); ele != queue.end(); ++ele )
    {
      out << *ele;
    }
    return out;
  }


  void apply(const int t)
  {
    if( queue.empty() )
    {
      return;
    }

    add_to_tails(t);
    std::cout << "\nqueue: ";
    print(std::cout);

    stack<entry_t> out;
    entry_t current;

    while( !queue.empty() )
    {
      current = queue.pop();
      search(current, out);
    }

    queue = std::move(out);
    std::cout << "\napplied " << t << ", remaining: "<< std::endl;
    print(std::cout);
    std::cout << "end remaining\n" << std::endl;
  }


  // searches from last node in entry until
  // either dead-end, nonterminal reached
  void search(entry_t& entry, stack<entry_t>& out)
  {
    // std::cout << "\nsearching " << entry;
    if( entry.empty() )
    {
      if( entry.tokens.empty() ) // add to out, could be exit,
                                 // else cleaned up w/ next token
      {
        out.push(std::move(entry));
      }

      return;
    }

    auto index = entry.pop();

    switch( lang[index] )
    {
      case 0: // EMPTY - add self to queue, move to next item
        out.push( entry.fork() );

        if( !entry.tokens.empty() )
        {
          search(entry, out);         // consume empty str
        }
        break;

      case 1: // OR - add second side to current queue, continue search on left side
        queue.push(std::move(entry.fork_to( right(index) )));

        entry.push( left(index) );
        search(entry, out);
        break;

      case 2: // AND -add right and search left, if nullable add right as
              // second search w/o left
        if( nullable[left(index)] )
        {
          queue.push(std::move(entry.fork_to( right(index) )));
        }

        entry.push( right(index) );
        entry.push( left(index) );
        search(entry, out);
        break;

      case 3: // STAR - search contained node, then STAR again
        entry.push( index );
        entry.push( left(index) );
        search(entry, out);
        break;

      case 4: // NONTERM - add pointed-to index, add to output for next round
        entry.push( lang[left(index)] );
        out.push(std::move(entry));
        break;

      default: // TOKENS - check entry's token, if front
               // matches, pop token, search rest
        if( entry.tokens.empty() ) // waiting for token... put back
        {
          entry.push( index );
          out.push( std::move(entry) );
        } else
        if( entry.tokens.front() == lang[index] )
        {
          entry.tokens.pop();
          search(entry, out);
        }
        break;
    }
  }


  bool set_nullable(int i, stack<std::pair<int, bool>>& prev)
  {
    for( auto it = prev.begin(); it != prev.end(); ++it )
    {
      if( it->first == i )
      {
        return it->second;
      }
    }

    switch( lang[i] )
    {
      case 0: // EMPTY
        prev.push(std::make_pair(i, true));
        break;

      case 1: // OR
        prev.push(std::make_pair(i,
                    set_nullable(left(i), prev) || set_nullable(right(i), prev)));
        break;

      case 2: // AND
        prev.push(std::make_pair(i,
                    set_nullable(left(i), prev) && set_nullable(right(i), prev)));
        break;

      case 3: // STAR
        prev.push(std::make_pair(i, true));
        break;

      case 4: // Nonterminal
        prev.push(std::make_pair(i, false));
        break;

      default: // tokens
        prev.push(std::make_pair(i, false));
        break;
    }

    return prev.front().second;
  }


  bool accept(entry_t& e)
  {
    std::cout << e;

    if( e.tokens.empty() )
    {
      for( auto it = e.stk.begin(); it != e.stk.end(); ++it )
      {
        if( !nullable[*it] ) // requires tokens to complete
        {
          return false;
        }
      }

      return true;
    } else
    {
      while( !e.stk.empty() &&  nullable[e.stk.front()] )  // remove nullable entries and search
      {
        e.pop();
      }

      stack<entry_t> out;
      search(e, out);

      return false;
    }
  }


  bool accepted()
  {
    for( auto it = queue.begin(); it != queue.end(); ++it )
    {
      if( it->empty() && it->tokens.empty() )
      {
        return true;
      }
    }

    for( auto it = queue.begin(); it != queue.end(); ++it )
    {
      if( accept(*it) )
      {
        return true;
      }
    }

    return false;
  }
};
