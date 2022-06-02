#ifndef INITIALIZE_PHON_SCRIPT_INCLUDE
#define INITIALIZE_PHON_SCRIPT_INCLUDE

static const char *initialize_script = R"_(
local dirs = [get_settings_directory(), get_plugin_directory(), get_metadata_directory(), get_script_directory()]

foreach dir in dirs do
if not exists(dir) then
create_directory(dir)
end
end

)_";

#endif /* INITIALIZE_PHON_SCRIPT_INCLUDE */