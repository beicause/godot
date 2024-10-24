/*
Copyright 2015 Google Inc. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <iomanip>

#include "static_error.h"
#include "string_utils.h"

namespace jsonnet::internal {

UString jsonnet_string_unparse(const UString &str, bool single)
{
    UStringStream ss;
    ss << (single ? U'\'' : U'\"');
    ss << jsonnet_string_escape(str, single);
    ss << (single ? U'\'' : U'\"');
    return ss.str();
}

UString jsonnet_string_escape(const UString &str, bool single)
{
    UStringStream ss;
    for (std::size_t i = 0; i < str.length(); ++i) {
        char32_t c = str[i];
        switch (c) {
            case U'\"': ss << (single ? U"\"" : U"\\\""); break;
            case U'\'': ss << (single ? U"\\\'" : U"\'"); break;
            case U'\\': ss << U"\\\\"; break;
            case U'\b': ss << U"\\b"; break;
            case U'\f': ss << U"\\f"; break;
            case U'\n': ss << U"\\n"; break;
            case U'\r': ss << U"\\r"; break;
            case U'\t': ss << U"\\t"; break;
            case U'\0': ss << U"\\u0000"; break;
            default: {
                if (c < 0x20 || (c >= 0x7f && c <= 0x9f)) {
                    // Unprintable, use \u
                    std::stringstream ss8;
                    ss8 << "\\u" << std::hex << std::setfill('0') << std::setw(4)
                        << (unsigned long)(c);
                    ss << decode_utf8(ss8.str());
                } else {
                    // Printable, write verbatim
                    ss << c;
                }
            }
        }
    }
    return ss.str();
}

unsigned long jsonnet_string_parse_unicode(const LocationRange &loc, const char32_t *c)
{
    unsigned long codepoint = 0;
    // Expect 4 hex digits.
    for (unsigned i = 0; i < 4; ++i) {
        auto x = (unsigned char)(c[i]);
        unsigned digit;
        if (x == '\0') {
            auto msg = "Truncated unicode escape sequence in string literal.";
            jsonnet_throw_static(StaticError(loc, msg));
        } else if (x >= '0' && x <= '9') {
            digit = x - '0';
        } else if (x >= 'a' && x <= 'f') {
            digit = x - 'a' + 10;
        } else if (x >= 'A' && x <= 'F') {
            digit = x - 'A' + 10;
        } else {
            std::stringstream ss;
            ss << "Malformed unicode escape character, "
               << "should be hex: '" << x << "'";
            jsonnet_throw_static(StaticError(loc, ss.str()));
        }
        codepoint *= 16;
        codepoint += digit;
    }
    return codepoint;
}

bool is_bmp_codepoint(const unsigned long codepoint)
{
    return codepoint < 0xd800 || (codepoint >= 0xe000 && codepoint < 0x10000);
}

char32_t decode_utf16_surrogates(const LocationRange &loc, const unsigned long high, const unsigned long low)
{
    if (high >= 0xd800 && high < 0xdc00 && low >= 0xdc00 && low < 0xe000) {
        return 0x10000 + ((high & 0x03ff) << 10) + (low & 0x03ff);
    } else {
        std::stringstream ss;
        ss << "Invalid UTF-16 bytes";
        jsonnet_throw_static(StaticError(loc, ss.str()));
    }
}

UString jsonnet_string_unescape(const LocationRange &loc, const UString &s)
{
    UString r;
    const char32_t *s_ptr = s.c_str();
    for (const char32_t *c = s_ptr; *c != U'\0'; ++c) {
        switch (*c) {
            case '\\':
                switch (*(++c)) {
                    case '"':
                    case '\'': r += *c; break;

                    case '\\': r += *c; break;

                    case '/': r += *c; break;

                    case 'b': r += '\b'; break;

                    case 'f': r += '\f'; break;

                    case 'n': r += '\n'; break;

                    case 'r': r += '\r'; break;

                    case 't': r += '\t'; break;

                    case 'u': {
                        ++c;  // Consume the 'u'.
                        unsigned long codepoint = jsonnet_string_parse_unicode(loc, c);

                        // Leave us on the last char, ready for the ++c at
                        // the outer for loop.
                        c += 3;
                        if (!is_bmp_codepoint(codepoint)) {
                           if (*(++c) != '\\') {
                                std::stringstream ss;
                                ss << "Invalid non-BMP Unicode escape in string literal";
                                jsonnet_throw_static(StaticError(loc, ss.str()));
                           }
                           if (*(++c) != 'u') {
                                std::stringstream ss;
                                ss << "Invalid non-BMP Unicode escape in string literal";
                                jsonnet_throw_static(StaticError(loc, ss.str()));
                           }
                           ++c;
                           unsigned long codepoint2 = jsonnet_string_parse_unicode(loc, c);
                           c += 3;
                           codepoint = decode_utf16_surrogates(loc, codepoint, codepoint2);
                       }
                       r += codepoint;
                    } break;

                    case '\0': {
                        auto msg = "Truncated escape sequence in string literal.";
                        jsonnet_throw_static(StaticError(loc, msg));
                    }

                    default: {
                        std::stringstream ss;
                        std::string utf8;
                        encode_utf8(*c, utf8);
                        ss << "Unknown escape sequence in string literal: '" << utf8 << "'";
                        jsonnet_throw_static(StaticError(loc, ss.str()));
                    }
                }
                break;

            default:
                // Just a regular letter.
                r += *c;
        }
    }
    return r;
}

}  // namespace jsonnet::internal
