/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cfenv>
#include <phon/runtime/class.hpp>
#include <phon/runtime.hpp>
#include <phon/runtime/compiler/compiler.hpp>
#include <phon/runtime/compiler/token.hpp>

#define VISIT() PHON_UNUSED(node); throw error("Cannot compile %", __FUNCTION__);
#define EMIT(...) code->append(node->line_no, __VA_ARGS__)
#define THROW(...) throw RuntimeError(node->line_no, __VA_ARGS__)

namespace phonometrica {

using Lexeme = Token::Lexeme;


Compiler::Compiler(Runtime *rt) : runtime(rt)
{

}

Handle<Closure> Compiler::compile(AutoAst ast)
{
	initialize();
	// dummy value to fill the slot occupied by the function. This slot is popped on return.
	code->append(ast->line_no, Opcode::PushNull);
	code->append(ast->line_no, Opcode::NewFrame, 0);
	int offset = code->get_current_offset() - 1;
	int previous_scope = open_scope(); // open module
	ast->visit(*this);
	close_scope(previous_scope);
	// Fix number of locals.
	code->backpatch_instruction(offset, (Instruction)routine->local_count());
	finalize();

	return make_handle<Closure>(runtime, std::move(this->routine));
}

void Compiler::initialize()
{
	scope_id = 0;
	current_scope = 0;
	set_routine(std::make_shared<Routine>(routine.get(), String()));
}

void Compiler::finalize()
{
	code->append_return();
	code = nullptr;
}

int Compiler::open_scope()
{
	int previous = current_scope;
	current_scope = ++scope_id;
	++scope_depth;

	return previous;
}

void Compiler::close_scope(int previous)
{
	--scope_depth;
	current_scope = previous;
}

void Compiler::visit_constant(ConstantLiteral *node)
{
	switch (node->lex)
	{
		case Lexeme::Null:
			EMIT(Opcode::PushNull);
			break;
		case Lexeme::True:
			EMIT(Opcode::PushBoolean, 1);
			break;
		case Lexeme::False:
			EMIT(Opcode::PushBoolean, 0);
			break;
		case Lexeme::Nan:
			EMIT(Opcode::PushNan);
			break;
		// Not a constant, but we put it here to avoid creating a new node.
		case Lexeme::Pass:
			break; // no-op
		default:
			THROW("[Internal error] Invalid constant in visit_constant()");
	}
}

void Compiler::visit_integer(IntegerLiteral *node)
{
	intptr_t value = node->value;

	// optimize small integers that can fit in an opcode.
	if (value >= (std::numeric_limits<int16_t>::min)() && value <= (std::numeric_limits<int16_t>::max)())
	{
		auto small_int = (int16_t) value;
		EMIT(Opcode::PushSmallInt, (Instruction) small_int);
	}
	else
	{
		auto index = routine->add_integer_constant(value);
		EMIT(Opcode::PushInteger, index);
	}
}

void Compiler::visit_float(FloatLiteral *node)
{
	auto index = routine->add_float_constant(node->value);
	EMIT(Opcode::PushFloat, index);
}

void Compiler::visit_string(StringLiteral *node)
{
	// Don't move the node's value here, because we might revisit the node in case of a self-assignment.
	auto index = routine->add_string_constant(node->value);
	EMIT(Opcode::PushString, index);
}

void Compiler::visit_unary(UnaryExpression *node)
{
	bool noop = false;

	// Convert negative numeric literals in place.
	if (node->op == Lexeme::OpMinus)
	{
		if (node->expr->is<FloatLiteral>())
		{
			auto e = static_cast<FloatLiteral*>(node->expr.get());
			std::feclearexcept(FE_ALL_EXCEPT);
			e->value = -e->value;
			if (fetestexcept(FE_OVERFLOW | FE_UNDERFLOW)) {
				throw RuntimeError(node->line_no, "[Math error] Invalid negative float literal");
			}
			noop = true;
		}
		else if (node->expr->is<IntegerLiteral>())
		{
			auto e = static_cast<IntegerLiteral*>(node->expr.get());
			if (e->value == (std::numeric_limits<intptr_t>::max)()) {
				throw RuntimeError(node->line_no, "[Math error] Invalid negative integer literal");
			}
			e->value = -e->value;
			noop = true;
		}
	}
	node->expr->visit(*this);

	switch (node->op)
	{
		case Lexeme::Not:
			EMIT(Opcode::Not);
			break;
		case Lexeme::OpMinus:
			if (!noop) EMIT(Opcode::Negate);
			break;
		default:
			THROW("[Internal error] Invalid operator in unary expression: %", Token::get_name(node->op));
	}
}

void Compiler::visit_binary(BinaryExpression *node)
{
	// Handle AND and OR.
	if (node->op == Lexeme::And)
	{
		// Don't evaluate rhs if lhs is false.
		node->lhs->visit(*this);
		auto jmp = code->append_jump(node->line_no, Opcode::JumpFalseAnd);
		node->rhs->visit(*this);
		code->backpatch(jmp);
		return;
	}
	if (node->op == Lexeme::Or)
	{
		// Don't evaluate rhs if lhs is true.
		node->lhs->visit(*this);
		auto jmp = code->append_jump(node->line_no, Opcode::JumpTrueOr);
		node->rhs->visit(*this);
		code->backpatch(jmp);
		return;
	}
	if (node->op == Lexeme::Dot)
	{
		node->lhs->is_compound = true;
		node->lhs->visit(*this);
		auto var = dynamic_cast<Variable*>(node->rhs.get());
		if (var)
		{
			auto name = routine->add_string_constant(var->name);
			EMIT(Opcode::PushString, name);

			if (visiting_assigned_lhs && !node->is_compound) {
				return; // SetIndex will be added by the assignment once we visit the RHS.
			}
			if (visiting_reference) {
				EMIT(Opcode::GetFieldRef);
			}
			else if (parsing_argument()) {
				EMIT(Opcode::GetFieldArg, Instruction(visit_arg));
			}
			else {
				EMIT(Opcode::GetField);
			}
			return;
		}
		else
		{
			THROW("[Syntax error] Invalid index in dotted expression (expected an identifier)");
		}
	}

	// Handle other operators.
	node->lhs->visit(*this);
	node->rhs->visit(*this);

	switch (node->op)
	{
		case Lexeme::OpPlus:
			EMIT(Opcode::Add);
			break;
		case Lexeme::OpMinus:
			EMIT(Opcode::Subtract);
			break;
		case Lexeme::OpStar:
			EMIT(Opcode::Multiply);
			break;
		case Lexeme::OpSlash:
			EMIT(Opcode::Divide);
			break;
		case Lexeme::OpPower:
			EMIT(Opcode::Power);
			break;
		case Lexeme::OpMod:
			EMIT(Opcode::Modulus);
			break;
		case Lexeme::OpEqual:
			EMIT(Opcode::Equal);
			break;
		case Lexeme::OpNotEqual:
			EMIT(Opcode::NotEqual);
			break;
		case Lexeme::OpLessThan:
			EMIT(Opcode::Less);
			break;
		case Lexeme::OpLessEqual:
			EMIT(Opcode::LessEqual);
			break;
		case Lexeme::OpGreaterThan:
			EMIT(Opcode::Greater);
			break;
		case Lexeme::OpGreaterEqual:
			EMIT(Opcode::GreaterEqual);
			break;
		case Lexeme::OpCompare:
			EMIT(Opcode::Compare);
			break;
		default:
			THROW("[Internal error] Invalid operator in binary expression: ", Token::get_name(node->op));
	}
}

void Compiler::visit_statements(StatementList *node)
{
	int scope = 0;
	if (node->open_scope) scope = open_scope();

	for (auto &stmt : node->statements) {
		stmt->visit(*this);
	}
	if (node->open_scope) close_scope(scope);
}

void Compiler::visit_declaration(Declaration *node)
{
	if (node->lhs.size() != 1 || node->rhs.size() > 1) {
		THROW("Multiple declaration not implemented");
	}
	auto ident = dynamic_cast<Variable*>(node->lhs.front().get());
	if (!ident) {
		THROW("[Syntax error] Expected a variable name in declaration");
	}

	try
	{
		if (!node->rhs.empty())
		{
			// We don't add the local before visiting the RHS, otherwise the RHS could reference the LHS which doesn't exist yet.
			node->rhs.front()->visit(*this);
			auto index = add_local(ident->name);
			EMIT(Opcode::DefineLocal, index);
		}
		else
		{
			// Locals don't need to be defined if they are null because they are automatically null-initialized by NewFrame.
			add_local(ident->name);
		}
	}
	catch (std::runtime_error &e)
	{
		THROW(e.what());
	}
}

void Compiler::visit_print_statement(PrintStatement *node)
{
	for (auto &e : node->list) {
		e->visit(*this);
	}
	Opcode op = node->new_line ? Opcode::PrintLine : Opcode::Print;
	EMIT(op, Instruction(node->list.size()));
}

void Compiler::visit_call(CallExpression *node)
{
	// First push the function.
	node->expr->visit(*this);

	// Prepare call, leave the function on the stack.
	EMIT(Opcode::Precall);

	// Next push the arguments.
	auto arg_flag = this->visit_arg;
	this->visit_arg = 0;
	for (auto &arg : node->args)
	{
		arg->visit(*this);
		this->visit_arg++;
	}
	this->visit_arg = arg_flag;

	// We can only pass 64 arguments to a function, so we use one byte of the instruction for the arguments
	// and one byte for reference flag.
	Instruction flag = node->return_reference ? (1 << 9) : 0;
	auto narg = Instruction(node->args.size());

	// Finally, make the call.
	EMIT(Opcode::Call, narg|flag);

	// Discard result if it's not used.
	if (node->discard_result) {
		EMIT(Opcode::Pop);
	}
}

void Compiler::visit_variable(Variable *node)
{
	// Try to find a local variable, otherwise try to get a global.
	auto index = routine->find_local(node->name, scope_depth);
	if (index)
	{
		if (parsing_argument())
		{
			EMIT(Opcode::GetLocalArg, *index, Instruction(this->visit_arg));
		}
		else if (visiting_reference || visiting_assigned_lhs)
		{
			if (visiting_indexed_lhs || visiting_assigned_lhs) {
				EMIT(Opcode::GetUniqueLocal, *index);
			}
			else {
				EMIT(Opcode::GetLocalRef, *index);
			}
		}
		else {
			EMIT(Opcode::GetLocal, *index);
		}
	}
	else if ((index = routine->resolve_upvalue(node->name, scope_depth)))
	{
		if (parsing_argument())
		{
			EMIT(Opcode::GetUpvalueArg, *index);
		}
		else if (visiting_reference || visiting_assigned_lhs)
		{
			if (visiting_indexed_lhs || visiting_assigned_lhs) {
				EMIT(Opcode::GetUniqueUpvalue, *index);
			}
			else {
				EMIT(Opcode::GetUpvalueRef, *index);
			}
		}
		else {
			EMIT(Opcode::GetUpvalue, *index);
		}
	}
	else
	{
		auto var = routine->add_string_constant(node->name);

		if (parsing_argument())
		{
			EMIT(Opcode::GetGlobalArg, var, Instruction(this->visit_arg));
		}
		else if (visiting_reference || visiting_assigned_lhs)
		{
			if (visiting_indexed_lhs || visiting_assigned_lhs) {
				EMIT(Opcode::GetUniqueGlobal, var);
			}
			else {
				EMIT(Opcode::GetGlobalRef, var);
			}
		}
		else {
			EMIT(Opcode::GetGlobal, var);
		}
	}
}

void Compiler::visit_assignment(Assignment *node)
{
	Variable *var;
	auto op = node->get_operator();

	// For self-assignment, we write an expression such as "x += y" as "x = x + y".
	if ((var = dynamic_cast<Variable*>(node->lhs.get())))
	{
		if (op == Lexeme::OpAssign)
		{
			node->rhs->visit(*this);
		}
		else
		{
			node->lhs->visit(*this);
			node->rhs->visit(*this);

			switch (op)
			{
				case Lexeme::OpConcat:
					EMIT(Opcode::Concat, 2);
					break;
				case Lexeme::OpPlus:
					EMIT(Opcode::Add);
					break;
				case Lexeme::OpMinus:
					EMIT(Opcode::Subtract);
					break;
				case Lexeme::OpStar:
					EMIT(Opcode::Multiply);
					break;
				case Lexeme::OpSlash:
					EMIT(Opcode::Divide);
					break;
				case Lexeme::OpPower:
					EMIT(Opcode::Power);
					break;
				case Lexeme::OpMod:
					EMIT(Opcode::Modulus);
					break;
				default:
					THROW("[Internal error] Invalid operator in self assignment");
			}
		}

		// Try to find a local variable, otherwise try to get a global.
		auto index = routine->find_local(var->name, scope_depth);
		if (index)
		{
			EMIT(Opcode::SetLocal, *index);
		}
		else if ((index = routine->resolve_upvalue(var->name, scope_depth)))
		{
			EMIT(Opcode::SetUpvalue, *index);
		}
		else
		{
			auto arg = routine->add_string_constant(var->name);
			EMIT(Opcode::SetGlobal, arg);
		}
		return;
	}

	IndexedExpression *lhs; BinaryExpression *dot;

	if ((lhs = dynamic_cast<IndexedExpression*>(node->lhs.get())))
	{
		visiting_assigned_lhs = true;
		node->lhs->visit(*this);
		visiting_assigned_lhs = false;

		if (op == Lexeme::OpAssign)
		{
			node->rhs->visit(*this);
		}
		else
		{
			node->lhs->visit(*this);
			node->rhs->visit(*this);

			switch (op)
			{
				case Lexeme::OpConcat:
					EMIT(Opcode::Concat, 2);
					break;
				case Lexeme::OpPlus:
					EMIT(Opcode::Add);
					break;
				case Lexeme::OpMinus:
					EMIT(Opcode::Subtract);
					break;
				case Lexeme::OpStar:
					EMIT(Opcode::Multiply);
					break;
				case Lexeme::OpSlash:
					EMIT(Opcode::Divide);
					break;
				case Lexeme::OpPower:
					EMIT(Opcode::Power);
					break;
				case Lexeme::OpMod:
					EMIT(Opcode::Modulus);
					break;
				default:
					THROW("[Internal error] Invalid operator in self assignment");
			}
		}

		EMIT(Opcode::SetIndex, Instruction(lhs->size()));
	}
	else if ((dot = dynamic_cast<BinaryExpression*>(node->lhs.get())) && dot->op == Lexeme::Dot)
	{
		visiting_assigned_lhs = true;
		node->lhs->visit(*this);
		visiting_assigned_lhs = false;
		node->rhs->visit(*this);
		EMIT(Opcode::SetField);
	}
	else
	{
		THROW("[Syntax error] Expected a variable name or an indexed expression on the left hand side in assignment");
	}
}

void Compiler::visit_assert_statement(AssertStatement *node)
{
	Instruction narg = (node->msg == nullptr) ? 1 : 2;
	node->expr->visit(*this);
	if (narg == 2) node->msg->visit(*this);
	EMIT(Opcode::Assert, narg);
}

void Compiler::visit_concat_expression(ConcatExpression *node)
{
	for (auto &e : node->list) {
		e->visit(*this);
	}
	EMIT(Opcode::Concat, Instruction(node->list.size()));
}

void Compiler::visit_if_condition(IfCondition *node)
{
    node->cond->visit(*this);
    // Jump to the next branch (we will need to backpatch)
    node->conditional_jump = code->append_jump(node->line_no, Opcode::JumpFalse);
    // Compile TRUE case
    node->block->visit(*this);
}

void Compiler::visit_if_statement(IfStatement *node)
{
	/*
	  Given a block such as:

	  if x < 0 then
		print "-"
	  else
		print "+"
	  end

	  We generate the following opcodes (simplified):

	  i01: PUSH x
	  i02: PUSH 0
	  i03: LESS_THAN
	  i04: JUMP_FALSE i08  ; jump to FALSE case if condition not satisfied
	  i05: PUSH "-"  ; TRUE case
	  i06: PRINT
	  i07: JUMP i10  ; skip FALSE case
	  i08: PUSH "+"  ; FALSE case
	  i09: PRINT
	  i10: END      ; end of the block

	  We need to backpatch forward addresses for the jumps since they
	  are not known when the code is compiled. Elsif branches work just
	  like a sequence of if branches.
	*/
	for (auto &stmt : node->if_conds)
	{
		auto if_cond = static_cast<IfCondition*>(stmt.get());
		if_cond->visit(*this);
		if_cond->unconditional_jump = code->append_jump(node->line_no, Opcode::Jump);
		// Now we are at the beginning of the next branch, we can backpatch JumpFalse
		code->backpatch(if_cond->conditional_jump);
	}

	// Compile the else branch
	if (node->else_block) {
		node->else_block->visit(*this);
	}

	// We can now backpatch the jump in i07 with i10
	for (auto &stmt : node->if_conds)
	{
		auto if_cond = static_cast<IfCondition*>(stmt.get());
		code->backpatch(if_cond->unconditional_jump);
	}
}

void Compiler::visit_while_statement(WhileStatement *node)
{
	int previous_break_count = break_count;
	int previous_continue_count = continue_count;
	break_count = continue_count = 0;
	int loop_start = code->get_current_offset();
	node->cond->visit(*this);
	int exit_jump = code->append_jump(node->line_no, Opcode::JumpFalse);
	node->body->visit(*this);
	backpatch_continues(previous_continue_count);
	code->append_jump(node->line_no, Opcode::Jump, loop_start);
	code->backpatch(exit_jump);
	backpatch_breaks(previous_break_count);
}

void Compiler::visit_repeat_statement(RepeatStatement *node)
{
	auto scope = open_scope();
	int previous_break_count = break_count;
	int previous_continue_count = continue_count;
	break_count = continue_count = 0;
	int loop_start = code->get_current_offset();
	node->body->visit(*this);
	node->cond->visit(*this);
	code->append_jump(node->line_no, Opcode::JumpFalse, loop_start);

	backpatch_breaks(previous_break_count);
	for (int i = 0; i < continue_count; i++)
	{
		int addr = continue_jumps.back();
		continue_jumps.pop_back();
		code->backpatch(addr, loop_start);
	}
	continue_count = previous_continue_count;
	close_scope(scope);
}

void Compiler::visit_for_statement(ForStatement *node)
{
	static String end_name("$end"), step_name("$step");
	auto scope = open_scope();
	int previous_break_count = break_count;
	int previous_continue_count = continue_count;
	break_count = continue_count = 0;

	// Initialize loop variable
	auto ident = dynamic_cast<Variable*>(node->var.get());
	node->start->visit(*this);
	auto var_index = add_local(ident->name);
	EMIT(Opcode::DefineLocal, var_index);

	// Evaluate end condition once and store it in a hidden variable.
	node->end->visit(*this);
	auto end_index = add_local(end_name);
	EMIT(Opcode::DefineLocal, end_index);

	// Evaluate step condition if it was provided and store it in a hidden variable;
	// otherwise, we use special-purpose instructions to increment/decrement the counter.
	Instruction step_index = (std::numeric_limits<Instruction>::max)();

	if (node->step)
	{
		node->step->visit(*this);
		step_index = add_local(step_name);
		EMIT(Opcode::DefineLocal, step_index);
	}

	// The loop starts here.
	int loop_start = code->get_current_offset();

	// End condition.
	EMIT(Opcode::GetLocal, var_index);
	EMIT(Opcode::GetLocal, end_index);
	auto cmp = node->down ? Opcode::Less : Opcode::Greater;
	EMIT(cmp);
	int jump_end = code->append_jump(node->line_no, Opcode::JumpTrue);

	// Compile block
	node->block->visit(*this);

	backpatch_continues(previous_continue_count);
	// Update counter
	if (node->step)
	{
		EMIT(Opcode::GetLocal, var_index);
		EMIT(Opcode::GetLocal, step_index);
		auto op = node->down ? Opcode::Subtract : Opcode::Add;
		EMIT(op);
		EMIT(Opcode::SetLocal, var_index);
	}
	else
	{
		auto op = node->down ? Opcode::DecrementLocal : Opcode::IncrementLocal;
		EMIT(op, var_index);
	}

	// Go back to the beginning of the loop.
	code->append_jump(node->line_no, Opcode::Jump, loop_start);
	code->backpatch(jump_end);
	backpatch_breaks(previous_break_count);

	close_scope(scope);
}

void Compiler::visit_foreach_statement(ForeachStatement *node)
{
	static String iter_name("$iter");
	auto scope = open_scope();
	int previous_break_count = break_count;
	int previous_continue_count = continue_count;
	break_count = continue_count = 0;

	// Create the loop variables. The key is optional, the value is always there.
	auto key_index = (std::numeric_limits<Instruction>::max)();
	if (node->key)
	{
		auto ident = dynamic_cast<Variable*>(node->key.get());
		key_index = add_local(ident->name);
	}
	Instruction val_index;
	bool ref_val = false;
	bool has_key = bool(node->key);
	ReferenceExpression *re;
	AutoAst val_expr;
	if ((re = dynamic_cast<ReferenceExpression*>(node->value.get())))
	{
		ref_val = true;
		val_expr = std::move(re->expr);
	}
	else
	{
		val_expr = std::move(node->value);
	}
	auto val_ident = dynamic_cast<Variable*>(val_expr.get());
	val_index = add_local(val_ident->name);

	auto iter_index = add_local(iter_name);

	// Create the iterator.
	node->collection->visit(*this);
	EMIT(Opcode::NewIterator, Instruction(ref_val));
	EMIT(Opcode::DefineLocal, iter_index);

	// The loop starts here.
	auto loop_start = code->get_current_offset();
	EMIT(Opcode::GetLocal, iter_index);
	EMIT(Opcode::TestIterator);
	auto jump_end = code->append_jump(node->line_no, Opcode::JumpFalse);
	// We need to clear the key and value in case they contain a reference.
	if (has_key) EMIT(Opcode::ClearLocal, key_index);
	EMIT(Opcode::ClearLocal, val_index);
	if (has_key)
	{
		EMIT(Opcode::GetLocal, iter_index);
		EMIT(Opcode::NextKey);
		EMIT(Opcode::SetLocal, key_index);
	}
	EMIT(Opcode::GetLocal, iter_index);
	EMIT(Opcode::NextValue);
	EMIT(Opcode::SetLocal, val_index);

	// Visit body.
	node->block->visit(*this);
	backpatch_continues(previous_continue_count);
	code->append_jump(node->line_no, Opcode::Jump, loop_start);
	code->backpatch(jump_end);
	backpatch_breaks(previous_break_count);

	close_scope(scope);
}

void Compiler::backpatch_breaks(int previous)
{
	for (int i = 0; i < break_count; i++)
	{
		int addr = break_jumps.back();
		break_jumps.pop_back();
		code->backpatch(addr);
	}
	break_count = previous;
}

void Compiler::visit_loop_exit(LoopExitStatement *node)
{
	int addr = code->append_jump(node->line_no, Opcode::Jump);

	if (node->lex == Lexeme::Break)
	{
		break_jumps.push_back(addr);
		break_count++;
	}
	else
	{
		assert(node->lex == Lexeme::Continue);
		continue_jumps.push_back(addr);
		continue_count++;
	}
}

void Compiler::backpatch_continues(int previous)
{
	for (int i = 0; i < continue_count; i++)
	{
		int addr = continue_jumps.back();
		continue_jumps.pop_back();
		code->backpatch(addr);
	}
	continue_count = previous;
}

void Compiler::visit_parameter(RoutineParameter *node)
{
	if (node->add_names)
	{
		auto ident = static_cast<Variable*>(node->variable.get());
		// From the function's point of view, the parameters are just the first locals.
		add_local(ident->name);
	}
	else
	{
		if (node->type)
		{
			// The expression must evaluate to a Class at runtime.
			node->type->visit(*this);
		}
		else
		{
			// Parameters with no type are implicitly tagged as Object.
			auto id = routine->add_string_constant(Class::get_name<Object>());
			EMIT(Opcode::GetGlobal, id);
		}
	}
}

void Compiler::visit_routine(RoutineDefinition *node)
{
	if (node->params.size() > PARAM_BITSET_SIZE) {
		throw RuntimeError(node->line_no, "[Syntax error] Maximum number of parameters exceeded (limit is %)", PARAM_BITSET_SIZE);
	}
	auto ident = static_cast<Variable*>(node->name.get());
	String name = ident ? ident->name : String();
	/////////////////////////auto func = create_function_symbol(node, name);

	// Compile inner routine.
	auto previous_scope = open_scope();
	auto outer_routine = routine;
	set_routine(std::make_shared<Routine>(routine.get(), name));
	EMIT(Opcode::NewFrame, 0);
	int frame_offset = code->get_current_offset() - 1;

	for (size_t i = 0; i < node->params.size(); i++)
	{
		auto &p = node->params[i];
		// Compile names in the new function.
		auto param = static_cast<RoutineParameter*>(p.get());
		param->add_names = true;
		if (param->by_ref) {
			routine->ref_flags[i] = true;
		}
		p->visit(*this);
	}
	node->body->visit(*this);
	EMIT(Opcode::Return);
	// Fix number of locals.
	code->backpatch_instruction(frame_offset, (Instruction)routine->local_count());
	close_scope(previous_scope);

	auto routine_index = outer_routine->add_routine(routine);
	set_routine(std::move(outer_routine));

	// Compile type information in the outer routine.
	for (auto &param : node->params)
	{
		static_cast<RoutineParameter*>(param.get())->add_names = false;
		param->visit(*this);
	}
	EMIT(Opcode::NewClosure, routine_index, Instruction(node->params.size()));
	if (node->is_expression()) {
		return;
	}
	if (node->local || scope_depth > 1)
	{
		// We might be defining an overload of an already existing routine, so we first try find_local().
		auto symbol = routine->find_local(name, scope_depth);
		auto index = symbol ? *symbol : add_local(name);
		EMIT(Opcode::SetLocal, index);
	}
	else
	{
		auto index = routine->add_string_constant(name);
		// Don't use DefineGlobal, since we might be adding an overload to an existing function.
		EMIT(Opcode::SetGlobal, index);
	}
}

Instruction Compiler::add_local(const String &name)
{
	return routine->add_local(name, current_scope, scope_depth);
}

void Compiler::set_routine(std::shared_ptr<Routine> r)
{
	this->code = r ? &r->code : nullptr;
	this->routine = std::move(r);
}

void Compiler::visit_return_statement(ReturnStatement *node)
{
	if (node->expr)
	{
		node->expr->visit(*this);
	}
	else
	{
		EMIT(Opcode::PushNull);
	}
	EMIT(Opcode::Return);
}

void Compiler::visit_reference_expression(ReferenceExpression *node)
{
	auto call = dynamic_cast<CallExpression*>(node->expr.get());
	if (call)
	{
		call->return_reference = true;
		call->visit(*this);
	}
	else
	{
		auto flag = visiting_reference;
		visiting_reference = true;
		node->expr->visit(*this);
		visiting_reference = flag;
	}
}

void Compiler::visit_list(ListLiteral *node)
{
	for (auto &item : node->items) {
		item->visit(*this);
	}
	EMIT(Opcode::NewList, Instruction(node->items.size()));
}

void Compiler::visit_table(TableLiteral *node)
{
	auto size = node->keys.size();
	for (size_t i = 0; i < size; i++)
	{
		node->keys[i]->visit(*this);
		node->values[i]->visit(*this);
	}
	EMIT(Opcode::NewTable, Instruction(size));
}

void Compiler::visit_debug_statement(DebugStatement *node)
{
	if (runtime->debug_mode()) {
		node->block->visit(*this);
	}
}

void Compiler::visit_throw_statement(ThrowStatement *node)
{
	node->expr->visit(*this);
	EMIT(Opcode::Throw);
}

void Compiler::visit_set(SetLiteral *node)
{
	for (auto &e : node->values) {
		e->visit(*this);
	}
	EMIT(Opcode::NewSet, Instruction(node->values.size()));
}

void Compiler::visit_array(ArrayLiteral *node)
{
	if (unlikely(node->nrow > (std::numeric_limits<Instruction>::max)() || node->ncol > (std::numeric_limits<Instruction>::max)()))
	{
		THROW("Array literal can have at most % rowns and % columns",
				(std::numeric_limits<Instruction>::max)(), (std::numeric_limits<Instruction>::max)());
	}

	for (auto &e : node->items) {
		e->visit(*this);
	}
	EMIT(Opcode::NewArray, Instruction(node->nrow), Instruction(node->ncol));
}

void Compiler::visit_index(IndexedExpression *node)
{
	visiting_indexed_lhs = true;
	node->expr->is_compound = true;
	node->expr->visit(*this);
	visiting_indexed_lhs = false;
	for (auto &i : node->indexes) {
		i->visit(*this);
	}

	if (visiting_assigned_lhs && !node->is_compound) {
		return; // SetIndex will be added by the assignment once we visit the RHS.
	}
	auto count = Instruction(node->size());
	if (visiting_reference) {
		EMIT(Opcode::GetIndexRef, count);
	}
	else if (parsing_argument()) {
		EMIT(Opcode::GetIndexArg, count, Instruction(visit_arg));
	}
	else {
		EMIT(Opcode::GetIndex, count);
	}
}

} // namespace phonometrica

#undef VISIT
#undef EMIT
#undef THROW