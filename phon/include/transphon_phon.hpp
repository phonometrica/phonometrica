#ifndef TRANSPHON_PHON_SCRIPT_INCLUDE
#define TRANSPHON_PHON_SCRIPT_INCLUDE

static const char *transphon_script = R"_(
local labels = []
local values = []

local annotation_files = get_annotations()

foreach annot in annotation_files do
local path = annot.path
append(values, path)
append(labels, get_base_name(path))
end

local default_output = join_path(get_user_directory(), "output.txt")
local ui = {
"title": "TransPhon",
"width": 500,
"height": 550,
"items": [
{ "type": "label", "text": "Choose output file:" },
{ "type": "file_selector", "name": "path", "title": "Select text file...", "default": default_output, "save": true },
{ "type": "label", "text": "Select layers separated by a comma, or leave empty to process all layers:" },
{ "type": "field", "name": "layers", "default": "1"},
{ "type": "label", "text": "Choose annotations:"},
{ "type": "check_list", "name": "annotations", "labels": labels, "values": values },
{ "type": "label", "text": "Choose event separator:"},
{ "type": "radio_buttons", "name": "separator", "values": ["space", "new line", "none"] }
]
}

local result = create_dialog(ui)

if result then
local sep = result["separator"]

local path = result["path"]
local output_file = File(path, "w")


local annotations = ref result["annotations"]
if is_empty(annotations) then
for i = 1 to annotation_files.length do
append(annotations, annotation_files[i].path)
end
end

foreach id in annotations do

for j = 1 to annotation_files.length do
if	annotation_files[j].path == id then

local annotation_file_name = get_base_name(annotation_files[j].path)
local annotation_file_name_length = annotation_file_name.length

local esthetic = ""

for k = 1 to annotation_file_name_length do
esthetic = esthetic & "%"
end

write_line(output_file, esthetic)
write_line(output_file, annotation_file_name)
write_line(output_file, esthetic)
write_line(output_file, "")

local tiers_list = [ ]
local n = 0

if is_empty(result["layers"]) then
local nlayer = annotation_files[j].nlayer
for l = 1 to nlayer do
append(tiers_list, l)
end
else
local nlayer_temp = result["layers"]
foreach l in split(nlayer_temp, ",") do
append(tiers_list, int(l))
end
end

foreach tier in tiers_list do

local layer_title = get_layer_label(annotation_files[j], tier)
write_line(output_file, "%%%% Layer: " & layer_title)
write_line(output_file, "")

local n_interval = get_event_count(annotation_files[j], tier)

for m = 1 to n_interval do
local text_in_interval = get_event_text(annotation_files[j], tier, m)

if is_empty(text_in_interval) then
continue
end

if sep == 1 then
write(output_file, text_in_interval & " ")
elsif sep == 2 then
write_line(output_file, text_in_interval)
elsif sep == 3 then
write(output_file, text_in_interval)
end
end

write_line(output_file, "")
write_line(output_file, "")
end
end

end
write_line(output_file, "")
write_line(output_file, "")
write_line(output_file, "")

end
close(output_file)

local show = ask("Annotations have been written to '" & path & "'.\nWould you like to view the file?")

if show then
view_text(path, get_base_name(path))
end
end
)_";

#endif /* TRANSPHON_PHON_SCRIPT_INCLUDE */