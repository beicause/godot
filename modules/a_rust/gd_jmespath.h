/**************************************************************************/
/*  gd_jmespath.h                                                         */
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

#ifndef GD_JMESPATH_H
#define GD_JMESPATH_H

#include "core/object/ref_counted.h"
#include "cxxbridge/cxx.h"
#include "cxxbridge/gd_jmespath.rs.h"

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

#endif // GD_JMESPATH_H
