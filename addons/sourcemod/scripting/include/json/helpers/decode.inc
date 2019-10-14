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

#if defined _json_helpers_decode_included
 #endinput
#endif
#define _json_helpers_decode_included

#include <string>

/**
 * @section Determine Buffer Contents
 */

/**
 * Checks whether the character at the beginning
 * of the buffer is whitespace.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer
 *                  is whitespace, false otherwise.
 */
stock bool json_is_whitespace(const char[] buffer)
{
    return buffer[0] == ' '
        || buffer[0] == '\t'
        || buffer[0] == '\r'
        || buffer[0] == '\n';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of a string.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer
 *                  is the start of a string, false otherwise.
 */
stock bool json_is_string(const char[] buffer)
{
    return buffer[0] == '"';
}

/**
 * Checks whether the buffer provided contains an int.
 *
 * @param buffer    String buffer of data.
 * @returns         True if buffer contains an int, false otherwise.
 */
stock bool json_is_int(const char[] buffer)
{
    bool starts_with_zero = false;
    bool has_digit_gt_zero = false;

    int length = strlen(buffer);
    for (int i = 0; i < length; ++i) {
        // allow minus as first character only
        if (i == 0 && buffer[i] == '-') {
            continue;
        }

        if (IsCharNumeric(buffer[i])) {
            if (buffer[i] == '0') {
                if (starts_with_zero) {
                    // detect repeating leading zeros
                    return false;
                } else if (! has_digit_gt_zero) {
                    starts_with_zero = true;
                }
            } else {
                has_digit_gt_zero = true;
            }
        } else {
            return false;
        }
    }

    // buffer must start with zero and have no other numerics before decimal
    // OR not start with zero and have other numerics
    return (starts_with_zero && ! has_digit_gt_zero)
        || (! starts_with_zero && has_digit_gt_zero);
}

/**
 * Checks whether the buffer provided contains a float.
 *
 * @param buffer    String buffer of data.
 * @returns         True if buffer contains a float, false otherwise.
 */
stock bool json_is_float(const char[] buffer)
{
    bool starts_with_zero = false;
    bool has_digit_gt_zero = false;
    bool after_decimal = false;
    bool has_digit_after_decimal = false;
    bool after_exponent = false;
    bool has_digit_after_exponent = false;

    int length = strlen(buffer);
    for (int i = 0; i < length; ++i) {
        // allow minus as first character only
        if (i == 0 && buffer[i] == '-') {
            continue;
        }

        // if we haven't encountered a decimal or exponent yet
        if (! after_decimal && ! after_exponent) {
            if (buffer[i] == '.') {
                // if we encounter a decimal before any digits
                if (! starts_with_zero && ! has_digit_gt_zero) {
                    return false;
                }

                after_decimal = true;
            } else if (buffer[i] == 'e' || buffer[i] == 'E') {
                // if we encounter an exponent before any non-zero digits
                if (starts_with_zero && ! has_digit_gt_zero) {
                    return false;
                }

                after_exponent = true;
            } else if (IsCharNumeric(buffer[i])) {
                if (buffer[i] == '0') {
                    if (starts_with_zero) {
                        // detect repeating leading zeros
                        return false;
                    } else if (! has_digit_gt_zero) {
                        starts_with_zero = true;
                    }
                } else {
                    has_digit_gt_zero = true;
                }
            } else {
                return false;
            }
        } else if (after_decimal && ! after_exponent) {
            // after decimal has been encountered, allow any numerics
            if (IsCharNumeric(buffer[i])) {
                has_digit_after_decimal = true;
            } else if (buffer[i] == 'e' || buffer[i] == 'E') {
                if (! has_digit_after_decimal) {
                    // detect exponents directly after decimal
                    return false;
                }

                after_exponent = true;
            } else {
                return false;
            }
        } else if (after_exponent) {
            if (
                (buffer[i] == '+' || buffer[i] == '-')
                && (buffer[i - 1] == 'e' || buffer[i - 1] == 'E')
            ) {
                // allow + or - directly after exponent
                continue;
            } else if (IsCharNumeric(buffer[i])) {
                has_digit_after_exponent = true;
            } else {
                return false;
            }
        }
    }

    if (starts_with_zero && has_digit_gt_zero) {
        /* if buffer starts with zero, there should
           be no other digits before the decimal */
        return false;
    }

    // if we have a decimal, there should be digit(s) after it
    if (after_decimal) {
        if (! has_digit_after_decimal) {
            return false;
        }
    }

    // if we have an exponent, there should be digit(s) after it
    if (after_exponent) {
        if (! has_digit_after_exponent) {
            return false;
        }
    }

    /* we should have reached an exponent, decimal, or both.
       otherwise, this number can be handled by the int parser */
    return after_decimal || after_exponent;
}

/**
 * Checks whether the buffer provided contains a bool.
 *
 * @param buffer    String buffer of data.
 * @returns         True if buffer contains a bool, false otherwise.
 */
stock bool json_is_bool(const char[] buffer)
{
    return StrEqual(buffer, "true") || StrEqual(buffer, "false");
}

/**
 * Checks whether the buffer provided contains null.
 *
 * @param buffer    String buffer of data.
 * @returns         True if buffer contains null, false otherwise.
 */
stock bool json_is_null(const char[] buffer)
{
    return StrEqual(buffer, "null");
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of an object.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer is
 *                  the start of an object, false otherwise.
 */
stock bool json_is_object(const char[] buffer)
{
    return buffer[0] == '{';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the end of an object.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer is
 *                  the end of an object, false otherwise.
 */
stock bool json_is_object_end(const char[] buffer)
{
    return buffer[0] == '}';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the start of an array.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer is
 *                  the start of an array, false otherwise.
 */
stock bool json_is_array(const char[] buffer)
{
    return buffer[0] == '[';
}

/**
 * Checks whether the character at the beginning
 * of the buffer is the end of an array.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer is
 *                  the end of an array, false otherwise.
 */
stock bool json_is_array_end(const char[] buffer)
{
    return buffer[0] == ']';
}

/**
 * Checks whether the character at the beginning of the buffer
 * is considered a valid 'end point' for some data, such as a
 * colon (indicating a key), a comma (indicating a new element),
 * or the end of an object or array.
 *
 * @param buffer    String buffer of data.
 * @returns         True if the first character in the buffer
 *                  is a valid data end point, false otherwise.
 */
stock bool json_is_at_end(const char[] buffer, bool is_array)
{
    return buffer[0] == ','
        || (! is_array && buffer[0] == ':')
        || json_is_object_end(buffer[0])
        || json_is_array_end(buffer[0]);
}

/**
 * @section Extract Contents from Buffer
 */

/**
 * Moves the position until it reaches a non-whitespace
 * character or the end of the buffer's maximum size.
 *
 * @param buffer    String buffer of data.
 * @param max_size  Maximum size of string buffer.
 * @param pos       Position to increment.
 * @returns         True if pos has not reached the end
 *                  of the buffer, false otherwise.
 */
stock bool json_skip_whitespace(const char[] buffer, int max_size, int &pos)
{
    while (json_is_whitespace(buffer[pos]) && pos < max_size) {
        ++pos;
    }

    return pos < max_size;
}

/**
 * Extracts a JSON cell from the buffer until
 * a valid end point is reached.
 *
 * @param buffer            String buffer of data.
 * @param max_size          Maximum size of string buffer.
 * @param pos               Position to increment.
 * @param output            String buffer to store output.
 * @param output_max_size   Maximum size of output string buffer.
 * @param is_array          Whether the decoder is processing an array.
 * @returns                 True if pos has not reached the end
 *                          of the buffer, false otherwise.
 */
stock bool json_extract_until_end(
    const char[] buffer,
    int max_size,
    int &pos,
    char[] output,
    int output_max_size,
    bool is_array
) {
    strcopy(output, output_max_size, "");

    // set start to position of first character in cell
    int start = pos;

    // while we haven't hit whitespace, an end point or the end of the buffer
    while (
        ! json_is_whitespace(buffer[pos])
        && ! json_is_at_end(buffer[pos], is_array)
        && pos < max_size
    ) {
        ++pos;
    }

    // set end to the current position
    int end = pos;

    // skip any following whitespace
    json_skip_whitespace(buffer, max_size, pos);

    // if we aren't at a valid endpoint, extraction has failed
    if (! json_is_at_end(buffer[pos], is_array)) {
        return false;
    }

    // copy only from start with length end - start + NULL terminator
    strcopy(output, end - start + 1, buffer[start]);

    return pos < max_size;
}

/**
 * Extracts a JSON string from the buffer until
 * a valid end point is reached.
 *
 * @param buffer            String buffer of data.
 * @param max_size          Maximum size of string buffer.
 * @param pos               Position to increment.
 * @param output            String buffer to store output.
 * @param output_max_size   Maximum size of output string buffer.
 * @param is_array          Whether the decoder is processing an array.
 * @returns                 True if pos has not reached the end
 *                          of the buffer, false otherwise.
 */
stock bool json_extract_string(
    const char[] buffer,
    int max_size,
    int &pos,
    char[] output,
    int output_max_size,
    bool is_array
) {
    strcopy(output, output_max_size, "");

    // increment past opening quote
    ++pos;

    // set start to position of first character in string
    int start = pos;

    // while we haven't hit the end of the buffer
    while (pos < max_size) {
        // check for unescaped control characters
        if (
            buffer[pos] == '\b'
            || buffer[pos] == '\f'
            || buffer[pos] == '\n'
            || buffer[pos] == '\r'
            || buffer[pos] == '\t'
        ) {
            return false;
        }

        if (buffer[pos] == '"') {
            // count preceding backslashes to check if quote is escaped
            int search_pos = pos;
            int preceding_backslashes = 0;
            while (search_pos > 0 && buffer[--search_pos] == '\\') {
                ++preceding_backslashes;
            }

            // if we have an even number of backslashes, the quote is not escaped
            if (preceding_backslashes % 2 == 0) {
                break;
            }
        }

        // pass over the character as it is part of the string
        ++pos;
    }

    // set end to the current position
    int end = pos;

    // increment past closing quote
    ++pos;

    // skip trailing whitespace
    if (! json_skip_whitespace(buffer, max_size, pos)) {
        return false;
    }

    // if we haven't reached an ending character at the end of the cell,
    // there is likely junk data not encapsulated by a string
    if (! json_is_at_end(buffer[pos], is_array)) {
        return false;
    }

    // copy only from start with length end - start + NULL terminator
    strcopy(output, end - start + 1, buffer[start]);
    json_unescape_string(output, max_size);

    return pos < max_size;
}

/**
 * Extracts an int from the buffer.
 *
 * @param buffer    String buffer of data.
 * @returns         Int value of the buffer.
 */
stock int json_extract_int(const char[] buffer)
{
    return StringToInt(buffer);
}

/**
 * Extracts a float from the buffer.
 *
 * @param buffer    String buffer of data.
 * @returns         Float value of the buffer.
 */
stock float json_extract_float(const char[] buffer)
{
    return StringToFloat(buffer);
}

/**
 * Extracts a bool from the buffer.
 *
 * @param buffer    String buffer of data.
 * @returns         Bool value of the buffer.
 */
stock bool json_extract_bool(const char[] buffer)
{
    return StrEqual(buffer, "true");
}
