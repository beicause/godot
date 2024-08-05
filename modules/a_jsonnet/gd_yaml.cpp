/**************************************************************************/
/*  gd_yaml.cpp                                                           */
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

#include "gd_yaml.h"
#include <nlohmann/json.hpp>
#include <ryml_all.hpp>

String ryml_json_to_yaml(String data) {
	c4::yml::Tree tree = c4::yml::parse_in_arena(c4::to_csubstr(data.utf8().get_data()));
	std::string ret = c4::yml::emitrs_yaml<std::string>(tree);
	return ret.c_str();
}
String ryml_yaml_to_json(String data, bool pretty) {
	c4::yml::Tree tree = c4::yml::parse_in_arena(c4::to_csubstr(data.utf8().get_data()));
	std::string ret = c4::yml::emitrs_json<std::string>(tree);
	if (pretty) {
		nlohmann::json json = nlohmann::json::parse(ret);
		ret = json.dump(2);
	}
	return ret.c_str();
}
