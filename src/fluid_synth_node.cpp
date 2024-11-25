#include "fluid_synth_node.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/input_event_midi.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <iostream>

using namespace godot;

void FluidSynthNode::_bind_methods() {
    // Settings methods
	ClassDB::bind_method(D_METHOD("settings_create"), &FluidSynthNode::settings_create);
	ClassDB::bind_method(D_METHOD("settings_change_int", "setting", "value"), &FluidSynthNode::settings_change_int);
	ClassDB::bind_method(D_METHOD("settings_change_dbl", "setting", "value"), &FluidSynthNode::settings_change_dbl);
	ClassDB::bind_method(D_METHOD("settings_change_str", "setting", "value"), &FluidSynthNode::settings_change_str);
	ClassDB::bind_method(D_METHOD("settings_get_int", "setting"), &FluidSynthNode::settings_get_int);
	ClassDB::bind_method(D_METHOD("settings_get_dbl", "setting"), &FluidSynthNode::settings_get_dbl);
	ClassDB::bind_method(D_METHOD("settings_get_str", "setting"), &FluidSynthNode::settings_get_str);
	ClassDB::bind_method(D_METHOD("settings_as_json_str"), &FluidSynthNode::settings_as_json_str);
	ClassDB::bind_method(D_METHOD("settings_save", "file_name"), &FluidSynthNode::settings_save);
	ClassDB::bind_method(D_METHOD("settings_load_from_json", "json_str"), &FluidSynthNode::settings_load_from_json);
	ClassDB::bind_method(D_METHOD("settings_load", "file_name"), &FluidSynthNode::settings_load);
	ClassDB::bind_method(D_METHOD("settings_delete"), &FluidSynthNode::settings_delete);

    // Synth methods
	ClassDB::bind_method(D_METHOD("synth_create", "sf_path", "listen_ext_input"), &FluidSynthNode::synth_create);
	ClassDB::bind_method(D_METHOD("synth_soundfont_load", "sf_path", "reset"), &FluidSynthNode::synth_soundfont_load);
	ClassDB::bind_method(D_METHOD("synth_delete"), &FluidSynthNode::synth_delete);
	ClassDB::bind_method(D_METHOD("synth_map_channel", "channel", "mapped_channel"), &FluidSynthNode::synth_map_channel);
	ClassDB::bind_method(D_METHOD("synth_setup_channel", "channel", "sfont_id", "bank_num", "program", "reverb", "chorus",
        "volume", "pan", "expression"), &FluidSynthNode::synth_setup_channel);
	ClassDB::bind_method(D_METHOD("synth_set_interpolation", "val"), &FluidSynthNode::synth_set_interpolation);
	ClassDB::bind_method(D_METHOD("synth_render_file", "midi_file", "output_file", "sf_path",
        "interpolation", "sample_rate", "bit_depth", "file_type"),
        &FluidSynthNode::synth_render_file);
	ClassDB::bind_method(D_METHOD("synth_soundfont_name", "sfont_id"), &FluidSynthNode::synth_soundfont_name);
	ClassDB::bind_method(D_METHOD("synth_soundfont_reset_presets", "sfont_id"), &FluidSynthNode::synth_soundfont_reset_presets);
	ClassDB::bind_method(D_METHOD("synth_soundfont_next_preset", "sfont_id"), &FluidSynthNode::synth_soundfont_next_preset);
	ClassDB::bind_method(D_METHOD("synth_play_messages", "indices", "data"), &FluidSynthNode::synth_play_messages);
	ClassDB::bind_method(D_METHOD("synth_system_reset"), &FluidSynthNode::synth_system_reset);

    // Player methods
	ClassDB::bind_method(D_METHOD("player_create"), &FluidSynthNode::player_create);
	ClassDB::bind_method(D_METHOD("player_delete"), &FluidSynthNode::player_delete);
	ClassDB::bind_method(D_METHOD("player_load_midi", "file_path"), &FluidSynthNode::player_load_midi);
	ClassDB::bind_method(D_METHOD("player_play", "loop_count"), &FluidSynthNode::player_play);
	ClassDB::bind_method(D_METHOD("player_seek", "tick"), &FluidSynthNode::player_seek);
	ClassDB::bind_method(D_METHOD("player_stop"), &FluidSynthNode::player_stop);
}

FluidSynthNode::FluidSynthNode() {
    settings = NULL;
    player = NULL;
    synth = NULL;
    adriver = NULL;
    for (int i = 0; i < 16; ++i) {
        channel_map[i] = i;
    }
    set_process_input(false);
}

FluidSynthNode::~FluidSynthNode() {
    player_delete();
    settings_delete();
    settings = NULL;
    player = NULL;
	if (synth != NULL) {
        synth_delete();
    }
    synth = NULL;
    adriver = NULL;
}

int FluidSynthNode::settings_create() {
    // Create the settings.
    settings = new_fluid_settings();
    if (settings == NULL) {
        WARN_PRINT_ED("Failed to create FluidSynth settings");
        return -1;
    }

    return 0;
}

int FluidSynthNode::settings_change_int(String setting, int value){
    if (settings != NULL) {
        if (fluid_settings_setint(settings, setting.ascii(), value) == FLUID_FAILED) {
            WARN_PRINT_ED(vformat("Failed to change setting: %s", setting));
            return -1;
        }
    }
    else {
        WARN_PRINT_ED("No FluidSynth settings to modify");
        return -1;
    }
    return 0;
}

int FluidSynthNode::settings_change_dbl(String setting, double value){
    if (settings != NULL) {
        if (fluid_settings_setnum(settings, setting.ascii(), value) == FLUID_FAILED) {
            WARN_PRINT_ED(vformat("Failed to change setting: %s", setting));
            return -1;
        }
    }
    else {
        WARN_PRINT_ED("No FluidSynth settings to modify");
        return -1;
    }
    return 0;
}

int FluidSynthNode::settings_change_str(String setting, String value){
    if (settings != NULL) {
        if (fluid_settings_setstr(settings, setting.ascii(), value.ascii()) == FLUID_FAILED) {
            WARN_PRINT_ED(vformat("Failed to change setting: %s", setting));
            return -1;
        }
    }
    else {
        WARN_PRINT_ED("No FluidSynth settings to modify");
        return -1;
    }
    return 0;
}

double FluidSynthNode::settings_get_dbl(String setting)
{
    if (settings != NULL)
    {
        if (fluid_settings_get_type(settings, setting.ascii()) == FLUID_NUM_TYPE)
        {
            double num;
            if (fluid_settings_getnum(settings, setting.ascii(), &num) == FLUID_OK){
                return num;
            }
        }
    }
    return -1.0;
}

int FluidSynthNode::settings_get_int(String setting)
{
    if (settings != NULL)
    {
        if (fluid_settings_get_type(settings, setting.ascii()) == FLUID_INT_TYPE)
        {
            int num;
            if (fluid_settings_getint(settings, setting.ascii(), &num) == FLUID_OK){
                return num;
            }
        }
    }
    return -1;
}

String FluidSynthNode::settings_get_str(String setting)
{
    if (settings != NULL)
    {
        if (fluid_settings_get_type(settings, setting.ascii()) == FLUID_STR_TYPE)
        {
            char** strval;
            if (fluid_settings_dupstr(settings, setting.ascii(), strval) == FLUID_OK){
                String rtn = String(*strval);
                fluid_free(strval);
                return rtn;
            }
        }
    }
    return String("");
}

void copy_setting(void *data, const char *name, int type) {
    fluid_settings_t** settings_array = (fluid_settings_t**)data;
    fluid_settings_t *original = settings_array[0];
    fluid_settings_t *copy = settings_array[1];

    switch(type) {
        case FLUID_NUM_TYPE:
            double num;
            if (fluid_settings_getnum(original, name, &num) == FLUID_OK){
                fluid_settings_setnum(copy, name, num);
            }
            break;
        case FLUID_INT_TYPE:
            int intval;
            if (fluid_settings_getint(original, name, &intval) == FLUID_OK){
                fluid_settings_setint(copy, name, intval);
            }
            break;
        case FLUID_STR_TYPE:
        case FLUID_SET_TYPE:
            char** strval;
            if (fluid_settings_dupstr(original, name, strval) == FLUID_OK){
                fluid_settings_setstr(copy, name, *strval);
            }
            break;
    }
}


int FluidSynthNode::settings_copy(fluid_settings_t *original, fluid_settings_t *copy) {
    if (original == NULL) {
        WARN_PRINT_ED("No FluidSynth settings to copy");
        return -1;
    }

    copy = new_fluid_settings();

    fluid_settings_t* settings_array[] = { original, copy };

    fluid_settings_foreach(original, settings_array, (fluid_settings_foreach_t)&copy_setting);

    return 0;
}


void get_settings_names(void *data, const char *name, int type) {
    char* names = (char *)data;
    if (strlen(names) + strlen(name) + 1 < 4096)
    {
        strcat(names, name);
        strcat(names, "|");
    }
}


String FluidSynthNode::settings_as_json_str()
{
    String json_str = String("{}");
    if (settings)
    {
        char buffer[4096];
        buffer[0] = 0;

        fluid_settings_foreach(settings, buffer, (fluid_settings_foreach_t)&get_settings_names);

        String names = String(buffer);
        if (names.length() > 0)
        {
            Dictionary settings_dict = {};
            PackedStringArray name_array = names.split("|");
            for (int i = 0; i < name_array.size(); ++i)
            {
                String name = name_array[i];
                if (name.length() > 0)
                {
                    switch (fluid_settings_get_type(settings, name.ascii()))
                    {
                        case FLUID_NUM_TYPE:
                            double num;
                            if (fluid_settings_getnum(settings, name.ascii(), &num) == FLUID_OK){
                                settings_dict[name] = num;
                            }
                            break;
                        case FLUID_INT_TYPE:
                            int intval;
                            if (fluid_settings_getint(settings, name.ascii(), &intval) == FLUID_OK){
                                settings_dict[name] = intval;
                            }
                            break;
                        case FLUID_STR_TYPE:
                        case FLUID_SET_TYPE:
                            char** strval;
                            if (fluid_settings_dupstr(settings, name.ascii(), strval) == FLUID_OK){
                                settings_dict[name] = String(*strval);
                                fluid_free(strval);
                            }
                            break;
                    }
                }
            }
            json_str = JSON::stringify(settings_dict);
        }
        else
        {
            WARN_PRINT_ED("FluidSynth setting names could not be retrieved");
        }
    }
    else
    {
        WARN_PRINT_ED("No FluidSynth settings to save");
    }
    return json_str;
}


int FluidSynthNode::settings_save(String file_name)
{
    if (settings)
    {
        String json = settings_as_json_str();
        Ref<FileAccess> save_file = FileAccess::open("user://" + file_name, FileAccess::ModeFlags::WRITE);
        save_file->store_line(json);
        save_file->close();
    }
    else
    {
        WARN_PRINT_ED("No FluidSynth settings to save");
        return -1;
    }
    return 0;
}


int FluidSynthNode::settings_load_from_json(String json_str)
{
    if (settings == NULL)
    {
        if (settings_create() != 0)
        {
            WARN_PRINT_ED("Could not create new FluidSynth settings");
            return -1;
        }
    }


    Dictionary nodeData = JSON::parse_string(json_str);
    if (!nodeData)
    {
        WARN_PRINT_ED("FluidSynth Could not parse settings file");
        return -1;
    }

    // Now we set the remaining variables.
    Array keys = nodeData.keys();
    for (int i = 0; i < keys.size(); ++i)
    {
        String key = keys[i];
        Variant value = nodeData[key];
        switch(fluid_settings_get_type(settings, key.ascii()))
        {
            case FLUID_STR_TYPE:
            case FLUID_SET_TYPE:
                settings_change_str(key, value);
                break;
            case FLUID_INT_TYPE:
                settings_change_int(key, value);
                break;
            case FLUID_NUM_TYPE:
                settings_change_dbl(key, value);
                break;
            default:
                break;
        }
    }
    return 0;
}


int FluidSynthNode::settings_load(String file_name)
{
    if (!FileAccess::file_exists("user://" + file_name))
    {
        WARN_PRINT_ED(vformat("FluidSynth settings file not found: %s", file_name));
        return -1;
    }

    if (settings == NULL)
    {
        if (settings_create() != 0)
        {
            WARN_PRINT_ED("Could not create new FluidSynth settings");
            return -1;
        }
    }

    // Load the file line by line and process that dictionary to restore the object
    // it represents.
    Ref<FileAccess> save_file = FileAccess::open("user://" + file_name, FileAccess::ModeFlags::READ);

    while (save_file->get_position() < save_file->get_length())
    {
        String jsonString = save_file->get_line();

        if (settings_load_from_json(jsonString) == -1)
        {
            WARN_PRINT_ED("FluidSynth Failed to load settings from JSON");
            continue;
        }
    }
    save_file->close();
    return 0;
}

int FluidSynthNode::settings_delete() {

    if (settings != NULL) {
        delete_fluid_settings(settings);
    }
    settings = NULL;

    return 0;
}

