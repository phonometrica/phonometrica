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

#ifndef PHONOMETRICA_TOPLEVEL_HPP
#define PHONOMETRICA_TOPLEVEL_HPP

#include <phon/runtime/common.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstdarg>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <climits>

/* Microsoft Visual C */
#ifdef _MSC_VER
#pragma warning(disable:4996) /* _CRT_SECURE_NO_WARNINGS */
#pragma warning(disable:4244) /* implicit conversion from double to int */
#pragma warning(disable:4267) /* implicit conversion of int to smaller int */
#define inline __inline
#if _MSC_VER < 1900 /* MSVC 2015 */
#define snprintf jsW_snprintf
#define vsnprintf jsW_vsnprintf
static int jsW_vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    int n;
    n = _vsnprintf(str, size, fmt, ap);
    str[size-1] = 0;
    return n;
}
static int jsW_snprintf(char *str, size_t size, const char *fmt, ...)
{
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = jsW_vsnprintf(str, size, fmt, ap);
    va_end(ap);
    return n;
}
#endif
#if _MSC_VER <= 1700 /* <= MSVC 2012 */
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define isfinite(x) _finite(x)
static __inline int signbit(double x) { __int64 i; memcpy(&i, &x, 8); return i>>63; }
#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
#endif
#endif

#define soffsetof(x, y) ((int)offsetof(x,y))
#define nelem(a) (int)(sizeof (a) / sizeof (a)[0])

#ifdef __GNUC__
#   define likely(x) __builtin_expect((x),1)
#   define unlikely(x) __builtin_expect((x),0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif

/* Limits */

#define PHON_STACKSIZE 1024      /* value stack size */
#define PHON_ENVLIMIT  64        /* environment stack size */
#define PHON_GCLIMIT   10000     /* run gc cycle every N allocations */
#define PHON_ASTLIMIT  100       /* max nested expressions */


namespace phonometrica {


struct Variant;
class Object;
struct Ast;
struct Function;
struct Environment;
struct StackTrace;
class Runtime;


struct StackTrace
{
    StackTrace() = default;
    ~StackTrace() = default;

    String name;
    String file;
    int line = 0;
};


int interpret(Runtime &rt, int argc, char **argv);

void initialize(Runtime &rt);


/* instruction size -- change to int if you get integer overflow syntax errors */
typedef unsigned short instruction_t;


void dump_strings(Runtime *J);


/* Portable strtod and printf float formatting */

void fmt_exp(char *p, int e);

int grisu2(double v, char *buffer, int *K);

double str_to_double(const char *as, char **aas);

/* Private stack functions */

void create_function(Runtime *J, Function *function, Environment *scope);

void create_script(Runtime *J, Function *function, Environment *scope);

void load_eval(Runtime *J, const String &filename, const String &source);

int is_array_index(Runtime *J, const String &str, int *idx);


void trap(Runtime *J, int pc); /* dump stack and environment to stdout */


} // namespace phonometrica

#endif // PHONOMETRICA_TOPLEVEL_HPP

#include "runtime.hpp"