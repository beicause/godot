#pragma once

#include "core/object/class_db.h"
#include "modules/register_module_types.h"

using namespace godot;

void initialize_a_rust_module(ModuleInitializationLevel p_level);
void uninitialize_a_rust_module(ModuleInitializationLevel p_level);
