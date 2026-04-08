# TODO

* Come up with a better name than `basedlang`
* Better integer arg parsing for `based` CLI
* Split up HLIR data model from AST -> HLIR compiler
* Make lexer actually handle the 32-bit unicode codepoints the character stream
  already emits
  * Maybe make our own string class? Interned?
* Split up HLIR compiler implementation (`compile.cpp`)
* Split up HLIR compiler tests
* Bytecode interpreter (4 part plan)
  1. Design bytecode, implement data model
  2. Implement bytecode interpreter
  3. Implement HLIR -> bytecode compilation
  4. Replace/remove HLIR interpreter
* Design/add alloca instruction for heap allocation
* Implement mutability for local `let` bindings
