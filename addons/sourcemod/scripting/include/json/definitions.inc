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
char JSON_STRING_NORMAL[][]  = {"\"",   "\\",   "/",   "\b",  "\f",  "\n",  "\r",  "\t"};
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
    return String_EndsWith(key, JSON_META_TYPE_KEY) ||
            String_EndsWith(key, JSON_META_LENGTH_KEY) ||
            String_EndsWith(key, JSON_META_HIDDEN_KEY) ||
            StrEqual(key, JSON_ARRAY_INDEX_KEY);
}
