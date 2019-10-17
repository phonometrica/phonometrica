#ifndef TRANSPHON_PHON_SCRIPT_INCLUDE
#define TRANSPHON_PHON_SCRIPT_INCLUDE

static const char *transphon_script = R"_(
var ui = {
    title: "TransPhon",
    width: 300,
    items: [
        { type: "label", text: "Choose output file:" },
        { type: "file_selector", name: "path", title: "Select text file...", default: "output.txt" },
        { type: "label", text: "Select layers separated by a comma, or leave empty to process all layers:" },
        { type: "field", name: "layers", default: "1"},
        { type: "label", text: "Choose annotations:"},
        { type: "label", text: "Choose event separator:"},
        { type: "radio_buttons", name: "separator", values: ["space", "new line", "none"] }    
    ]
}

var labels = []
var values = []

var n = 0

foreach var annot in get_annotations() do
	n++
	var path = annot.path
	values.append(String(n))
	labels.append(system.get_base_name(path))
end

var item = { "type": "check_list", "name": "annotations", "labels": labels, "values": values }
ui.items.insert(6, item)

var result = create_dialog(ui)

if result then

	var output_file = File(result.path,"w")
	
	var annotation_files = get_annotations()
	var n = 0
	
	if result.annotations == [] then
		foreach var annot in annotation_files do
			n++
			result.annotations.append(n)	
		end
	else 
		var temp_list = []
		foreach var n_annot in result.annotations do 
			temp_list.append(Number(n_annot))
		end
		result.annotations = temp_list
	end
	
	foreach var id in result.annotations do
	
		var annotation_file_name = labels[id]
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
			var nlayer = annotation_files[id].layer_count
			repeat
				n++
				tiers_list.append(n)
			until n == nlayer		
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
			
			var layer_title = (annotation_files[id].get_layer_label(tier))
			output_file.write_line("%%%% Layer: " + layer_title)
			output_file.write_line("")
		

			var regex_any_text = new Regex("[a-z]")
			var n_interval = annotation_files[id].get_event_count(tier)
	
			for j=1 to n_interval do 
				var text_in_interval = annotation_files[id].get_event_text(tier, j)
				
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
		output_file.write_line("")
		output_file.write_line("")
		output_file.write_line("")
	
	end
	output_file.close()
	print("Annotations were saved in '" + output_file.path + "'")
else
	alert("Annotations were not saved.")
end)_";

#endif /* TRANSPHON_PHON_SCRIPT_INCLUDE */