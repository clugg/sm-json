/**
 * vim: set ts=4 :
 * =============================================================================
 * sm-json
 * Provides a pure SourcePawn implementation of JSON encoding and decoding.
 * https://github.com/clugg/sm-json
 *
 * sm-json (C)2018 James Dickens. (clug)
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
 * @param obj           Object to encode.
 * @param output        String buffer to store output.
 * @param maxlen        Maximum size of string buffer.
 * @param pretty_print  Should the output be pretty printed (newlines and spaces)? [optional, default: false]
 * @param depth         The current depth of the encoder. [optional, default: 0]
 */
stock void json_encode(JSON_Object obj, char[] output, int maxlen, bool pretty_print = false, int depth = 0) {
    bool is_array = obj.IsArray;
    bool is_empty = true;
    int builder_size;

    // used in key iterator
    int str_length = 1;
    int int_value;
    int cell_length = 0;

    strcopy(output, maxlen, (is_array) ? "[" : "{");

    StringMapSnapshot snap = obj.Snapshot();
    int key_length = 0;
    for (int i = 0; i < obj.Length; ++i) {
        key_length = snap.KeyBufferSize(i);
        char[] key = new char[key_length];

        if (is_array) {
            obj.GetIndexString(key, key_length, i);
        } else {
            snap.GetKey(i, key, key_length);
        }

        // skip meta-keys
        if (json_is_meta_key(key)) {
            continue;
        }

        // skip keys that are marked as hidden
        if (obj.GetKeyHidden(key)) {
            continue;
        }

        // skip keys of unknown type
        JSON_CELL_TYPE type = obj.GetKeyType(key);
        if (type == Type_Invalid) {
            continue;
        }

        // if we are dealing with a string, prepare the str_value variable for fetching
        if (type == Type_String) {
            str_length = obj.GetKeyLength(key);
        }
        char[] str_value = new char[str_length + 1];

        // determine the length of the char[] needed to represent our cell data
        cell_length = 0;
        switch (type) {
            case Type_String: {
                // get the string value early, as its cell_length is determined by its contents
                obj.GetString(key, str_value, str_length + 1);
                cell_length = json_cell_string_size(str_length);
            }
            case Type_Int: {
                // get the int value early, as its cell_length is determined by its contents
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

        // fit the contents into the cell
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
                json_encode(value, cell, cell_length, pretty_print, depth + 1);
            }
        }

        // make the builder fit our key:value
        builder_size = cell_length + 1;  // use previously determined cell length and +1 for comma
        if (!is_array) {
            builder_size += json_cell_string_size(strlen(key)) + 1;  // get the length of the key, +1 for :
        }

        if (pretty_print && !is_array) {
            // 1 space after colon
            builder_size += strlen(JSON_PP_AFTER_COLON);
        }

        char[] builder = new char[builder_size];
        strcopy(builder, builder_size, "");
        // add the key if it's an object
        if (!is_array) {
            json_cell_string(key, builder, builder_size);
            StrCat(builder, builder_size, ":");
            if (pretty_print) {
                StrCat(builder, builder_size, JSON_PP_AFTER_COLON);
            }
        }

        // add the value and a trailing comma
        StrCat(builder, builder_size, cell);
        StrCat(builder, builder_size, ",");

        if (pretty_print) {
            StrCat(output, maxlen, JSON_PP_NEWLINE);
            for (int j = 0; j < depth + 1; ++j) {
                StrCat(output, maxlen, JSON_PP_INDENT);
            }
        }

        // send builder to output
        StrCat(output, maxlen, builder);

        is_empty = false;
    }

    delete snap;

    if (!is_empty) {
        // remove the final comma
        output[strlen(output) - 1] = '\0';
    }

    if (pretty_print) {
        StrCat(output, maxlen, JSON_PP_NEWLINE);
    }

    if (pretty_print) {
        for (int j = 0; j < depth; ++j) {
            StrCat(output, maxlen, JSON_PP_INDENT);
        }
    }

    // append closing bracket
    StrCat(output, maxlen, (is_array) ? "]" : "}");
}

/**
 * Decodes a JSON string into its JSON_Object representation.
 *
 * @param buffer    Buffer to decode.
 * @param result    Object to store output in. Setting this allows loading over an existing JSON_Object, 'refreshing' it as opposed to creating a new one. [optional, default: null]
 * @param pos       Current position of the decoder as a bytes offset into the buffer.
 * @param depth     Current depth of the decoder as child elements in the object.
 * @returns         JSON_Object representation of buffer, or null if decoding failed (buffer didn't contain valid JSON).
 */
stock JSON_Object json_decode(const char[] buffer, JSON_Object result = null, int &pos = 0, int depth = 0) {
    int length = strlen(buffer);
    bool is_array = false;

    // skip preceding whitespace
    if (!json_skip_whitespace(buffer, length, pos)) {
        LogError("json_decode: buffer ended early at %d", pos);
        return null;
    }

    if (json_is_object(buffer[pos])) {
        is_array = false;
    } else if (json_is_array(buffer[pos])) {
        is_array = true;
    } else {
        LogError("json_decode: character not identified as object or array at %d", pos);
        return null;
    }

    if (result == null) {
        result = new JSON_Object(is_array);
    }

    bool empty_checked = false;
    char[] key = new char[length];
    char[] cell = new char[length];

    // while we haven't reached the end of our structure
    while ((!is_array && !json_is_object_end(buffer[pos]))
        || (is_array && !json_is_array_end(buffer[pos]))) {
        // pos is either an opening structure or comma, so increment past it
        ++pos;

        // if we are at the end of an object or array
        // and haven't checked for empty yet, we can stop here (empty structure)
        if (((!is_array && json_is_object_end(buffer[pos]))
            || (is_array && json_is_array_end(buffer[pos])))
            && !empty_checked) {
            break;
        }

        empty_checked = true;

        // skip any whitespace preceding the element
        if (!json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at %d", pos);
            return null;
        }

        // if dealing with an object, look for the key
        if (!is_array) {
            if (!json_is_string(buffer[pos])) {
                LogError("json_decode: expected key string at %d", pos);
                return null;
            }

            // extract the key from the buffer
            json_extract_string(buffer, length, pos, key, length, is_array);

            // skip any whitespace following the key
            if (!json_skip_whitespace(buffer, length, pos)) {
                LogError("json_decode: buffer ended early at %d", pos);
                return null;
            }

            // ensure that we find a colon
            if (buffer[pos++] != ':') {
                LogError("json_decode: expected colon after key at %d", pos);
                return null;
            }

            // skip any whitespace following the colon
            if (!json_skip_whitespace(buffer, length, pos)) {
                LogError("json_decode: buffer ended early at %d", pos);
                return null;
            }
        }

        if (json_is_object(buffer[pos]) || json_is_array(buffer[pos])) {
            // if we are dealing with an object or array
            // fetch the existing object if one exists at the key
            JSON_Object current = (!is_array) ? result.GetObject(key) : null;

            // decode recursively
            JSON_Object value = json_decode(buffer, current, pos, depth + 1);

            // decoding failed, error will be logged in json_decode
            if (value == null) {
                return null;
            }

            if (is_array) {
                result.PushObject(value);
            } else {
                result.SetObject(key, value);
            }
        } else if (json_is_string(buffer[pos])) {
            // if we are dealing with a string, attempt to extract it
            if (!json_extract_string(buffer, length, pos, cell, length, is_array)) {
                LogError("json_decode: couldn't extract string at %d", pos);
                return null;
            }

            if (is_array) {
                result.PushString(cell);
            } else {
                result.SetString(key, cell);
            }
        } else {
            if (!json_extract_until_end(buffer, length, pos, cell, length, is_array)) {
                LogError("json_decode: couldn't extract until end at %d", pos);
                return null;
            }

            if (strlen(cell) == 0) {
                LogError("json_decode: empty cell encountered at %d", pos);
                return null;
            }

            if (json_is_int(cell)) {
                int value = json_extract_int(cell);
                if (is_array) {
                    result.PushInt(value);
                } else {
                    result.SetInt(key, value);
                }
            } else if (json_is_float(cell)) {
                float value = json_extract_float(cell);
                if (is_array) {
                    result.PushFloat(value);
                } else {
                    result.SetFloat(key, value);
                }
            } else if (json_is_bool(cell)) {
                bool value = json_extract_bool(cell);
                if (is_array) {
                    result.PushBool(value);
                } else {
                    result.SetBool(key, value);
                }
            } else if (json_is_null(cell)) {
                if (is_array) {
                    result.PushHandle(null);
                } else {
                    result.SetHandle(key, null);
                }
            } else {
                LogError("json_decode: unknown type encountered at %d: %s", pos, cell);
                return null;
            }
        }

        if (!json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at %d", pos);
            return null;
        }
    }

    // skip remaining whitespace and ensure we're at the end of the buffer
    ++pos;
    if (json_skip_whitespace(buffer, length, pos) && depth == 0) {
        LogError("json_decode: unexpected data after end of structure at %d", pos);
        return null;
    }

    return result;
}

/**
 * Recursively cleans up JSON_Objects and any objects referenced within.
 *
 * @param obj JSON_Object to clean up.
 */
void json_cleanup(JSON_Object obj) {
    bool is_array = obj.IsArray;

    int key_length = 0;
    StringMapSnapshot snap = obj.Snapshot();
    for (int i = 0; i < snap.Length; ++i) {
        key_length = snap.KeyBufferSize(i);
        char[] key = new char[key_length];

        // ignore meta keys
        snap.GetKey(i, key, key_length);
        if (json_is_meta_key(key)) {
            continue;
        }

        // only clean up objects
        JSON_CELL_TYPE type = obj.GetKeyType(key);
        if (type != Type_Object) {
            continue;
        }

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
