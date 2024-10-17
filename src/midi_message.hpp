#ifndef FLUIDSYNTHNODE_MIDI_MESSAGE_HPP
#define FLUIDSYNTHNODE_MIDI_MESSAGE_HPP

#include <godot_cpp/classes/object.hpp>

namespace godot {

class MidiMessage : public Object {
    GDCLASS(MidiMessage, Object)

private:
    uint8_t _status;
    uint8_t* _data;
    int32_t _data_length;
    uint32_t _tick;

protected:
    static void _bind_methods();

public:
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

    MidiMessage();
    ~MidiMessage();
    MidiMessage(const MidiMessage &msg);
    MidiMessage(const PackedByteArray &data_array, int *current_index);
    MidiMessage(uint8_t type, uint8_t channel, uint8_t *data, int32_t data_length, uint32_t tick);
    MidiMessage(uint8_t system_type, uint8_t *data, int32_t data_length, uint32_t tick);

    uint8_t get_msg_type() { return _status & 0xF0; }
    void set_msg_type(uint8_t type) { _status = (_status & 0x0F) | (type & 0xF0); }
    uint8_t get_sys_msg_type() { return _status; }
    void set_sys_msg_type(uint8_t type) { _status = type; }
    uint8_t get_channel() { return _status & 0x0F; }
    void set_channel(uint8_t channel) { _status = (_status & 0xF0) | (channel & 0x0F); }

    int32_t get_msg_length(uint32_t *current_tick);
    int32_t append_msg(PackedByteArray &data_array, uint32_t current_tick);
};
}

#endif
