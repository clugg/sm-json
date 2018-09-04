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

#if defined _json_decode_helpers_included
 #endinput
#endif
#define _json_decode_helpers_included

#include <string>

/**
 * @section Analysing format of incoming JSON cells.
 */

/**
 * Checks whether the character at the given
 * position in the buffer is whitespace.
 *
 * @param buffer    String buffer of data.
 * @param pos       Position to check in buffer.
 * @return          True if buffer[pos] is whitespace, false otherwise.
 */
stock bool json_is_whitespace(const char[] buffer, int &pos) {
    return buffer[pos] == ' ' || buffer[pos] == '\t' ||
            buffer[pos] == '\r' || buffer[pos] == '\n';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of a string.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer[0] is the start of a string, false otherwise.
 */
stock bool json_is_string(const char[] buffer) {
    return buffer[0] == '"';
}

/**
 * Checks whether the buffer provided contains an int.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer contains an int, false otherwise.
 */
stock bool json_is_int(const char[] buffer) {
    int length = strlen(buffer);
    if (buffer[0] != '+' && buffer[0] != '-' && !IsCharNumeric(buffer[0])) {
        return false;
    }

    for (int i = 0; i < length; ++i) {
        if (!IsCharNumeric(buffer[i])) return false;
    }

    return true;
}

/**
 * Checks whether the buffer provided contains a float.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer contains a float, false otherwise.
 */
stock bool json_is_float(const char[] buffer) {
    bool has_decimal = false;
    int length = strlen(buffer);
    if (buffer[0] != '+' && buffer[0] != '-' && buffer[0] != '.' && !IsCharNumeric(buffer[0])) {
        return false;
    }

    for (int i = 0; i < length; ++i) {
        if (buffer[i] == '.') {
            if (has_decimal) {
                return false;
            }

            has_decimal = true;
        } else if (!IsCharNumeric(buffer[i])) {
            return false;
        }
    }

    return true;
}

/**
 * Checks whether the buffer provided contains a bool.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer contains a bool, false otherwise.
 */
stock bool json_is_bool(const char[] buffer) {
    return StrEqual(buffer, "true") ||
            StrEqual(buffer, "false");
}

/**
 * Checks whether the buffer provided contains null.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer contains null, false otherwise.
 */
stock bool json_is_null(const char[] buffer) {
    return StrEqual(buffer, "null");
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of an object.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer[0] is the start of an object, false otherwise.
 */
stock bool json_is_object(const char[] buffer) {
    return buffer[0] == '{';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the end of an object.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer[0] is the end of an object, false otherwise.
 */
stock bool json_is_object_end(const char[] buffer) {
    return buffer[0] == '}';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of an array.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer[0] is the start of an array, false otherwise.
 */
stock bool json_is_array(const char[] buffer) {
    return buffer[0] == '[';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of an array.
 *
 * @param buffer    String buffer of data.
 * @return          True if buffer[0] is the start of an array, false otherwise.
 */
stock bool json_is_array_end(const char[] buffer) {
    return buffer[0] == ']';
}

/**
 * Checks whether the character at the given position in the buffer
 * is considered a valid 'end point' for some data, such as a
 * colon (indicating a key), a comma (indicating a new element),
 * or the end of an object or array.
 *
 * @param buffer    String buffer of data.
 * @param pos       Position to check in buffer.
 * @return          True if buffer[pos] is a valid data end point, false otherwise.
 */
stock bool json_is_at_end(const char[] buffer, int &pos, bool is_array) {
    return buffer[pos] == ',' ||
            (!is_array && buffer[pos] == ':') ||
            json_is_object_end(buffer[pos]) ||
            json_is_array_end(buffer[pos]);
}

/**
 * Moves the position until it reaches a non-whitespace
 * character or the end of the buffer's maximum size.
 *
 * @param buffer    String buffer of data.
 * @param maxlen    Maximum size of string buffer.
 * @param pos       Position to increment.
 * @return          True if pos is not at the end of the buffer, false otherwise.
 */
stock bool json_skip_whitespace(const char[] buffer, int maxlen, int &pos) {
    while (json_is_whitespace(buffer, pos) && pos < maxlen) {
        ++pos;
    }

    return pos < maxlen;
}

/**
 * Extracts a JSON cell from the buffer until
 * a valid end point is reached.
 *
 * @param buffer            String buffer of data.
 * @param maxlen            Maximum size of string buffer.
 * @param pos               Position to increment.
 * @param output            String buffer to store output.
 * @param output_maxlen     Maximum size of output string buffer.
 * @param is_array          Whether the decoder is currently processing an array.
 * @return                  True if pos is not at the end of the buffer, false otherwise.
 */
stock bool json_extract_until_end(const char[] buffer, int maxlen, int &pos, char[] output, int output_maxlen, bool is_array) {
    // extracts a string from current pos until a valid 'end point'
    strcopy(output, output_maxlen, "");

    int start = pos;
    while (!json_is_whitespace(buffer, pos) && !json_is_at_end(buffer, pos, is_array) && pos < maxlen) {
        ++pos;
    }
    int end = pos - 1;

    // skip trailing whitespace
    json_skip_whitespace(buffer, maxlen, pos);

    if (!json_is_at_end(buffer, pos, is_array)) return false;
    strcopy(output, end - start + 2, buffer[start]);

    return pos < maxlen;
}


/**
 * Extracts a JSON string from the buffer until
 * a valid end point is reached.
 *
 * @param buffer            String buffer of data.
 * @param maxlen            Maximum size of string buffer.
 * @param pos               Position to increment.
 * @param output            String buffer to store output.
 * @param output_maxlen     Maximum size of output string buffer.
 * @param is_array          Whether the decoder is currently processing an array.
 * @return                  True if pos is not at the end of the buffer, false otherwise.
 */
stock bool json_extract_string(const char[] buffer, int maxlen, int &pos, char[] output, int output_maxlen, bool is_array) {
    // extracts a string which needs to be quote-escaped
    strcopy(output, output_maxlen, "");

    ++pos;
    int start = pos;
    while (!(buffer[pos] == '"' && buffer[pos - 1] != '\\') && pos < maxlen) {
        ++pos;
    }
    int end = pos - 1;

    // jump 1 ahead since we ended on " instead of an ending char
    ++pos;

    // skip trailing whitespace
    json_skip_whitespace(buffer, maxlen, pos);

    if (!json_is_at_end(buffer, pos, is_array)) return false;
    // copy only from start with length end - start + 2 (+2 for NULL terminator and something else)
    strcopy(output, end - start + 2, buffer[start]);
    json_unescape_string(output, maxlen);

    return pos < maxlen;
}

/**
 * Extracts an int from the buffer.
 *
 * @param buffer            String buffer of data.
 * @return                  Int value of the buffer.
 */
stock int json_extract_int(const char[] buffer) {
    return StringToInt(buffer);
}

/**
 * Extracts a float from the buffer.
 *
 * @param buffer            String buffer of data.
 * @return                  Float value of the buffer.
 */
stock float json_extract_float(const char[] buffer) {
    return StringToFloat(buffer);
}

/**
 * Extracts a bool from the buffer.
 *
 * @param buffer            String buffer of data.
 * @return                  Bool value of the buffer.
 */
stock bool json_extract_bool(const char[] buffer) {
    return StrEqual(buffer, "true");
}
