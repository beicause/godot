#pragma once

#include "core/object/ref_counted.h"
#include "cxx.h"
#include "gd_jmespath.rs.h"

class JMESVariable : public RefCounted {
	GDCLASS(JMESVariable, RefCounted);

protected:
	static void _bind_methods() {
		ClassDB::bind_static_method("JMESVariable", D_METHOD("create_from_json_str", "str"), &JMESVariable::create_from_json_str);
		ClassDB::bind_method(D_METHOD("to_json_str", "pretty"), &JMESVariable::to_json_str);
	}

public:
	rust::Box<jmespath::JMESVariable> inst = jmespath::jmesvariable_create_from_json_str("");

	static Ref<JMESVariable> create_from_json_str(String str) {
		Ref<JMESVariable> ret;
		ret.instantiate();
		ret->inst = jmespath::jmesvariable_create_from_json_str(rust::Str(str.utf8().get_data()));
		return ret;
	}

	String to_json_str(bool pretty) const {
		return inst->to_json_str(pretty).c_str();
	}
};

class JMESExpr : public RefCounted {
	GDCLASS(JMESExpr, RefCounted);

	rust::Box<jmespath::JMESExpr> inst = jmespath::jmesexpr_create_from_str("");

protected:
	static void _bind_methods() {
		ClassDB::bind_static_method("JMESExpr", D_METHOD("create_from_str", "str"), &JMESExpr::create_from_str);
		ClassDB::bind_method(D_METHOD("to_str"), &JMESExpr::to_str);
		ClassDB::bind_method(D_METHOD("search", "data"), &JMESExpr::search);
	}

public:
	static Ref<JMESExpr> create_from_str(String str) {
		Ref<JMESExpr> ret;
		ret.instantiate();
		ret->inst = jmespath::jmesexpr_create_from_str(rust::Str(str.utf8().get_data()));
		return ret;
	}
	String to_str() const {
		return inst->to_str().c_str();
	}
	String search(Ref<JMESVariable> data) const {
		return inst->search(*data->inst).c_str();
	}
};
