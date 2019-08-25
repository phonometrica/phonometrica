/**************************************************************************************
 * Copyright (C) 2013-2019, Artifex Software                                          *
 *           (C) 2019, Julien Eychenne <jeychenne@gmail.com>                          *
 *                                                                                    *
 * Permission to use, copy, modify, and/or distribute this software for any purpose   *
 * with or without fee is hereby granted, provided that the above copyright notice    *
 * and this permission notice appear in all copies.                                   *
 *                                                                                    *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH      *
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND    *
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, *
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,     *
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    *
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        *
 * SOFTWARE.                                                                          *
 *                                                                                    *
 **************************************************************************************/

#ifndef PHONOMETRICA_RUNTIME_HPP
#define PHONOMETRICA_RUNTIME_HPP

#include <cstdint>
#include <unordered_set>
#include <phon/string.hpp>
#include <phon/runtime/toplevel.hpp>


namespace phonometrica {

// Defined in the GUI
class Console;

// An environment stores local variables for a given lexical scope. Environments can be nested:
// each new function defines a new environment, although this is optimized out if the
// locals are not referenced by a nested scope. The top-level environment is the global environment.
struct Environment
{
	Environment *outer;
	Object *variables;

	Environment *gcnext;
	int gcmark;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Runtime final
{
public:

    Runtime();

    Runtime(const Runtime &) = delete;

    Runtime(Runtime &&) = delete;

    ~Runtime();

    void collect(bool report = false);

    void report(const String &message);

    void set_report_callback(report_callback_t report);

    panic_callback_t at_panic(panic_callback_t panic);

    int do_string(const String &source);

    int do_file(const String &filename);

    void load_string(const String &filename, const String &source);

    void load_file(const String &filename);

    void eval();

    int pcall(int n);

    void call(int n);

    int pconstruct(int n);

    void construct(int n);

    String ref();

    void unref(const String &ref);

    void get_registry(const String &name);

    void set_registry(const String &name);

    void del_registry(const String &name);

    void get_global(const String &name);

    void set_global(const String &name);

    void def_global(const String &name, int atts);

    bool has_field(int idx, const String &name);

    void get_field(int idx, const String &name);

    void set_field(int idx, const String &name);

    void def_field(int idx, const String &name, int atts);

    void del_field(int idx, const String &name);

    // Add accessor to the object on top of the stack
    void add_accessor(const String &name, native_callback_t getter);

    void add_accessor(const String &name, native_callback_t getter, native_callback_t setter);

    // TODO: make this method private
    void def_accessor(int idx, const String &name, int atts);

    void current_function();

    void push_global();

    void push_null();

    void push_undefined();

    void push(double v);

    void push_boolean(bool value); // avoid ambiguity with int

    void push_int(intptr_t i) { push(double(i)); } // avoid ambiguity with int

    void push(const char *v, intptr_t n);

    void push(String &&v);

    void push(const String &v);

    void push_iterator(int idx);

    void push(Object *v);

    void push(const Variant &v);

    void push(Variant &&v);

    std::optional<Variant> next_iterator(int idx);

    void new_objectx();

    void new_object();

    void new_list(intptr_t size = 0);

    void push(Array<Variant> lst);

    void new_boolean(bool v);

    void new_number(double v);

    void new_string(const String &v);

    void new_native_function(native_callback_t fun, const String &name, int length);

    void new_native_constructor(native_callback_t fun, native_callback_t con, const String &name, int length);

    void new_user_data(Object *meta, const char *tag, std::any data);

    void new_user_data(const char *tag, std::any data);

    void new_user_data(const char *tag, std::any data, has_field_callback_t has, put_callback_t put, delete_callback_t destroy);

    void new_regex(const String &pattern, int flags);

    void pop(int n = 1);

    // Number of variants on the stack, including [this].
    int top_count() const;

    void rot(int n);

    void copy(int idx);

    void remove(int idx);

    bool is_defined(int idx) const;

    bool is_null(int idx) const;

    bool is_boolean(int idx) const;

    bool is_number(int idx) const;

    bool is_string(int idx) const;

    bool is_primitive(int idx) const;

    bool is_object(int idx) const;

    bool is_list(int idx) const;

    bool is_regex(int idx) const;

    bool is_file(int idx) const;

    bool is_user_data(int idx, const char *tag) const;

    bool is_error(int idx) const;

    bool is_coercible(int idx) const;

    bool is_callable(int idx) const;

    bool to_boolean(int idx) const;

	Regex & to_regex(int idx);

    File &to_file(int idx);

    Array<Variant> &to_list(int idx);

    Object *to_object(int idx);

    Object *to_object(Variant *v);

    void add_method(const char *name, native_callback_t cfun, int n);

    void add_global_function(const char *name, native_callback_t cfun, int n);

    void add_math_constant(const char *name, double number);

    void add_string_field(const char *name, const char *string);

    double to_number(int idx);

    intptr_t to_integer(int idx);

    int to_int32(int idx);

    unsigned int to_uint32(int idx);

    short to_int16(int idx);

    unsigned short to_uint16(int idx);

    double to_number(Variant *v);

    int arg_count() { return top_count() - 1; }

    intptr_t to_integer(Variant *v);

    String to_string(int idx);

    std::any & to_user_data(int idx, const char *tag);

    std::any & to_user_data(int idx); // don't check tag

    template<class T>
    T cast_user_data(int idx)
    {
        try
        {
            return std::any_cast<T>(to_user_data(idx));
        }
        catch (std::bad_any_cast &)
        {
            throw raise("Type error", "Wrong userdata type");
        }
    }

    String internalize(const String &s);

    bool has_field(Object *obj, const String &name);

    void get_field(Object *obj, const String &name);

    void set_field(Object *obj, const String &name);

    void def_field(Object *obj, const String &name, int atts, Variant *value, Object *getter, Object *setter);

    int del_field(Object *obj, const String &name);

    void get_field(Object *obj, intptr_t idx);

    void set_field(Object *obj, intptr_t idx);

    void check_stack(int n);

    // TODO: make stack_index() private (use get() instead)
    Variant *stack_index(int idx);

    const Variant *stack_index(int idx) const;

    Variant &get(int idx);

    const Variant &get(int idx) const;

    char32_t read_char() { return pos == source.end() ? 0 : source.next_codepoint(pos); }

    void clear_stack();

    std::runtime_error raise(const char *error_category, std::exception &e);

    std::runtime_error raise(const char *error_category, const char *fmt, ...);

    void dump_stack();

    bool is_text_mode() const { return text_mode; }

    void set_text_mode(bool value) { text_mode = value; }

    int get_stack_trace(int skip);

    bool is_stack_empty() const { return stack == top; }

private:

    void initialize();

    void init_object();

    void init_list();

    void init_function();

    void init_boolean();

    void init_number();

    void init_string();

    void init_regexp();

    void init_file();

    void init_math();

    void init_json();

    void init_date();

    void init_system();


public:

    void *actx = nullptr; // TODO: get rid of allocator context
    alloc_callback_t alloc = nullptr;
    report_callback_t report_callback = nullptr;
    panic_callback_t panic = nullptr;
    std::function<void(const String &)> print;
    Console *console = nullptr;

    // Standard locations for modules.
    Array<String> import_directories;

    std::unordered_set<String> strings;

    int default_strict = 0;
    int strict = 0;
    bool text_mode = true;

    String null_string, undef_string, true_string, false_string;

    /* parser input source */
    String filename;
    String source;
    String::const_iterator pos; // position in the source file
    int line = 0;

    /* lexer state */
    String lexbuf;

    int lexline = 0;
    char32_t lexchar = 0;
    int lasttoken = 0;
    int newline = 0;

    /* parser state */
    int astdepth = 0;
    int astline = 0;
    int lookahead = 0;
    String text;
    double number = 0;
    Ast *gcast = nullptr; /* list of allocated nodes to free after parsing */

    /* runtime environment */
    Object *object_meta = nullptr;
    Object *list_meta = nullptr;
    Object *function_meta = nullptr;
    Object *boolean_meta = nullptr;
    Object *number_meta = nullptr;
    Object *string_meta = nullptr;
    Object *regex_meta = nullptr;
    Object *date_meta = nullptr;
    Object *file_meta = nullptr;

    // Objects that must be protected from garbage collection.
    Array<Object*> permanent_objects;

    int nextref = 0; /* for js_ref use */
    Object *R = nullptr; /* registry of hidden values */
    Object *G = nullptr; /* the global object */
    Environment *E = nullptr; /* current environment scope */
    Environment *GE = nullptr; /* global environment scope (at the root) */

    /* execution stack */
    Variant *top = nullptr, *bot = nullptr;
    Variant *stack = nullptr;

    /* garbage collector list */
    int gcpause = 0;
    int gcmark = 0;
    int gccounter = 0;
    Environment *gcenv = nullptr;
    Function *gcfun = nullptr;
    Object *gcobj = nullptr;

    /* namespaces on the call stack but currently not in scope */
    int nstop = 0;
    Environment *nsstack[PHON_ENVLIMIT];

    /* debug info stack trace */
    int tracetop = 0;
    StackTrace trace[PHON_ENVLIMIT];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Environment *new_environment(Runtime *J, Object *variables, Environment *outer);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PHON_EMBED_SCRIPTS
#define run_script(e, name) e.do_string(name##_script)
#define get_script_content(e, name) name##_script
#else
#define run_script(e, name) e.do_file(Settings::get_std_script(e, #name))
#define get_script_content(e, name) File::read_all(Settings::get_std_script(e, #name))
#endif

} // namespace phonometrica

#endif // PHONOMETRICA_RUNTIME_HPP
