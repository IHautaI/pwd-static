#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>

#include "hydra.hpp"

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
    hydra<int>* nil;
    hydra<int>* stk;
    hydra<int>* tokens;

    bool empty()
    {
      return stk == nil;
    }

    template<typename T>
    void push(const T& t)
    {
      stk = push(stk, t);
    }

    template<typename T>
    void push_back(const T& t)
    {
      push_back(stk, t);
    }

    template<typename T>
    void push_token(const T& t)
    {
      tokens = push(tokens, t);
    }


    int next()
    {
      return pop(stk);
    }

    int pop_token()
    {
      return pop(tokens);
    }

    iter<hydra<int>, int> stack_begin()
    {
      return iter<hydra<int>, int>(stk);
    }

    iter<hydra<int>, int> stack_end()
    {
      return iter<hydra<int>, int>(nil);
    }

    iter<hydra<int>, int> token_begin()
    {
      return iter<hydra<int>, int>(tokens);
    }

    iter<hydra<int>, int> token_end()
    {
      return iter<hydra<int>, int>(nil);
    }

    entry_t fork()
    {
      return entry_t(nil, stk, tokens);
    }

    entry_t fork_to(int i)
    {
      entry_t val(nil, stk, tokens);
      push(val.stk, i);
      return val;
    }

    entry_t(hydra<int>* nil)
    : nil(nil)
    , stk(nil)
    , tokens(nil)
    {}

    entry_t(hydra<int>* nil, hydra<int>* stk, hydra<int>* tokens)
    : nil(nil)
    , stk(stk)
    , tokens(tokens)
    {}

    entry_t(entry_t&& o)
    : nil(std::move(o.nil))
    , stk(std::move(o.stk))
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
      for( auto it = e.stk_begin(); it != e.stk_end(); ++it )
      {
        std::cout << *it << " -> ";
      }
      std::cout << " | ";

      for( auto it = e.token_begin(); it != e.tokens_end(); ++it )
      {
        std::cout << *it << " -> ";
      }
      std::cout << std::endl;
      return out;
    }
  };



  int lang[size];
  std::unique_ptr<hydra<int>> nil;
  std::unique_ptr<hydra<entry_t>> entry_nil;
  hydra<entry_t> queue;
  bool nullable[size];



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
    auto start = entry_t(nil.get());
    start.push(0);
    push(queue, std::move(start));

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
    auto start = entry_t(nil.get());
    start.push(0);
    push(queue, std::move(start));
  }


  void add_to_tails(const int t)
  {
    auto hits = std::vector<hydra<int>*>();
    std::vector<hydra<int>*>::iterator it;

    for( auto n = queue.begin(); n != queue.end(); ++n )
    {
      if( n->tokens.empty() )
      {
        n->push_token(t);
      } else
      {
        it = std::find(hits.begin(), hits.end(), n->tokens.last());
        if( it == hits.end() )
        {
          hits.push_back( push_back(n->tokens, t) );
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
    // std::cout << "\nqueue: ";
    // print(std::cout);

    auto out = hydra<entry_t>(entry_nil);
    auto current = entry_t(0, nil);

    while( !queue.empty() )
    {
      current = queue.pop();
      search(*current, out);
    }

    queue = std::move(out);
    // std::cout << "\napplied " << t << ", remaining: "<< std::endl;
    // print(std::cout);
    // std::cout << "end remaining\n" << std::endl;
  }


  // searches from last node in entry until
  // either dead-end, nonterminal reached
  void search(entry_t& entry, hydra<entry_t>& out)
  {
    // std::cout << "searching: " << entry << std::endl;

    if( entry.empty() )
    {
      if( entry.tokens.empty() ) // add to out, could be exit,
                                 // else cleaned up w/ next token
      {
        // std::cout << "added empty \n";
        push(out, std::move(entry));
      }

      return;
    }

    auto index = entry.pop();

    switch( lang[index] )
    {
      case 0: // EMPTY - if tokens empty, wait else consume empty and go
        if( entry.tokens.empty() )
        {
          out.push( entry.fork() );
        } else
        {
          push(queue, std::move(entry));

          // search(entry, out);         // consume empty str
        }
        break;

      case 1: // OR - add second side to current queue, continue search on left side
        push(queue, std::move(entry.fork_to( right(index) )));
        // std::cout << "fork to " << right(index) << std::endl;

        entry.push( left(index) );

        push(queue, std::move(entry) );

        // search(entry, out);
        break;

      case 2: // AND -add right and search left, if nullable add right as
              // second search w/o left
        if( nullable[left(index)] )
        {
          push(queue, std::move(entry.fork_to( right(index) )) );
          // std::cout << "fork to " << right(index) << std::endl;
        }

        entry.push( right(index) );
        entry.push( left(index) );
        push(queue, std::move(entry) );

        // search(entry, out);
        break;

      case 3: // STAR - search contained node, then STAR again
        entry.push( index );
        entry.push( left(index) );

        push(queue, std::move(entry) );

        // search(entry, out);
        break;

      case 4: // NONTERM - add pointed-to index, add to output for next round
        entry.push( lang[left(index)] );
        push(out, std::move(entry) );
        break;

      default: // TOKENS - check entry's token, if front
               // matches, pop token, search rest
        if( entry.tokens.empty() ) // waiting for token... put back
        {
          entry.push( index );

          push(out, std::move(entry));
          // std::cout << "added " << out.front();
        } else if( entry.tokens->value == lang[index] )
        {
          entry.pop_token();

          push(queue, std::move(entry));
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
      for( auto it = e.stack_begin(); it != e.stack_end(); ++it )
      {
        if( !nullable[*it] ) // requires tokens to complete
        {
          return false;
        }
      }

      return true;
    }
      return false;
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


template<size_t N, typename... Us>
Lang<N> make_lang(Us&&... us)
{
  using entry_t = typename Lang<N>::entry_t;

  auto* nil = new hydra<int>(0);
  auto* nil_entry = new hydra<entry_t>(std::move(entry_t(nil)));

  return Lang<N>{{std::forward<Us>(us)...},
                 std::unique_ptr<hydra<int>>(nil),
                 std::unique_ptr<hydra<entry_t>>(nil_entry)};
}
