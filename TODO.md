# TODO

* Come up with a better name than `basedlang`
* Better integer arg parsing for `based` CLI
* Split up HLIR from AST -> HLIR compiler
* Make lexer actually handle the 32-bit unicode codepoints the character stream
  already emits
  * Maybe make our own string class? Interned?
* Split up HLIR compiler tests
* Split up HLIR compiler implementation
* Bytecode interpreter (4 part plan)
  1. Design bytecode, implement data model
  2. Implement bytecode interpreter
  3. Implement HLIR -> bytecode compilation
  4. Replace/remove HLIR interpreter
