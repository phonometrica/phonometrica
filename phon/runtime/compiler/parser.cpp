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

#include <iostream>
#include <phon/runtime.hpp>
#include <phon/runtime/compiler/parser.hpp>

#if 0
#define trace_ast() std::cerr << token.line_no << ": " << __FUNCTION__ << std::endl;
#else
#define trace_ast()
#endif

namespace phonometrica {

Parser::Parser(Runtime *rt) : runtime(rt)
{

}

void Parser::accept()
{
	token = scanner.read_token();
}

void Parser::report_error(const std::string &hint, const char *error_type)
{
	// Move error indicator to the beginning of the current token
	scanner.report_error(hint, token.size(), error_type);
}

void Parser::expect(Lexeme lex, const char *hint)
{
	if (!token.is(lex))
	{
		auto msg = utils::format("expected \"%\" % but got \"%\"", Token::get_name(lex), hint, token.get_name());
		report_error(msg);
	}
	accept();
}

void Parser::expect_separator()
{
	if (token.is(Lexeme::Eot)) return;
	if (token.is_separator()) {
		accept();
	}
	else {
		report_error("Expected a new line or a semicolon");
	}
}

bool Parser::check(Lexeme lex)
{
	return token.is(lex);
}

bool Parser::accept(Lexeme lex)
{

	if (check(lex))
	{
		accept();
		return true;
	}

	return false;
}

AutoAst Parser::parse_file(const String &path)
{
	scanner.load_file(path);
	return parse();
}

AutoAst Parser::parse_string(const String &path)
{
	scanner.load_string(path);
	return parse();
}

int Parser::get_line()
{
	return (int) token.line_no;
}

void Parser::initialize()
{

}

AutoAst Parser::parse()
{
	initialize();
	accept();
	auto line = get_line();
	AstList block;
	while (token.is_separator()) accept();

	// Consume options (if any).
	while (accept(Lexeme::Option))
	{
		parse_option();
	}

	// Parse statements.
	while (!check(Lexeme::Eot))
	{
		block.push_back(parse_statement());
		while (token.is_separator()) accept();
	}
	expect(Lexeme::Eot, "at end of file");
	return std::make_unique<StatementList>(line, std::move(block));
}

AutoAst Parser::parse_statement()
{
	trace_ast();

	if (accept(Lexeme::Print))
	{
		return parse_print_statement();
	}
	if (accept(Lexeme::Local))
	{
		while (accept(Lexeme::Eol)) { }
		if (accept(Lexeme::Function)) {
			return parse_function_declaration(true);
		}

		return parse_declaration();
	}
	else if (accept(Lexeme::If))
	{
		return parse_if_statement();
	}
	else if (accept(Lexeme::While))
	{
		return parse_while_statement();
	}
	else if (accept(Lexeme::For))
	{
		return parse_for_statement();
	}
	else if (accept(Lexeme::Foreach))
	{
		return parse_foreach_statement();
	}
	else if (accept(Lexeme::Function))
	{
		return parse_function_declaration(false);
	}
	else if (accept(Lexeme::Return))
	{
		return parse_return_statement();
	}
	else if (accept(Lexeme::Repeat))
	{
		return parse_repeat_statement();
	}
	else if (accept(Lexeme::Break))
	{
		return make<LoopExitStatement>(Lexeme::Break);
	}
	else if (accept(Lexeme::Continue))
	{
		return make<LoopExitStatement>(Lexeme::Continue);
	}
	else if (accept(Lexeme::Assert))
	{
		return parse_assertion();
	}
	else if (accept(Lexeme::Do))
	{
		return parse_statements(true);
	}
	else if (accept(Lexeme::Debug))
	{
		return parse_debug_statement();
	}
	else if (accept(Lexeme::Throw))
	{
		return parse_throw_statement();
	}
	else if (accept(Lexeme::Pass))
	{
		// Stuff this in constant literal to avoid creating a new AST node. This is a no-op anyway.
		return make<ConstantLiteral>(Lexeme::Pass);
	}
	else
	{
		auto stmt = parse_expression_statement();
		CallExpression *e;
		if ((e = dynamic_cast<CallExpression*>(stmt.get())))
		{
			e->discard_result = true;
		}

		return stmt;
	}
}

AutoAst Parser::parse_statements(bool open_scope)
{
	trace_ast();
	AstList block;
	auto line = get_line();
	while (token.is_separator()) { accept(); }

	while (!check(Lexeme::End))
	{
		block.push_back(parse_statement());
		while (token.is_separator()) accept();
	}
	accept(Lexeme::End);

	return std::make_unique<StatementList>(line, std::move(block), open_scope);
}

AutoAst Parser::parse_if_block()
{
	trace_ast();
	AstList block;
	auto line = get_line();
	while (token.is_separator()) { accept(); }

	while (!check(Lexeme::End) && !check(Lexeme::Elsif) && !check(Lexeme::Else))
	{
		block.push_back(parse_statement());
		while (token.is_separator()) accept();
	}
	accept(Lexeme::End);

	return std::make_unique<StatementList>(line, std::move(block), true);
}

AutoAst Parser::parse_print_statement()
{
	trace_ast();
	AstList lst;
	bool add_newline = true;
	lst.push_back(parse_expression());
	while (accept(Lexeme::Comma))
	{
		if (accept(Lexeme::Eol))
		{
			add_newline = false;
			break;
		}
		lst.push_back(parse_expression());
	}

	return make<PrintStatement>(std::move(lst), add_newline);
}

AutoAst Parser::parse_expression_statement()
{
	trace_ast();
	auto e = parse_expression();
	if (accept(Lexeme::OpAssign))
	{
		return make<Assignment>(std::move(e), parse_expression());
	}
	else if (check(Lexeme::OpAssignPlus) || check(Lexeme::OpAssignMinus) || check(Lexeme::OpAssignStar) || check(Lexeme::OpAssignSlash) ||
		check(Lexeme::OpAssignConcat) || check(Lexeme::OpAssignPower) || check(Lexeme::OpAssignMod))
	{
		auto op = token.id;
		accept();

		return make<Assignment>(std::move(e), parse_expression(), op);
	}
	return e;
}

AutoAst Parser::parse_expression()
{
	trace_ast();
	return parse_conditional_expression();
}

AutoAst Parser::parse_declaration()
{
	trace_ast();
	// rhs may be empty if the variable(s) are declared but not assigned.
	AstList lhs, rhs;
	lhs.push_back(parse_identifier("in variable declaration"));

	// Multiple declaration?
	while (accept(Lexeme::Comma))
	{
		lhs.push_back(parse_identifier("in variable declaration"));
	}

	if (accept(Lexeme::OpAssign))
	{
		rhs.push_back(parse_expression());
	}

	while (accept(Lexeme::Comma))
	{
		rhs.push_back(parse_expression());
	}

	if (! rhs.empty() && lhs.size() != rhs.size()) {
		report_error("Invalid declaration: the number of elements on the left hand side and right and side doesn't match");
	}
	if (!token.is_separator() && !token.is(Lexeme::Eot)) {
		report_error("Invalid declaration");
	}

	return make<Declaration>(std::move(lhs), std::move(rhs));
}

AutoAst Parser::parse_identifier(const char *msg)
{
	trace_ast();
	// This may not be an identifier, but we will throw if that's not the case...
	auto ident = token.spelling;
	expect(Lexeme::Identifier, msg);
	// ... now we are sure that ident is indeed an indentifier.

	return make<Variable>(runtime->intern_string(ident));
}

AutoAst Parser::parse_or_expression()
{
	trace_ast();
	auto e = parse_and_expression();

	if (check(Lexeme::Or))
	{
		accept();
		return make<BinaryExpression>(Lexeme::Or, std::move(e), parse_or_expression());
	}

	return e;
}

AutoAst Parser::parse_and_expression()
{
	trace_ast();
	auto e = parse_not_expression();

	if (check(Lexeme::And))
	{
		accept();
		return make<BinaryExpression>(Lexeme::And, std::move(e), parse_and_expression());
	}

	return e;
}

AutoAst Parser::parse_not_expression()
{
	trace_ast();
	if (accept(Lexeme::Not))
	{
		return make<UnaryExpression>(Lexeme::Not, parse_comp_expression());
	}

	return parse_comp_expression();
}

AutoAst Parser::parse_comp_expression()
{
	trace_ast();
	auto e = parse_additive_expression();

	if (check(Lexeme::OpEqual) || check(Lexeme::OpNotEqual) || check(Lexeme::OpGreaterEqual) || check(Lexeme::OpGreaterThan) || check(Lexeme::OpLessEqual)
		|| check(Lexeme::OpLessThan) || check(Lexeme::OpCompare))
	{
		auto op = token.id;
		accept();
		e = make<BinaryExpression>(op, std::move(e), parse_additive_expression());
	}

	return e;
}

AutoAst Parser::parse_additive_expression()
{
	trace_ast();
	auto e = parse_multiplicative_expression();

	if (accept(Lexeme::OpConcat)) {
		return parse_concat_expression(std::move(e));
	}
	while (check(Lexeme::OpPlus) || check(Lexeme::OpMinus))
	{
		auto op = token.id;
		accept();
		e = make<BinaryExpression>(op, std::move(e), parse_multiplicative_expression());
	}

	return e;
}

AutoAst Parser::parse_multiplicative_expression()
{
	trace_ast();
	auto e = parse_signed_expression();

	while (check(Lexeme::OpStar) || check(Lexeme::OpSlash) || check(Lexeme::OpMod))
	{
		auto op = token.id;
		accept();
		e = make<BinaryExpression>(op, std::move(e), parse_signed_expression());
	}

	return e;
}

AutoAst Parser::parse_signed_expression()
{
	trace_ast();
	if (accept(Lexeme::OpMinus))
		return make<UnaryExpression>(Lexeme::OpMinus, parse_exponential_expression());

	return parse_exponential_expression();
}

AutoAst Parser::parse_exponential_expression()
{
	trace_ast();
	auto e = parse_call_expression();

	while (accept(Lexeme::OpPower))
	{
		e = make<BinaryExpression>(Lexeme::OpPower, std::move(e), parse_call_expression());
	}
	return e;
}

AutoAst Parser::parse_call_expression()
{
	trace_ast();
	auto e = parse_ref_expression();

	LOOP:
	if (accept(Lexeme::Dot))
	{
		e = make<BinaryExpression>(Lexeme::Dot, std::move(e), parse_identifier("in dot expression"));
		goto LOOP;
	}
	else if (accept(Lexeme::LSquare))
	{
		AstList lst;
		lst.push_back(parse_expression());
		while (accept(Lexeme::Comma))
		{
			lst.push_back(parse_expression());
		}
		expect(Lexeme::RSquare, "in index");
		e = make<IndexedExpression>(std::move(e), std::move(lst));
		goto LOOP;
	}
	else if (accept(Lexeme::LParen))
	{
		e = make<CallExpression>(std::move(e), parse_arguments());
		goto LOOP;
	}

	return e;
}

AutoAst Parser::parse_ref_expression()
{
	trace_ast();
	if (accept(Lexeme::Ref)) {
		return make<ReferenceExpression>(parse_expression());
	}
	else if (accept(Lexeme::Function)) {
		return parse_function_expression();
	}

	return parse_primary_expression();
}

AutoAst Parser::parse_primary_expression()
{
	trace_ast();
	if (check(Lexeme::Identifier))
	{
		auto name = std::move(token.spelling);
		accept();

		return make<Variable>(std::move(name));
	}
	else if (check(Lexeme::StringLiteral))
	{
		auto s = runtime->intern_string(token.spelling);
		accept();
		return make<StringLiteral>(std::move(s));
	}
	else if (check(Lexeme::IntegerLiteral))
	{
		bool ok;
		intptr_t value = token.spelling.to_int(&ok);
		if (!ok) report_error("Invalid integer");
		accept();
		return make<IntegerLiteral>(value);
	}
	else if (check(Lexeme::FloatLiteral))
	{
		bool ok;
		double value = token.spelling.to_float(&ok);
		if (!ok) report_error("Invalid float number");
		accept();
		return make<FloatLiteral>(value);
	}
	else if (check(Lexeme::True) || check(Lexeme::False) || check(Lexeme::Null) || check(Lexeme::Nan))
	{
		auto value = token.id;
		accept();
		return make<ConstantLiteral>(value);
	}
	else if (accept(Lexeme::LSquare))
	{
		return parse_list_literal();
	}
	else if (accept(Lexeme::OpAt))
	{
		expect(Lexeme::LSquare, "in array literal");
		return parse_array_literal();
	}
	else if (accept(Lexeme::LCurl))
	{
		return parse_table_literal();
	}
	else if (accept(Lexeme::LParen))
	{
		auto e = parse_expression();
		expect(Lexeme::RParen, "in parenthesized expression");
		return e;
	}

	report_error("Invalid primary expression");
	return nullptr;
}

AstList Parser::parse_arguments()
{
	trace_ast();
	AstList args;

	if (accept(Lexeme::RParen))
	{
		return args;
	}
	args.push_back(parse_expression());

	while (accept(Lexeme::Comma))
	{
		args.push_back(parse_expression());
	}
	expect(Lexeme::RParen, "in argument list");

	return args;
}

AstList Parser::parse_parameters()
{
	trace_ast();
	AstList params;

	if (accept(Lexeme::RParen))
	{
		return params;
	}
	params.push_back(parse_parameter());

	while (accept(Lexeme::Comma))
	{
		params.push_back(parse_parameter());
	}
	expect(Lexeme::RParen, "in parameter list");

	return params;
}

AutoAst Parser::parse_parameter()
{
	trace_ast();
	bool by_ref = accept(Lexeme::Ref);
	auto var = parse_identifier("in parameter list");
	AutoAst type;
	if (accept(Lexeme::As)) {
		type = parse_expression();
	}

	return make<RoutineParameter>(std::move(var), std::move(type), by_ref);
}

AutoAst Parser::parse_assertion()
{
	trace_ast();
	auto e = parse_expression();
	AutoAst msg;

	if (accept(Lexeme::Comma))
	{
		msg = parse_expression();
	}

	return make<AssertStatement>(std::move(e), std::move(msg));
}

AutoAst Parser::parse_concat_expression(AutoAst e)
{
	trace_ast();
	AstList lst;
	lst.push_back(std::move(e));
	lst.push_back(parse_multiplicative_expression());

	while (accept(Lexeme::OpConcat))
	{
		skip_empty_lines();
		lst.push_back(parse_multiplicative_expression());
	}

	return make<ConcatExpression>(std::move(lst));
}

AutoAst Parser::parse_if_statement()
{
	trace_ast();
	AstList ifs;
	AutoAst else_block;
	auto line = get_line();
	auto e = parse_expression();
	expect(Lexeme::Then, "in \"if\" statement");
	auto block = parse_if_block();
	ifs.push_back(make<IfCondition>(std::move(e), std::move(block)));

	while (accept(Lexeme::Elsif))
	{
		e = parse_expression();
		expect(Lexeme::Then, "in \"elsif\" condition");
		block = parse_if_block();
		ifs.push_back(make<IfCondition>(std::move(e), std::move(block)));
	}
	if (accept(Lexeme::Else)) {
		else_block = parse_if_block();
	}

	return std::make_unique<IfStatement>(line, std::move(ifs), std::move(else_block));
}

AutoAst Parser::parse_while_statement()
{
	trace_ast();
	auto line = get_line();
	auto e = parse_expression();
	expect(Lexeme::Do, "in while statement");
	auto block = parse_statements(true);

	return std::make_unique<WhileStatement>(line, std::move(e), std::move(block));
}


AutoAst Parser::parse_repeat_statement()
{
	trace_ast();
	auto line = get_line();
	AstList block;
	skip_empty_lines();
	while (!accept(Lexeme::Until))
	{
		block.push_back(parse_statement());
		while (token.is_separator()) accept();
	}
	// The compiler we create the scope so that the until condition is in the same scope as the block.
	auto body = std::make_unique<StatementList>(line, std::move(block), false);
	auto cond = parse_expression();

	return std::make_unique<RepeatStatement>(line, std::move(cond), std::move(body));
}

AutoAst Parser::parse_for_statement()
{
	trace_ast();
	constexpr const char *hint = "in for loop";
	auto line = get_line();
	AutoAst e1, e2, e3;
	bool down = false;
	auto var = parse_identifier(hint);
	expect(Lexeme::OpAssign, hint);
	e1 = parse_expression();

	if (accept(Lexeme::To))
	{
		e2 = parse_expression();
	}
	else if (accept(Lexeme::Downto))
	{
		e2 = parse_expression();
		down = true;
	}
	else
	{
		report_error("Expected \"to\" or \"downto\" in for loop");
	}

	if (accept(Lexeme::Step)) {
		e3 = parse_expression();
	}
	expect(Lexeme::Do, hint);
	// Don't open a scope for the block: we will open it ourselves so that we can include the loop variable in it.
	auto block = parse_statements(false);

	return std::make_unique<ForStatement>(line, std::move(var), std::move(e1), std::move(e2), std::move(e3), std::move(block), down);
}

AutoAst Parser::parse_foreach_statement()
{
	trace_ast();
	constexpr const char *hint = "in foreach loop";
	auto line = get_line();
	AutoAst key;
	if (accept(Lexeme::Ref)) {
		key = make<ReferenceExpression>(parse_identifier(hint));
	}
	else {
		key = parse_identifier(hint);
	}
	AutoAst val;

	if (accept(Lexeme::Comma))
	{
		if (accept(Lexeme::Ref)) {
			val = make<ReferenceExpression>(parse_identifier(hint));
		}
		else {
			val = parse_identifier(hint);
		}
	}
	else
	{
		std::swap(key, val);
	}
	if (dynamic_cast<ReferenceExpression*>(key.get())) {
		report_error("Key in \"foreach\" loop cannot be grabbed by reference");
	}
	expect(Lexeme::In, hint);
	auto coll = parse_expression();
	// We need a reference, which will be grabbed by the iterator.
	if (!dynamic_cast<ReferenceExpression*>(coll.get())) {
		coll = make<ReferenceExpression>(std::move(coll));
	}
	expect(Lexeme::Do, hint);
	// Don't open a scope for the block: we will open it ourselves so that we can include the loop variable in it.
	auto block = parse_statements(false);

	return std::make_unique<ForeachStatement>(line, std::move(key), std::move(val), std::move(coll), std::move(block));
}

AutoAst Parser::parse_conditional_expression()
{
	trace_ast();
	auto e = parse_or_expression();
	if (accept(Lexeme::If))
	{

		AstList if_cond;
		if_cond.push_back(make<IfCondition>(parse_expression(), std::move(e)));
		expect(Lexeme::Else, "in conditional expression");
		auto else_block = parse_expression();
		e = make<IfStatement>(std::move(if_cond), std::move(else_block));
	}

	return e;
}

AutoAst Parser::parse_function_declaration(bool local)
{
	trace_ast();
	int line = get_line();
	constexpr const char *hint = "in function declaration";
	auto name = parse_identifier(hint);
	expect(Lexeme::LParen, hint);
	auto params = parse_parameters();
	// Don't open a scope for the block: the function will do it so that we include the parameters in the scope.
	auto body = parse_statements(false);

	return std::make_unique<RoutineDefinition>(line, std::move(name), std::move(params), std::move(body), local, false);
}


AutoAst Parser::parse_function_expression()
{
	trace_ast();
	int line = get_line();
	constexpr const char *hint = "in function expression";
	expect(Lexeme::LParen, hint);
	auto params = parse_parameters();
	// Don't open a scope for the block: the function will do it so that we include the parameters in the scope.
	auto body = parse_statements(false);

	return std::make_unique<RoutineDefinition>(line, nullptr, std::move(params), std::move(body), true, false);
}

AutoAst Parser::parse_return_statement()
{
	trace_ast();
	AutoAst e;

	if (!token.is_separator())
	{
		e = parse_expression();
	}

	return make<ReturnStatement>(std::move(e));
}

AutoAst Parser::parse_member_expression()
{
	trace_ast();
	auto e = parse_ref_expression();

	while (accept(Lexeme::Dot))
	{
		e = make<BinaryExpression>(Lexeme::Dot, std::move(e), parse_identifier("in member expression"));
	}

	return e;
}

AutoAst Parser::parse_list_literal()
{
	trace_ast();
	auto line = get_line();
	skip_empty_lines();
	if (accept(Lexeme::RSquare)) {
		return make<ListLiteral>(AstList());
	}
	AstList items;
	items.push_back(parse_expression());
	skip_empty_lines();

	while (accept(Lexeme::Comma))
	{
		skip_empty_lines();
		items.push_back(parse_expression());
	}
	skip_empty_lines();
	expect(Lexeme::RSquare, "at the end of list or array literal");

	return std::make_unique<ListLiteral>(line, std::move(items));
}

AutoAst Parser::parse_array_literal()
{
	intptr_t prev_ncol = -1;
	intptr_t ncol = 1;
	intptr_t nrow = 1;

	auto line = get_line();
	skip_empty_lines();
	if (accept(Lexeme::RSquare)) {
		return make<ArrayLiteral>(AstList(), 0, 0);
	}
	AstList items;
	items.push_back(parse_expression());
	skip_empty_lines();

	while (token.is(Lexeme::Comma) || token.is(Lexeme::Semicolon))
	{
		if (token.is(Lexeme::Semicolon))
		{
			if (ncol != prev_ncol && prev_ncol != -1)
			{
				report_error("[Syntax error] inconsistent number of rows in array declaration");
			}
			else
			{
				nrow++;
				prev_ncol = ncol;
				ncol = 0;
			}
		}
		accept();
		skip_empty_lines();
		items.push_back(parse_expression());
		ncol++;
		skip_empty_lines();
	}
	expect(Lexeme::RSquare, "in array literal");

	return std::make_unique<ArrayLiteral>(line, std::move(items), nrow, ncol);
}

AutoAst Parser::parse_table_literal()
{
	trace_ast();
	constexpr const char *hint = "in table literal";
	auto line = get_line();
	skip_empty_lines();
	if (accept(Lexeme::RCurl)) {
		return make<TableLiteral>(AstList(), AstList());
	}
	AstList keys, values;
	keys.push_back(parse_expression());

	if (accept(Lexeme::Colon))
	{
		values.push_back(parse_expression());

		while (accept(Lexeme::Comma))
		{
			skip_empty_lines();
			keys.push_back(parse_expression());
			expect(Lexeme::Colon, hint);
			values.push_back(parse_expression());
		}
		skip_empty_lines();
		expect(Lexeme::RCurl, hint);

		return std::make_unique<TableLiteral>(line, std::move(keys), std::move(values));
	}
	else
	{
		while (accept(Lexeme::Comma))
		{
			skip_empty_lines();
			keys.push_back(parse_expression());
		}
		skip_empty_lines();
		expect(Lexeme::RCurl, "in set literal");

		return std::make_unique<SetLiteral>(line, std::move(keys));
	}
}

void Parser::parse_option()
{
	trace_ast();

	if (!check(Lexeme::Debug))
	{
		auto msg = utils::format("Invalid option: expected \"debug\", got %", token.spelling);
		report_error(msg);
	}
	auto option = token.id;
	accept();
	bool value = true;
	if (accept(Lexeme::OpAssign))
	{
		if (accept(Lexeme::True)) {
			// pass
		}
		else if (accept(Lexeme::False)) {
			value = false;
		}
		else {
			report_error("Option value should be \"true\" (default) or \"false\"");
		}
	}
	skip_empty_lines();

	if (option == Lexeme::Debug) {
		runtime->set_debug_mode(value);
	}
	else {
		report_error("Internal error", "Invalid option");
	}
}

AutoAst Parser::parse_debug_statement()
{
	trace_ast();
	auto line = get_line();
	AutoAst body = accept(Lexeme::Eol) ? parse_statements(true) : parse_statement();

	return std::make_unique<DebugStatement>(line, std::move(body));
}

AutoAst Parser::parse_throw_statement()
{
	trace_ast();
	return make<ThrowStatement>(parse_expression());
}

} // namespace phonometrica

#undef trace_ast