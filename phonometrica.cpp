#include <iostream>
#ifdef PHON_GUI
#include <phon/gui/application.hpp>
#include <phon/application/settings.hpp>
#else
#include <phon/runtime.hpp>
#endif
#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/initialize_phon.hpp>
#endif

#if PHON_MACOS
#include <phon/utils/file_system.hpp>
#include <phon/application/settings.hpp>
#include <cstdlib>
#endif

using namespace phonometrica;

static void show_usage()
{
	std::cout << "Usage: program [option] file" << std::endl;
	std::cout << "Options: " << std::endl;
	std::cout << " -l\t(list)\tlist bytecode (disassemble) file" << std::endl;
	std::cout << " -r\t(run)\texecute file" << std::endl;
	std::cout << " -a\t(all)\tdisassemble and execute file" << std::endl;
}

static void initialize(Runtime &rt)
{
	rt["phon"] = make_handle<Module>(&rt, "phon");

#ifdef PHON_GUI
	// On macOS, move old settings from ~/Applications/Phonometrica to ~/.phonometrica if the user had a version of
	// Phonometrica prior to 0.8.
#if PHON_MACOS
	using namespace filesystem;
	auto old_settings = join(user_directory(), "Applications", "Phonometrica");
	if (!exists(Settings::settings_directory()) && exists(old_settings))
	{
		std::system("mv ~/Applications/Phonometrica ~/.phonometrica");
	}
#endif

	Settings::initialize(&rt);
	Settings::read();
	run_script(rt, initialize);
#endif
}

static void finalize(Runtime &)
{
#ifdef PHON_GUI
	Settings::write();
#endif
}

#if PHON_WINDOWS
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	Runtime runtime;
#if PHON_WINDOWS
	int argc = 1;
	char **argv = nullptr;
#endif
	runtime.set_text_mode(argc > 1);
	initialize(runtime);

#ifdef PHON_GUI
#if PHON_WINDOWS
    SetProcessDPIAware();
#endif
	wxApp *app = new Application(runtime, "");// argv[0]);
	wxApp::SetInstance(app);
#endif

	int error_code = 0;

	try
	{
		if (argc > 2)
		{
			String option(argv[1]), path(argv[2]);

			if (option == "-l") // list
			{
				auto closure = runtime.compile_file(path);
				runtime.disassemble(*closure, "main");
			}
			else if (option == "-r") // run
			{
				runtime.do_file(path);
			}
			else if (option == "-a") // all
			{
				auto closure = runtime.compile_file(path);
				runtime.disassemble(*closure, "main");
				puts("-------------------------------------------------------------------\n");
				runtime.interpret(closure);
			}
			else
			{
				show_usage();
				error_code = 1;
			}
		}
		else if (argc > 1)
		{
			String path(argv[1]);
			runtime.do_file(path);
		}
		else
		{
#ifdef PHON_GUI


#if PHON_WINDOWS
            error_code = wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
#else
			error_code = wxEntry(argc, argv);
#endif // PHON_WINDOWS
#else
			show_usage();
			error_code = 1;
#endif
		}
	}
	catch (RuntimeError &e)
	{
		utils::fprintf(stderr, "Error on line %:\n", e.line_no());
		utils::print(stderr, e.what());
		utils::print(stderr, "\n");
		error_code = 1;

	}
	catch (std::bad_alloc &)
	{
		utils::print(stderr, "out of memory error\n");
		utils::print(stderr, "\n");
		error_code = 1;
	}
	catch (std::exception &e)
	{
		utils::print(stderr, e.what());
		utils::print(stderr, "\n");
		error_code = 1;
	}
	finalize(runtime);

	return error_code;
}
