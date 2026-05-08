# TODO

* Implement mutability for local `let` bindings
* Add `Function` bytecode type
  * Stop using `Immediate` for function indices
* Compress the bytecode emission functions with macros
* Switch arithmetic bytecode ops to have dst register as the last operand
* Switch load / store ops to have address as first operand, offset as second
  operand
