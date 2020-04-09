/**
 * vim: set ts=4 :
 * =============================================================================
 * sm-json
 * Provides a pure SourcePawn implementation of JSON encoding and decoding.
 * https://github.com/clugg/sm-json
 *
 * sm-json (C)2020 James Dickens. (clug)
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
#include <json/definitions>
#include <json/helpers/string>

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
