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

#if defined _json_encode_helpers_included
 #endinput
#endif
#define _json_encode_helpers_included

#include <string>

/**
 * @section Calculating buffer sizes for JSON cells.
 */

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of a string.
 *
 * @param maxlen    The string's current length or buffer size.
 * @return          Maximum buffer length.
 */
stock int json_cell_string_size(int maxlen) {
    return (maxlen * 2) + 3;  // * 2 for potential escaping, + 2 for surrounding quotes + NULL
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of an int.
 *
 * @param input    The int.
 * @return          Maximum buffer length.
 */
stock int json_cell_int_size(int input) {
    if (input == 0) {
        return 2;  // "0" + NULL
    }

    return ((input < 0) ? 1 : 0) + RoundToFloor(Logarithm(FloatAbs(float(input)), 10.0)) + 2;  // neg sign + number of digits + NULL
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of a float.
 *
 * @return          Maximum buffer length.
 */
stock int json_cell_float_size() {
    return JSON_FLOAT_BUFFER_SIZE;  // fixed-length
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of a bool.
 *
 * @return          Maximum buffer length.
 */
stock int json_cell_bool_size() {
    return 6;  // "true|false" + NULL
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of null.
 *
 * @return          Maximum buffer length.
 */
stock int json_cell_null_size() {
    return 5;  // "null" + NULL
}

/**
 * @section Generating JSON cells.
 */

/**
 * Generates the JSON cell representation of a string.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param maxlen    Maximum size of string buffer.
 */
stock void json_cell_string(const char[] input, char[] output, int maxlen) {
    strcopy(output, maxlen, "_"); // add dummy char at start so first quotation isn't escaped
    StrCat(output, maxlen, input); // add input string to output
    // escape everything according to JSON spec
    json_escape_string(output, maxlen);

    // surround string with quotations
    output[0] = '"';
    StrCat(output, maxlen, "\"");
}

/**
 * Generates the JSON cell representation of an int.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param maxlen    Maximum size of string buffer.
 */
stock void json_cell_int(int input, char[] output, int maxlen) {
    IntToString(input, output, maxlen);
}

/**
 * Generates the JSON cell representation of a float.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param maxlen    Maximum size of string buffer.
 */
stock void json_cell_float(float input, char[] output, int maxlen) {
    FloatToString(input, output, maxlen);
}

/**
 * Generates the JSON cell representation of a bool.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param maxlen    Maximum size of string buffer.
 */
stock void json_cell_bool(bool input, char[] output, int maxlen) {
    strcopy(output, maxlen, (input) ? "true" : "false");
}

/**
 * Generates the JSON cell representation of null.
 *
 * @param output    String buffer to store output.
 * @param maxlen    Maximum size of string buffer.
 */
stock void json_cell_null(char[] output, int maxlen) {
    strcopy(output, maxlen, "null");
}
