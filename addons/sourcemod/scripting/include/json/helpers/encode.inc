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

#if defined _json_helpers_encode_included
 #endinput
#endif
#define _json_helpers_encode_included

#include <string>

/**
 * @section Calculate Buffer Size for Value
 */

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of a string.
 *
 * @param length    The length of the string.
 * @returns         Maximum buffer length.
 */
stock int json_cell_string_size(int length)
{
    // double for potential escaping, + 2 for outside quotes + NULL terminator
    return (length * 2) + 3;
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of an int.
 *
 * @param input     Value to calculate maximum buffer length for.
 * @returns         Maximum buffer length.
 */
stock int json_cell_int_size(int input)
{
    if (input == 0) {
        // "0" + NULL terminator
        return 2;
    }

    int result = 0;
    if (input < 0) {
        // negative sign
        result += 1;
    }

    // calculate number of digits in number
    result += RoundToFloor(Logarithm(FloatAbs(float(input)), 10.0)) + 1;

    // NULL terminator
    result += 1;

    return result;
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of a float.
 *
 * @returns     Maximum buffer length.
 */
stock int json_cell_float_size()
{
    return JSON_FLOAT_BUFFER_SIZE;
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of a bool.
 *
 * @returns     Maximum buffer length.
 */
stock int json_cell_bool_size()
{
    // "true"|"false" + NULL terminator
    return 6;
}

/**
 * Calculates the maximum buffer length required to
 * store the JSON cell representation of null.
 *
 * @returns     Maximum buffer length.
 */
stock int json_cell_null_size()
{
    // "null" + NULL terminator
    return 5;
}

/**
 * @section Convert Values to JSON Cells
 */

/**
 * Generates the JSON cell representation of a string.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param max_size  Maximum size of string buffer.
 */
stock void json_cell_string(const char[] input, char[] output, int max_size)
{
    // add dummy char that won't be escaped to replace with a quote later
    strcopy(output, max_size, "?");

    // add input string to output
    StrCat(output, max_size, input);

    // escape the output string
    json_escape_string(output, max_size);

    // surround string with quotations
    output[0] = '"';
    StrCat(output, max_size, "\"");
}

/**
 * Generates the JSON cell representation of an int.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param max_size  Maximum size of string buffer.
 */
stock void json_cell_int(int input, char[] output, int max_size)
{
    IntToString(input, output, max_size);
}

/**
 * Generates the JSON cell representation of a float.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param max_size  Maximum size of string buffer.
 */
stock void json_cell_float(float input, char[] output, int max_size)
{
    FloatToString(input, output, max_size);

    // trim trailing 0s from float output up until decimal point
    int last_char = strlen(output) - 1;
    while (output[last_char] == '0' && output[last_char - 1] != '.') {
        output[last_char--] = '\0';
    }
}

/**
 * Generates the JSON cell representation of a bool.
 *
 * @param input     Value to generate output for.
 * @param output    String buffer to store output.
 * @param max_size  Maximum size of string buffer.
 */
stock void json_cell_bool(bool input, char[] output, int max_size)
{
    strcopy(output, max_size, (input) ? "true" : "false");
}

/**
 * Generates the JSON cell representation of null.
 *
 * @param output    String buffer to store output.
 * @param max_size  Maximum size of string buffer.
 */
stock void json_cell_null(char[] output, int max_size)
{
    strcopy(output, max_size, "null");
}
