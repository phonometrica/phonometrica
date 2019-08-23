#ifndef INITIALIZE_PHON_SCRIPT_INCLUDE
#define INITIALIZE_PHON_SCRIPT_INCLUDE

const char *initialize_script = R"_(

phon.load_plugins = function()
    var load_plugin = function(plugin)
        var init_file = system.join_path(plugin, "init.phon")

        if system.is_file(init_file) then
            load(init_file)
        end
    end

    var plugin_dir = phon.config.plugin_directory
    var dirs = system.list_directory(plugin_dir, false)

    foreach var v in dirs do
        var path = system.join_path(plugin_dir, v)
        load_plugin(path)
    end
end

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