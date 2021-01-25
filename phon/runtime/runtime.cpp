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
 * Created: 22/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cfenv>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <phon/runtime/runtime.hpp>
#include <phon/regex.hpp>
#include <phon/file.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/utils/file_system.hpp>


#define CATCH_ERROR catch (std::runtime_error &e) { RUNTIME_ERROR(e.what()); }
#define RUNTIME_ERROR(...) throw RuntimeError(get_current_line(), __VA_ARGS__)

#if 0
#	define trace_op() std::cerr << std::setw(6) << std::left << (ip-1-code->data()) << "\t" << std::setw(15) << Code::get_opcode_name(*(ip-1)) << "stack size = " << intptr_t(top - stack.data()) << std::endl;
#else
#	define trace_op()
#endif


namespace phonometrica {

bool Runtime::initialized = false;

Runtime::Runtime(String prog_path, intptr_t stack_size) :
		print([](const String &s) { utils::printf(s); }), stack(stack_size, Variant()), parser(this), compiler(this),
		prog_path(std::move(prog_path)), get_item_string(intern_string("get_item")), set_item_string(intern_string("set_item")),
		get_field_string(intern_string("get_field")), set_field_string(intern_string("set_field")),
		length_string(intern_string("length"))
{
	srand(time(nullptr));

	if (! initialized)
	{
		utils::init_random_seed();
		Token::initialize();
		initialized = true;
	}

	create_builtins();
	set_global_namespace();
	this->top = this->stack.begin();
	this->limit = this->stack.end();
}

phonometrica::Runtime::~Runtime()
{
	// Make sure we don't double free variants that have been destructed but are not null.
	for (auto var = top; var < stack.end(); var++) {
		new (var) Variant;
	}
	stack.clear();
	globals.drop()->release();

	// Finalize classes manually: this is necessary because we must finalize Class last.
	for (auto &cls : classes) {
		cls->finalize();
	}
	collect();

	for (size_t i = classes.size(); i-- > 2; )
	{
		auto ptr = classes[i].drop();
		ptr->release();
	}
	classes[0].drop()->release();
	classes[1].drop()->release();
}

void Runtime::add_candidate(Collectable *obj)
{
	if (is_full()) {
		collect();
	}
	assert(obj->previous == nullptr);

	// obj becomes the new root
	auto *old_root = gc_root;
	obj->next = old_root;

	if (old_root != nullptr) {
		old_root->previous = obj;
	}
	gc_root = obj;
	gc_count++;
}

void Runtime::remove_candidate(Collectable *obj)
{
	if (obj == gc_root)
	{
		assert(obj->previous == nullptr);
		gc_root = obj->next;
		if (gc_root) {
			gc_root->previous = nullptr;
		}
		obj->next = nullptr;
	}
	else
	{
		if (obj->previous != nullptr)
		{
			obj->previous->next = obj->next;
		}
		if (obj->next != nullptr)
		{
			obj->next->previous = obj->previous;
		}
		obj->previous = nullptr;
		obj->next = nullptr;
	}
	gc_count--;
}

void Runtime::create_builtins()
{
	// We need to boostrap the class system, since we are creating class instances but the class type doesn't exist
	// yet. We can't create it first because it inherits from Object.
	auto object_class = create_type<Object>("Object", nullptr, Class::Index::Object);
	auto raw_object_class = object_class.get();

	auto class_class = create_type<Class>("Class", raw_object_class, Class::Index::Class);
	assert(class_class->inherits(raw_object_class));

	assert(object_class.object()->get_class() == nullptr);
	assert(class_class.object()->get_class() == nullptr);

	object_class.object()->set_class(class_class.get());
	class_class.object()->set_class(class_class.get());

	// Create other builtin types.
	auto null_type = create_type<std::nullptr_t>("Null", raw_object_class, Class::Index::Null);
	auto bool_class = create_type<bool>("Boolean", raw_object_class, Class::Index::Boolean);
	auto num_class = create_type<Number>("Number", raw_object_class, Class::Index::Number);
	auto int_class = create_type<intptr_t>("Integer", num_class.get(), Class::Index::Integer);
	auto float_class = create_type<double>("Float", num_class.get(), Class::Index::Float);
	auto string_class = create_type<String>("String", raw_object_class, Class::Index::String);
	auto regex_class = create_type<Regex>("Regex", raw_object_class, Class::Index::Regex);
	auto list_class = create_type<List>("List", raw_object_class, Class::Index::List);
	auto array_class = create_type<Array<double>>("Array", raw_object_class, Class::Index::Array);
	auto table_class = create_type<Table>("Table", raw_object_class, Class::Index::Table);
	auto file_class = create_type<File>("File", raw_object_class, Class::Index::File);
	auto module_class = create_type<Module>("Module", raw_object_class, Class::Index::Module);
	// Function and Closure have the same name because the difference is an implementation detail.
	auto func_class = create_type<Function>("Function", raw_object_class, Class::Index::Function);
	create_type<Closure>("Function", raw_object_class, Class::Index::Closure);
	auto set_class = create_type<Set>("Set", raw_object_class, Class::Index::Set);

	// Iterators are currently not exposed to users.
	create_type<Iterator>("Iterator", raw_object_class, Class::Index::Iterator);
	create_type<ListIterator>("Iterator", raw_object_class, Class::Index::ListIterator);
	create_type<TableIterator>("Iterator", raw_object_class, Class::Index::TableIterator);
	create_type<StringIterator>("Iterator", raw_object_class, Class::Index::StringIterator);
	create_type<FileIterator>("Iterator", raw_object_class, Class::Index::FileIterator);
	create_type<RegexIterator>("Iterator", raw_object_class, Class::Index::RegexIterator);

	// Sanity checks
	assert(object_class.object()->get_class() != nullptr);
	assert(class_class.object()->get_class() != nullptr);
	assert((Class::get<Class>()) != nullptr);

	globals = make_handle<Module>(this, "global");

#define GLOB(T, h) add_global(Class::get_name<T>(), std::move(h));
	GLOB(Object, object_class);
	GLOB(Class, class_class);
	GLOB(bool, bool_class);
	GLOB(Number, num_class);
	GLOB(intptr_t, int_class);
	GLOB(double, float_class);
	GLOB(String, string_class);
	GLOB(Regex, regex_class);
	GLOB(List, list_class);
	GLOB(Array<double>, array_class);
	GLOB(Table, table_class);
	GLOB(File, file_class);
	GLOB(Function, func_class);
	GLOB(Module, module_class);
	GLOB(Set, set_class);
#undef GLOB
}

void Runtime::push_null()
{
	new(var()) Variant();
}

Variant &Runtime::push()
{
	return *(new(var()) Variant());
}

void Runtime::push(double n)
{
	new(var()) Variant(n);
}

void Runtime::push_int(intptr_t n)
{
	new(var()) Variant(n);
}

void Runtime::push(bool b)
{
	new(var()) Variant(b);
}

void Runtime::push(const Variant &v)
{
	new(var()) Variant(v);
}

void Runtime::push(Variant &&v)
{
	new(var()) Variant(std::move(v));
}

void Runtime::push(String s)
{
	new(var()) Variant(std::move(s));
}

Variant *Runtime::var()
{
	check_capacity();
	return this->top++;
}

void Runtime::check_capacity()
{
	if (this->top == this->limit) {
		throw error("[Runtime error] Stack overflow.");
	}
}

void Runtime::ensure_capacity(int n)
{
	if (top + n >= limit) {
		throw error("[Runtime error] Stack overflow.");
	}
}

void Runtime::check_underflow()
{
	if (this->top == this->stack.begin())
	{
		RUNTIME_ERROR("[Internal error] Stack underflow");
	}
}

void Runtime::pop(int n)
{
	auto limit = top - n;

	if (unlikely(limit < stack.begin()))
	{
		// Clean up stack
		while (top > stack.begin())
		{
			(--top)->~Variant();
		}
		RUNTIME_ERROR("[Internal error] Stack underflow");
	}

	while (top > limit)
	{
		(--top)->~Variant();
	}
}

Variant & Runtime::peek(int n)
{
	return *(top + n);
}

void Runtime::negate()
{
	auto &var = peek();

	if (var.is_integer())
	{
		intptr_t value = -raw_cast<intptr_t>(var);
		pop();
		push_int(value);
	}
	else if (var.is_float())
	{
		double value = -raw_cast<double>(var);
		pop();
		push(value);
	}
	else
	{
		throw error("[Type error] Negation operator expected a Number, got a %", var.class_name());
	}
}

void Runtime::math_op(char op)
{
	auto &v1 = peek(-2).resolve();
	auto &v2 = peek(-1).resolve();
	std::feclearexcept(FE_ALL_EXCEPT);

	if (v1.is_number() && v2.is_number())
	{
		switch (op)
		{
			case '+':
			{
				if (v1.is_integer() && v2.is_integer())
				{
					auto x = cast<intptr_t>(v1);
					auto y = cast<intptr_t>(v2);
					pop(2);
					if ((x < 0.0) == (y < 0.0) && std::abs(y) > (std::numeric_limits<intptr_t>::max)() - std::abs(x)) {
						RUNTIME_ERROR("[Math error] Integer overflow");
					}
					push_int(x+y);
				}
				else
				{
					auto x = v1.get_number();
					auto y = v2.get_number();
					pop(2);
					auto result = x + y;
					check_float_error();
					push(result);
				}
				return;
			}
			case '-':
			{
				if (v1.is_integer() && v2.is_integer())
				{
					auto x = cast<intptr_t>(v1);
					auto y = cast<intptr_t>(v2);
					pop(2);
					push_int(x - y);
				}
				else
				{
					auto x = v1.get_number();
					auto y = v2.get_number();
					pop(2);
					auto result = x - y;
					check_float_error();
					push(result);
				}
				return;
			}
			case '*':
			{
				if (v1.is_integer() && v2.is_integer())
				{
					auto x = cast<intptr_t>(v1);
					auto y = cast<intptr_t>(v2);
					pop(2);
					push_int(x * y);
				}
				else
				{
					auto x = v1.get_number();
					auto y = v2.get_number();
					pop(2);
					auto result = x * y;
					check_float_error();
					push(result);
				}
				return;
			}
			case '/':
			{
				auto x = v1.get_number();
				auto y = v2.get_number();
				pop(2);
				auto result = x / y;
				check_float_error();
				push(result);
				return;
			}
			case '^':
			{
				auto x = v1.get_number();
				auto y = v2.get_number();
				pop(2);
				auto result = pow(x, y);
				check_float_error();
				push(result);
				return;
			}
			case '%':
			{
				if (v1.is_integer() && v2.is_integer())
				{
					auto x = cast<intptr_t>(v1);
					auto y = cast<intptr_t>(v2);
					pop(2);
					push_int(x % y);
				}
				else
				{
					auto x = v1.get_number();
					auto y = v2.get_number();
					pop(2);
					push(std::fmod(x, y));
				}
				return;
			}
			default:
				break;
		}
	}

	pop(2);
	char opstring[2] = { op, '\0' };
	RUNTIME_ERROR("[Type error] Cannot apply math operator '%' to % and %", opstring, v1.class_name(), v2.class_name());
}

void Runtime::check_float_error()
{
	if (fetestexcept(FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO | FE_INVALID))
	{
		if (fetestexcept(FE_OVERFLOW)) {
			throw error("[Math error] Number overflow");
		}
		if (fetestexcept(FE_UNDERFLOW)) {
			throw error("[Math error] Number underflow");
		}
		if (fetestexcept(FE_DIVBYZERO)) {
			throw error("[Math error] Division by zero");
		}
		if (fetestexcept(FE_INVALID)) {
			throw error("[Math error] Undefined number");
		}
	}
}

Variant Runtime::interpret(Handle <Closure> &closure)
{
	if (current_frame) {
		current_frame->previous_routine = current_routine;
	}
	assert(!closure->routine->is_native());
	Routine &routine = *(reinterpret_cast<Routine*>(closure->routine.get()));
	current_routine = &routine;
	code = &routine.code;
	auto old_ip = ip;
	ip = routine.code.data();

	while (true)
	{
		auto op = static_cast<Opcode>(*ip++);

		switch (op)
		{
			case Opcode::Add:
			{
				trace_op();
				math_op('+');
				break;
			}
			case Opcode::Assert:
			{
				trace_op();
				int narg = *ip++;
				bool value = peek(-narg).to_boolean();
				if (!value)
				{
					auto msg = (narg == 2) ? utils::format("Assertion failed: %", peek(-1).to_string()) : std::string("Assertion failed");
					RUNTIME_ERROR(msg);
				}
				break;
			}
			case Opcode::Call:
			{
				trace_op();
				Instruction flags = *ip++;
				// TODO: handle return by reference
				needs_ref = flags & (1<<9);
				int narg = flags & 255;

				auto &v = peek(-narg - 1);
				// Precall already checked that we have a function object.
				auto &func = raw_cast<Function>(v);
				std::span<Variant> args(top - narg, narg);

				try 
				{
					auto c = func.find_closure(args);
					if (!c) {
						report_call_error(func, args);
					}

					if (c->routine->is_native())
					{
						try
						{
							auto &r = reinterpret_cast<NativeRoutine&>(*(c->routine));
							auto result = r(*this, args);
							pop(narg + 1);
							push(std::move(result));
						}
						CATCH_ERROR
					}
					else
					{
						// The arguments are on top of the stack. We adjust the top of the stack accordingly.
						top -= narg;
						current_frame->ip = ip;
						push(interpret(c));
					}
				}
				CATCH_ERROR
				needs_ref = false;
				break;
			}
			case Opcode::ClearLocal:
			{
				trace_op();
				auto &v = current_frame->locals[*ip++];
				v.clear();
				break;
			}
			case Opcode::Compare:
			{
				trace_op();
				auto &v1 = peek(-2);
				auto &v2 = peek(-1);
				int result = v1.compare(v2);
				pop(2);
				push_int(result);
				break;
			}
			case Opcode::Concat:
			{
				trace_op();
				int narg = *ip++;
				String s;
				for (int i = narg; i > 0; i--) {
					s.append(peek(-i).to_string());
				}
				pop(narg);
				push(std::move(s));
				break;
			}
			case Opcode::DecrementLocal:
			{
				trace_op();
				int index = *ip++;
				auto &v = current_frame->locals[index];
				assert(v.is_integer());
				raw_cast<intptr_t>(v)--;
				break;
			}
			case Opcode::DefineLocal:
			{
				trace_op();
				Variant &local = current_frame->locals[*ip++];
				local = std::move(peek());
				pop();
				break;
			}
			case Opcode::Divide:
			{
				trace_op();
				math_op('/');
				break;
			}
			case Opcode::Equal:
			{
				trace_op();
				auto &v2 = peek(-1);
				auto &v1 = peek(-2);
				bool value = (v1 == v2);
				pop(2);
				push(value);
				break;
			}
			case Opcode::GetField:
			{
				trace_op();
				get_field(false);
				break;
			}
			case Opcode::GetFieldArg:
			{
				trace_op();
				bool by_ref = current_frame->ref_flags[*ip++];
				if (by_ref) {
					RUNTIME_ERROR("Passing dotted expression as an argument by reference is not yet supported");
				}
				get_field(by_ref);
				break;
			}
			case Opcode::GetFieldRef:
			{
				trace_op();
				get_field(true);
				break;
			}
			case Opcode::GetGlobal:
			{
				trace_op();
				auto name = routine.get_string(*ip++);
				auto it = globals->find(name);
				if (it == globals->end()) {
					RUNTIME_ERROR("[Symbol error] Undefined variable \"%\"", name);
				}
				push(it->second.resolve());
				break;
			}
			case Opcode::GetGlobalArg:
			{
				trace_op();
				auto name = routine.get_string(*ip++);
				bool by_ref = current_frame->ref_flags[*ip++];
				auto it = globals->find(name);
				if (it == globals->end()) {
					RUNTIME_ERROR("[Symbol error] Undefined variable \"%\"", name);
				}
				if (by_ref)
				{
					it->second.unshare();
					push(it->second.make_alias());
				}
				else
				{
					push(it->second.resolve());
				}
				break;
			}
			case Opcode::GetGlobalRef:
			{
				trace_op();
				auto name = routine.get_string(*ip++);
				auto it = globals->find(name);
				if (it == globals->end()) {
					RUNTIME_ERROR("[Symbol error] Undefined variable \"%\"", name);
				}
				it->second.unshare();
				push(it->second.make_alias());
				break;
			}
			case Opcode::GetIndex:
			{
				trace_op();
				get_index(*ip++, false);
				break;
			}
			case Opcode::GetIndexArg:
			{
				trace_op();
				int count = *ip++;
				bool by_ref = current_frame->ref_flags[*ip++];
				if (by_ref) {
					RUNTIME_ERROR("Passing indexed expression as an argument by reference is not yet supported");
				}
				get_index(count, by_ref);
				break;
			}
			case Opcode::GetIndexRef:
			{
				trace_op();
				get_index(*ip++, true);
				break;
			}
			case Opcode::GetLocal:
			{
				trace_op();
				auto &v = current_frame->locals[*ip++];
				push(v.resolve());
				break;
			}
			case Opcode::GetLocalArg:
			{
				trace_op();
				auto &v = current_frame->locals[*ip++];
				bool by_ref = current_frame->ref_flags[*ip++];
				if (by_ref) {
					push(v.make_alias());
				}
				else {
					push(v.resolve());
				}
				break;
			}
			case Opcode::GetLocalRef:
			{
				trace_op();
				Variant &v = current_frame->locals[*ip++];
				push(v.make_alias());
				break;
			}
			case Opcode::GetUniqueGlobal:
			{
				trace_op();
				auto name = routine.get_string(*ip++);
				auto it = globals->find(name);
				if (it == globals->end()) {
					RUNTIME_ERROR("[Symbol error] Undefined variable \"%\"", name);
				}
				push(it->second.unshare());
				break;
			}
			case Opcode::GetUniqueLocal:
			{
				trace_op();
				push(current_frame->locals[*ip++].unshare());
				break;
			}
			case Opcode::GetUniqueUpvalue:
			{
				trace_op();
				push(closure->upvalues[*ip++].unshare());
				break;
			}
			case Opcode::GetUpvalue:
			{
				trace_op();
				auto &v = closure->upvalues[*ip++];
				push(v.resolve());
				break;
			}
			case Opcode::GetUpvalueArg:
			{
				trace_op();
				auto &v = closure->upvalues[*ip++];
				bool by_ref = current_frame->ref_flags[*ip++];
				if (by_ref) {
					push(v.make_alias());
				}
				else {
					push(v.resolve());
				}
				break;
			}
			case Opcode::GetUpvalueRef:
			{
				trace_op();
				Variant &v = closure->upvalues[*ip++];
				push(v.make_alias());
				break;
			}
			case Opcode::Greater:
			{
				trace_op();
				auto &v2 = peek(-1);
				auto &v1 = peek(-2);
				bool value = (v1.compare(v2) > 0);
				pop(2);
				push(value);
				break;
			}
			case Opcode::GreaterEqual:
			{
				trace_op();
				auto &v2 = peek(-1);
				auto &v1 = peek(-2);
				bool value = (v1.compare(v2) >= 0);
				pop(2);
				push(value);
				break;
			}
			case Opcode::IncrementLocal:
			{
				trace_op();
				int index = *ip++;
				auto &v = current_frame->locals[index];
				assert(v.is_integer());
				raw_cast<intptr_t>(v)++;
				break;
			}
			case Opcode::Jump:
			{
				trace_op();
				int addr = Code::read_integer(ip);
				ip = code->data() + addr;
				break;
			}
			case Opcode::JumpFalse:
			{
				trace_op();
				int addr = Code::read_integer(ip);
				bool value = peek().to_boolean();
				pop();
				if (!value) ip = code->data() + addr;
				break;
			}
			case Opcode::JumpFalseAnd:
			{
				trace_op();
				int addr = Code::read_integer(ip);
				bool value = peek().to_boolean();
				if (!value) ip = code->data() + addr;
				else pop();
				break;
			}
			case Opcode::JumpTrue:
			{
				trace_op();
				int addr = Code::read_integer(ip);
				bool value = peek().to_boolean();
				pop();
				if (value) ip = code->data() + addr;
				break;
			}
			case Opcode::JumpTrueOr:
			{
				trace_op();
				int addr = Code::read_integer(ip);
				bool value = peek().to_boolean();
				if (value) ip = code->data() + addr;
				else pop();
				break;
			}
			case Opcode::Less:
			{
				trace_op();
				auto &v2 = peek(-1);
				auto &v1 = peek(-2);
				bool value = (v1.compare(v2) < 0);
				pop(2);
				push(value);
				break;
			}
			case Opcode::LessEqual:
			{
				trace_op();
				auto &v2 = peek(-1);
				auto &v1 = peek(-2);
				bool value = (v1.compare(v2) <= 0);
				pop(2);
				push(value);
				break;
			}
			case Opcode::Modulus:
			{
				trace_op();
				math_op('%');
				break;
			}
			case Opcode::Multiply:
			{
				trace_op();
				math_op('*');
				break;
			}
			case Opcode::Negate:
			{
				trace_op();
				negate();
				break;
			}
			case Opcode::NewArray:
			{
				trace_op();
				int nrow = *ip++;
				int ncol = *ip++;
				int narg = nrow * ncol;
				if (nrow == 1)
				{
					Array<double> array(ncol, 0.0);
					for (int i = 1; i <= ncol; i++)
					{
						array(i) = peek(-ncol + i - 1).to_float();
					}
					pop(narg);
					push(make_handle<Array<double>>(std::move(array)));
				}
				else
				{
					int k = narg;
					Array<double> array(nrow, ncol, 0.0);
					for (int i = 1; i <= nrow; i++)
					{
						for (int j = 1; j <= ncol; j++)
						{
							array(i,j) = peek(-k).to_float();
							k--;
						}
					}
					pop(narg);
					push(make_handle<Array<double>>(std::move(array)));
				}
				break;
			}
			case Opcode::NewClosure:
			{
				trace_op();
				const int index = *ip++;
				const int narg = *ip++;
				auto r = routine.get_routine(index);
				if (!r->sealed())
				{
					for (int i = narg; i > 0; i--)
					{
						auto &v = peek(-i);
						if (!check_type<Class>(v)) {
							RUNTIME_ERROR("Expected a Class object as type of parameter %", (narg + 1 - i));
						}
						r->add_parameter_type(v.handle<Class>());
					}
					r->seal();
				}
				pop(narg);
				auto rout = r.get();
				auto c = make_handle<Closure>(this, std::move(r));
				for (int i = 0; i < rout->upvalue_count(); i++)
				{
					auto upvalue = rout->upvalues[i];
					Variant *var;
					if (upvalue.is_local) {
						var = &current_frame->locals[upvalue.index];
					}
					else {
						var = &closure->upvalues[upvalue.index];
					}
					c->upvalues.emplace_back(var->make_alias());
				}
				push(make_handle<Function>(this, rout->name(), std::move(c)));
				break;
			}
			case Opcode::NewFrame:
			{
				trace_op();
				push_call_frame(closure.object(), *ip++);

				break;
			}
			case Opcode::NewIterator:
			{
				trace_op();
				bool ref_val = bool(*ip++);
				auto v = std::move(peek());
				pop();
				if (check_type<List>(v)) {
					push(make_handle<ListIterator>(std::move(v), ref_val));
				}
				else if (check_type<Table>(v)) {
					push(make_handle<TableIterator>(std::move(v), ref_val));
				}
				else if (check_type<File>(v)) {
					push(make_handle<FileIterator>(std::move(v), ref_val));
				}
				else if (check_type<Regex>(v)) {
					push(make_handle<RegexIterator>(std::move(v), ref_val));
				}
				else if (check_type<String>(v)) {
					push(make_handle<StringIterator>(std::move(v), ref_val));
				}
				else {
					RUNTIME_ERROR("Type % is not iterable", v.class_name());
				}
				break;
			}
			case Opcode::NewList:
			{
				trace_op();
				int narg = *ip++;
				List lst(narg);
				for (int i = narg; i > 0; i--) {
					lst[narg+1-i] = std::move(peek(-i));
				}
				pop(narg);
				push(make_handle<List>(this, std::move(lst)));
				break;
			}
			case Opcode::NewTable:
			{
				trace_op();
				int narg = *ip++ * 2;
				Table::Storage tab;
				for (int i = narg; i > 0; i -= 2)
				{
					auto &key = peek(-i).resolve();
					auto &val = peek(-i+1).resolve();
					try {
						tab.insert({ std::move(key), std::move(val) });
					}
					CATCH_ERROR
				}
				pop(narg);
				push(make_handle<Table>(this, std::move(tab)));
				break;
			}
			case Opcode::NewSet:
			{
				trace_op();
				int narg = *ip++;
				Set::Storage set;
				for (int i = narg; i > 0; i--) {
					set.insert(std::move(peek(-i)));
				}
				pop(narg);
				push(make_handle<Set>(this, std::move(set)));
				break;
			}
			case Opcode::NextKey:
			{
				trace_op();
				try {
					auto v = std::move(peek());
					pop();
					auto &it = raw_cast<Iterator>(v);
					push(it.get_key());
				}
				CATCH_ERROR
				break;
			}
			case Opcode::NextValue:
			{
				trace_op();
				try {
					auto v = std::move(peek());
					pop();
					auto &it = raw_cast<Iterator>(v);
					push(it.get_value());
				}
				CATCH_ERROR

				break;
			}
			case Opcode::Not:
			{
				trace_op();
				bool value = peek().to_boolean();
				pop();
				push(!value);
				break;
			}
			case Opcode::NotEqual:
			{
				trace_op();
				auto &v2 = peek(-1);
				auto &v1 = peek(-2);
				bool value = (v1 != v2);
				pop(2);
				push(value);
				break;
			}
			case Opcode::Pop:
			{
				trace_op();
				pop();
				break;
			}
			case Opcode::Power:
			{
				trace_op();
				math_op('^');
				break;
			}
			case Opcode::Precall:
			{
				trace_op();
				auto &v = peek();
				Handle<Function> func;
				if (check_type<Function>(v))
				{
					func = v.resolve().handle<Function>();
				}
				else if (check_type<Class>(v))
				{
					// Replace the class with its constructor.
					auto cls = v.handle<Class>();
					pop();
					try {
						func = cls->get_constructor();
						push(func);
					}
					CATCH_ERROR
				}
				else
				{
					RUNTIME_ERROR("Expected a Function or a Class, got a %", v.class_name());
				}

				current_frame->ref_flags = func->ref_flags;
				break;
			}
			case Opcode::Print:
			{
				trace_op();
				int narg = *ip++;
				for (int i = narg; i > 0; i--)
				{
					auto s = peek(-i).to_string();
					this->print(s);
				}
				pop(narg);
				break;
			}
			case Opcode::PrintLine:
			{
				trace_op();
				int narg = *ip++;
				for (int i = narg; i > 0; i--)
				{
					auto s = peek(-i).to_string();
					this->print(s);
				}
				static String new_line("\n");
				this->print(new_line);
				pop(narg);
				break;
			}
			case Opcode::PushBoolean:
			{
				trace_op();
				bool value = bool(*ip++);
				push(value);
				break;
			}
			case Opcode::PushFalse:
			{
				trace_op();
				push(false);
				break;
			}
			case Opcode::PushFloat:
			{
				trace_op();
				double value = routine.get_float(*ip++);
				push(value);
				break;
			}
			case Opcode::PushInteger:
			{
				trace_op();
				intptr_t value = routine.get_integer(*ip++);
				push_int(value);
				break;
			}
			case Opcode::PushNan:
			{
				trace_op();
				push(std::nan(""));
				break;
			}
			case Opcode::PushNull:
			{
				trace_op();
				push_null();
				break;
			}
			case Opcode::PushSmallInt:
			{
				trace_op();
				push_int((int16_t) *ip++);
				break;
			}
			case Opcode::PushString:
			{
				trace_op();
				String value = routine.get_string(*ip++);
				push(std::move(value));
				break;
			}
			case Opcode::PushTrue:
			{
				trace_op();
				push(true);
				break;
			}
			case Opcode::Return:
			{
				trace_op();
				auto result = pop_call_frame();
				ip = old_ip;
				return result;
			}
			case Opcode::SetField:
			{
				trace_op();
				auto &v = peek(-3);
				auto cls = v.get_class();
				std::span<Variant> args(&v, 3);
				auto method = cls->get_method(set_field_string);
				if (!method) {
					RUNTIME_ERROR("[Type error] Member access is not supported for % values", v.class_name());
				}
				auto c = method->find_closure(args);
				if (!c) {
					report_call_error(*method, args);
				}
				try {
					call_method(c, args);
				}
				CATCH_ERROR
				pop(3);
				break;
			}
			case Opcode::SetGlobal:
			{
				trace_op();
				auto name = routine.get_string(*ip++);
				globals->insert({std::move(name), std::move(peek())});
				pop();
				break;
			}
			case Opcode::SetIndex:
			{
				trace_op();
				int count = *ip++ + 2; // add indexed expression and value
				auto &v = peek(-count);
				auto cls = v.get_class();
				std::span<Variant> args(&v, count);
				auto method = cls->get_method(set_item_string);
				if (!method) {
					RUNTIME_ERROR("[Type error] % type is not index-assignable");
				}
				auto c = method->find_closure(args);
				if (!c) {
					report_call_error(*method, args);
				}
				try {
					call_method(c, args);
				}
				CATCH_ERROR
				pop(count);
				break;
			}
			case Opcode::SetLocal:
			{
				trace_op();
				Variant &v = current_frame->locals[*ip++];
				try {
					v = std::move(peek());
				}
				CATCH_ERROR
				pop();
				break;
			}
			case Opcode::SetUpvalue:
			{
				trace_op();
				Variant &v = closure->upvalues[*ip++];
				try {
					v = std::move(peek());
				}
				CATCH_ERROR
				pop();
				break;
			}
			case Opcode::Subtract:
			{
				trace_op();
				math_op('-');
				break;
			}
			case Opcode::TestIterator:
			{
				trace_op();
				auto v = std::move(peek());
				pop();
				auto &it = raw_cast<Iterator>(v);
				push(!it.at_end());
				break;
			}
			case Opcode::Throw:
			{
				String msg;
				try {
					msg = peek().to_string();
					pop();
				}
				CATCH_ERROR
				RUNTIME_ERROR("[Runtime error] %", msg);
			}
			default:
				throw error("[Internal error] Invalid opcode: %", (int)op);
		}
	}

	return Variant();
}

void Runtime::disassemble(const Routine &routine, const String &name)
{
	printf("========================= %s =========================\n", name.data());
	printf("strings: %d, large integers: %d, floats: %d, routines: %d\n", (int)routine.string_pool.size(), (int) routine.integer_pool.size(),
		   (int) routine.float_pool.size(), (int) routine.routine_pool.size());
	printf("offset    line   instruction    operands   comments\n");
	size_t size = routine.code.size();

	for (size_t offset = 0; offset < size; )
	{
		offset += disassemble_instruction(routine, offset);
	}

	for (auto &r : routine.routine_pool)
	{
		printf("\n");
		disassemble(*r, r->name());
	}

}

void Runtime::disassemble(const Closure &closure, const String &name)
{
	auto &routine = *(reinterpret_cast<Routine*>(closure.routine.get()));
	disassemble(routine, name);
}

size_t Runtime::print_simple_instruction(const char *name)
{
	printf("%s\n", name);
	return 1;
}

size_t Runtime::disassemble_instruction(const Routine &routine, size_t offset)
{
	auto op = static_cast<Opcode>(routine.code[offset]);
	printf("%6zu   %5d   ", offset, routine.code.get_line(offset));

	switch (op)
	{
		case Opcode::Add:
		{
			return print_simple_instruction("ADD");
		}
		case Opcode::Assert:
		{
			int narg = routine.code[offset + 1];
			printf("ASSERT         %-5d\n", narg);
			return 2;
		}
		case Opcode::Call:
		{
			int narg = routine.code[offset + 1];
			printf("CALL           %-5d\n", narg);
			return 2;
		}
		case Opcode::ClearLocal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_local_name(index);
			printf("CLEAR_LOCAL    %-5d     ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::Compare:
		{
			return print_simple_instruction("COMPARE");
		}
		case Opcode::Concat:
		{
			int narg = routine.code[offset+1];
			printf("CONCAT         %-5d\n", narg);
			return 2;
		}
		case Opcode::DecrementLocal:
		{
			int index = routine.code[offset + 1];
			printf("DEC_LOCAL      %-5d\n", index);
			return 2;
		}
		case Opcode::DefineLocal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_local_name(index);
			printf("DEFINE_LOCAL   %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::Divide:
		{
			return print_simple_instruction("DIVIDE");
		}
		case Opcode::Equal:
		{
			return print_simple_instruction("EQUAL");
		}
		case Opcode::GetField:
		{
			return print_simple_instruction("GET_FIELD");
		}
		case Opcode::GetFieldArg:
		{
			int index = routine.code[offset + 1];
			printf("GET_FIELD_ARG  %-5d\n", index);
			return 2;
		}
		case Opcode::GetFieldRef:
		{
			return print_simple_instruction("GET_FIELD_REF");
		}
		case Opcode::GetGlobal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_string(index);
			printf("GET_GLOBAL     %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::GetGlobalArg:
		{
			int index = routine.code[offset + 1];
			int narg = routine.code[offset + 2];
			String value = routine.get_string(index);
			printf("GET_GLOBAL_ARG %-5d %-5d; %s\n", index, narg, value.data());
			return 3;
		}
		case Opcode::GetGlobalRef:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_string(index);
			printf("GET_GLOBAL_REF %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::GetIndex:
		{
			int count = routine.code[offset + 1];
			printf("GET_INDEX      %-5d\n", count);
			return 2;
		}
		case Opcode::GetIndexArg:
		{
			int count = routine.code[offset + 1];
			int index = routine.code[offset + 2];
			printf("GET_INDEX_ARG %-5d %-5d\n", count, index);
			return 3;
		}
		case Opcode::GetIndexRef:
		{
			int count = routine.code[offset + 1];
			printf("GET_INDEX_REF  %-5d\n", count);
			return 2;
		}
		case Opcode::GetLocal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_local_name(index);
			printf("GET_LOCAL      %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::GetLocalArg:
		{
			int index = routine.code[offset + 1];
			int narg = routine.code[offset + 2];
			String value = routine.get_local_name(index);
			printf("GET_LOCAL_ARG  %-5d %-5d; %s\n", index, narg, value.data());
			return 3;
		}
		case Opcode::GetLocalRef:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_local_name(index);
			printf("GET_LOCAL_REF  %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::GetUniqueGlobal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_string(index);
			printf("GET_UNIQUE_GLOBAL %-5d   ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::GetUniqueLocal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_local_name(index);
			printf("GET_UNIQUE_LOCAL %-5d    ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::GetUniqueUpvalue:
		{
			int index = routine.code[offset + 1];
			printf("GET_UNIQUE_UPVALUE %-5d\n", index);
			return 2;
		}
		case Opcode::GetUpvalue:
		{
			int index = routine.code[offset + 1];
			printf("GET_UPVALUE    %-5d\n", index);
			return 2;
		}
		case Opcode::GetUpvalueArg:
		{
			int index = routine.code[offset + 1];
			int narg = routine.code[offset + 2];
			printf("GET_UPVALUE_ARG %-5d %-5d\n", index, narg);
			return 3;
		}
		case Opcode::GetUpvalueRef:
		{
			int index = routine.code[offset + 1];
			printf("GET_UPVALUE_REF %-5d\n", index);
			return 2;
		}

		case Opcode::Greater:
		{
			return print_simple_instruction("GREATER");
		}
		case Opcode::GreaterEqual:
		{
			return print_simple_instruction("GREATER_EQUAL");
		}
		case Opcode::IncrementLocal:
		{
			int index = routine.code[offset + 1];
			printf("INC_LOCAL      %-5d\n", index);
			return 2;
		}
		case Opcode::Jump:
		{
			auto ptr = routine.code.data() + offset + 1;
			int addr = Code::read_integer(ptr);
			printf("JUMP           %-5d\n", addr);
			return 1 + Code::IntSerializer::IntSize;
		}
		case Opcode::JumpFalse:
		{
			auto ptr = routine.code.data() + offset + 1;
			int addr = Code::read_integer(ptr);
			printf("JUMP_FALSE     %-5d\n", addr);
			return 1 + Code::IntSerializer::IntSize;
		}
		case Opcode::JumpFalseAnd:
		{
			auto ptr = routine.code.data() + offset + 1;
			int addr = Code::read_integer(ptr);
			printf("JUMP_FALSE_AND %-5d\n", addr);
			return 1 + Code::IntSerializer::IntSize;
		}
		case Opcode::JumpTrue:
		{
			auto ptr = routine.code.data() + offset + 1;
			int addr = Code::read_integer(ptr);
			printf("JUMP_TRUE      %-5d\n", addr);
			return 1 + Code::IntSerializer::IntSize;
		}
		case Opcode::JumpTrueOr:
		{
			auto ptr = routine.code.data() + offset + 1;
			int addr = Code::read_integer(ptr);
			printf("JUMP_TRUE_OR   %-5d\n", addr);
			return 1 + Code::IntSerializer::IntSize;
		}
		case Opcode::Less:
		{
			return print_simple_instruction("LESS");
		}
		case Opcode::LessEqual:
		{
			return print_simple_instruction("LESS_EQUAL");
		}
		case Opcode::Modulus:
		{
			return print_simple_instruction("MODULUS");
		}
		case Opcode::Multiply:
		{
			return print_simple_instruction("MULTIPLY");
		}
		case Opcode::Negate:
		{
			return print_simple_instruction("NEGATE");
		}
		case Opcode::NewArray:
		{
			int nrow = routine.code[offset+1];
			int ncol = routine.code[offset+2];
			printf("NEW_ARRAY      %-5d %-5d\n", nrow, ncol);
			return 3;
		}
		case Opcode::NewClosure:
		{
			int index = routine.code[offset + 1];
			int narg = routine.code[offset + 2];
			auto r = routine.get_routine(index);
			printf("NEW_CLOSURE    %-3d %-5d  ; <%p>\n", index, narg, r.get());
			return 3;
		}
		case Opcode::NewFrame:
		{
			int nlocal = routine.code[offset+1];
			printf("NEW_FRAME      %-5d\n", nlocal);
			return 2;
		}
		case Opcode::NewIterator:
		{
			bool ref_val = bool(routine.code[offset+1]);
			printf("NEW_ITER       %-5d\n", ref_val);
			return 2;
		}
		case Opcode::NewList:
		{
			int nlocal = routine.code[offset+1];
			printf("NEW_LIST       %-5d\n", nlocal);
			return 2;
		}
		case Opcode::NewTable:
		{
			int len = routine.code[offset+1];
			printf("NEW_TABLE      %-5d\n", len);
			return 2;
		}
		case Opcode::NewSet:
		{
			int nlocal = routine.code[offset+1];
			printf("NEW_SET        %-5d\n", nlocal);
			return 2;
		}
		case Opcode::NextKey:
		{
			return print_simple_instruction("NEXT_KEY");
		}
		case Opcode::NextValue:
		{
			return print_simple_instruction("NEXT_VALUE");
		}
		case Opcode::Not:
		{
			return print_simple_instruction("NOT");
		}
		case Opcode::NotEqual:
		{
			return print_simple_instruction("NOT_EQUAL");
		}
		case Opcode::Pop:
		{
			return print_simple_instruction("POP");
		}
		case Opcode::Power:
		{
			return print_simple_instruction("POWER");
		}
		case Opcode::Precall:
		{
			return print_simple_instruction("PRECALL");
		}
		case Opcode::Print:
		{
			int narg = routine.code[offset+1];
			printf("PRINT         %-5d\n", narg);
			return 2;
		}
		case Opcode::PrintLine:
		{
			int narg = routine.code[offset+1];
			printf("PRINT_LINE     %-5d\n", narg);
			return 2;

		}
		case Opcode::PushBoolean:
		{
			int value = routine.code[offset + 1];
			auto str = value ? "true" : "false";
			printf("PUSH_BOOLEAN   %-5d      ; %s\n", value, str);
			return 2;
		}
		case Opcode::PushFalse:
		{
			return print_simple_instruction("PUSH_FALSE");
		}
		case Opcode::PushFloat:
		{
			int index = routine.code[offset + 1];
			double value = routine.get_float(index);
			printf("PUSH_FLOAT     %-5d      ; %f\n", index, value);
			return 2;
		}
		case Opcode::PushInteger:
		{
			int index = routine.code[offset + 1];
			intptr_t value = routine.get_integer(index);
			printf("PUSH_INTEGER   %-5d      ; %" PRIdPTR "\n", index, value);
			return 2;
		}
		case Opcode::PushNan:
		{
			return print_simple_instruction("PUSH_NAN");
		}
		case Opcode::PushNull:
		{
			return print_simple_instruction("PUSH_NULL");
		}
		case Opcode::PushSmallInt:
		{
			int value = (int16_t) routine.code[offset + 1];
			printf("PUSH_SMALL_INT %-5d\n", value);
			return 2;
		}
		case Opcode::PushString:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_string(index);
			printf("PUSH_STRING    %-5d      ; \"%s\"\n", index, value.data());
			return 2;
		}
		case Opcode::PushTrue:
		{
			return print_simple_instruction("PUSH_TRUE");
		}
		case Opcode::Return:
		{
			return print_simple_instruction("RETURN");
		}
		case Opcode::SetField:
		{
			return print_simple_instruction("SET_FIELD");
		}
		case Opcode::SetGlobal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_string(index);
			printf("SET_GLOBAL     %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::SetIndex:
		{
			int count = routine.code[offset + 1];
			printf("SET_INDEX      %-5d\n", count);
			return 2;
		}
		case Opcode::SetLocal:
		{
			int index = routine.code[offset + 1];
			String value = routine.get_local_name(index);
			printf("SET_LOCAL      %-5d      ; %s\n", index, value.data());
			return 2;
		}
		case Opcode::Subtract:
		{
			return print_simple_instruction("SUBTRACT");
		}
		case Opcode::TestIterator:
		{
			return print_simple_instruction("TEST_ITER");
		}
		case Opcode::Throw:
		{
			return print_simple_instruction("THROW");
		}
		default:
			printf("Unknown opcode %d", static_cast<int>(op));
	}

	return 1;
}

Handle<Closure> Runtime::compile_file(const String &path)
{
	this->clear();
	auto ast = parser.parse_file(path);

	return compiler.compile(std::move(ast));
}

Handle<Closure> Runtime::compile_string(const String &code)
{
	this->clear();
	auto ast = parser.parse_string(code);

	return compiler.compile(std::move(ast));
}

Variant Runtime::do_file(const String &path)
{
	auto old_path = current_path;
	current_path = path;
	auto closure = compile_file(path);
	auto result = interpret(closure);
	current_path = old_path;

	return result;
}

Variant Runtime::do_string(const String &code)
{
	auto old_path = current_path;
	current_path = String();
	auto closure = compile_string(code);
	auto result = interpret(closure);
	current_path = old_path;

	return result;
}

int Runtime::get_current_line() const
{
	auto offset = int(ip - 1 - code->data());
	return code->get_line(offset);
}

String Runtime::intern_string(const String &s)
{
	auto result = strings.insert(s);
	return *result.first;
}

void Runtime::push_call_frame(TObject<Closure> *closure, int nlocal)
{
	// FIXME: valgrind complains about a fishy size passed to malloc here when PHON_STD_UNORDERED_MAP is defined.
	frames.push_back(std::make_unique<CallFrame>());
	current_frame = frames.back().get();
	ensure_capacity(nlocal);
	current_frame->current_closure = closure;
	current_frame->locals = top;
	current_frame->nlocal = nlocal;
	int argc = current_routine->arg_count();
	top += argc;
	for (int i = argc; i < nlocal; i++) {
		new (top++) Variant;
	}
}

Variant Runtime::pop_call_frame()
{
	Variant result;

	// If there's a value left on top of the stack, it is taken as the return value.
	auto locals_end = current_frame->locals + current_frame->nlocal;
	if (top > locals_end) {
		result = std::move(*--top);
	}

	// Clean current frame.
	auto n = int(top - current_frame->locals);
	assert(n >= 0);
	// Account for the function that was left on the stack, if there's one.
	int extra = calling_method ? 0 : 1;
	pop(n + extra); // pop the frame

	// Restore previous frame.
	frames.pop_back();
	current_frame = frames.empty() ? nullptr : frames.back().get();

	if (frames.empty())
	{
		current_frame = nullptr;
		code = nullptr;
		current_routine = nullptr;
		ip = nullptr;
	}
	else
	{
		current_frame = frames.back().get();
		code = &current_frame->previous_routine->code;
		current_routine = current_frame->previous_routine;
		ip = current_frame->ip;
	}

	return result;
}

void Runtime::add_global(String name, Variant value)
{
	globals->insert({ std::move(name), std::move(value) });
}

void Runtime::add_global(const String &name, NativeCallback cb, std::initializer_list<Handle<Class>> sig, ParamBitset ref)
{
	(*globals)[name] = make_handle<Function>(this, this, name, std::move(cb), sig, ref);
}

void Runtime::get_index(int count, bool by_ref)
{
	needs_ref = by_ref;
	count++; // add indexed expression
	auto &v = peek(-count);
	Variant result;
	std::span<Variant> args(&v, count);
	auto cls = v.get_class();
	auto method = cls->get_method(get_item_string);
	if (!method) {
		RUNTIME_ERROR("[Type error] % type is not indexable");
	}
	auto closure = method->find_closure(args);
	if (!closure) {
		report_call_error(*method, args);
	}
	try {
		result = call_method(closure, args);
	}
	CATCH_ERROR
	pop(count);
	push(std::move(result));
	needs_ref = false;
}

void Runtime::get_field(bool by_ref)
{
	needs_ref = by_ref;
	auto &v = peek(-2);
	Variant result;
	std::span<Variant> args(&v, 2);
	auto cls = v.get_class();
	auto method = cls->get_method(get_field_string);
	if (!method) {
		RUNTIME_ERROR("[Type error] Member access is not supported for type %", v.class_name());
	}
	auto closure = method->find_closure(args);
	if (!closure) {
		report_call_error(*method, args);
	}
	try {
		result = call_method(closure, args);
	}
	CATCH_ERROR
	pop(2);
	push(std::move(result));
	needs_ref = false;
}

bool Runtime::needs_reference() const
{
	return needs_ref;
}

void Runtime::clear()
{
	needs_ref = false;
}

Variant &Runtime::operator[](const String &key)
{
	return (*globals)[key];
}

bool Runtime::debug_mode() const
{
	return debugging;
}

void Runtime::set_debug_mode(bool value)
{
	debugging = value;
}

void Runtime::report_call_error(const Function &func, std::span<Variant> args)
{
	Array<String> types;

	for (auto &arg : args) {
		types.append(arg.class_name());
	}
	String candidates;
	for (auto &c : func.closures) {
		candidates.append(c->routine->get_definition());
		candidates.append('\n');
	}
	RUNTIME_ERROR("Cannot resolve call to function '%' with the following argument types: (%).\nCandidates are:\n%",
				  func.name(), String::join(types, ", "), candidates);
}


void Runtime::collect()
{
	if (gc_paused) {
		return;
	}
	mark_candidates();
	auto candidate = gc_root;
	while (candidate != nullptr)
	{
		scan(candidate);
		candidate = candidate->next;
	}
	collect_candidates();
}

void Runtime::mark_candidates()
{
	auto candidate = gc_root;

	while (candidate != nullptr)
	{
		if (candidate->is_purple())
		{
			mark_grey(candidate);
		}
		else
		{
			remove_candidate(candidate);

			if (candidate->is_black() && !candidate->is_used()) {
				candidate->destroy();
			}
		}
		candidate = candidate->next;
	}
}

void Runtime::mark_grey(Collectable *candidate)
{
	if (!candidate->is_grey())
	{
		candidate->mark_grey();
		auto traverse = candidate->get_class()->traverse;

		if (traverse)
		{
			// Run trial deletion on each child
			auto lambda = [](Collectable *child) {
				child->remove_reference();
				mark_grey(child);
			};

			traverse(candidate, lambda);
		}
	}
}

void Runtime::scan(Collectable *candidate)
{
	if (candidate->is_grey())
	{
		if (candidate->is_used())
		{
			// There must be an external reference.
			scan_black(candidate);
		}
		else
		{
			// This looks like garbage
			candidate->mark_white();
			auto traverse = candidate->get_class()->traverse;

			if (traverse)
			{
				auto lambda = [](Collectable *child) {
					scan(child);
				};

				traverse(candidate, lambda);
			}
		}
	}
}

void Runtime::scan_black(Collectable *candidate)
{
	// Repair reference count of live data.
	candidate->mark_black();
	auto traverse = candidate->get_class()->traverse;

	if (traverse)
	{
		auto lambda = [](Collectable *child) {
			// Undo trial deletion
			child->add_reference();

			if (!child->is_black()) {
				scan_black(child);
			}
		};

		traverse(candidate, lambda);
	}
}

void Runtime::collect_candidates()
{
	Collectable *candidate;

	while ((candidate = pop_candidate())) {
		collect_white(candidate);
	}
}

void Runtime::collect_white(Collectable *ref)
{
	if (ref->is_white() && !ref->is_candidate())
	{
		// Free-list objects are black
		ref->mark_black();
		auto traverse = ref->get_class()->traverse;

		if (traverse) {
			traverse(ref, collect_white);
		}
		ref->destroy();
	}
}

void Runtime::suspend_gc()
{
	gc_paused = true;
}

void Runtime::resume_gc()
{
	gc_paused = false;
}

Variant Runtime::call_method(Handle<Closure> &c, std::span<Variant> args)
{
	if (c->routine->is_native())
	{
		auto &r = reinterpret_cast<NativeRoutine&>(*(c->routine));
		return r(*this, args);
	}
	else
	{
		auto flag = calling_method;
		calling_method = true;
		top -= args.size();
		auto v = interpret(c);
		calling_method = flag;

		return v;
	}
}

Collectable *Runtime::pop_candidate()
{
	auto cand = gc_root;
	if (cand)
	{
		gc_root = cand->next;
		cand->next = nullptr;
		if (gc_root) gc_root->previous = nullptr;
	}

	return cand;
}

void Runtime::add_import_path(const String &path)
{
	auto it = std::find(import_paths.begin(), import_paths.end(), path);
	if (it == import_paths.end()) {
		import_paths.push_back(path);
	}
}

void Runtime::remove_import_path(const String &path)
{
	auto it = std::find(import_paths.begin(), import_paths.end(), path);
	if (it != import_paths.end()) {
		import_paths.erase(it);
	}
}

String Runtime::find_import(String name)
{
	using namespace filesystem;

	if (!name.ends_with(PHON_FILE_EXTENSION))
		name.append(PHON_FILE_EXTENSION);

	auto path = join(directory_name(current_path), name);
	if (exists(path)) {
		return path;
	}

	for (auto &dir : import_paths)
	{
		auto path = join(dir, name);
		if (exists(path)) {
			return path;
		}
	}

	throw error("[Input/Output error] Cannot find file \"%s\"", name);
}

void Runtime::call(int narg)
{
	// Call a function on top of the stack
	auto old_ip = ip; // NULL?

	auto &v = peek(-narg - 1);
	auto &func = cast<Function>(v);
	std::span<Variant> args(top - narg, narg);

	try
	{
		auto c = func.find_closure(args);
		if (!c) {
			report_call_error(func, args);
		}

		if (c->routine->is_native())
		{
			auto &r = reinterpret_cast<NativeRoutine&>(*(c->routine));
			auto result = r(*this, args);
			pop(narg + 1);
			push(std::move(result));
		}
		else
		{
			// The arguments are on top of the stack. We adjust the top of the stack accordingly.
			top -= narg;
			if (current_frame) {
				current_frame->ip = ip;
			}
			push(interpret(c));
		}
	}
	catch (std::exception &e)
	{
		throw RuntimeError(get_current_line(), e.what());
	}
	auto new_ip = ip;
	assert(old_ip == new_ip);
}

Variant Runtime::import_module(const String &name)
{
	auto it = imports.find(name);
	if (it == imports.end()) {
		return reload_module(name);
	}

	return it->second;
}

Variant Runtime::reload_module(const String &name)
{
	auto path = find_import(name);
	auto result = do_file(path);
	imports.insert({ path, result });

	return result;
}

void Runtime::printf(const char *fmt, ...) const
{
	char buffer[256];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, 256, fmt, args);
	va_end(args);
	this->print(buffer);
}

} // namespace phonometrica

#undef CATCH_ERROR
#undef RUNTIME_ERROR
#undef trace_op