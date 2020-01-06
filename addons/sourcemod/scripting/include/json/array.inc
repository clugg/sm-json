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

#if defined _json_array_included
 #endinput
#endif
#define _json_array_included

#include <string>
#include <json/definitions>
#include <json/helpers/encode>
#include <json/object>

methodmap JSON_Array < JSON_Object
{
    /**
     * @section Helpers
     */

    /**
     * Views the instance as its parent to access overridden methods.
     */
    property JSON_Object parent
    {
        public get()
        {
            return view_as<JSON_Object>(this);
        }
    }

    /**
     * The number of elements in the array.
     */
    property int Length
    {
        public get()
        {
            int length = 0;
            this.parent.GetValue(JSON_ARRAY_INDEX_KEY, length);

            return length;
        }

        public set(int value)
        {
            this.parent.SetValue(JSON_ARRAY_INDEX_KEY, value);
        }
    }

    /**
     * Increases the length of the array by 1 and returns true.
     *
     * @returns     True.
     */
    public bool IncrementIndex()
    {
        this.Length += 1;

        return true;
    }

    /**
     * Checks whether the object has an index.
     *
     * @param index     Index to check existence of.
     * @returns         True if the index exists, false otherwise.
     */
    public bool HasKey(int index)
    {
        return index >= 0 && index < this.Length;
    }

    /**
     * Outputs an index as a string with optional validation.
     *
     * @param index     Index to output.
     * @param output    String buffer to store output.
     * @param max_size  Maximum size of string buffer.
     * @param validate  Should we check that the index is valid before
     *                  converting it to a string? [default: true]
     * @returns         True if the index is valid, false otherwise.
     */
    public bool GetIndexAsString(
        int index,
        char[] output,
        int max_size,
        bool validate = true
    )
    {
        if (validate && ! this.HasKey(index)) {
            return false;
        }

        return IntToString(index, output, max_size) > 0;
    }

    /**
     * @section Internal Getters
     */

    /**
     * Gets the cell type stored at an index.
     *
     * @param index     Index to get value type for.
     * @returns         Value type for index provided,
     *                  or Type_Invalid if it does not exist.
     */
    public JSON_CELL_TYPE GetKeyType(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return Type_Invalid;
        }

        return this.parent.GetKeyType(key);
    }

    /**
     * Gets the length of the string stored at an index.
     *
     * @param index     Index to get string length for.
     * @returns         Length of string at index provided,
     *                  or -1 if it is not a string/does not exist.
     */
    public int GetKeyLength(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return -1;
        }

        return this.parent.GetKeyLength(key);
    }

    /**
     * Gets whether the index should be hidden from encoding.
     *
     * @param index     Index to get hidden state for.
     * @returns         Whether or not the index should be hidden.
     */
    public bool GetKeyHidden(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return false;
        }

        return this.parent.GetKeyHidden(key);
    }

    /**
     * @section Internal Setters
     */

    /**
     * Sets the cell type stored at a index.
     *
     * @param index     Index to set value type for.
     * @param type      Type to set index to.
     * @returns         True on success, false otherwise.
     */
    public bool SetKeyType(int index, JSON_CELL_TYPE type)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return false;
        }

        return this.parent.SetKeyType(key, type);
    }

    /**
     * Sets the length of the string stored at a index.
     *
     * @param index     Index to set string length for.
     * @param length    Length to set string to.
     * @returns         True on success, false otherwise.
     */
    public bool SetKeyLength(int index, int length)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return false;
        }

        return this.parent.SetKeyLength(key, length);
    }

    /**
     * Sets whether the index should be hidden from encoding.
     *
     * @param index     Index to set hidden state for.
     * @param hidden    Wheter or not the index should be hidden.
     * @returns         True on success, false otherwise.
     */
    public bool SetKeyHidden(int index, bool hidden)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return false;
        }

        return this.parent.SetKeyHidden(key, hidden);
    }

    /**
     * @section Getters
     */

    /**
     * Retrieves the value stored at an index.
     *
     * @param index     Index to retrieve value for.
     * @param value     Variable to store value.
     * @returns         True on success. False if the key is not set,
     *                  or the key is set as an array or string (not a value).
     */
    public any GetValue(int index, any &value)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return -1;
        }

        return this.parent.GetValue(key, value);
    }

    /**
     * Retrieves the string stored at an index.
     *
     * @param index     Index to retrieve string value for.
     * @param value     Buffer to store value.
	 * @param max_size  Maximum size of string buffer.
	 * @param size      Optional parameter to store the number of bytes written to the buffer.
     * @returns         True on success. False if the key is not set,
     *                  or the key is set as a value or array (not a string).
     */
    public bool GetString(int index, char[] value, int max_size, int &size = 0)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return false;
        }

        return this.parent.GetString(key, value, max_size, size);
    }

    /**
     * Retrieves the int stored at an index.
     *
     * @param index     Index to retrieve int value for.
     * @returns         Value stored at index.
     */
    public int GetInt(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return -1;
        }

        return this.parent.GetInt(key);
    }

    /**
     * Retrieves the float stored at an index.
     *
     * @param index     Index to retrieve float value for.
     * @returns         Value stored at index.
     */
    public float GetFloat(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return -1.0;
        }

        return this.parent.GetFloat(key);
    }

    /**
     * Retrieves the bool stored at an index.
     *
     * @param index     Index to retrieve bool value for.
     * @returns         Value stored at index.
     */
    public bool GetBool(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return false;
        }

        return this.parent.GetBool(key);
    }

    /**
     * Retrieves null stored at an index.
     *
     * @param index     Index to retrieve null value for.
     * @returns         Value stored at index.
     */
    public Handle GetNull(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return null;
        }

        return this.parent.GetNull(key);
    }

    /**
     * Retrieves the JSON object stored at an index.
     *
     * @param index     Index to retrieve object value for.
     * @returns         Value stored at index.
     */
    public JSON_Object GetObject(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key))) {
            return null;
        }

        return this.parent.GetObject(key);
    }

    /**
     * @section Setters
     */

    /**
     * Pushes a string to the end of the array.
     *
     * @param value     Value to push.
     * @returns         True on success, false otherwise.
     */
    public bool PushString(const char[] value)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(this.Length, key, sizeof(key), false)) {
            return false;
        }

        return this.parent.SetString(key, value)
            && this.IncrementIndex();
    }

    /**
     * Pushes an int to the end of the array.
     *
     * @param value     Value to push.
     * @returns         True on success, false otherwise.
     */
    public bool PushInt(int value)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(this.Length, key, sizeof(key), false)) {
            return false;
        }

        return this.parent.SetInt(key, value)
            && this.IncrementIndex();
    }

    /**
     * Pushes a float to the end of the array.
     *
     * @param value     Value to push.
     * @returns         True on success, false otherwise.
     */
    public bool PushFloat(float value)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(this.Length, key, sizeof(key), false)) {
            return false;
        }

        return this.parent.SetFloat(key, value)
            && this.IncrementIndex();
    }

    /**
     * Pushes a bool to the end of the array.
     *
     * @param value     Value to push.
     * @returns         True on success, false otherwise.
     */
    public bool PushBool(bool value)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(this.Length, key, sizeof(key), false)) {
            return false;
        }

        return this.parent.SetBool(key, value)
            && this.IncrementIndex();
    }

    /**
     * Pushes null to the end of the array.
     *
     * @returns         True on success, false otherwise.
     */
    public bool PushNull()
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(this.Length, key, sizeof(key), false)) {
            return false;
        }

        return this.parent.SetNull(key)
            && this.IncrementIndex();
    }

    /**
     * Pushes a JSON object to the end of the array.
     *
     * @param value     Value to push.
     * @returns         True on success, false otherwise.
     */
    public bool PushObject(JSON_Object value)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(this.Length, key, sizeof(key), false)) {
            return false;
        }

        return this.parent.SetObject(key, value)
            && this.IncrementIndex();
    }

    /**
     * @section Search Helpers
     */

    /**
     * Finds the index of a value in the array.
     *
     * @param value     Value to search for.
     * @returns         The index of the value if it is found, -1 otherwise.
     */
    public int IndexOf(any value)
    {
        any current;
        for (int i = 0; i < this.Length; ++i) {
            if (this.GetValue(i, current) && value == current) {
                return i;
            }
        }

        return -1;
    }

    /**
     * Finds the index of a string in the array.
     *
     * @param value     String to search for.
     * @returns         The index of the string if it is found, -1 otherwise.
     */
    public int IndexOfString(const char[] value)
    {
        for (int i = 0; i < this.Length; ++i) {
            if (this.GetKeyType(i) != Type_String) {
                continue;
            }

            int current_size = this.GetKeyLength(i) + 1;
            char[] current = new char[current_size];
            this.GetString(i, current, current_size);
            if (StrEqual(value, current)) {
                return i;
            }
        }

        return -1;
    }

    /**
     * Determines whether the array contains a value.
     *
     * @param value     Value to search for.
     * @returns         True if the value is found, false otherwise.
     */
    public bool Contains(any value)
    {
        return this.IndexOf(value) != -1;
    }

    /**
     * Determines whether the array contains a string.
     *
     * @param value     String to search for.
     * @returns         True if the string is found, false otherwise.
     */
    public bool ContainsString(const char[] value)
    {
        return this.IndexOfString(value) != -1;
    }

    /**
     * @section StringMap Overrides
     */

    /**
     * Clears all entries and ensures the array index is re-set.
     */
    public void Clear()
    {
        this.parent.Clear();
        this.Length = 0;
    }

    /**
     * Removes an index and its related meta-keys from the array,
     * and shifts down all following element indices.
     *
     * @param key   Key to remove.
     * @returns     True on success, false if the value was never set.
     */
    public bool Remove(int index)
    {
        char key[JSON_INDEX_BUFFER_SIZE];
        if (! this.GetIndexAsString(index, key, sizeof(key), false)) {
            return false;
        }

        // remove existing value at index
        if (! this.parent.Remove(key)) {
            return false;
        }

        // shift all following elements down
        char target[JSON_INDEX_BUFFER_SIZE];
        for (int i = index + 1; i < this.Length; ++i) {
            if (
                ! this.GetIndexAsString(i, key, sizeof(key), false)
                || ! this.GetIndexAsString(i - 1, target, sizeof(target), false)
            ) {
                return false;
            }

            JSON_CELL_TYPE type = this.GetKeyType(i);

            switch (type) {
                case Type_String: {
                    int str_length = this.GetKeyLength(i);
                    char[] str_value = new char[str_length];

                    this.GetString(i, str_value, str_length + 1);
                    this.parent.SetString(target, str_value);
                }
                case Type_Int: {
                    this.parent.SetInt(target, this.GetInt(i));
                }
                case Type_Float: {
                    this.parent.SetFloat(target, this.GetFloat(i));
                }
                case Type_Bool: {
                    this.parent.SetBool(target, this.GetBool(i));
                }
                case Type_Null: {
                    this.parent.SetNull(target);
                }
                case Type_Object: {
                    this.parent.SetObject(target, this.GetObject(i));
                }
            }

            this.parent.SetKeyHidden(target, this.GetKeyHidden(i));
            this.parent.Remove(key);
        }

        this.Length -= 1;

        return true;
    }

    /**
     * @section Constructor
     */

    /**
     * Creates a new JSON_Array.
     *
     * @returns     A new JSON_Array.
     */
    public JSON_Array()
    {
        JSON_Array self = view_as<JSON_Array>(new JSON_Object());
        self.Clear();

        return self;
    }
};
