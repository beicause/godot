/**************************************************************************/
/*  gd_jsonnet.h                                                          */
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

#ifndef GD_JSONNET_H
#define GD_JSONNET_H

#include "core/object/ref_counted.h"
#include "thirdparty/jsonnet/include/libjsonnet++.h"

String ryml_json_to_yaml(String str);
String ryml_yaml_to_json(String data, bool pretty = false);

class JSONNet : public RefCounted {
	GDCLASS(JSONNet, RefCounted);

protected:
	static void _bind_methods();
	jsonnet::Jsonnet jnet;

public:
	/// Return the version string of the Jsonnet interpreter.  Conforms to
	/// semantic versioning https://semver.org/. If this does not match
	/// LIB_JSONNET_VERSION then there is a mismatch between header and compiled
	/// library.
	static String version() {
		return jsonnet::Jsonnet::version().c_str();
	}

	/// Sets the maximum stack depth.
	void set_max_stack(uint32_t depth) {
		jnet.setMaxStack(depth);
	}

	/// Sets the number of objects required before a garbage collection cycle is
	/// allowed.
	void set_gc_min_objects(uint32_t objects) {
		jnet.setGcMinObjects(objects);
	}

	/// Run the garbage collector after this amount of growth in the number of
	/// objects.
	void set_gc_growth_trigger(double growth) {
		jnet.setGcGrowthTrigger(growth);
	}

	/// Set whether to expect a string as output and don't JSON encode it.
	void set_string_output(bool string_output) {
		jnet.setStringOutput(string_output);
	}

	/// Set the number of lines of stack trace to display (0 to display all).
	void set_max_trace(uint32_t lines) {
		jnet.setMaxTrace(lines);
	}

	/// Add to the default import callback's library search path.
	void add_import_path(const String &path) {
		jnet.addImportPath(path.utf8().get_data());
	}

	/// Bind a string top-level argument for a top-level parameter.
	///
	/// Argument values are copied so memory should be managed by caller.
	void bind_tla_var(const String &key, const String &value) {
		jnet.bindTlaVar(key.utf8().get_data(), value.utf8().get_data());
	}

	/// Bind a code top-level argument for a top-level parameter.
	///
	/// Argument values are copied so memory should be managed by caller.
	void bind_tla_code_var(const String &key, const String &value) {
		jnet.bindTlaCodeVar(key.utf8().get_data(), value.utf8().get_data());
	}

	/// Bind a Jsonnet external variable to the given value.
	///
	/// Argument values are copied so memory should be managed by caller.
	void bind_ext_var(const String &key, const String &value) {
		jnet.bindExtVar(key.utf8().get_data(), value.utf8().get_data());
	}

	/// Bind a Jsonnet external code variable to the given value.
	///
	/// Argument values are copied so memory should be managed by caller.
	void bind_ext_code_var(const String &key, const String &value) {
		jnet.bindExtCodeVar(key.utf8().get_data(), value.utf8().get_data());
	}

	/// Evaluate a file containing Jsonnet code to return a JSON string.
	///
	/// This method returns true if the Jsonnet code is successfully evaluated.
	/// Otherwise, it returns false, and the error output can be returned by
	/// calling LastError();
	///
	/// @param filename Path to a file containing Jsonnet code.
	/// @param output Pointer to string to contain the output.
	/// @return true if the Jsonnet code was successfully evaluated, false
	///         otherwise.
	String evaluate_file(const String &filename) {
		std::string ret;
		jnet.evaluateFile(filename.utf8().get_data(), &ret);
		return ret.c_str();
	}

	/// Evaluate a string containing Jsonnet code to return a JSON string.
	///
	/// This method returns true if the Jsonnet code is successfully evaluated.
	/// Otherwise, it returns false, and the error output can be returned by
	/// calling LastError();
	///
	/// @param filename Path to a file (used in error message).
	/// @param snippet Jsonnet code to execute.
	/// @param output Pointer to string to contain the output.
	/// @return true if the Jsonnet code was successfully evaluated, false
	///         otherwise.
	String evaluate_snippet(const String &snippet, const String &filename = "") {
		std::string output;
		bool res = jnet.evaluateSnippet(filename.utf8().get_data(), snippet.utf8().get_data(), &output);
		if (!res) {
			ERR_PRINT(vformat("Jsonnet error: %s", last_error()));
			return "";
		}
		return output.c_str();
	}

	/// Evaluate a file containing Jsonnet code, return a number of JSON files.
	///
	/// This method returns true if the Jsonnet code is successfully evaluated.
	/// Otherwise, it returns false, and the error output can be returned by
	/// calling LastError();
	///
	/// @param filename Path to a file containing Jsonnet code.
	/// @param outputs Pointer to map which will store the output map of filename
	///        to JSON string.
	Dictionary evaluate_file_multi(const String &filename) {
		std::map<std::string, std::string> outputs;
		bool res = jnet.evaluateFileMulti(filename.utf8().get_data(), &outputs);
		Dictionary ret;
		if (!res) {
			ERR_PRINT(vformat("Jsonnet error: %s", last_error()));
			return ret;
		}
		for (auto &it : outputs) {
			ret[it.first.c_str()] = it.second.c_str();
		}
		return ret;
	}

	/// Evaluate a string containing Jsonnet code, return a number of JSON files.
	///
	/// This method returns true if the Jsonnet code is successfully evaluated.
	/// Otherwise, it returns false, and the error output can be returned by
	/// calling LastError();
	///
	/// @param filename Path to a file containing Jsonnet code.
	/// @param snippet Jsonnet code to execute.
	/// @param outputs Pointer to map which will store the output map of filename
	///        to JSON string.
	/// @return true if the Jsonnet code was successfully evaluated, false
	///         otherwise.
	Dictionary evaluate_snippet_multi(const String &snippet, const String &filename = "") {
		std::map<std::string, std::string> outputs;
		bool res = jnet.evaluateSnippetMulti(filename.utf8().get_data(), snippet.utf8().get_data(), &outputs);
		Dictionary ret;
		if (!res) {
			ERR_PRINT(vformat("Jsonnet error: %s", last_error()));
			return ret;
		}
		for (auto &it : outputs) {
			ret[it.first.c_str()] = it.second.c_str();
		}
		return ret;
	}

	/// Returns the last error raised by Jsonnet.
	String last_error() const {
		return jnet.lastError().c_str();
	}

	JSONNet() {
		bool res = jnet.init();
		if (!res) {
			ERR_PRINT("jsonnet init failed");
		}
	}
};
#endif // GD_JSONNET_H
