/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: read/write application settings. This class only has static methods.                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SETTINGS_HPP
#define PHONOMETRICA_SETTINGS_HPP

#include <phon/string.hpp>
#include <phon/runtime/variant.hpp>

namespace phonometrica {

class Runtime;


class Settings final
{
public:

    Settings() = delete;

    static void read(Runtime &rt);

    static void write(Runtime &rt);

    static String settings_directory();

    static String plugin_directory();

    static String metadata_directory();

    static String user_script_directory();

    static String config_path();

    static String get_string(Runtime &rt, const String &name);

    static bool get_boolean(Runtime &rt, const String &name);

    static double get_number(Runtime &rt, const String &name);

    static double get_number(Runtime &rt, const String &category, const String &name);

    static int get_int(Runtime &rt, const String &name);

    static Array<Variant> &get_list(Runtime &rt, const String &name);

    static void set_value(Runtime &rt, const String &key, String value);

    static void set_value(Runtime &rt, const String &key, bool value);

    static void set_value(Runtime &rt, const String &key, double value);

    static void set_value(Runtime &rt, const String &key, Array<Variant> value);

    static void set_value(Runtime &rt, const String &category, const String &key, double value);


    static void initialize(Runtime &rt);

    static String get_std_plugin_directory();

    static String get_std_script(Runtime &rt, String name);

    static String get_last_directory(Runtime &rt);

    static void set_last_directory(Runtime &rt, const String &path);
};

} // namespace phonometrica

#endif // PHONOMETRICA_SETTINGS_HPP
