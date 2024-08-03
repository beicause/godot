#include "register_types.h"
#include "gd_glicol.h"
#include "gd_jmespath.h"
#include "gd_json_converter.h"

#ifdef TOOLS_ENABLED
#include "editor/resource_importer_json_converter.h"
static Ref<ResourceImporterJSONConverter> resource_importer_json_converter;
#endif // TOOLS_ENABLED

void initialize_a_rust_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<Glicol>();
	ClassDB::register_class<JMESExpr>();
	ClassDB::register_class<JMESVariable>();
	ClassDB::register_class<JSONConverter>();

#ifdef TOOLS_ENABLED
	resource_importer_json_converter.instantiate();
	ResourceFormatImporter::get_singleton()->add_importer(resource_importer_json_converter);
#endif // TOOLS_ENABLED
}

void uninitialize_a_rust_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

#ifdef TOOLS_ENABLED
	ResourceFormatImporter::get_singleton()->remove_importer(resource_importer_json_converter);
	resource_importer_json_converter.unref();
#endif
}
