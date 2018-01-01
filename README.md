# pwd-static

Parsing With Derivatives
Alternate version

This version of parsing with derivatives uses a static language representation.  The 
derivative is implemented as a search rather than generating new nodes in the language.
This allows the nullability of all nodes to be precalculated.

Current version is a recognizer, full parser to come.

This project was inspired by playing with parsing with derivatives in C++, and 
making some observations through multiple failures.


Search by node type:

* And - search left child, then right (push right child on stack)
* Or - search left child, search right child with stack copy (stack copies use shared tails)
* Token - if a match, pop stack and search the returned index, else destruct
* Empty - if token stack is empty, add to output stack (the staging for the next search round)
* Star - search left child, then this node again (push Star node on stack)
* Null - No need for Null nodes

