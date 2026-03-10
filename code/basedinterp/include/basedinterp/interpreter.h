#ifndef BASEDINTERP_INTERPRETER_H
#define BASEDINTERP_INTERPRETER_H

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "basedir/ir.h"
#include "value.h"

namespace basedinterp
{

  class Interpreter
  {
  public:
    class Runtime_error: public std::runtime_error
    {
    public:
      using std::runtime_error::runtime_error;
    };

    explicit Interpreter(basedir::Program const *program);

    Value call(std::string const &name, std::vector<Value> arguments);

    Value call(std::size_t function_index, std::vector<Value> arguments);

  private:
    struct Stack_frame
    {
      std::vector<std::shared_ptr<Value>> locals;
    };

    basedir::Program const *_program;
    std::vector<std::shared_ptr<Value>> _globals;
    std::vector<Stack_frame> _call_stack;

    void push_frame(std::size_t frame_size);

    void pop_frame();

    std::shared_ptr<Value> &local_ptr(std::size_t index);

    Value &local(std::size_t index);

    Value execute_function(
      std::size_t function_index,
      std::vector<Value> const &arguments
    );

    void execute_statement(basedir::Statement const &statement);

    void execute_let_statement(basedir::Let_statement const &statement);

    void execute_while_statement(basedir::While_statement const &statement);

    void execute_expression_statement(
      basedir::Expression_statement const &statement
    );

    Value evaluate_expression(basedir::Expression const &expression);

    Value evaluate_unary(basedir::Unary_expression const &expression);

    Value evaluate_binary(basedir::Binary_expression const &expression);

    Value evaluate_assign(basedir::Assign_expression const &expression);

    Value evaluate_call(basedir::Call_expression const &expression);

    Value evaluate_block(basedir::Block_expression const &expression);

    Value evaluate_if(basedir::If_expression const &expression);
  };

} // namespace basedinterp

#endif // BASEDINTERP_INTERPRETER_H
