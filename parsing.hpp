#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <deque>

#include "heads.hpp"

using hydra = hydras<int>;
using head = heads<int>;

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
    head stack;
    head tokens;

    friend std::ostream& operator<<(std::ostream& out, entry_t& e)
    {
      auto s = e.stack;
      while( !s.empty() )
      {
        std::cout << s.value() << " -> ";
        s.pop();
      }
      std::cout << " | ";

      s = e.tokens;
      while( !s.empty() )
      {
        std::cout << s.value() << " -> ";
        s.pop();
      }
      std::cout << std::endl;
      return out;
    }

    auto fork_to(const int i)
    {
      auto e = entry_t{stack, tokens};
      e.stack.push(i);
      return e;
    }
  };
  using queue_t = std::deque<entry_t>;


  // grouped for simple bracket-initialization
  int lang[size];
  std::unique_ptr<hydra> stack_nil;
  std::unique_ptr<hydra> token_nil;

  queue_t queue;
  bool nullable[size];



  static int left(const int i)
  {
    return 2 * i + 1;
  }

  static int right(const int i)
  {
    return 2 * (i + 1);
  }


  void init()
  {
    auto start = head(stack_nil.get());
    start.push(0);
    queue.push_front(entry_t{start, token_nil.get()});

    auto prev = std::vector<std::pair<int, bool>>();
    set_nullable(0, prev);

    // std::cout <<"nullability: \n";
    for( auto& x : prev )
    {
      if( lang[x.first] != 4 )
      {
      nullable[x.first] = x.second;
      }
    }

    for( auto& x : prev )
    {
      if( lang[x.first] == 4 ) // nonterm
      {
        nullable[x.first] = nullable[lang[left(x.first)]];
      }
      // std::cout << "(" << it->first << "," << nullable[it->first] << ")";
    }
    // std::cout << std::endl;
  }


  void reset()
  {
    queue.clear();
    stack_nil.reset(new hydra);
    token_nil.reset(new hydra);
    auto start = head(stack_nil.get());
    start.push(0);
    queue.push_front(entry_t{start, token_nil.get()});
  }


  std::ostream& print(std::ostream& out)
  {
    for( auto ele = queue.begin(); ele != queue.end(); ++ele )
    {
      out << *ele;
    }
    return out;
  }


  void clean()
  {
    for( auto& x : queue )
    {
      x.stack.mark();
    }

    for( auto& x : queue )
    {
      x.stack.clear_unmarked();
      x.stack.unmark();
    }

    stack_nil->clear_unmarked();
    stack_nil->unmark();
  }


  void add_token(const int t)
  {
    if( !token_nil->empty() )
    {
      token_nil->splice(t);
    }

    for( auto& x : queue )
    {
      if( x.tokens.empty() )
      {
        x.tokens.push(t);
      }
    }
  }


  void apply(const int t)
  {
    if( queue.empty() )
    {
      return;
    }

    add_token(t);

    // std::cout << "\nqueue: ";
    // print(std::cout);

    auto out = queue_t();

    while( !queue.empty() )
    {
      search(queue.front(), out);
      queue.pop_front();
    }

    queue = std::move(out);
    // std::cout << "\napplied " << t << ", remaining: "<< std::endl;
    // print(std::cout);
    // std::cout << "end remaining\n" << std::endl;

    clean();

    std::cout << "size: " << stack_nil->full_size() << std::endl;
  }


  // searches from last node in entry until
  // either dead-end, nonterminal reached
  void search(entry_t& entry, queue_t& out)
  {
    // std::cout << "searching: " << entry << std::endl;

    if( entry.stack.empty() )
    {
      if( entry.tokens.empty() ) // add to out, could be exit,
                                 // else cleaned up w/ next token
      {
        // std::cout << "added empty \n";
        out.push_back(entry);
      }

      return;
    }

    auto index = entry.stack.value();
    entry.stack.pop();

    switch( lang[index] )
    {
      case 0: // EMPTY - if tokens empty, wait else consume empty and go
        if( entry.tokens.empty() )
        {
          out.push_back( entry );
        } else
        {
          queue.push_back(entry);
          // search(entry, out);         // consume empty str
        }
        break;

      case 1: // OR - add second side to current queue, continue search on left side
        queue.push_back( entry.fork_to( right(index) ) );
        // std::cout << "fork to " << right(index) << std::endl;

        entry.stack.push( left(index) );

        queue.push_back( entry );

        // search(entry, out);
        break;

      case 2: // AND -add right and search left, if nullable add right as
              // second search w/o left
        if( nullable[left(index)] )
        {
          queue.push_back( entry.fork_to( right(index) ) );
          // std::cout << "fork to " << right(index) << std::endl;
        }

        entry.stack.push( right(index) );
        entry.stack.push( left(index) );

        queue.push_back( entry );

        // search(entry, out);
        break;

      case 3: // STAR - search contained node, then STAR again
        entry.stack.push( index );
        entry.stack.push( left(index) );

        queue.push_back( entry );

        // search(entry, out);
        break;

      case 4: // NONTERM - add pointed-to index, add to output for next round
        entry.stack.push( lang[left(index)] );

        out.push_back( entry );
        break;

      default: // TOKENS - check entry's token, if front
               // matches, pop token, search rest
        if( entry.tokens.empty() ) // waiting for token... put back
        {
          entry.stack.push( index );

          out.push_back(entry);
          // std::cout << "added " << out.front();
        } else if( entry.tokens.value() == lang[index] )
        {
          entry.tokens.pop();

          queue.push_back(entry);
          // search(entry, out);
        }
        break;
    }
  }


  bool set_nullable(int i, std::vector<std::pair<int, bool>>& prev)
  {
    for( auto& x : prev )
    {
      if( x.first == i )
      {
        return x.second;
      }
    }

    bool l, r;

    switch( lang[i] )
    {
      case 0: // EMPTY
        prev.emplace_back(i, true);
        break;

      case 1: // OR
        l = set_nullable(left(i), prev);
        r = set_nullable(right(i), prev);

        prev.emplace_back(i, l || r);
        break;

      case 2: // AND
        l = set_nullable(left(i), prev);
        r = set_nullable(right(i), prev);

        prev.emplace_back(i, l && r);
        break;

      case 3: // STAR
        prev.emplace_back(i, true);
        break;

      case 4: // Nonterminal
        prev.emplace_back(i, false);
        break;

      default: // tokens
        prev.emplace_back(i, false);
        break;
    }

    return prev.back().second;
  }


  bool accept(entry_t& e)
  {
    if( e.tokens.empty() )
    {
      auto s = e.stack;
      while( !s.empty() )
      {
        if( !nullable[s.value()] ) // requires tokens to complete
        {
          return false;
        }
        s.pop();
      }
      return true;
    }
      return false;
  }


  bool accepted()
  {
    for( auto& x : queue )
    {
      if( x.stack.empty() && x.tokens.empty() )
      {
        return true;
      }
    }

    for( auto& x : queue )
    {
      if( accept(x) )
      {
        return true;
      }
    }

    return false;
  }
};


template<size_t N, typename... Us>
Lang<N> make_lang(Us&&... us)
{
  using entry_t = typename Lang<N>::entry_t;


  return Lang<N>{{std::forward<Us>(us)...},
                 std::make_unique<hydra>(),
                 std::make_unique<hydra>()};
}
