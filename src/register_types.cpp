#include "register_types.h"

#include "fluid_synth_node.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_fluidsynth_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(FluidSynthNode);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_NOTE_OFF", FluidSynthNode::MIDI_MSG_TYPE_NOTE_OFF);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_NOTE_ON", FluidSynthNode::MIDI_MSG_TYPE_NOTE_ON);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_POLY_KEY_PRESSURE", FluidSynthNode::MIDI_MSG_TYPE_POLY_KEY_PRESSURE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_CONTROL_CHANGE", FluidSynthNode::MIDI_MSG_TYPE_CONTROL_CHANGE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_PROGRAM_CHANGE", FluidSynthNode::MIDI_MSG_TYPE_PROGRAM_CHANGE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_CHANNEL_PRESSURE", FluidSynthNode::MIDI_MSG_TYPE_CHANNEL_PRESSURE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_PITCH_BEND", FluidSynthNode::MIDI_MSG_TYPE_PITCH_BEND);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_MSG_TYPE_SYSTEM", FluidSynthNode::MIDI_MSG_TYPE_SYSTEM);

    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_SYSTEM_EXCLUSIVE", FluidSynthNode::MIDI_SYS_MSG_TYPE_SYSTEM_EXCLUSIVE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_TIME_CODE_QTR_FRAME", FluidSynthNode::MIDI_SYS_MSG_TYPE_TIME_CODE_QTR_FRAME);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_SONG_POSITION_PTR", FluidSynthNode::MIDI_SYS_MSG_TYPE_SONG_POSITION_PTR);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_SONG_SELECT", FluidSynthNode::MIDI_SYS_MSG_TYPE_SONG_SELECT);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_TUNE_REQUEST", FluidSynthNode::MIDI_SYS_MSG_TYPE_TUNE_REQUEST);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_END_OF_EXCLUSIVE", FluidSynthNode::MIDI_SYS_MSG_TYPE_END_OF_EXCLUSIVE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_TIMING_CLOCK", FluidSynthNode::MIDI_SYS_MSG_TYPE_TIMING_CLOCK);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_START", FluidSynthNode::MIDI_SYS_MSG_TYPE_START);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_CONTINUE", FluidSynthNode::MIDI_SYS_MSG_TYPE_CONTINUE);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_STOP", FluidSynthNode::MIDI_SYS_MSG_TYPE_STOP);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_ACTIVE_SENSING", FluidSynthNode::MIDI_SYS_MSG_TYPE_ACTIVE_SENSING);
    ClassDB::bind_integer_constant("FluidSynthNode", "", "MIDI_SYS_MSG_TYPE_SYSTEM_RESET", FluidSynthNode::MIDI_SYS_MSG_TYPE_SYSTEM_RESET);
}

void uninitialize_fluidsynth_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT fluidsynth_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_fluidsynth_module);
	init_obj.register_terminator(uninitialize_fluidsynth_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}