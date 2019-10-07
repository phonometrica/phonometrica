#ifndef INITIALIZE_PHON_SCRIPT_INCLUDE
#define INITIALIZE_PHON_SCRIPT_INCLUDE

static const char *initialize_script = R"_(
var config = phon.config

config.get_documentation_page = function(page)
    page = system.nativize(page)
    return system.join_path(config.documentation_directory, page)
end


var dirs = [config.settings_directory, config.plugin_directory, config.metadata_directory, config.script_directory]

foreach var dir in dirs do
    if not system.exists(dir) then
        system.create_directory(dir)
    end 
end 

)_";

#endif /* INITIALIZE_PHON_SCRIPT_INCLUDE */