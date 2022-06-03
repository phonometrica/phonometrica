/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 27/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: the compiler turns an AST into an executable routine.                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_COMPILER_HPP
#define PHONOMETRICA_COMPILER_HPP

#include <memory>
#include <vector>
#include <phon/runtime/compiler/ast.hpp>

namespace phonometrica {

class Compiler final : public AstVisitor
{
public:

	explicit Compiler(Runtime *rt);

	Handle<Closure> compile(AutoAst ast);

	void visit_constant(ConstantLiteral *node) override;
	void visit_integer(IntegerLiteral *node) override;
	void visit_float(FloatLiteral *node) override;
	void visit_string(StringLiteral *node) override;
	void visit_list(ListLiteral *node) override;
	void visit_array(ArrayLiteral *node) override;
	void visit_table(TableLiteral *node) override;
	void visit_set(SetLiteral *node) override;
	void visit_unary(UnaryExpression *node) override;
	void visit_binary(BinaryExpression *node) override;
	void visit_statements(StatementList *node) override;
	void visit_declaration(Declaration *node) override;
	void visit_print_statement(PrintStatement *node) override;
	void visit_debug_statement(DebugStatement *node) override;
	void visit_throw_statement(ThrowStatement *node) override;
	void visit_call(CallExpression *node) override;
	void visit_parameter(RoutineParameter *node) override;
	void visit_routine(RoutineDefinition *node) override;
	void visit_variable(Variable *node) override;
	void visit_assignment(Assignment *node) override;
	void visit_assert_statement(AssertStatement *node) override;
	void visit_concat_expression(ConcatExpression *node) override;
	void visit_if_condition(IfCondition *node) override;
	void visit_if_statement(IfStatement *node) override;
	void visit_while_statement(WhileStatement *node) override;
	void visit_repeat_statement(RepeatStatement *node) override;
	void visit_for_statement(ForStatement *node) override;
	void visit_foreach_statement(ForeachStatement *node) override;
	void visit_loop_exit(LoopExitStatement *node) override;
	void visit_return_statement(ReturnStatement *node) override;
	void visit_reference_expression(ReferenceExpression *node) override;
	void visit_index(IndexedExpression *node) override;

private:

	// Routine being compiled.
	std::shared_ptr<Routine> routine;

	void initialize();

	void finalize();

	int open_scope();

	void close_scope(int previous);

	void backpatch_breaks(int previous);

	void backpatch_continues(int previous);

	Instruction add_local(const String &name);

	void set_routine(std::shared_ptr<Routine> r);

	bool parsing_argument() const { return visit_arg >= 0; }

	// Pointer to the current runtime.
	Runtime *runtime;

	// Code of the routine being compiled.
	Code *code = nullptr;

	// For each block that may contain breaks, we set break_count to 0. Whenever a break is found,
	// the counter is incremented and the address to be backpatched is pushed onto break_jumps. At
	// the end of the block, we reset the counter and backpatch the required number of addresses.
	std::vector<int> break_jumps;
	int break_count = 0;

	// "continue" statements follow the same logic as breaks.
	std::vector<int> continue_jumps;
	int continue_count = 0;

	// Keep track of scopes.
	int current_scope = 0;

	// Generate unique ID's for scopes (0 = global scope, 1 = module scope).
	int scope_id = 0;

	// In addition to scope ID's, we record each scope's depth to resolve non-local variables.
	int scope_depth = 0;

	// When visiting arguments, we emit special opcodes to account for the fact that they might need to be passed by reference.
	// The runtime will check the function's bitset to see whether this is the case. This flag provides the index of the argument
	// currently being parsed. -1 indicates that we're not parsing any argument.
	int visit_arg = -1;

	// Flag used when visiting a reference.
	bool visiting_reference = false;

	// When visiting a mutated indexed expression, we need to emit special opcodes to ensure it is unshared.
	bool visiting_indexed_lhs = false;

	bool visiting_assigned_lhs = false;
};

} // namespace phonometrica

#endif // PHONOMETRICA_COMPILER_HPP
