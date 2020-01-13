/**
 * vim: set ts=4 :
 * =============================================================================
 * sm-json
 * Provides a pure SourcePawn implementation of JSON encoding and decoding.
 * https://github.com/clugg/sm-json
 *
 * sm-json (C)2019 James Dickens. (clug)
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
#include <json/helpers/decode>
#include <json/helpers/encode>
#include <json/helpers/string>
#include <json/object>
#include <json/array>

/**
 * Encodes a JSON instance into its string representation.
 *
 * @param obj           Object to encode.
 * @param output        String buffer to store output.
 * @param max_size      Maximum size of string buffer.
 * @param pretty_print  Should the output be pretty printed (newlines and spaces)? [default: false]
 * @param depth         The current depth of the encoder. [default: 0]
 */
stock void json_encode(
    JSON_Object obj,
    char[] output,
    int max_size,
    bool pretty_print = false,
    int depth = 0
)
{
    JSON_Array arr = null;

    bool is_empty = true;
    bool is_array = obj.IsArray;
    if (is_array) {
        arr = view_as<JSON_Array>(obj);
    }

    int builder_size;

    // used in key iterator
    int str_length = 1;
    int int_value;
    int cell_length = 0;

    StringMapSnapshot snap = null;
    int json_size = 0;
    if (is_array) {
        json_size = arr.Length;

        strcopy(output, max_size, "[");
    } else {
        snap = obj.Snapshot();
        json_size = snap.Length;

        strcopy(output, max_size, "{");
    }

    int key_length = 0;
    for (int i = 0; i < json_size; ++i) {
        key_length = (is_array) ? JSON_INDEX_BUFFER_SIZE : snap.KeyBufferSize(i);
        char[] key = new char[key_length];

        if (is_array) {
            IntToString(i, key, key_length);
        } else {
            snap.GetKey(i, key, key_length);

            // skip meta-keys
            if (json_is_meta_key(key)) {
                continue;
            }
        }

        // skip keys that are marked as hidden
        if (obj.GetKeyHidden(key)) {
            continue;
        }

        JSON_CELL_TYPE type = obj.GetKeyType(key);
        // skip keys of unknown type
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
                cell_length = max_size;
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
                JSON_Object child = obj.GetObject(key);
                json_encode(child, cell, cell_length, pretty_print, depth + 1);
            }
        }

        // make the builder fit our key:value
        // use previously determined cell length and + 1 for ,
        builder_size = cell_length + 1;
        if (! is_array) {
            // get the length of the key and + 1 for :
            builder_size += json_cell_string_size(strlen(key)) + 1;

            if (pretty_print) {
                builder_size += strlen(JSON_PP_AFTER_COLON);
            }
        }

        char[] builder = new char[builder_size];
        strcopy(builder, builder_size, "");

        // add the key if we're working with an object
        if (! is_array) {
            json_cell_string(key, builder, builder_size);
            StrCat(builder, builder_size, ":");

            if (pretty_print) {
                StrCat(builder, builder_size, JSON_PP_AFTER_COLON);
            }
        }

        // add the value and a trailing comma
        StrCat(builder, builder_size, cell);
        StrCat(builder, builder_size, ",");

        // prepare pretty printing then send builder to output afterwards
        if (pretty_print) {
            StrCat(output, max_size, JSON_PP_NEWLINE);

            for (int j = 0; j < depth + 1; ++j) {
                StrCat(output, max_size, JSON_PP_INDENT);
            }
        }

        StrCat(output, max_size, builder);

        is_empty = false;
    }

    if (snap != null) {
        delete snap;
    }

    if (! is_empty) {
        // remove the final comma
        output[strlen(output) - 1] = '\0';

        if (pretty_print) {
            StrCat(output, max_size, JSON_PP_NEWLINE);

            for (int j = 0; j < depth; ++j) {
                StrCat(output, max_size, JSON_PP_INDENT);
            }
        }
    }

    // append closing bracket
    StrCat(output, max_size, (is_array) ? "]" : "}");
}

/**
 * Decodes a JSON string into a JSON instance.
 *
 * @param buffer    Buffer to decode.
 * @param existing  Object to store output in. Setting this allows loading over
 *                  an existing JSON instance, 'refreshing' it as opposed to
 *                  creating a new one. [default: null]
 * @param pos       Current position of the decoder as a bytes offset into the buffer.
 * @param depth     Current depth of the decoder as child elements in the object.
 * @returns         JSON instance or null if decoding failed (buffer didn't contain valid JSON).
 * @error           If the buffer does not contain valid JSON, an error will be thrown.
 *                  If the buffer does not contain the same structure type as
 *                  the existing parameter if one is provided, an error will be thrown.
 *                  For example, an object cannot be reliably decoded over an existing
 *                  array and vice versa. Only array over array and object over object
 *                  decoding is supported.
 */
stock JSON_Object json_decode(
    const char[] buffer,
    JSON_Object existing = null,
    int &pos = 0,
    int depth = 0
)
{
    int length = strlen(buffer);
    bool is_array = false;

    // skip preceding whitespace
    if (! json_skip_whitespace(buffer, length, pos)) {
        LogError("json_decode: buffer ended early at position %d", pos);

        return null;
    }

    JSON_Array arr = null;
    JSON_Object obj = null;

    if (json_is_object(buffer[pos])) {
        is_array = false;

        if (existing == null) {
            obj = new JSON_Object();
        } else {
            if (existing.IsArray) {
                LogError("json_decode: attempting to decode an array over an existing object");

                return null;
            }

            obj = view_as<JSON_Object>(existing);
        }
    } else if (json_is_array(buffer[pos])) {
        is_array = true;

        if (existing == null) {
            arr = new JSON_Array();
        } else {
            if (! existing.IsArray) {
                LogError("json_decode: attempting to decode an object over an existing array");

                return null;
            }

            arr = view_as<JSON_Array>(existing);
        }
    } else {
        LogError("json_decode: character not identified as object or array at position %d", pos);

        return null;
    }

    bool empty_checked = false;
    char[] key = new char[length];
    char[] cell = new char[length];

    // while we haven't reached the end of our structure
    while (! is_array && ! json_is_object_end(buffer[pos])
        || is_array && ! json_is_array_end(buffer[pos])) {
        // pos is either an opening structure or comma, so increment past it
        ++pos;

        // skip any whitespace preceding the element
        if (! json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at position %d", pos);

            return null;
        }

        // if we are at the end of an object or array
        // and haven't checked for empty yet, we can stop here (empty structure)
        if ((! is_array && json_is_object_end(buffer[pos])
            || is_array && json_is_array_end(buffer[pos]))
            && ! empty_checked) {
            break;
        }

        empty_checked = true;

        // if dealing with an object, look for the key
        if (! is_array) {
            if (! json_is_string(buffer[pos])) {
                LogError("json_decode: expected key string at position %d", pos);

                return null;
            }

            // extract the key from the buffer
            json_extract_string(buffer, length, pos, key, length, is_array);

            // skip any whitespace following the key
            if (! json_skip_whitespace(buffer, length, pos)) {
                LogError("json_decode: buffer ended early at position %d", pos);

                return null;
            }

            // ensure that we find a colon
            if (buffer[pos++] != ':') {
                LogError("json_decode: expected colon after key at position %d", pos);

                return null;
            }

            // skip any whitespace following the colon
            if (! json_skip_whitespace(buffer, length, pos)) {
                LogError("json_decode: buffer ended early at position %d", pos);

                return null;
            }
        }

        if (json_is_object(buffer[pos]) || json_is_array(buffer[pos])) {
            // if we are dealing with an object or array
            // fetch the existing object if one exists at the key
            JSON_Object current = (! is_array) ? obj.GetObject(key) : null;

            // decode recursively
            JSON_Object value = json_decode(buffer, current, pos, depth + 1);

            // decoding failed, error will be logged in json_decode
            if (value == null) {
                return null;
            }

            if (is_array) {
                arr.PushObject(value);
            } else {
                obj.SetObject(key, value);
            }
        } else if (json_is_string(buffer[pos])) {
            // if we are dealing with a string, attempt to extract it
            if (! json_extract_string(buffer, length, pos, cell, length, is_array)) {
                LogError("json_decode: couldn't extract string at position %d", pos);

                return null;
            }

            if (is_array) {
                arr.PushString(cell);
            } else {
                obj.SetString(key, cell);
            }
        } else {
            if (! json_extract_until_end(buffer, length, pos, cell, length, is_array)) {
                LogError("json_decode: couldn't extract until end at position %d", pos);

                return null;
            }

            if (strlen(cell) == 0) {
                LogError("json_decode: empty cell encountered at position %d", pos);

                return null;
            }

            if (json_is_int(cell)) {
                int value = json_extract_int(cell);
                if (is_array) {
                    arr.PushInt(value);
                } else {
                    obj.SetInt(key, value);
                }
            } else if (json_is_float(cell)) {
                float value = json_extract_float(cell);
                if (is_array) {
                    arr.PushFloat(value);
                } else {
                    obj.SetFloat(key, value);
                }
            } else if (json_is_bool(cell)) {
                bool value = json_extract_bool(cell);
                if (is_array) {
                    arr.PushBool(value);
                } else {
                    obj.SetBool(key, value);
                }
            } else if (json_is_null(cell)) {
                if (is_array) {
                    arr.PushNull();
                } else {
                    obj.SetNull(key);
                }
            } else {
                LogError("json_decode: unknown type encountered at position %d: %s", pos, cell);

                return null;
            }
        }

        if (! json_skip_whitespace(buffer, length, pos)) {
            LogError("json_decode: buffer ended early at position %d", pos);

            return null;
        }
    }

    // skip remaining whitespace and ensure we're at the end of the buffer
    ++pos;
    if (json_skip_whitespace(buffer, length, pos) && depth == 0) {
        LogError("json_decode: unexpected data after end of structure at position %d", pos);

        return null;
    }

    JSON_Object result;
    if (is_array) {
        result = view_as<JSON_Object>(arr);
    } else {
        result = view_as<JSON_Object>(obj);
    }

    return result;
}

/**
 * Merges the entries from the second object to the first,
 * optionally replacing existing entries with the same key.
 *
 * @param to        Object to merge on to.
 * @param from      Object to merge entries from.
 * @param replace   Should existing keys be replaced if they exist in both objects?
 * @returns         True on success, false otherwise.
 * @error           If the objects are not the same structure type
 *                  as one another, an error will be thrown.
 *                  For example, an object cannot be reliably merged over an array
 *                  and vice versa. Only array over array and object over object
 *                  merging is supported.
 */
stock bool json_merge(JSON_Object to, JSON_Object from, bool replace = true)
{
    if (to.IsArray && ! from.IsArray) {
        LogError("json_merge: attempting to merge an object onto an array");

        return false;
    } else if (! to.IsArray && from.IsArray) {
        LogError("json_merge: attempting to merge an array onto an object");

        return false;
    }

    JSON_Array to_arr = null;

    bool is_array = to.IsArray;
    if (is_array) {
        to_arr = view_as<JSON_Array>(to);
    }

    StringMapSnapshot snap = null;
    int json_size = 0;
    if (is_array) {
        json_size = view_as<JSON_Array>(from).Length;
    } else {
        snap = from.Snapshot();
        json_size = snap.Length;
    }

    int key_length = 0;
    for (int i = 0; i < json_size; ++i) {
        key_length = (is_array) ? JSON_INDEX_BUFFER_SIZE : snap.KeyBufferSize(i);
        char[] key = new char[key_length];

        if (is_array) {
            IntToString(i, key, key_length);
        } else {
            snap.GetKey(i, key, key_length);

            // skip meta-keys
            if (json_is_meta_key(key)) {
                continue;
            }

            // skip already existing keys if we aren't in replace mode
            if (! replace && to.HasKey(key)) {
                continue;
            }
        }

        JSON_CELL_TYPE type = from.GetKeyType(key);
        // skip keys of unknown type
        if (type == Type_Invalid) {
            continue;
        }

        // merge value onto structure
        switch (type) {
            case Type_String: {
                int length = from.GetKeyLength(key);
                char[] value = new char[length + 1];
                from.GetString(key, value, length + 1);

                if (is_array) {
                    to_arr.PushString(value);
                } else {
                    to.SetString(key, value);
                }
            }
            case Type_Int: {
                int value = from.GetInt(key);

                if (is_array) {
                    to_arr.PushInt(value);
                } else {
                    to.SetInt(key, value);
                }
            }
            case Type_Float: {
                float value = from.GetFloat(key);

                if (is_array) {
                    to_arr.PushFloat(value);
                } else {
                    to.SetFloat(key, value);
                }
            }
            case Type_Bool: {
                bool value = from.GetBool(key);

                if (is_array) {
                    to_arr.PushBool(value);
                } else {
                    to.SetBool(key, value);
                }
            }
            case Type_Null: {
                if (is_array) {
                    to_arr.PushNull();
                } else {
                    to.SetNull(key);
                }
            }
            case Type_Object: {
                JSON_Object value = from.GetObject(key);

                if (is_array) {
                    to_arr.PushObject(value);
                } else {
                    to.SetObject(key, value);
                }
            }
        }

        // set hidden attribute
        bool hidden = from.GetKeyHidden(key);
        if (is_array) {
            to_arr.SetKeyHidden(to_arr.Length - 1, hidden);
        } else {
            to.SetKeyHidden(key, hidden);
        }
    }

    return true;
}

/**
 * Creates a shallow copy of the specified object.
 *
 * @param obj   Object to copy.
 * @returns     A shallow copy of the specified object.
 */
stock JSON_Object json_copy_shallow(JSON_Object obj)
{
    JSON_Object result = (obj.IsArray) ? view_as<JSON_Object>(new JSON_Array()) : new JSON_Object();
    result.Merge(obj);

    return result;
}

/**
 * Creates a deep copy of the specified object.
 *
 * @param obj   Object to copy.
 * @returns     A deep copy of the specified object.
 */
stock JSON_Object json_copy_deep(JSON_Object obj)
{
    JSON_Object result = (obj.IsArray) ? view_as<JSON_Object>(new JSON_Array()) : new JSON_Object();
    result.Merge(obj);

    int key_length = 0;
    StringMapSnapshot snap = result.Snapshot();
    for (int i = 0; i < snap.Length; ++i) {
        key_length = snap.KeyBufferSize(i);
        char[] key = new char[key_length];

        // ignore meta keys
        snap.GetKey(i, key, key_length);
        if (json_is_meta_key(key)) {
            continue;
        }

        // only deep copy objects
        JSON_CELL_TYPE type = obj.GetKeyType(key);
        if (type != Type_Object) {
            continue;
        }

        JSON_Object value = obj.GetObject(key);
        result.SetObject(key, value.DeepCopy());
    }

    return result;
}

/**
 * Recursively cleans up a JSON instance and any JSON instances stored within.
 *
 * @param obj   JSON instance to clean up.
 */
stock void json_cleanup(JSON_Object obj)
{
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

        JSON_Object value = obj.GetObject(key);
        if (value != null) {
            json_cleanup(value);
            delete value;
        }
    }
    delete snap;

    if (obj.IsArray) {
        JSON_Array obj_arr = view_as<JSON_Array>(obj);
        obj_arr.Clear();
    } else {
        obj.Clear();
    }
}
