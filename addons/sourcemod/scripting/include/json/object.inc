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

#if defined _json_object_included
 #endinput
#endif
#define _json_object_included

#include <string>
#include <json/definitions>
#include <json/encode_helpers>
#include <json>

methodmap JSON_Object < StringMap {
    /**
     * Creates a new JSON_Object.
     *
     * @param is_array      Should the object created be an array? [optional, default: false]
     * @return A new JSON_Object.
     */
    public JSON_Object(bool is_array = false) {
        StringMap obj = CreateTrie();
        if (is_array) {
            obj.SetValue(JSON_ARRAY_INDEX_KEY, 0);
        }
        return view_as<JSON_Object>(obj);
    }

    /**
     * Checks whether the object has a key.
     *
     * @param key   Key to check existence of.
     * @return      True if the key exists, false otherwise.
     */
    public bool HasKey(const char[] key) {
        int int_value;
        char str_value[1];

        return this.GetValue(key, int_value) ||
                this.GetString(key, str_value, sizeof(str_value));
    }

    /**
     * @section Array helpers.
     */

    /**
     * Whether the current object is an array.
     */
    property bool IsArray {
        public get() {
            return this.HasKey(JSON_ARRAY_INDEX_KEY);
        }
    }

    /**
     * The current index of the object if it is an array, or -1 otherwise.
     */
    property int CurrentIndex {
        public get() {
            if (!this.IsArray) return -1;

            int result;
            return (this.GetValue(JSON_ARRAY_INDEX_KEY, result)) ? result : -1;
        }
    }

    /**
     * The number of items in the object if it is an array,
     * or the number of keys (including meta-keys) otherwise.
     */
    property int Length {
        public get() {
            StringMapSnapshot snap = this.Snapshot();
            int length = (this.IsArray) ? this.CurrentIndex : snap.Length;
            delete snap;
            return length;
        }
    }

    /**
     * Increments the current index of the object.
     *
     * @return True on success, false if the current object is not an array.
     */
    public bool IncrementIndex() {
        int current = this.CurrentIndex;
        if (current == -1) return false;

        return this.SetValue(JSON_ARRAY_INDEX_KEY, current + 1);
    }

    /**
     * Gets the string representation of an array index.
     *
     * @param output    String buffer to store output.
     * @param maxlen    Maximum size of string buffer.
     * @param key       Key to get string for. [optional, default: current index]
     * @returns         True on success, false otherwise.
     */
    public int GetIndexString(char[] output, int maxlen, int key = -1) {
        key = (key == -1) ? this.CurrentIndex : key;
        if (key == -1) return false;

        return IntToString(key, output, maxlen);
    }

    /**
     * Removes an item from the object by index.
     *
     * @param index     Index of object to remove.
     * @returns         True on success, false if the current object
     *                  is not an array or the index does not exist.
     */
    public bool RemoveIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.Remove(key);
    }

    /**
     * @section Internal getters.
     */

    /**
     * Gets the cell type stored at a key.
     *
     * @param key   Key to get value type for.
     * @return      Value type for key provided, or Type_Invalid if it does not exist.
     */
    public JSON_CELL_TYPE GetKeyType(const char[] key) {
        int maxlen = strlen(key) + strlen(JSON_META_TYPE_KEY) + 1;
        char[] type_key = new char[maxlen];
        Format(type_key, maxlen, "%s%s", key, JSON_META_TYPE_KEY);

        JSON_CELL_TYPE type;
        return (this.GetValue(type_key, type)) ? type : Type_Invalid;
    }

    /**
     * Gets the cell type stored at an index.
     *
     * @param index     Index to get value type for.
     * @return          Value type for index provided, or Type_Invalid if it does not exist.
     */
    public JSON_CELL_TYPE GetKeyTypeIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return Type_Invalid;

        return this.GetKeyType(key);
    }

    /**
     * Gets the length of the string stored at a key.
     *
     * @param key   Key to get string length for.
     * @return      Length of string at key provided, or -1 if it is not a string/does not exist.
     */
    public int GetKeyLength(const char[] key) {
        int maxlen = strlen(key) + strlen(JSON_META_LENGTH_KEY) + 1;
        char[] length_key = new char[maxlen];
        Format(length_key, maxlen, "%s%s", key, JSON_META_LENGTH_KEY);

        int length;
        return (this.GetValue(length_key, length)) ? length : -1;
    }

    /**
     * Gets the length of the string stored at an index.
     *
     * @param index     Index to get string length for.
     * @return          Length of string at index provided, or -1 if it is not a string/does not exist.
     */
    public int GetKeyLengthIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return -1;

        return this.GetKeyLength(key);
    }

    /**
     * Gets whether the key should be hidden from encoding.
     *
     * @param key   Key to get hidden state for.
     * @return      Whether or not the key should be hidden.
     */
    public bool GetKeyHidden(const char[] key) {
        int maxlen = strlen(key) + strlen(JSON_META_HIDDEN_KEY) + 1;
        char[] length_key = new char[maxlen];
        Format(length_key, maxlen, "%s%s", key, JSON_META_HIDDEN_KEY);

        bool hidden;
        return (this.GetValue(length_key, hidden)) ? hidden : false;
    }

    /**
     * Gets whether the index should be hidden from encoding.
     *
     * @param index     Index to get hidden state for.
     * @return          Whether or not the index should be hidden.
     */
    public bool GetKeyHiddenIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.GetKeyHidden(key);
    }

    /**
     * @section Internal setters.
     */

    /**
     * Sets the cell type stored at a key.
     *
     * @param key   Key to set value type for.
     * @param type  Type to set key to.
     * @return      True on success, false otherwise.
     */
    public bool SetKeyType(const char[] key, JSON_CELL_TYPE type) {
        int maxlen = strlen(key) + strlen(JSON_META_TYPE_KEY) + 1;
        char[] type_key = new char[maxlen];
        Format(type_key, maxlen, "%s%s", key, JSON_META_TYPE_KEY);

        return this.SetValue(type_key, type);
    }

    /**
     * Sets the cell type stored at an index.
     *
     * @param index     Index to set value type for.
     * @param type      Type to set index to.
     * @return          True on success, false otherwise.
     */
    public bool SetKeyTypeIndexed(int index, JSON_CELL_TYPE value) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetKeyType(key, value);
    }

    /**
     * Sets the length of the string stored at a key.
     *
     * @param key       Key to set string length for.
     * @param length    Length to set string to.
     * @return          True on success, false otherwise.
     */
    public bool SetKeyLength(const char[] key, int length) {
        int maxlen = strlen(key) + strlen(JSON_META_LENGTH_KEY) + 1;
        char[] length_key = new char[maxlen];
        Format(length_key, maxlen, "%s%s", key, JSON_META_LENGTH_KEY);

        return this.SetValue(length_key, length);
    }

    /**
     * Sets the length of the string stored at an index.
     *
     * @param index       Index to set string length for.
     * @param length      Length to set string to.
     * @return            True on success, false otherwise.
     */
    public bool SetKeyLengthIndexed(int index, int length) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetKeyLength(key, length);
    }

    /**
     * Sets whether the key should be hidden from encoding.
     *
     * @param key       Key to set hidden state for.
     * @param hidden    Wheter or not the key should be hidden.
     * @return          True on success, false otherwise.
     */
    public bool SetKeyHidden(const char[] key, bool hidden) {
        int maxlen = strlen(key) + strlen(JSON_META_HIDDEN_KEY) + 1;
        char[] hidden_key = new char[maxlen];
        Format(hidden_key, maxlen, "%s%s", key, JSON_META_HIDDEN_KEY);

        return this.SetValue(hidden_key, hidden);
    }

    /**
     * Sets whether the index should be hidden from encoding.
     *
     * @param index       Index to set hidden state for.
     * @param hidden      Wheter or not the index should be hidden.
     * @return            True on success, false otherwise.
     */
    public bool SetKeyHiddenIndexed(int index, bool hidden) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetKeyHidden(key, hidden);
    }

    /**
     * @section Object getters.
     */

    /**
     * Retrieves the int stored at a key.
     *
     * @param key   Key to retrieve int value for.
     * @returns     Value stored at key.
     */
    public int GetInt(const char[] key) {
        int value;
        return (this.GetValue(key, value)) ? value : -1;
    }

    /**
     * Retrieves the int stored at an index.
     *
     * @param index      Index to retrieve int value for.
     * @returns         Value stored at index.
     */
    public int GetIntIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return -1;

        return this.GetInt(key);
    }

    /**
     * Retrieves the float stored at a key.
     *
     * @param key   Key to retrieve float value for.
     * @returns     Value stored at key.
     */
    public float GetFloat(const char[] key) {
        float value;
        return (this.GetValue(key, value)) ? value : -1.0;
    }

    /**
     * Retrieves the float stored at an index.
     *
     * @param index     Index to retrieve float value for.
     * @returns         Value stored at index.
     */
    public float GetFloatIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return -0.0;

        return this.GetFloat(key);
    }

    /**
     * Retrieves the bool stored at a key.
     *
     * @param key   Key to retrieve bool value for.
     * @returns     Value stored at key.
     */
    public bool GetBool(const char[] key) {
        bool value;
        return (this.GetValue(key, value)) ? value : false;
    }

    /**
     * Retrieves the bool stored at an index.
     *
     * @param index     Index to retrieve bool value for.
     * @returns         Value stored at index.
     */
    public bool GetBoolIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.GetBool(key);
    }

    /**
     * Retrieves the handle stored at a key.
     *
     * @param key   Key to retrieve handle value for.
     * @returns     Value stored at key.
     */
    public Handle GetHandle(const char[] key) {
        Handle value;
        return (this.GetValue(key, value)) ? value : null;
    }

    /**
     * Retrieves the handle stored at an index.
     *
     * @param index     Index to retrieve handle value for.
     * @returns         Value stored at index.
     */
    public Handle GetHandleIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return null;

        return this.GetHandle(key);
    }

    /**
     * Retrieves the object stored at a key.
     *
     * @param key   Key to retrieve object value for.
     * @returns     Value stored at key.
     */
    public JSON_Object GetObject(const char[] key) {
        return view_as<JSON_Object>(this.GetHandle(key));
    }

    /**
     * Retrieves the object stored at an index.
     *
     * @param index     Index to retrieve object value for.
     * @returns         Value stored at index.
     */
    public JSON_Object GetObjectIndexed(int index) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return null;

        return this.GetObject(key);
    }

    /**
     * @section Object setters.
     */

    /**
     * Sets the string stored at a key.
     *
     * @param key       Key to set to string value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetString(const char[] key, const char[] value, bool replace = true) {
        return this.SetString(key, value, replace) &&
                this.SetKeyType(key, Type_String) &&
                this.SetKeyLength(key, strlen(value));
    }

    /**
     * Sets the string stored at an index.
     *
     * @param index     Index to set to string value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetStringIndexed(int index, const char[] value, bool replace = true) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetString(key, value, replace);
    }

    /**
     * Sets the int stored at a key.
     *
     * @param key       Key to set to int value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetInt(const char[] key, int value, bool replace = true) {
        return this.SetValue(key, value, replace) &&
                this.SetKeyType(key, Type_Int);
    }

    /**
     * Sets the int stored at an index.
     *
     * @param index     Index to set to int value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetIntIndexed(int index, int value, bool replace = true) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetInt(key, value, replace);
    }

    /**
     * Sets the float stored at a key.
     *
     * @param key       Key to set to float value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetFloat(const char[] key, float value, bool replace = true) {
        return this.SetValue(key, value, replace) &&
                this.SetKeyType(key, Type_Float);
    }

    /**
     * Sets the float stored at an index.
     *
     * @param index     Index to set to float value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetFloatIndexed(int index, float value, bool replace = true) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetFloat(key, value, replace);
    }

    /**
     * Sets the bool stored at a key.
     *
     * @param key       Key to set to bool value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetBool(const char[] key, bool value, bool replace = true) {
        return this.SetValue(key, value, replace) &&
                this.SetKeyType(key, Type_Bool);
    }

    /**
     * Sets the bool stored at an index.
     *
     * @param index     Index to set to bool value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetBoolIndexed(int index, bool value, bool replace = true) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetBool(key, value, replace);
    }

    /**
     * Sets the handle stored at a key.
     *
     * @param key       Key to set to handle value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetHandle(const char[] key, Handle value = null, bool replace = true) {
        return this.SetValue(key, value, replace) &&
                this.SetKeyType(key, Type_Null);
    }

    /**
     * Sets the handle stored at an index.
     *
     * @param index     Index to set to handle value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetHandleIndexed(int index, Handle value = null, bool replace = true) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetHandle(key, value, replace);
    }

    /**
     * Sets the object stored at a key.
     *
     * @param key       Key to set to object value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetObject(const char[] key, JSON_Object value, bool replace = true) {
        return this.SetValue(key, value, replace) &&
                this.SetKeyType(key, Type_Object);
    }

    /**
     * Sets the object stored at an index.
     *
     * @param index     Index to set to object value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetObjectIndexed(int index, JSON_Object value, bool replace = true) {
        char key[JSON_INDEX_BUFFER_LENGTH];
        if (!this.GetIndexString(key, sizeof(key), index)) return false;

        return this.SetObject(key, value, replace);
    }

    /**
     * @section Array setters.
     */

    /**
     * Pushes a string to the end of the array.
     *
     * @param value     Value to push.
     * @retusn          True on success, false otherwise.
     */
    public bool PushString(const char[] value) {
        return this.SetStringIndexed(this.CurrentIndex, value) &&
                this.IncrementIndex();
    }

    /**
     * Pushes an int to the end of the array.
     *
     * @param value     Value to push.
     * @retusn          True on success, false otherwise.
     */
    public bool PushInt(int value) {
        return this.SetIntIndexed(this.CurrentIndex, value) &&
                this.IncrementIndex();
    }

    /**
     * Pushes a float to the end of the array.
     *
     * @param value     Value to push.
     * @retusn          True on success, false otherwise.
     */
    public bool PushFloat(float value) {
        return this.SetFloatIndexed(this.CurrentIndex, value) &&
                this.IncrementIndex();
    }

    /**
     * Pushes a bool to the end of the array.
     *
     * @param value     Value to push.
     * @retusn          True on success, false otherwise.
     */
    public bool PushBool(bool value) {
        return this.SetBoolIndexed(this.CurrentIndex, value) &&
                this.IncrementIndex();
    }

    /**
     * Pushes a handle to the end of the array.
     *
     * @param value     Value to push.
     * @retusn          True on success, false otherwise.
     */
    public bool PushHandle(Handle value = null) {
        return this.SetHandleIndexed(this.CurrentIndex, value) &&
                this.IncrementIndex();
    }

    /**
     * Pushes an object to the end of the array.
     *
     * @param value     Value to push.
     * @retusn          True on success, false otherwise.
     */
    public bool PushObject(JSON_Object value) {
        return this.SetObjectIndexed(this.CurrentIndex, value) &&
                this.IncrementIndex();
    }

    /**
     * @section Generic.
     */

    /**
     * Encodes the object into its string representation.
     *
     * @param output    String buffer to store output.
     * @param maxlen    Maximum size of string buffer.
     */
    public void Encode(char[] output, int maxlen) {
        json_encode(this, output, maxlen);
    }

    /**
     * Decodes a JSON string into this object.
     *
     * @param buffer    Buffer to decode.
     */
    public void Decode(const char[] buffer) {
        json_decode(buffer, this);
    }

    /**
     * Recursively cleans up the object and any objects referenced within.
     */
    public void Cleanup() {
        json_cleanup(this);
    }
};
