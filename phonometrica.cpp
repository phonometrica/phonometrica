/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 26/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: main entry point.                                                                                         *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifdef PHON_GUI
#include <phon/gui/application.hpp>
#include <phon/gui/main_window.hpp>
#include <phon/application/sound.hpp>
#include <phon/utils/file_system.hpp>
#endif

#include <clocale>
#include <phon/runtime/toplevel.hpp>
#include <phon/utils/helpers.hpp>

using namespace phonometrica;

#ifdef PHON_ENABLE_LOGGING
void message_handler(QtMsgType, const QMessageLogContext&, const QString& msg)
{
	static QFile file(filesystem::join(filesystem::user_directory(), "phonometrica.log"));
	if (!file.isOpen()) file.open(QIODevice::Append);
	static QTextStream stream(&file);
	stream << msg << "\n";
	stream.flush();
};
#endif

int main(int argc, char *argv[])
{

#ifdef PHON_ENABLE_LOGGING
	qInstallMessageHandler(message_handler);
#endif

	PHON_LOG("*******************************************************");
	PHON_LOG("New session");
	Runtime rt;
	initialize(rt);
	PHON_LOG("Runtime initialized");

	try
    {
#ifdef PHON_GUI
	    Sound::set_sound_formats();

        bool text_mode = (argc > 1);

        PHON_LOG("Creating application");
        Application app(argc, argv);
#if PHON_WINDOWS
		QCoreApplication::addLibraryPath("./");
#endif
        rt.set_text_mode(text_mode);
        setlocale(LC_NUMERIC, "C"); // for proper floating point conversion
        PHON_LOG("Creating main window");
        MainWindow win(rt);
        auto version = utils::get_version();

        app.setOrganizationDomain("phonometrica-ling.org");
        app.setApplicationName("Phonometrica");
        app.setApplicationVersion(version.data());

        if (argc > 1)
        {
            return interpret(rt, argc, argv);
        }
        else
        {
            win.show();
            return app.exec();
        }
#else
	    setlocale(LC_NUMERIC, "C"); // for proper floating point conversion
        return interpret(runtime, argc, argv);
#endif
    }
    catch (std::bad_alloc &)
    {
        fprintf(stderr, "out of memory error\n");
    }
    catch (std::exception &e)
    {
        utils::print(stderr, e.what());
        utils::print(stderr, "\n");

        return 1;
    }
}
