#ifndef WRITE_SETTINGS_PHON_SCRIPT_INCLUDE
#define WRITE_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *write_settings_script = R"_(
var f = new File(phon.config.path, "w")
var settings = json.stringify(phon.settings, null, 4)

f.write_line("# This file was generated automatically.")
f.write_line("# Do not edit it unless you know what you are doing.\n")
f.write("phon.settings = ")
f.write(settings)
f.close()

)_";

#endif /* WRITE_SETTINGS_PHON_SCRIPT_INCLUDE */