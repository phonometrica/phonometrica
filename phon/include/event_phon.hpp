#ifndef EVENT_PHON_SCRIPT_INCLUDE
#define EVENT_PHON_SCRIPT_INCLUDE

const char *event_script = R"_(
var bindings = {}
var slot_id = 0

event = {}

event.create = function() 
    return "__evt_" + String(slot_id++)
end

event.connect = function(id, slot)
    var callbacks = bindings.get(id, null)

    if callbacks then
        foreach var s in callbacks do
            if slot == s then
                return
            end
        end

        callbacks.append(slot)
    else
        bindings[id] = [ slot ]
    end
end

event.disconnect = function(id, slot)
    var callbacks = bindings.get(id, null)

    if callbacks then
        for var i = callbacks.length downto 1 do
            if callbacks[i] == slot then
                callbacks.remove_at(i)
                return
            end
        end
    end
end

event.emit = function(id, arg)
    var callbacks = bindings.get(id, null)

    if callbacks then
		var result = []

        foreach var callback in callbacks do
            var r = callback(arg)
			result.append(r)
        end

		return result
    end
end

event.annotation_loaded = "__evt_annotation_loaded"
event.sound_loaded = "__evt_sound_loaded"
event.script_loaded = "__evt_script_loaded"
event.document_loaded = "__evt_document_loaded"
event.dataset_loaded = "__evt_dataset_loaded"
event.project_loaded = "__evt_project_loaded"

)_";

#endif /* EVENT_PHON_SCRIPT_INCLUDE */