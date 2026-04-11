# TODO

* Split up IR data model from compiler frontend.
* Make lexer actually handle the 32-bit unicode codepoints the character stream
  already emits
  * Maybe make our own string class? Interned?
* Split up HLIR compiler tests
* Bytecode interpreter (4 part plan)
  1. Design bytecode, implement data model
  2. Implement bytecode interpreter
  3. Implement HLIR -> bytecode compilation
  4. Replace/remove HLIR interpreter
* Design/Implement alloca instruction for heap allocation (depends on bytecode interpreter)
* Implement mutability for local `let` bindings (depends on alloca)
* More ergonomic function declaration (e.g., `fn ident(x: Int32): Int32 => x`)
  * In addition to current syntax. This could also support recursion by name.
