#ifndef WRITE_SETTINGS_PHON_SCRIPT_INCLUDE
#define WRITE_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *write_settings_script = R"_(
local settings = dump_json(phon.settings)
local f = File(get_config_path(), "w")

write_line(f, "# This file was generated automatically.")
write_line(f, "# Do not edit it unless you know what you are doing.")
write(f, "phon.settings = ")
write(f, settings)
close(f)

)_";

#endif /* WRITE_SETTINGS_PHON_SCRIPT_INCLUDE */