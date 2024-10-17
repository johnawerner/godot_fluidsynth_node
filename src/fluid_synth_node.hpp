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

    // Settings
    /**
     * @brief Create a settings object.
     * 
     * @return int Returns 0 on success.
     */
    int create_settings();

    /**
     * @brief Change an integer setting.
     * 
     * @param setting Setting name.
     * @param value New value.
     * @return int Returns 0 on success.
     */
    int change_setting_int(String setting, int value);
    
    /**
     * @brief Change a float/double setting.
     * 
     * @param setting Setting name.
     * @param value New value.
     * @return int Returns 0 on success.
     */
    int change_setting_dbl(String setting, double value);
    
    /**
     * @brief Change a String setting.
     * 
     * @param setting Setting name.
     * @param value New value.
     * @return int Returns 0 on success.
     */
    int change_setting_str(String setting, String value);

    /**
     * @brief Deletes the internal settings structure.
     * 
     * @return int Returns 0 on success.
     */
    int delete_settings();

    int copy_settings(fluid_settings_t *original, fluid_settings_t *copy);
    void copy_setting(void *data, const char *name, int type);
    // Synth
    /**
     * @brief Creates a new Fluid Synth instance and loads a SoundFont.
     * 
     * @param sf_path Path to the SoundFont to be loaded.
     * @param listen_ext_input Should the synth respond to external MIDI input.
     * @return int Returns 0 on success.
     */
    int load_synth(String sf_path, bool listen_ext_input);

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
    int render_file(String midi_file, String output_file, String sf_path,
        int interpolation, double sample_rate, String bit_depth, String file_type);
    int load_soundfont(String sf_path, bool reset);
    int unload_synth();
    void map_channel(int channel, int mapped_channel);
    int setup_channel(int channel, int program, int reverb, int chorus,
        int volume = 100, int pan = 64, int expression = 127);
    int set_interpolation(int method);
    void _input(const Ref<InputEvent> &event) override;


    // Player
    int create_midi_player();
    int delete_midi_player();
    int player_load_midi(String file_path);
    int player_play(int loop_count = 0);
    int player_seek(int tick);
    int player_stop();
};

}

#endif