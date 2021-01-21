/***********************************************************************************************************************
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
 * Created: 22/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: Function object. Functions support multiple dispatch: a function may have several overloads, and the       *
 * correct routine is selected at runtime based on the number and types of arguments.                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNCTION_HPP
#define PHONOMETRICA_FUNCTION_HPP

#include <optional>
#include <vector>
#include <phon/runtime/code.hpp>
#include <phon/runtime/callback.hpp>

namespace phonometrica {

class Runtime;
class Function;
class Class;

//---------------------------------------------------------------------------------------------------------------------


// A Callable is an internal abstract base type used to represent one particular signature for a function. Each function has at least
// one callable, and each callable is owned by at least one function. Callable has two subclasses: NativeRoutine, which is implemented in C++,
// and Routine, which a user-defined function. Callables are an implementation detail and are not visible to users.
class Callable
{
public:

	explicit Callable(const String &name) : _name(name) { } // routine without parameters

	Callable(const String &name, std::vector<Handle<Class>> sig, ParamBitset ref_flags);

	virtual ~Callable() = default;

	virtual bool is_native() const = 0;

	int arg_count() const { return int(signature.size()); }

	bool check_ref(ParamBitset ref) { return ref_flags == ref; }

	void add_parameter_type(Handle<Class> cls) { signature.push_back(std::move(cls)); }

	String name() const { return _name; }

	int get_cost(std::span<Variant> args) const;

	String get_definition() const;

	virtual int upvalue_count() const { return 0; }

protected:

	friend class Function;
	friend class Closure;

	// Type of positional arguments.
	std::vector<Handle<Class>> signature;

	// Indicates whether a parameter is a reference (1) or a value (0).
	ParamBitset ref_flags;

	// For debugging and stack traces.
	String _name;
};


//----------------------------------------------------------------------------------------------------------------------

// A routine implemented in C++.
struct NativeRoutine final : public Callable
{
	NativeRoutine(const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref_flags = 0);

	bool is_native() const override { return true; }

	NativeCallback callback;

	Variant operator()(Runtime &rt, std::span<Variant> args);
};


//----------------------------------------------------------------------------------------------------------------------

// A user-defined routine.
class Routine final : public Callable
{
public:

	struct Local
	{
		String name;
		int scope, depth;
	};

	// Represents a non-local variable referenced by an inner function.
	struct Upvalue
	{
		// Index of the variable in the surrounding function.
		Instruction index;

		// If true, the captured upvalue is a local variable in the surrounding function; otherwise it is itself an upvalue
		// that references a local or another upvalue in its surrounding function. All non-local upvalues eventually resolve to
		// a local one.
		bool is_local;

		bool operator==(const Upvalue &other) noexcept { return this->index == other.index && this->is_local == other.is_local; }
	};

	Routine(Routine *parent, const String &name);

	Routine(Routine *parent, const String &name, std::vector<Handle<Class>> sig, ParamBitset ref_flags);

	bool is_native() const override { return false; }

	Instruction add_integer_constant(intptr_t i);

	Instruction add_float_constant(double n);

	Instruction add_string_constant(String s);

	Instruction add_routine(std::shared_ptr<Routine> r);

	Instruction add_local(const String &name, int scope, int depth);

	std::optional<Instruction> find_local(const String &name, int scope_depth) const;

	std::optional<Instruction> resolve_upvalue(const String &name, int scope_depth);

	double get_float(intptr_t i) const { return float_pool[i]; }

	intptr_t get_integer(intptr_t i) const { return integer_pool[i]; }

	String get_string(intptr_t i) const { return string_pool[i]; }

	std::shared_ptr<Routine> get_routine(intptr_t i) const { return routine_pool[i]; }

	String get_local_name(intptr_t i) const { return locals[i].name; }

	int local_count() const;

	bool sealed() const { return is_sealed; }

	int upvalue_count() const override { return int(upvalues.size()); }

private:

	friend class Runtime;
	friend class Compiler;
	friend class Closure;

	// Bytecode.
	Code code;

	void seal() { is_sealed = true; }

	Instruction add_upvalue(Instruction index, bool local);

	template<class T>
	Instruction add_constant(std::vector<T> &vec, T value)
	{
		auto it = std::find(vec.begin(), vec.end(), value);

		if (it == vec.end())
		{
			if (unlikely(vec.size() == (std::numeric_limits<Instruction>::max)())) {
				throw error("Maximum number of constants exceeded");
			}
			vec.push_back(std::move(value));
			return Instruction(vec.size() - 1);
		}

		return Instruction(std::distance(vec.begin(), it));
	}

	// Constant pools.
	std::vector<double> float_pool;
	std::vector<intptr_t> integer_pool;
	std::vector<String> string_pool;
	std::vector<std::shared_ptr<Routine>> routine_pool;

	// Local variables.
	std::vector<Local> locals;

	std::vector<Upvalue> upvalues;

	// Enclosing routine (this is used to find upvalues).
	Routine *parent = nullptr;

	// We define the signature once at runtime and seal the routine.
	bool is_sealed = false;
};

//----------------------------------------------------------------------------------------------------------------------

// Instantiation of a Callable that captures over its environment.
class Closure final
{
public:

	explicit Closure(std::shared_ptr<Callable> r) : routine(std::move(r)) { }

	void traverse(const GCCallback &callback);

private:

	friend class Runtime;
	friend class Function;

	static void traverse(Routine &r, const GCCallback &callback);

	std::shared_ptr<Callable> routine;

	std::vector<Variant> upvalues;
};


//----------------------------------------------------------------------------------------------------------------------

// A Function object (also known as 'generic function' in languages with multiple dispatch).
// Functions are first-class objects. They can have several signatures, each of which is represented by a routine.
// Whenever a function is called, the appropriate routine is selected based on the number and type of the arguments.
// Note that all functions are wrapped in a Closure, which may optionally capture the function's lexical environment.
// However, this is an implementation detail and from the user's perspective functions and closures are indistinguishable.
class Function final
{
public:

	explicit Function(String name) : _name(std::move(name)) { }

	Function(const Function &) = delete;

	Function(Function &&) noexcept = default;

	Function(String name, Handle<Closure> c);

	Function(Runtime *rt, const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref_flags = 0);

	String name() const { return _name.empty() ? "<anonymous>" : _name; }

	void add_closure(Handle<Closure> c);

	Handle<Closure> find_closure(std::span<Variant> args);

	ParamBitset reference_flags() const { return ref_flags; }

	void traverse(const GCCallback &callback);

private:

	friend class Variant;
	friend class Runtime;

	// Name provided when the function was declared. Anonymous functions don't have a name.
	String _name;

	// Each function signature is represented by a different routine, which may be native or user-defined.
	std::vector<Handle<Closure>> closures;

	ParamBitset ref_flags;

	// Maximum number of arguments that this function allows.
	int max_argc = 0;
};


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

namespace meta {

static inline
String to_string(const Function &f)
{
	return String::format("<function %s at %p>", f.name().data(), std::addressof(f));
}

static inline
String to_string(const Callable &c)
{
	return String::format("<function %s at %p>", c.name().data(), std::addressof(c));
}

static inline
void traverse(Function &f, const GCCallback &callback)
{
	f.traverse(callback);
}

static inline
void traverse(Closure &c, const GCCallback &callback)
{
	c.traverse(callback);
}

} // namespace phonometrica::meta


} // namespace phonometrica

#endif // PHONOMETRICA_FUNCTION_HPP
