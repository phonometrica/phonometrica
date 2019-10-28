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

#ifndef PHONOMETRICA_COMPILE_HPP
#define PHONOMETRICA_COMPILE_HPP

#include <optional>

namespace phonometrica {

enum js_OpCode
{
    OP_POP,        /* A -- */
    OP_DUP,        /* A -- A A */
    OP_DUP2,    /* A B -- A B A B */
    OP_ROT2,    /* A B -- B A */
    OP_ROT3,    /* A B C -- C A B */
    OP_ROT4,    /* A B C D -- D A B C */

    OP_NUMBER_0,    /* -- 0 */
    OP_NUMBER_1,    /* -- 1 */
    OP_NUMBER_POS,    /* -K- K */
    OP_NUMBER_NEG,    /* -K- -K */

    OP_NUMBER,    /* -N- <number> */
    OP_STRING,    /* -S- <string> */
    OP_CLOSURE,    /* -F- <closure> */

    OP_NEWLIST,
    OP_NEWOBJECT,
    OP_NEWREGEXP,    /* -S,opts- <regexp> */

    OP_UNDEF,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,

    OP_THIS,
    OP_CURRENT,    /* currently executing function object */

    OP_INITLOCAL,    /* <value> -K- */
    OP_GETLOCAL,    /* -K- <value> */
    OP_SETLOCAL,    /* <value> -K- <value> */
    OP_DELLOCAL,    /* -K- false */

    OP_INITVAR,    /* <value> -S- */
    OP_DEFVAR,    /* -S- */
    OP_HASVAR,    /* -S- ( <value> | undefined ) */
    OP_GETVAR,    /* -S- <value> */
    OP_SETVAR,    /* <value> -S- <value> */
    OP_DELVAR,    /* -S- <success> */

    OP_IN,        /* <name> <obj> -- <exists?> */

    OP_INITPROP,    /* <obj> <key> <val> -- <obj> */
    OP_INITGETTER,    /* <obj> <key> <closure> -- <obj> */
    OP_INITSETTER,    /* <obj> <key> <closure> -- <obj> */

    OP_GETPROP,    /* <obj> <name> -- <value> */
    OP_GETPROP_S,    /* <obj> -S- <value> */
    OP_SETPROP,    /* <obj> <name> <value> -- <value> */
    OP_SETPROP_S,    /* <obj> <value> -S- <value> */
    OP_DELPROP,    /* <obj> <name> -- <success> */
    OP_DELPROP_S,    /* <obj> -S- <success> */

    OP_ITERATOR,    /* <obj> -- <iobj> */
    OP_NEXTITER,    /* <iobj> -- ( <iobj> <name> true | false ) */

    OP_EVAL,    /* <args...> -(numargs)- <returnvalue> */
    OP_CALL,    /* <closure> <this> <args...> -(numargs)- <returnvalue> */
    OP_NEW,        /* <closure> <args...> -(numargs)- <returnvalue> */

    OP_TYPEOF,
    OP_POS,
    OP_NEG,
    OP_BITNOT,
    OP_LOGNOT,
    OP_INC,        /* <x> -- ToNumber(x)+1 */
    OP_DEC,        /* <x> -- ToNumber(x)-1 */
    OP_POSTINC,    /* <x> -- ToNumber(x)+1 ToNumber(x) */
    OP_POSTDEC,    /* <x> -- ToNumber(x)-1 ToNumber(x) */

    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_ADD,
    OP_SUB,
    OP_SHL,
    OP_SHR,
    OP_USHR,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_JCASE,
    OP_CONCAT,
    OP_BITXOR,
    OP_BITOR,

    OP_INSTANCEOF,

    OP_DEBUGGER,
    OP_JUMP,
    OP_JTRUE,
    OP_JFALSE,
    OP_RETURN,

    OP_LINE,    /* -K- */
};

// Simple trick to convert back and forth between an intptr_t and bytecodes.
constexpr size_t OpSize = sizeof(intptr_t) / sizeof(instruction_t);
union SizeOpcode {
    intptr_t size;
    instruction_t ins[OpSize];
};

struct Function
{
    String name;
    int script = 0;
    int lightweight = 0;
    int strict = 0;
    int arguments = 0;
    int numparams = 0;

    std::vector<instruction_t> code;
    std::vector<Function*> funtab;
    std::vector<double> numtab;
    std::vector<String> strtab;
    std::vector<String> vartab;

    String filename;
    int line = 0, lastline = 0;

    Function *gcnext = nullptr;
    int gcmark = 0;
};

Function *jsC_compilefunction(Runtime *J, Ast *prog);

Function *jsC_compilescript(Runtime *J, Ast *prog, int default_strict);

const char *jsC_opcodestring(enum js_OpCode opcode);

void jsC_dumpfunction(Runtime *J, Function *fun);

} // namespace phonometrica

#endif // PHONOMETRICA_COMPILE_HPP
