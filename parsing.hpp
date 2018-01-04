#include <cstring>
#include <iostream>

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
    hydra<int>& nil;
    hydra<int>* stk;
    hydra<int>* tokens;

    bool empty()
    {
      return stk->next == nullptr;
    }

    template<typename T>
    void push(const T& t)
    {
      push(stk, t);
    }

    template<typename T>
    void push_back(const T& t)
    {
      push_back(stk, t);
    }

    int pop()
    {
      stk = pop(stk);
      return stk->value;
    }

    entry_t fork()
    {
      return entry_t(nil, stk, tokens);
    }

    entry_t fork_to(int i)
    {
      entry_t val(nil, stk, tokens);
      val.push(i);
      return val;
    }

    entry_t(int i, const nil_t<int>& nil) // only for constructing first time in queue, rest get nil from it
    : stk(nil)
    , tokens(nil)
    {
      stk.push(i);
    }

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
  nil_t<int> nil;
  nil_t<entry_t> entry_nil;
  stack<entry_t> queue;
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
    queue.push(entry_t(0, nil));

    auto prev = stack<std::pair<int, bool>>(make_nil<std::pair<int,bool>>());
    set_nullable(0, prev);

    // std::cout <<"nullability: \n";
    for( auto it = prev.begin(); it != prev.end(); ++it )
    {
      if( lang[it->first] != 4 )
      {
      nullable[it->first] = it->second;
      }
    }

    for( auto it = prev.begin(); it != prev.end(); ++it )
    {
      if( lang[it->first] == 4 ) // nonterm
      {
        nullable[it->first] = nullable[lang[left(it->first)]];
      }
      // std::cout << "(" << it->first << "," << nullable[it->first] << ")";
    }
    // std::cout << std::endl;
  }


  void reset()
  {
    queue.clear();
    queue.push(entry_t(0, nil));
  }


  void add_to_tails(const int t)
  {
    auto hits = std::vector<s_node<int>*>();
    std::vector<s_node<int>*>::iterator it;

    for( auto n = queue.begin(); n != queue.end(); ++n )
    {
      if( n->tokens.empty() )
      {
        n->tokens.push(t);
      } else
      {
        auto i = n->tokens.last();

        it = std::find(hits.begin(), hits.end(), i.get());
        if( it == hits.end() )
        {
          hits.push_back( n->tokens.push_back(t) );
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

    stack<entry_t> out(entry_nil);
    entry_t current(0, nil);

    while( !queue.empty() )
    {
      current = queue.pop();
      search(current, out);
    }

    queue = std::move(out);
    // std::cout << "\napplied " << t << ", remaining: "<< std::endl;
    // print(std::cout);
    // std::cout << "end remaining\n" << std::endl;
  }


  // searches from last node in entry until
  // either dead-end, nonterminal reached
  void search(entry_t& entry, stack<entry_t>& out)
  {
    // std::cout << "searching: " << entry << std::endl;

    if( entry.empty() )
    {
      if( entry.tokens.empty() ) // add to out, could be exit,
                                 // else cleaned up w/ next token
      {
        // std::cout << "added empty \n";
        out.push( std::move(entry) );
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
          queue.push( std::move(entry) );

          // search(entry, out);         // consume empty str
        }
        break;

      case 1: // OR - add second side to current queue, continue search on left side
        queue.push( std::move(entry.fork_to( right(index) )) );
        // std::cout << "fork to " << right(index) << std::endl;

        entry.push( left(index) );
        queue.push( std::move(entry) );

        // search(entry, out);
        break;

      case 2: // AND -add right and search left, if nullable add right as
              // second search w/o left
        if( nullable[left(index)] )
        {
          queue.push( std::move(entry.fork_to( right(index) )) );
          // std::cout << "fork to " << right(index) << std::endl;
        }

        entry.push( right(index) );
        entry.push( left(index) );
        queue.push( std::move(entry) );

        // search(entry, out);
        break;

      case 3: // STAR - search contained node, then STAR again
        entry.push( index );
        entry.push( left(index) );
        queue.push( std::move(entry) );

        // search(entry, out);
        break;

      case 4: // NONTERM - add pointed-to index, add to output for next round
        entry.push( lang[left(index)] );
        out.push( std::move(entry) );
        break;

      default: // TOKENS - check entry's token, if front
               // matches, pop token, search rest
        if( entry.tokens.empty() ) // waiting for token... put back
        {
          entry.push( index );
          out.push( std::move(entry) );
          // std::cout << "added " << out.front();
        } else if( entry.tokens.front() == lang[index] )
        {
          entry.tokens.pop();
          queue.push( std::move(entry) );
          // search(entry, out);
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

    bool l, r;

    switch( lang[i] )
    {
      case 0: // EMPTY
        prev.push(std::make_pair(i, true));
        break;

      case 1: // OR
        l = set_nullable(left(i), prev);
        r = set_nullable(right(i), prev);
        prev.push(std::make_pair(i, l || r));
        break;

      case 2: // AND
        l = set_nullable(left(i), prev);
        r = set_nullable(right(i), prev);
        prev.push(std::make_pair(i, l && r));
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
  auto nil = make_nil<int>(0);
  auto entry_nil = make_nil<typename Lang<N>::entry_t>(0, nil);
  return Lang<N>{{std::forward<Us>(us)...},
                  nil,
                  entry_nil,
                  entry_nil};
}
