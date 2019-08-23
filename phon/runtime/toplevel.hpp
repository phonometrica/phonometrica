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

// On macOS 10.13, we get the following error
// "call to unavailable function 'any_cast': introduced in macOS 10.14"
#if PHON_MACOS
#include <phon/third_party/any.hpp>
namespace std {
    using namespace linb;
}
#else
#include <any>
#endif
#include <functional>
#include <phon/string.hpp>

/* noreturn is a GCC extension */
#ifdef __GNUC__
#define PHON_NORETURN __attribute__((noreturn))
#else
#ifdef _MSC_VER
#define PHON_NORETURN __declspec(noreturn)
#else
#define PHON_NORETURN
#endif
#endif

/* GCC can do type checking of printf strings */
#ifdef __printflike
#define PHON_PRINTFLIKE __printflike
#else
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#define PHON_PRINTFLIKE(fmtarg, firstvararg) \
    __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#else
#define PHON_PRINTFLIKE(fmtarg, firstvararg)
#endif
#endif

namespace phonometrica {

class Environment;

using native_callback_t = std::function<void(Environment &J)>;
typedef void *(*alloc_callback_t)(void *memctx, void *ptr, int size);
typedef void (*panic_callback_t)(Environment *J);
typedef void (*finalize_callback_t)(Environment *J, std::any &any);
typedef int (*has_field_callback_t)(Environment *J, std::any &any, const String &name);
typedef int (*put_callback_t)(Environment *J, std::any &any, const String &name);
typedef int (*delete_callback_t)(Environment *J, std::any &any, const String &name);
typedef void (*report_callback_t)(Environment *J, const String &message);


/* Regex flags */
enum
{
    PHON_REGEXP_G = 1,
    PHON_REGEXP_I = 2,
    PHON_REGEXP_M = 4,
};

/* Field attribute flags */
enum
{
    PHON_READONLY = 1,
    PHON_DONTENUM = 2,
    PHON_DONTCONF = 4,
};


int js_getlength(Environment *J, int idx);
void js_setlength(Environment *J, int idx, int len);
int js_hasindex(Environment *J, int idx, int i);
void js_getindex(Environment *J, int idx, int i);
void js_setindex(Environment *J, int idx, int i);
void js_delindex(Environment *J, int idx, int i);



void js_dup(Environment *J);
void js_dup2(Environment *J);
void js_rot2(Environment *J);
void js_rot3(Environment *J);
void js_rot4(Environment *J);
void js_rot2pop1(Environment *J);
void js_rot3pop2(Environment *J);

void js_concat(Environment *J);
int js_compare(Environment *J, int *okay);
int js_equal(Environment *J);
int js_instanceof(Environment *J);

} // namespace phonometrica

#endif // PHONOMETRICA_TOPLEVEL_HPP
