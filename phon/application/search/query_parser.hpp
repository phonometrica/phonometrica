/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 10/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Parse a stream of token (produced by the query editor) into a abstract syntax tree which can be consumed   *
 * by a query object.                                                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_PARSER_HPP
#define PHONOMETRICA_QUERY_PARSER_HPP

#include <phon/array.hpp>
#include <phon/application/search/search_node.hpp>

namespace phonometrica {

class QueryParser final
{
public:

	enum class Type
	{
		Null,
		LParen,
		Rparen,
		And,
		Or,
		Not,
		Constraint
	};

	struct Token
	{
		Type type;
		AutoSearchNode node;
	};

	explicit QueryParser(Array<Token> tokens) : tokens(std::move(tokens)) { }

	~QueryParser() = default;

	AutoSearchNode parse();

private:

	/*
	 * Grammar:
	 * expression := or_expression
	 * or_expression := and_expression ( 'OR' and_expression ) *
	 * and_expression := primary ( 'AND' primary ) *
	 * primary = '(' expression ')' | constraint | 'NOT' primary
	 */

	AutoSearchNode parse_expression();

	AutoSearchNode parse_or_expression();

	AutoSearchNode parse_and_expression();

	AutoSearchNode parse_primary();

	Token *nextToken();

	void accept() { readToken(); }

	void accept(Type t, const char *msg);

	void readToken() { the_token = nextToken(); }

	Array<Token> tokens;

	Token *the_token = nullptr;

	int token_index = 0;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_PARSER_HPP
