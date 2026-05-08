# TODO

* Implement mutability for local `let` bindings
* Change bytecode operand order for `load_sp_*`, `store_sp_*` instructions
  * `load_sp_* <offset> <dst_reg>`
  * `store_sp_* <offset> <src_reg>`
* Add `Function` bytecode type
  * Stop using `Immediate` for function indices
* Switch arithmetic bytecode ops to have dst register as the last operand
