# pwd-static

Parsing with Derivatives
Alternate version

This version of parsing with derivatives uses a static language representation.  The
derivative is implemented as a search rather than generating new nodes in the language.
This allows the nullability of all nodes to be precalculated.

Current version is a recognizer, full parser to come.

This project was inspired by playing with parsing with derivatives in C++, and
making some observations through multiple failures.

The search uses singly-linked lists with shared tails (using std::shared_ptr).
Entries in the search queue hold two of these, one for nodes to be visited
and one for tokens to be consumed.


Search by node type:

* And - search left child and then right, and a copy will search the right child directly
* Or - search left child, search right child with a copy
* Token - if a match, pop stack and search the returned index, else destruct
* Empty - if token stack is empty, add to output stack (the staging for the next search round)
* Star - search left child, then this node again (push Star node on stack)
* Null - No need for Null nodes


This leads to something more like standard parsing with a stack, but retains
the nice properties of PwD.

Current work:
figure out how to make it better
then do it

previous idea was bad, will not do
next idea - better?
