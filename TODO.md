# TODO

* Come up with a better name than `basedlang`
* Better integer arg parsing for `based` CLI
* Split up modules more:
  * Split up AST from parser
  * Split up HLIR from AST -> HLIR compiler
* Make lexer actually handle unicode codepoints
  * Maybe make our own string class? Interned?
* Implement single and double precision floating point types
  * Literals - lexer
  * Types - hlir
  * Operators - hlir 
  * Instructions - hlir
