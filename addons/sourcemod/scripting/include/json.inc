/**
 * vim: set ts=4 :
 * =============================================================================
 * sm-json
 * Provides a pure SourcePawn implementation of JSON encoding and decoding.
 * https://github.com/clugg/sm-json
 * 
 * sm-json (C)2018 James D. (clug)
 * SourceMod (C)2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 */

#if defined _json_included
 #endinput
#endif
#define _json_included

#include <string>
#include <json/definitions>
#include <json/string_helpers>
#include <json/encode_helpers>
#include <json/decode_helpers>
#include <json/object>

/**
 * Encodes a JSON_Object into its string representation.
 *
 * @param obj       Object to encode.
 * @param output    String buffer to store output.
 * @param maxlen    Maximum size of string buffer.
 */
stock void json_encode(JSON_Object obj, char[] output, int maxlen) {
    char[] key = new char[maxlen];
    bool is_array = obj.IsArray;
    bool is_empty = true;
    int builder_size;

    // used in key iterator
    int str_length = 1;
    int int_value;
    int cell_length = 0;

    strcopy(output, maxlen, (is_array) ? "[" : "{");

    StringMapSnapshot snap = obj.Snapshot();
    for (int i = 0; i < obj.Length; ++i) {
        if (is_array) {
            obj.GetIndexString(key, maxlen, i);
        } else {
            snap.GetKey(i, key, maxlen);
        }

        if (json_is_meta_key(key)) continue;  // skip meta-keys
        if (obj.GetKeyHidden(key)) continue;  // skip hidden keys

        JSON_CELL_TYPE type = obj.GetKeyType(key);
        if (type == Type_Invalid) continue;  // skip keys of unknown type

        str_length = 1;
        if (type == Type_String) {
            str_length = obj.GetKeyLength(key);
        }
        char[] str_value = new char[str_length + 1];

        cell_length = 0;
        switch (type) {
            case Type_String: {
                obj.GetString(key, str_value, str_length + 1);
                cell_length = json_cell_string_size(str_length) + 1;
            }
            case Type_Int: {
                int_value = obj.GetInt(key);
                cell_length = json_cell_int_size(int_value);
            }
            case Type_Float: {
                cell_length = json_cell_float_size();
            }
            case Type_Bool: {
                cell_length = json_cell_bool_size();
            }
            case Type_Null: {
                cell_length = json_cell_null_size();
            }
            case Type_Object: {
                cell_length = maxlen;
            }
        }

        char[] cell = new char[cell_length];
        switch (type) {
            case Type_String: {
                json_cell_string(str_value, cell, cell_length);
            }
            case Type_Int: {
                json_cell_int(int_value, cell, cell_length);
            }
            case Type_Float: {
                float value = obj.GetFloat(key);
                json_cell_float(value, cell, cell_length);
            }
            case Type_Bool: {
                bool value = obj.GetBool(key);
                json_cell_bool(value, cell, cell_length);
            }
            case Type_Null: {
                json_cell_null(cell, cell_length);
            }
            case Type_Object: {
                JSON_Object value = obj.GetObject(key);
                json_encode(value, cell, cell_length);
            }
        }

        // make the builder fit our key:value
        builder_size = cell_length + 1;  // cell_length is length of value and null terminator, +1 for ,
        if (!is_array) {
            builder_size += json_cell_string_size(strlen(key)) + 1;  // get the length of the key, +1 for :
        }

        char[] builder = new char[builder_size];
        strcopy(builder, builder_size, "");
        // add the key if it's an object
        if (!is_array) {
            json_cell_string(key, builder, builder_size);
            StrCat(builder, builder_size, ":");
        }

        // add the value and a trailing comma, then send builder to output
        StrCat(builder, builder_size, cell);
        StrCat(builder, builder_size, ",");
        StrCat(output, maxlen, builder);

        is_empty = false;
    }

    delete snap;

    if (is_empty) {
        // append a comma so that it is replaced with closing bracket
        StrCat(output, maxlen, ",");
    }

    // replace final , with closing bracket
    output[strlen(output) - 1] = (is_array) ? ']' : '}';
}

/**
 * Decodes a JSON string into its JSON_Object representation.
 *
 * @param buffer    Buffer to decode.
 * @param result    Object to store output in. Setting this allows loading over an existing JSON_Object, 'refreshing' it as opposed to creating a new one. [optional, default: null]
 * @param pos       Current position of the decoder as a bytes offset into the buffer.
 * @return          JSON_Object representation of buffer, or null if decoding failed (buffer didn't contain valid JSON).
 */
stock JSON_Object json_decode(const char[] buffer, JSON_Object result = null, int &pos = 0) {
    int length = strlen(buffer);
    bool is_array = false;

    if (json_is_object(buffer[pos])) {
        is_array = false;
    } else if (json_is_array(buffer[pos])) {
        is_array = true;
    } else {
        LogError("json_decode: character not identified as object or array at %d", pos);
        return null;
    }

    if (result == null) result = new JSON_Object(is_array);

    char key[JSON_BUFFER_SIZE];
    char string[JSON_BUFFER_SIZE];
    while ((!is_array && !json_is_object_end(buffer[pos])) ||
            (is_array && !json_is_array_end(buffer[pos]))) {
        ++pos;
        if (!json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at %d", pos);
            return null;
        }

        if (!is_array) {
            if (!json_is_string(buffer[pos])) {
                LogError("json_decode: expected key string at %d", pos);
                return null;
            }

            json_extract_string(buffer, length, pos, key, sizeof(key), is_array);  // fetch object key first
            if (!json_skip_whitespace(buffer, length, pos)) {
                LogError("json_decode: buffer ended early at %d", pos);
                return null;
            }
            if (buffer[pos++] != ':') {
                LogError("json_decode: expected colon after key at %d", pos);
                return null;
            }
        }

        if (!json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at %d", pos);
            return null;
        }

        if (json_is_object(buffer[pos]) || json_is_array(buffer[pos])) {
            JSON_Object current = (!is_array) ? result.GetObject(key) : null;

            JSON_Object value = json_decode(buffer, current, pos);
            if (value == null) return null;  // drop back a call, the recursive decode failed

            if (is_array) {
                result.PushObject(value);
            } else {
                result.SetObject(key, value);
            }
            // increment past end bracket
            ++pos;
        } else if (json_is_string(buffer[pos])) {
            if (!json_extract_string(buffer, length, pos, string, sizeof(string), is_array)) {
                LogError("json_decode: couldn't extract string at %d", pos);
                return null;
            }

            if (is_array) {
                result.PushString(string);
            } else {
                result.SetString(key, string);
            }
        } else {
            if (!json_extract_until_end(buffer, length, pos, string, sizeof(string), is_array)) {
                LogError("json_decode: couldn't extract until end at %d", pos);
                return null;
            }

            if (json_is_int(string)) {
                int value = json_extract_int(string);
                if (is_array) {
                    result.PushInt(value);
                } else {
                    result.SetInt(key, value);
                }
            } else if (json_is_float(string)) {
                float value = json_extract_float(string);
                if (is_array) {
                    result.PushFloat(value);
                } else {
                    result.SetFloat(key, value);
                }
            } else if (json_is_bool(string)) {
                bool value = json_extract_bool(string);
                if (is_array) {
                    result.PushBool(value);
                } else {
                    result.SetBool(key, value);
                }
            } else if (json_is_null(string)) {
                if (is_array) {
                    result.PushHandle(null);
                } else {
                    result.SetHandle(key, null);
                }
            }
        }

        if (!json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at %d", pos);
            return null;
        }
    }

    return result;
}

/**
 * Recursively cleans up JSON_Objects and any objects referenced within.
 *
 * @param obj       JSON_Object to clean up.
 */
void json_cleanup(JSON_Object obj) {
    char key[JSON_BUFFER_SIZE];
    bool is_array = obj.IsArray;
    StringMapSnapshot snap = obj.Snapshot();
    for (int i = 0; i < snap.Length; ++i) {
        snap.GetKey(i, key, sizeof(key));
        if (json_is_meta_key(key)) continue;

        JSON_CELL_TYPE type = obj.GetKeyType(key);
        if (type != Type_Object) continue;

        JSON_Object nested_obj = obj.GetObject(key);
        if (nested_obj != null) {
            nested_obj.Cleanup();
            delete nested_obj;
        }
    }

    obj.Clear();
    delete snap;

    if (is_array) {
        obj.SetValue(JSON_ARRAY_INDEX_KEY, 0);
    }
}
