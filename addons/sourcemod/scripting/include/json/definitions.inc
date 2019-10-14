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

#if defined _json_definitions_included
 #endinput
#endif
#define _json_definitions_included

#include <string>
#include <json/helpers/string>

/**
 * @section Pretty Print Constants
 *
 * Used to determine how pretty printed JSON should be formatted when encoded.
 * You can modify these if you prefer your JSON formatted differently.
 */

#define JSON_PP_AFTER_COLON " "
#define JSON_PP_INDENT "    "
#define JSON_PP_NEWLINE "\n"

/**
 * @section Buffer Size Constants
 *
 * You may need to change these if you are working with very large arrays or floating point numbers.
 */

#define JSON_FLOAT_BUFFER_SIZE 32
#define JSON_INDEX_BUFFER_SIZE 16

/**
 * @section Meta-key Constants
 *
 * Used to store metadata for each key in an object.
 * You shouldn't need to change these unless working with keys that may clash with them.
 */

#define JSON_ARRAY_INDEX_KEY "__array_index"
#define JSON_META_TYPE_KEY   ":type"
#define JSON_META_LENGTH_KEY ":length"
#define JSON_META_HIDDEN_KEY ":hidden"

/**
 * @section General
 */

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
 * Checks whether the key provided is a meta-key that should only be used internally.
 *
 * @param key   Key to check.
 * @returns     True when it is a meta-key, false otherwise.
 */
stock bool json_is_meta_key(char[] key)
{
    return json_string_endswith(key, JSON_META_TYPE_KEY)
        || json_string_endswith(key, JSON_META_LENGTH_KEY)
        || json_string_endswith(key, JSON_META_HIDDEN_KEY)
        || StrEqual(key, JSON_ARRAY_INDEX_KEY);
}
