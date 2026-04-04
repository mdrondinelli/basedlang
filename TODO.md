# TODO

* [code/basedhlir/include/basedhlir/interpret.h](/home/marlon/repos/basedlang/code/basedhlir/include/basedhlir/interpret.h#L14): use `std::exception` as the base class for `Fuel_exhausted_error`.
* Revise compiled HLIR / interpreter not to depend on `Binary_operator_overload`, `Unary_operator_overload`
  * Instructions should carry the information of how to execute them some other way.
  * Might need separate instructions for each math op.
  * `Binary_operator_overload`, `Unary_operator_overload` base class APIs could be extended to have a `compile` function that emits the correct instruction(s) corresponding to the operation.
* Implement sized integer literals and integer types other than `Int32`.
