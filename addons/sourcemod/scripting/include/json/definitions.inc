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

#if defined _json_definitions_included
 #endinput
#endif
#define _json_definitions_included

#include <string>
#include <json/string_helpers>

// you may need to tinker with these for large objects/floating point numbers
#define JSON_BUFFER_SIZE 8192
#define JSON_FLOAT_BUFFER_SIZE 32
#define JSON_INDEX_BUFFER_LENGTH 16

// meta-key defines
#define JSON_ARRAY_INDEX_KEY "__array_index"
#define JSON_META_TYPE_KEY   ":type"
#define JSON_META_LENGTH_KEY ":length"
#define JSON_META_HIDDEN_KEY ":hidden"

/**
 * Types of cells within a JSON object
 */
enum JSON_CELL_TYPE {
    Type_Invalid = -1,
    Type_String = 0,
    Type_Int,
    Type_Float,
    Type_Bool,
    Type_Null,
    Type_Object
};

/**
 * Multi-dimensional char array mapping characters to their escaped form
 */
char JSON_STRING_NORMAL[][]  = {"\"",   "\\",   "/",   "\b",  "\f",  "\n",  "\r",  "\t" };
char JSON_STRING_ESCAPED[][] = {"\\\"", "\\\\", "\\/", "\\b", "\\f", "\\n", "\\r", "\\t"};

/**
 * Escapes a string based on the rules defined in json/definitions.inc
 *
 * @param buffer        String buffer.
 * @param maxlength     Maximum buffer length.
 */
public void json_escape_string(char[] buffer, int maxlen) {
    for (int i = 0; i < sizeof(JSON_STRING_NORMAL); ++i) {
        ReplaceString(buffer, maxlen, JSON_STRING_NORMAL[i], JSON_STRING_ESCAPED[i]);
    }
}

/**
 * Unescapes a string based on the rules defined in json/definitions.inc
 *
 * @param buffer        String buffer.
 * @param maxlength     Maximum buffer length.
 */
public void json_unescape_string(char[] buffer, int maxlen) {
    for (int i = 0; i < sizeof(JSON_STRING_NORMAL); ++i) {
        ReplaceString(buffer, maxlen, JSON_STRING_ESCAPED[i], JSON_STRING_NORMAL[i]);
    }
}

/**
 * Checks whether the key provided is an meta-key that
 * should only be used internally.
 *
 * @param key   Key to check.
 * @return      True when it is a meta-key, false otherwise.
 */
stock bool json_is_meta_key(char[] key) {
    return json_string_endswith(key, JSON_META_TYPE_KEY) ||
            json_string_endswith(key, JSON_META_LENGTH_KEY) ||
            json_string_endswith(key, JSON_META_HIDDEN_KEY) ||
            StrEqual(key, JSON_ARRAY_INDEX_KEY);
}
