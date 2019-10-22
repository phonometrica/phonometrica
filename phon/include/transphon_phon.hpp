#ifndef TRANSPHON_PHON_SCRIPT_INCLUDE
#define TRANSPHON_PHON_SCRIPT_INCLUDE

static const char *transphon_script = R"_(
var default_output = system.join_path(system.user_directory, "output.txt")
var ui = {
    title: "TransPhon",
    width: 300,
    items: [
        { type: "label", text: "Choose output file:" },
        { type: "file_selector", name: "path", title: "Select text file...", default: default_output },
        { type: "label", text: "Select layers separated by a comma, or leave empty to process all layers:" },
        { type: "field", name: "layers", default: "1"},
        { type: "label", text: "Choose annotations:"},
        { type: "label", text: "Choose event separator:"},
        { type: "radio_buttons", name: "separator", values: ["space", "new line", "none"] }    
    ]
}

var labels = []
var values = []

var annotation_files = get_annotations()

foreach var annot in annotation_files do
	var path = annot.path
	values.append(path)
	labels.append(system.get_base_name(path))
end

var item = { "type": "check_list", "name": "annotations", "labels": labels, "values": values }
ui.items.insert(6, item)

var result = create_dialog(ui)

if result then

	var path = result.path
	var output_file = File(path, "w")
	
	
	if result.annotations == [] then
		for var i=1 to annotation_files.length do 
			result.annotations.append(annotation_files[i].path)
		end
	end

	foreach var id in result.annotations do

		for var j=1 to annotation_files.length do
			if	annotation_files[j].path == id then
	
				var annotation_file_name = system.get_base_name(annotation_files[j].path)
				var annotation_file_name_length = annotation_file_name.length
				
				var esthetic = ""
		
				for k=1 to annotation_file_name_length do
					esthetic = esthetic + "%"
				end
				
				output_file.write_line(esthetic)
				output_file.write_line(annotation_file_name)
				output_file.write_line(esthetic)
				output_file.write_line("")
			
				var tiers_list = [ ]
				var n = 0
		
				if result.layers == "" then
					var nlayer = annotation_files[j].layer_count
					for var l=1 to nlayer do
						tiers_list.append(l)
					end
				else													
					var regex_layer = new Regex("[1-9]")
					var nlayer_temp = result.layers
					repeat
						regex_layer.match(nlayer_temp)
						tiers_list.append(Number(regex_layer.group()))
						nlayer_temp = nlayer_temp.remove(regex_layer.group())
					until regex_layer.match(nlayer_temp) == false		
				end
				
				foreach var tier in tiers_list do
					
					var layer_title = (annotation_files[j].get_layer_label(tier))
					output_file.write_line("%%%% Layer: " + layer_title)
					output_file.write_line("")
				
		
					var regex_any_text = new Regex("[a-z]")
					var n_interval = annotation_files[j].get_event_count(tier)
			
					for m=1 to n_interval do 
						var text_in_interval = annotation_files[j].get_event_text(tier, m)
						
						if regex_any_text.match(text_in_interval) then
							
							if result.separator == 1 then
								output_file.write(text_in_interval + " ")
							elsif result.separator == 2 then
								output_file.write_line(text_in_interval)				
							elsif result.separator == 3 then
								output_file.write(text_in_interval)
							end
						end
					
					end
				
					output_file.write_line("")
					output_file.write_line("")
				end

			end	

		end	
		output_file.write_line("")
		output_file.write_line("")
		output_file.write_line("")
	
	end
	output_file.close()

	var show = ask("Annotations have been written to '" + path + "'.\nWould you like to view the file?")
	
	if show then
		view_text(path, system.get_base_name(path), 500)
	end
end)_";

#endif /* TRANSPHON_PHON_SCRIPT_INCLUDE */