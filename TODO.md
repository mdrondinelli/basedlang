# TODO

* `Unary_plus::matches` stores 4 explicit type pointers and must be updated manually when a new
  integer type is added — inconsistent with how `Simple_unary_operator_overload<T>` instances work.
* Come up with a better name than `basedlang`
* Better integer arg parsing for `based` CLI
* Split up modules more:
  * Split up AST from parser
  * Split up HLIR from AST -> HLIR compiler
* Make lexer actually handle unicode codepoints
  * Maybe make our own string class? Interned?
* Add a helper function for the lexeme stream to read a character and advance
  the location, instead of repeating this pattern all over the place
