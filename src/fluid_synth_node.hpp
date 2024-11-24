#ifndef FLUIDSYNTHNODE_H
#define FLUIDSYNTHNODE_H

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/node.hpp>
#include <fluidsynth.h>

namespace godot {

class FluidSynthNode : public Node {
	GDCLASS(FluidSynthNode, Node)

private:
    int channel_map[16];
    fluid_audio_driver_t *adriver;
    fluid_player_t *player;
    fluid_settings_t *settings;
    fluid_synth_t *synth;

protected:
	static void _bind_methods();

public:
	FluidSynthNode();
	~FluidSynthNode();


    // Constants
    static const uint8_t MIDI_MSG_TYPE_NOTE_OFF = 0x80;
    static const uint8_t MIDI_MSG_TYPE_NOTE_ON = 0x90;
    static const uint8_t MIDI_MSG_TYPE_POLY_KEY_PRESSURE = 0xA0;
    static const uint8_t MIDI_MSG_TYPE_CONTROL_CHANGE = 0xB0;
    static const uint8_t MIDI_MSG_TYPE_PROGRAM_CHANGE = 0xC0;
    static const uint8_t MIDI_MSG_TYPE_CHANNEL_PRESSURE = 0xD0;
    static const uint8_t MIDI_MSG_TYPE_PITCH_BEND = 0xE0;
    static const uint8_t MIDI_MSG_TYPE_SYSTEM = 0xF0;

    static const uint8_t MIDI_SYS_MSG_TYPE_SYSTEM_EXCLUSIVE = 0xF0;
    static const uint8_t MIDI_SYS_MSG_TYPE_TIME_CODE_QTR_FRAME = 0xF1;
    static const uint8_t MIDI_SYS_MSG_TYPE_SONG_POSITION_PTR = 0xF2;
    static const uint8_t MIDI_SYS_MSG_TYPE_SONG_SELECT = 0xF3;
    static const uint8_t MIDI_SYS_MSG_TYPE_TUNE_REQUEST = 0xF6;
    static const uint8_t MIDI_SYS_MSG_TYPE_END_OF_EXCLUSIVE = 0xF7;
    static const uint8_t MIDI_SYS_MSG_TYPE_TIMING_CLOCK = 0xF8;
    static const uint8_t MIDI_SYS_MSG_TYPE_START = 0xFA;
    static const uint8_t MIDI_SYS_MSG_TYPE_CONTINUE = 0xFB;
    static const uint8_t MIDI_SYS_MSG_TYPE_STOP = 0xFC;
    static const uint8_t MIDI_SYS_MSG_TYPE_ACTIVE_SENSING = 0xFE;
    static const uint8_t MIDI_SYS_MSG_TYPE_SYSTEM_RESET = 0xFF;

    // Settings
    /**
     * @brief Create a settings object.
     * 
     * @return int Returns 0 on success.
     */
    int settings_create();

    /**
     * @brief Change an integer setting.
     * 
     * @param setting Setting name.
     * @param value New value.
     * @return int Returns 0 on success.
     */
    int settings_change_int(String setting, int value);
    
    /**
     * @brief Change a float/double setting.
     * 
     * @param setting Setting name.
     * @param value New value.
     * @return int Returns 0 on success.
     */
    int settings_change_dbl(String setting, double value);
    
    /**
     * @brief Change a String setting.
     * 
     * @param setting Setting name.
     * @param value New value.
     * @return int Returns 0 on success.
     */
    int settings_change_str(String setting, String value);

    double settings_get_dbl(String setting);
    int settings_get_int(String setting);
    String settings_get_str(String setting);

    /**
     * @brief Deletes the internal settings structure.
     * 
     * @return int Returns 0 on success.
     */
    int settings_delete();

    int settings_copy(fluid_settings_t *original, fluid_settings_t *copy);
    String settings_as_json_str();
    int settings_save(String file_name);
    int settings_load_from_json(String json_str);
    int settings_load(String file_name);


    // Synth
    /**
     * @brief Creates a new Fluid Synth instance and loads a SoundFont.
     * 
     * @param sf_path Path to the SoundFont to be loaded.
     * @param listen_ext_input Should the synth respond to external MIDI input.
     * @return int Returns Soundfont id on success, -1 on failure.
     */
    int synth_create(String sf_path, bool listen_ext_input);

    /**
     * @brief Renders the specified MIDI file as a .wav file, using
     *        the current settings.
     * 
     * @param midi_file Path to MIDI file to be rendered.
     * @param output_file Path to output file, can use file extension to determine type.
     * @param sf_path Path to SoundFont to be used for rendering.
     * @param interpolation Type of interpolation to use, 4th Order is default.
     *                      (0 = None, 1 = Linear, 2 = 4th Order, 3 = 7th Order)
     * @return int Returns 0 on success.
     */
    int synth_render_file(String midi_file, String output_file, String sf_path,
        int interpolation, double sample_rate, String bit_depth, String file_type);
    int synth_soundfont_load(String sf_path, bool reset);
    String synth_soundfont_name(int sfont_id);
    void synth_soundfont_reset_presets(int sfont_id);
    String synth_soundfont_next_preset(int sfont_id);
    int synth_delete();
    void synth_map_channel(int channel, int mapped_channel);
    int synth_setup_channel(int channel, int sfont_id, int bank_num, int program, int reverb, int chorus,
        int volume = 100, int pan = 64, int expression = 127);
    int synth_set_interpolation(int method);
    int synth_play_messages(PackedInt32Array indices, PackedByteArray data);
    int synth_system_reset();
    void _input(const Ref<InputEvent> &event) override;


    // Player
    int player_create();
    int player_delete();
    int player_load_midi(String file_path);
    int player_play(int loop_count = 0);
    int player_seek(int tick);
    int player_stop();
};

}

#endif