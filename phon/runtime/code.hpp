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
 * Created: 24/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: bytecode object, which represents a chunk of compiled code.                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CODE_HPP
#define PHONOMETRICA_CODE_HPP

#include <algorithm>
#include <utility>
#include <vector>
#include <phon/runtime/definitions.hpp>

namespace phonometrica {

using Instruction = uint16_t;



enum class Opcode : Instruction
{
	Assert,
	Add,
	Call,
	ClearLocal,
	Compare,
	Concat,
	DecrementLocal,
	DefineLocal,
	GetField,			// Get field by value
	GetFieldArg,		// Get field by value or by reference
	GetFieldRef,		// Get field by reference
	GetGlobal,			// Get global by value
	GetGlobalArg,		// Get global either by value or by reference
	GetGlobalRef,		// Get global by reference
	GetIndex,			// Get index by value
	GetIndexArg,		// Get index by value or by reference
	GetIndexRef,		// Get index by reference
	GetLocal,			// Get global by value
	GetLocalArg,		// Get global either by value or by reference
	GetLocalRef,		// Get global by reference
	GetUniqueGlobal,	// Unshare and push a global
	GetUniqueLocal,		// Unshare and push a local
	GetUniqueUpvalue,	// Unshare and push a non-local variable
	GetUpvalue,			// Get non-local variable by value
	GetUpvalueArg,		// Get non-local variable by value or by reference
	GetUpvalueRef,		// Get non-local variable by reference
	Divide,
	Equal,
	Greater,
	GreaterEqual,
	IncrementLocal,
	Jump,
	JumpFalse,
	JumpFalseAnd,
	JumpTrue,
	JumpTrueOr,
	Less,
	LessEqual,
	Modulus,
	Multiply,
	Negate,
	NewArray,
	NewClosure,
	NewFrame,
	NewIterator,
	NewList,
	NewSet,
	NewTable,
	NextKey,
	NextValue,
	Not,
	NotEqual,
	Pop,
	Power,
	Precall,
	Print,
	PrintLine,
	PushBoolean,
	PushFalse,
	PushFloat,
	PushInteger,
	PushNan,
	PushNull,
	PushSmallInt,
	PushString,
	PushTrue,
	Return,
	SetField,
	SetGlobal,
	SetIndex,
	SetLocal,
	SetUpvalue,
	Subtract,
	TestIterator,
	Throw
};


class Code final
{
	using Storage = std::vector<Instruction>;

	// For error reporting.
	using LineNo = uint16_t;

public:

	// Simple trick to convert back and forth between an int and bytecodes.
	union IntSerializer
	{
		IntSerializer() = default;
		IntSerializer(int i) : value(i) { }
		static constexpr size_t IntSize = sizeof(int) / sizeof(Instruction);
		int value;
		Instruction ins[IntSize];
	};

	Code() = default;

	Code(const Code &) = delete;

	Code(Code &&) = default;

	~Code() = default;

	void append(intptr_t line_no, Instruction i) { add_line(line_no); code.push_back(i); }

	void append(intptr_t line_no, Opcode op) { append(line_no, static_cast<Instruction>(op)); }

	void append(intptr_t line_no, Opcode op, Instruction i) { append(line_no, op); append(line_no, i); }

	void append(intptr_t line_no, Opcode op, Instruction i1, Instruction i2) { append(line_no, op); append(line_no, i1); append(line_no, i2); }

	static int read_integer(const Instruction *&ip);

	void append_return();

	const Instruction *data() const { return code.data(); }

	const Instruction *end() const { return code.data() + code.size(); }

	const Instruction &operator[](size_t i) const { return code[i]; }

	size_t size() const { return code.size(); }

	int get_line(int offset) const;

	void backpatch_instruction(int at, Instruction value);

	void backpatch(int at);

	void backpatch(int at, int value);

	int append_jump(intptr_t line_no, Opcode jmp);

	int append_jump(intptr_t line_no, Opcode jmp, int addr);

	int get_current_offset() const { return int(code.size()); }

	static const char *get_opcode_name(Instruction op);

private:

	void add_line(intptr_t line_no);

	// Byte codes.
	Storage code;

	// Line numbers on which byte codes are found, for error reporting.
	// first = line number; second = number of instructions on that line
	std::vector<std::pair<LineNo,LineNo>> lines;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CODE_HPP
