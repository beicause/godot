/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "register_types.h"
#include "gd_lz4.h"
#include "resource_loader_jsonz.h"
#include "resource_loader_txtz.h"

Ref<ResourceFormatLoaderJSONZ> resource_loader_jsonz;
Ref<ResourceFormatLoaderTXTZ> resource_loader_txtz;
Ref<ResourceFormatSaverTXTZ> resource_saver_txtz;

#ifdef TOOLS_ENABLED
#include "editor/resource_importer_txtz.h"
static Ref<ResourceImporterTXTZ> resource_importer_txtz;
#endif // TOOLS_ENABLED

void initialize_a_lz4_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Lz4>();
	ClassDB::register_class<TXTZFile>();

	resource_loader_jsonz.instantiate();
	resource_loader_txtz.instantiate();
	resource_saver_txtz.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_jsonz);
	ResourceLoader::add_resource_format_loader(resource_loader_txtz);
	ResourceSaver::add_resource_format_saver(resource_saver_txtz);

#ifdef TOOLS_ENABLED
	resource_importer_txtz.instantiate();
	ResourceFormatImporter::get_singleton()->add_importer(resource_importer_txtz);
#endif // TOOLS_ENABLED
}

void uninitialize_a_lz4_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ResourceLoader::remove_resource_format_loader(resource_loader_jsonz);
	ResourceLoader::remove_resource_format_loader(resource_loader_txtz);
	ResourceSaver::remove_resource_format_saver(resource_saver_txtz);
	resource_loader_jsonz.unref();
	resource_loader_txtz.unref();
	resource_saver_txtz.unref();

#ifdef TOOLS_ENABLED
	ResourceFormatImporter::get_singleton()->remove_importer(resource_importer_txtz);
	resource_importer_txtz.unref();
#endif
}
