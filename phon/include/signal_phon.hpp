#ifndef SIGNAL_PHON_SCRIPT_INCLUDE
#define SIGNAL_PHON_SCRIPT_INCLUDE

static const char *signal_script = R"_(
local bindings = {}
local slot_id = 0

function create_signal()
local id = slot_id
slot_id += 1

return "__SIGNAL_" & id
end

function connect(id as String, slot as Function)
if not contains(bindings, id) then
bindings[id] = [ slot ]
else
local callbacks	= ref bindings[id]

foreach s in callbacks do
if slot == s then
return
end
end

append(callbacks, slot)
end
end

function disconnect(id as String, slot as Function)
if contains(bindings, id) then
local callbacks = ref bindings[id]

for i = callbacks.length downto 1 do
if callbacks[i] == slot then
remove_at(callbacks, i)
return
end
end
end
end

function emit(id as String, args as Object)
if contains(bindings, id) then
local callbacks = bindings[id]
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

SIGNAL_ANNOTATION_IMPORTED = "__SIGNAL_ANNOTATION_IMPORTED"
SIGNAL_SOUND_IMPORTED = "__SIGNAL_SOUND_IMPORTED"
SIGNAL_ANNOTATION_LOADED = "__SIGNAL_ANNOTATION_LOADED"
SIGNAL_SOUND_LOADED = "__SIGNAL_SOUND_LOADED"
SIGNAL_SCRIPT_LOADED = "__SIGNAL_SCRIPT_LOADED"
SIGNAL_DATASET_LOADED = "__SIGNAL_DATASET_LOADED"
SIGNAL_PROJECT_LOADED = "__SIGNAL_PROJECT_LOADED"

)_";

#endif /* SIGNAL_PHON_SCRIPT_INCLUDE */