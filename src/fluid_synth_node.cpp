#include "fluid_synth_node.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/input_event_midi.hpp>

using namespace godot;

void FluidSynthNode::_bind_methods() {
    // Settings methods
	ClassDB::bind_method(D_METHOD("create_settings"), &FluidSynthNode::create_settings);
	ClassDB::bind_method(D_METHOD("change_setting_int", "setting", "value"), &FluidSynthNode::change_setting_int);
	ClassDB::bind_method(D_METHOD("change_setting_dbl", "setting", "value"), &FluidSynthNode::change_setting_dbl);
	ClassDB::bind_method(D_METHOD("change_setting_str", "setting", "value"), &FluidSynthNode::change_setting_str);
	ClassDB::bind_method(D_METHOD("delete_settings"), &FluidSynthNode::delete_settings);

    // Synth methods
	ClassDB::bind_method(D_METHOD("load_synth", "sf_path", "listen_ext_input"), &FluidSynthNode::load_synth);
	ClassDB::bind_method(D_METHOD("load_soundfont", "sf_path", "reset"), &FluidSynthNode::load_soundfont);
	ClassDB::bind_method(D_METHOD("unload_synth"), &FluidSynthNode::unload_synth);
	ClassDB::bind_method(D_METHOD("map_channel", "channel", "mapped_channel"), &FluidSynthNode::map_channel);
	ClassDB::bind_method(D_METHOD("setup_channel", "channel", "program", "reverb", "chorus",
        "volume", "pan", "expression"), &FluidSynthNode::setup_channel);
	ClassDB::bind_method(D_METHOD("set_interpolation", "val"), &FluidSynthNode::set_interpolation);
	ClassDB::bind_method(D_METHOD("render_file", "midi_file", "output_file", "sf_path",
        "interpolation", "sample_rate", "bit_depth", "file_type"),
        &FluidSynthNode::render_file);

    // Player methods
	ClassDB::bind_method(D_METHOD("create_midi_player"), &FluidSynthNode::create_midi_player);
	ClassDB::bind_method(D_METHOD("delete_midi_player"), &FluidSynthNode::delete_midi_player);
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
    delete_midi_player();
    delete_settings();
    settings = NULL;
    player = NULL;
	if (synth != NULL) {
        unload_synth();
    }
    synth = NULL;
    adriver = NULL;
}

int FluidSynthNode::create_settings() {
    // Create the settings.
    settings = new_fluid_settings();
    if (settings == NULL) {
        WARN_PRINT_ED("Failed to create FluidSynth settings");
        return -1;
    }

    return 0;
}

int FluidSynthNode::change_setting_int(String setting, int value){
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

int FluidSynthNode::change_setting_dbl(String setting, double value){
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

int FluidSynthNode::change_setting_str(String setting, String value){
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

int FluidSynthNode::copy_settings(fluid_settings_t *original, fluid_settings_t *copy) {
    if (original == NULL) {
        WARN_PRINT_ED("No FluidSynth settings to copy");
        return -1;
    }

    copy = new_fluid_settings();

    fluid_settings_t* settings_array[] = { original, copy };

    fluid_settings_foreach(original, settings_array, (fluid_settings_foreach_t)&copy_setting);

    return 0;
}

void FluidSynthNode::copy_setting(void *data, const char *name, int type) {
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
            char** strval;
            if (fluid_settings_dupstr(original, name, strval) == FLUID_OK){
                fluid_settings_setstr(copy, name, *strval);
            }
            break;
        case FLUID_SET_TYPE:
            break;
    }
}

int FluidSynthNode::delete_settings() {

    if (settings != NULL) {
        delete_fluid_settings(settings);
    }
    settings = NULL;

    return 0;
}

int FluidSynthNode::load_synth(String sf_path, bool listen_ext_input) {
    // Don't create if synth exists
    if (synth != NULL) {
        WARN_PRINT_ED("FluidSynth instance exists, unload before creating a new one");
        return -1;
    }

    if (settings == NULL) {
        WARN_PRINT_ED("FluidSynth settings have not been created");
        return -1;
    }

    // Create the synthesizer
    synth = new_fluid_synth(settings);
    if(synth == NULL)
    {
        unload_synth();
        WARN_PRINT_ED("Failed to create FluidSynth");
        return -1;
    }

    // Load the soundfont
    if (load_soundfont(sf_path, true) == -1) {
        unload_synth();
        return -1;
    }

    /* Create the audio driver. The synthesizer starts playing as soon
       as the driver is created. */
    adriver = new_fluid_audio_driver(settings, synth);
    if (adriver == NULL)
    {
        unload_synth();
        WARN_PRINT_ED("Failed to create audio driver for FluidSynth");
        return -1;
    }

    set_process_input(listen_ext_input);

    return 0;
}

int FluidSynthNode::load_soundfont(String sf_path, bool reset) {

    /* Load a SoundFont and reset presets (so that new instruments
     * get used from the SoundFont)
     * Depending on the size of the SoundFont, this will take some time to complete...
     */
    if (synth == NULL) {
        WARN_PRINT_ED("Create a FluidSynth instance before loading a SoundFont");
        return -1;
    }

    int cur_sfont_id = fluid_synth_sfload(synth, sf_path.ascii(), reset ? 1 : 0);
    if (cur_sfont_id == FLUID_FAILED) {
        WARN_PRINT_ED(vformat("Failed to load SoundFont: %s", sf_path));
        return -1;
    }

    return cur_sfont_id;
}

int FluidSynthNode::unload_synth() {
    /* Clean up */
    set_process_input(false);
    delete_fluid_audio_driver(adriver);
    adriver = NULL;
    delete_fluid_synth(synth);
    synth = NULL;

    return 0;
}

void FluidSynthNode::map_channel(int channel, int mapped_channel) {
    channel_map[channel] = mapped_channel;
}

int FluidSynthNode::setup_channel(int channel, int program, int reverb, int chorus,
    int volume, int pan, int expression) {
    
    if (synth == NULL) {
        WARN_PRINT_ED("Create a FluidSynth instance before channel setup");
        return -1;
    }
    
    // Reset all controllers
    fluid_synth_cc(synth, channel, 121, 0);

    fluid_synth_program_change(synth, channel, program);
    fluid_synth_cc(synth, channel, 91, reverb);
    fluid_synth_cc(synth, channel, 93, chorus);
    fluid_synth_cc(synth, channel, 7, volume);
    fluid_synth_cc(synth, channel, 10, pan);
    fluid_synth_cc(synth, channel, 11, expression);

    return 0;
}

int FluidSynthNode::set_interpolation(int method) {
    fluid_interp interp_method = FLUID_INTERP_DEFAULT;
    switch(method) {
        case 0:
            interp_method = FLUID_INTERP_NONE;
            break;
        case 1:
            interp_method = FLUID_INTERP_LINEAR;
            break;
        case 2:
            interp_method = FLUID_INTERP_4THORDER;
            break;
        case 3:
            interp_method = FLUID_INTERP_7THORDER;
            break;
    }

    if (fluid_synth_set_interp_method(synth, -1, interp_method) == FLUID_FAILED) {
        WARN_PRINT_ED("Failed to set interpolation method");
        return -1;
    }

    return 0;
}

int FluidSynthNode::render_file(String midi_file, String output_file, String sf_path,
                                int interpolation, double sample_rate, String bit_depth,
                                String file_type)
{
    if (settings == NULL) {
        WARN_PRINT_ED("FluidSynth settings have not been created");
        return -1;
    }

    fluid_settings_t* tmp_settings = new_fluid_settings();
    
    if (copy_settings(settings, tmp_settings) == 0) {

        // specify the file to store the audio to
        // make sure you compiled fluidsynth with libsndfile to get a real wave file
        // otherwise this file will only contain raw s16 stereo PCM
        fluid_settings_setstr(tmp_settings, "audio.file.name", output_file.ascii());

        // use number of samples processed as timing source, rather than the system timer
        fluid_settings_setstr(tmp_settings, "player.timing-source", "sample");
        
        // since this is a non-realtime scenario, there is no need to pin the sample data
        fluid_settings_setint(tmp_settings, "synth.lock-memory", 0);

        // Set the sample rate for rendering, valid values 8000.0 - 96000.0
        if ((sample_rate >= 8000.0) && (sample_rate <= 96000.0))
        {
            fluid_settings_setnum(tmp_settings, "synth.sample-rate", sample_rate);
        }

        // Set the file format, i.e. storage type for sample data
        // Valid values:
        //    'double' = 64 bit floating point
        //    'float' = 32 bit floating point,
        //    's16' = 16 bit signed PCM,
        //    's24' = 24 bit signed PCM,
        //    's32' = 32 bit signed PCM,
        //    's8' = 8 bit signed PCM and
        //    'u8' = 8 bit unsigned PCM.
        fluid_settings_setstr(tmp_settings, "audio.file.format", bit_depth.ascii());

        fluid_settings_setstr(tmp_settings, "audio.file.type", file_type.ascii());

        fluid_synth_t* tmp_synth = new_fluid_synth(tmp_settings);
        
        fluid_synth_sfload(tmp_synth, sf_path.ascii(), true);

        fluid_interp interp_method = FLUID_INTERP_DEFAULT;
        switch(interpolation) {
            case 0:
                interp_method = FLUID_INTERP_NONE;
                break;
            case 1:
                interp_method = FLUID_INTERP_LINEAR;
                break;
            case 2:
                interp_method = FLUID_INTERP_4THORDER;
                break;
            case 3:
                interp_method = FLUID_INTERP_7THORDER;
                break;
        }

        if (fluid_synth_set_interp_method(tmp_synth, -1, interp_method) == FLUID_FAILED) {
            WARN_PRINT_ED("Failed to set interpolation method");
            delete_fluid_synth(tmp_synth);
            delete_fluid_settings(tmp_settings);
            return -1;
        }
        
        fluid_player_t *player = new_fluid_player(tmp_synth);
        fluid_player_add(player, midi_file.ascii());
        fluid_player_play(player);
        
        fluid_file_renderer_t* renderer = new_fluid_file_renderer(tmp_synth);
        
        while (fluid_player_get_status(player) == FLUID_PLAYER_PLAYING)
        {
            if (int rtn = fluid_file_renderer_process_block(renderer) != FLUID_OK)
            {
                break;
            }
        }
        
        // just for sure: stop the playback explicitly and wait until finished
        fluid_player_stop(player);
        fluid_player_join(player);
        
        delete_fluid_file_renderer(renderer);
        delete_fluid_player(player);
        delete_fluid_synth(tmp_synth);
    }
    else {
        WARN_PRINT_ED("Failed to create settings copy");
        delete_fluid_settings(tmp_settings);
        return -1;
    }

    delete_fluid_settings(tmp_settings);
    
    return 0;
}


void FluidSynthNode::_input(const Ref<InputEvent> &event) {
    InputEventMIDI* midi_event;
    if ((midi_event = dynamic_cast<InputEventMIDI*>(*event)) != nullptr ) {
        int channel = channel_map[midi_event->get_channel()];
        switch(midi_event->get_message()) {
            case MIDI_MESSAGE_NOTE_OFF:
                fluid_synth_noteoff(synth, channel, midi_event->get_pitch());
                break;
            case MIDI_MESSAGE_NOTE_ON:
                fluid_synth_noteon(synth, channel, midi_event->get_pitch(),
                    midi_event->get_velocity());
                break;
            case MIDI_MESSAGE_AFTERTOUCH:
                fluid_synth_key_pressure(synth, channel, midi_event->get_pitch(),
                    midi_event->get_pressure());
                break;
            case MIDI_MESSAGE_CHANNEL_PRESSURE:
                fluid_synth_channel_pressure(synth, channel, midi_event->get_pressure());
                break;
            case MIDI_MESSAGE_CONTROL_CHANGE:
                fluid_synth_cc(synth, channel, midi_event->get_controller_number(),
                    midi_event->get_controller_value());
                break;
            case MIDI_MESSAGE_PITCH_BEND:
                fluid_synth_pitch_bend(synth, channel, midi_event->get_pitch());
                break;
            case MIDI_MESSAGE_PROGRAM_CHANGE:
                fluid_synth_program_change(synth, channel, midi_event->get_instrument());
                break;
            default:
                break;
        }
    }
}

int FluidSynthNode::create_midi_player() {
    if (player != NULL) {
        WARN_PRINT_ED("FluidSynth player already exists");
        return -1;
    }

    player = new_fluid_player(synth);

    if (player == NULL) {
        WARN_PRINT_ED("Creating FluidSynth player failed");
        return -1;
    }

    return 0;
}

int FluidSynthNode::delete_midi_player() {
    delete_fluid_player(player);
    player = NULL;
    return 0;
}

int FluidSynthNode::player_load_midi(String file_path) {

    if (fluid_player_add(player, file_path.ascii()) == FLUID_FAILED){
        WARN_PRINT_ED("FluidSynth player failed to load MIDI file");
        return -1;
    }

    return 0;
}

int FluidSynthNode::player_play(int loop_count) {
    if (player != NULL) {
        if (loop_count >= -1) {
            fluid_player_set_loop(player, loop_count);
        }

        if (fluid_player_play(player) == FLUID_FAILED) {
            WARN_PRINT_ED("FluidSynth player failed to play");
            return -1;
        }
    }
    return 0;
}

int FluidSynthNode::player_seek(int tick) {
    if (player != NULL) {
        if (fluid_player_seek(player, tick) == FLUID_FAILED) {
            WARN_PRINT_ED("FluidSynth player failed to seek in file");
            return -1;
        }
    }
    return 0;
}

int FluidSynthNode::player_stop() {
    if (player != NULL) {
        fluid_player_stop(player);
    }
    return 0;
}
