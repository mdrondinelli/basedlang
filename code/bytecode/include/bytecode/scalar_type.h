#ifndef BENSON_BYTECODE_SCALAR_TYPE_H
#define BENSON_BYTECODE_SCALAR_TYPE_H

namespace benson::bytecode
{

  enum class Scalar_type
  {
    int8,
    int16,
    int32,
    int64,
    float_,
    double_,
    bool_,
    void_,
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_SCALAR_TYPE_H
