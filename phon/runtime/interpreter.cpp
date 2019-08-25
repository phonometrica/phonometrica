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

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <optional>
#if PHON_WINDOWS
#include <io.h>
#define isatty(x) _isatty(x)
#else
#include <unistd.h>
#endif
#include <phon/file.hpp>
#include <phon/runtime/toplevel.hpp>
#include <phon/utils/print.hpp>

namespace phonometrica {

static char *xoptarg; /* Global argument pointer. */
static int xoptind = 0; /* Global argv index. */
static int xgetopt(int argc, char *argv[], const char *optstring)
{
    static char *scan = nullptr; /* Private scan pointer. */

    char c;

    xoptarg = nullptr;

    if (!scan || *scan == '\0')
    {
        if (xoptind == 0)
            xoptind++;

        if (xoptind >= argc || argv[xoptind][0] != '-' || argv[xoptind][1] == '\0')
            return EOF;
        if (argv[xoptind][1] == '-' && argv[xoptind][2] == '\0')
        {
            xoptind++;
            return EOF;
        }

        scan = argv[xoptind] + 1;
        xoptind++;
    }

    c = *scan++;
    auto place = strchr(optstring, c);

    if (!place || c == ':')
    {
        fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
        return '?';
    }

    place++;
    if (*place == ':')
    {
        if (*scan != '\0')
        {
            xoptarg = scan;
            scan = nullptr;
        }
        else if (xoptind < argc)
        {
            xoptarg = argv[xoptind];
            xoptind++;
        }
        else
        {
            fprintf(stderr, "%s: option requires argument -%c\n", argv[0], c);
            return ':';
        }
    }

    return c;
}

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else

void using_history(void)
{}

void add_history(const String &s)
{}

void rl_bind_key(int key, void (*fun)(void))
{}

void rl_insert(void)
{}

std::optional<String> readline(const char *prompt)
{
    fputs(prompt, stdout);
    return utils::read_line();
}

#endif

#define PS1 "> "

static void builtin_gc(Runtime &rt)
{
    auto report = rt.to_boolean(1);
    rt.collect(report);
    rt.push_null();
}

static void builtin_load(Runtime &rt)
{
    int i, n = rt.top_count();
    for (i = 1; i < n; ++i)
    {
        rt.load_file(rt.to_string(i));
        rt.push_null();
        rt.call(0);
        rt.pop(1);
    }
    rt.push_null();
}

static void builtin_readline(Runtime &rt)
{
    auto line = readline("");
    if (!line)
    {
        rt.push_null();
        return;
    }
    rt.push(*line);
    if (!line->empty())
        add_history(*line);
}


static void builtin_compile(Runtime &rt)
{
    auto source = rt.to_string(1);
    auto filename = rt.is_defined(2) ? rt.to_string(2) : "[string]";
    rt.load_string(filename, source);
}

// TODO: set print handler in environment from print() and system.write()
static void builtin_print(Runtime &rt)
{
    int i, top = rt.top_count();
    for (i = 1; i < top; ++i)
    {
        auto s = rt.to_string(i);
        rt.print(s);
    }
    if (rt.is_text_mode())
    {
        rt.print("\n");
    }
    rt.push_null();
}

static void builtin_quit(Runtime &rt)
{
    exit(rt.to_number(1));
}

static void builtin_error(Runtime &rt)
{
    auto msg = rt.to_string(1);
    rt.pop();
    throw rt.raise("Error", "%s\n", msg.data());
}

static auto require_script = R"__(
function require(name)
    var cache = require.cache
    if name in cache then
        return cache[name]
    end
    var exports = {};
    cache[name] = exports
    var path = system.find_module(name)
    Function('exports', system.read_file(path))(exports)
    return exports
end
require.cache = Object.create(null)
)__";

static auto assert_script = R"__(
function assert(cond, msg)
    if not cond then
        if not msg then
            msg = "assertion failed"
        end
        error(msg)
    end
end
)__";

static int eval_print(Runtime *J, const String &source)
{
    try
    {
        J->load_string("[string]", source);
        J->push_null();
        J->pcall(0);
        J->pop(1);
        return 0;
    }
    catch (std::runtime_error &e)
    {
        utils::print(stderr, e.what());
        utils::print(stderr, "\n");
        return 1;
    }
}

static char *read_stdin()
{
    int n = 0;
    int t = 512;
    char *s = nullptr;

    for (;;)
    {
        char *ss = (char *) realloc(s, t);
        if (!ss)
        {
            free(s);
            fprintf(stderr, "cannot allocate storage for stdin contents\n");
            return nullptr;
        }
        s = ss;
        n += fread(s + n, 1, t - n - 1, stdin);
        if (n < t - 1)
            break;
        t *= 2;
    }

    if (ferror(stdin))
    {
        free(s);
        fprintf(stderr, "error reading stdin\n");
        return nullptr;
    }

    s[n] = 0;
    return s;
}

static void usage()
{
    fprintf(stderr, "Usage: phonometrica [options] [script [script_args*]]\n");
    fprintf(stderr, "\t-i: Start interpreter in console mode.\n");
    fprintf(stderr, "\t-v: Show version.\n");
    fprintf(stderr, "\t(no option) Start program with user interface");

    exit(1);
}

static int show_version()
{
    printf("Phonometrica version %d.%d.%d\n", PHON_VERSION_MAJOR, PHON_VERSION_MINOR, PHON_VERSION_MICRO);
    return 0;
}

void initialize(Runtime &rt)
{
    rt.new_native_function(builtin_gc, "gc", 0);
    rt.set_global("gc");

    rt.new_native_function(builtin_load, "load", 1);
    rt.set_global("load");

    rt.new_native_function(builtin_compile, "compile", 2);
    rt.set_global("compile");

    rt.new_native_function(builtin_print, "print", 0);
    rt.set_global("print");

    rt.new_native_function(builtin_readline, "read_line", 0);
    rt.set_global("read_line");

    rt.new_native_function(builtin_quit, "quit", 1);
    rt.set_global("quit");

    rt.new_native_function(builtin_error, "error", 1);
    rt.set_global("error");

    rt.do_string(require_script);
    rt.do_string(assert_script);
}

int interpret(Runtime &rt, int argc, char **argv)
{

    std::optional<String> input;

    int status = 0;
    int interactive = 0;
    int i, c;

    while ((c = xgetopt(argc, argv, "iv")) != -1)
    {
        switch (c)
        {
        default:
            usage();
            break;
        case 'i':
            interactive = 1;
            break;
        case 'v':
            return show_version();
        }
    }

    // 'J' stands for 'Javascript'. This is carried over from MuJS.
    auto J = &rt;
    initialize(rt);

    if (xoptind == argc)
    {
        interactive = 1;
    }
    else
    {
        c = xoptind++;

        J->new_list();
        i = 0;
        while (xoptind < argc)
        {
            J->push(argv[xoptind++]);
            js_setindex(J, -2, i++);
        }
        rt.set_global("script_args");

        if (J->do_file(argv[c]))
            status = 1;
    }

    if (interactive)
    {
        if (isatty(0))
        {
            using_history();
            rl_bind_key('\t', rl_insert);
            input = readline(PS1);
            while (input)
            {
                eval_print(J, *input);
                if (!input->empty())
                    add_history(*input);
        
                input = readline(PS1);
            }
            putchar('\n');
        }
        else
        {
            input = read_stdin();
            if (!input || !J->do_string(*input))
                status = 1;
        }
    }

    return status;
}


} // namespace phonometrica

