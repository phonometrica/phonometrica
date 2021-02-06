#ifndef EVENT_PHON_SCRIPT_INCLUDE
#define EVENT_PHON_SCRIPT_INCLUDE

static const char *event_script = R"_(
local bindings = {}
local slot_id = 0

function new_event()
local id = slot_id
slot_id += 1
return "__evt_" & id
end

function connect(id as String, slot as Function)
local callbacks = ref get(bindings, id)

if callbacks then
foreach s in callbacks do
if slot == s then
return
end
end

append(callbacks, slot)
else
bindings[id] = [ slot ]
end
end

function disconnect(id as String, slot as Function)
local callbacks = ref get(bindings, id)

if callbacks then
for i = callbacks.length downto 1 do
if callbacks[i] == slot then
remove_at(callbacks, i)
return
end
end
end
end

function emit(id as String, args as Object)
local callbacks = ref get(bindings, id)

if callbacks then
local result = []

foreach callback in callbacks do
append(result, callback(args))
end

return result
end
end

function emit(id as String)
emit(id, null)
end

EVT_ANNOTATION_LOADED = "__evt_annotation_loaded"
EVT_SOUND_LOADED = "__evt_sound_loaded"
EVT_SCRIPT_LOADED = "__evt_document_loaded"
EVT_DOCUMENT_LOADED = "__evt_script_loaded"
EVT_DATASET_LOADED = "__evt_dataset_loaded"
EVT_PROJECT_LOADED = "__evt_project_loaded"

)_";

#endif /* EVENT_PHON_SCRIPT_INCLUDE */