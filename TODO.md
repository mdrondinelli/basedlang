# TODO

* Split up HLIR compiler tests
* Bytecode interpreter (4 part plan)
  1. Design bytecode, implement data model
  2. Implement bytecode interpreter
  3. Implement HLIR -> bytecode compilation
  4. Replace/remove HLIR interpreter
* Design/Implement alloca instruction for heap allocation (depends on bytecode interpreter)
* Implement mutability for local `let` bindings (depends on alloca)
