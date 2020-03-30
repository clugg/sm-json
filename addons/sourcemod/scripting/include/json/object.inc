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

#if defined _json_object_included
 #endinput
#endif
#define _json_object_included

#include <string>
#include <json/definitions>
#include <json/helpers/encode>

methodmap JSON_Object < StringMap
{
    /**
     * @section Helpers
     */

    /**
     * Views the instance as its parent to access overridden methods.
     */
    property StringMap parent
    {
        public get()
        {
            return view_as<StringMap>(this);
        }
    }

    /**
     * Checks whether the object has a key.
     *
     * @param key   Key to check existence of.
     * @returns     True if the key exists, false otherwise.
     */
    public bool HasKey(const char[] key)
    {
        int dummy_int;
        char dummy_str[1];

        return this.GetValue(key, dummy_int)
            || this.GetString(key, dummy_str, sizeof(dummy_str));
    }

    /**
     * Whether the current object is an array.
     */
    property bool IsArray {
        public get()
        {
            return this.HasKey(JSON_ARRAY_INDEX_KEY);
        }
    }

    /**
     * The number of keys in the object (including meta-keys).
     */
    property int Length {
        public get()
        {
            StringMapSnapshot snap = this.Snapshot();
            int length = snap.Length;
            delete snap;

            return length;
        }
    }

    /**
     * @section Internal Getters
     */

    /**
     * Gets the cell type stored at a key.
     *
     * @param key   Key to get value type for.
     * @returns     Value type for key provided,
     *              or Type_Invalid if it does not exist.
     */
    public JSON_CELL_TYPE GetKeyType(const char[] key)
    {
        int max_size = strlen(key) + strlen(JSON_META_TYPE_KEY) + 1;
        char[] type_key = new char[max_size];
        FormatEx(type_key, max_size, "%s%s", key, JSON_META_TYPE_KEY);

        JSON_CELL_TYPE type;
        return (this.GetValue(type_key, type)) ? type : Type_Invalid;
    }

    /**
     * Gets the length of the string stored at a key.
     *
     * @param key   Key to get string length for.
     * @returns     Length of string at key provided,
     *              or -1 if it is not a string/does not exist.
     */
    public int GetKeyLength(const char[] key)
    {
        int max_size = strlen(key) + strlen(JSON_META_LENGTH_KEY) + 1;
        char[] length_key = new char[max_size];
        FormatEx(length_key, max_size, "%s%s", key, JSON_META_LENGTH_KEY);

        int length;
        return (this.GetValue(length_key, length)) ? length : -1;
    }

    /**
     * Gets whether the key should be hidden from encoding.
     *
     * @param key   Key to get hidden state for.
     * @returns     Whether or not the key should be hidden.
     */
    public bool GetKeyHidden(const char[] key)
    {
        int max_size = strlen(key) + strlen(JSON_META_HIDDEN_KEY) + 1;
        char[] hidden_key = new char[max_size];
        FormatEx(hidden_key, max_size, "%s%s", key, JSON_META_HIDDEN_KEY);

        return this.HasKey(hidden_key);
    }

    /**
     * @section Internal Setters
     */

    /**
     * Sets the cell type stored at a key.
     *
     * @param key   Key to set value type for.
     * @param type  Type to set key to.
     * @returns     True on success, false otherwise.
     */
    public bool SetKeyType(const char[] key, JSON_CELL_TYPE type)
    {
        int max_size = strlen(key) + strlen(JSON_META_TYPE_KEY) + 1;
        char[] type_key = new char[max_size];
        FormatEx(type_key, max_size, "%s%s", key, JSON_META_TYPE_KEY);

        return this.SetValue(type_key, type);
    }

    /**
     * Sets the length of the string stored at a key.
     *
     * @param key       Key to set string length for.
     * @param length    Length to set string to.
     * @returns         True on success, false otherwise.
     */
    public bool SetKeyLength(const char[] key, int length)
    {
        int max_size = strlen(key) + strlen(JSON_META_LENGTH_KEY) + 1;
        char[] length_key = new char[max_size];
        FormatEx(length_key, max_size, "%s%s", key, JSON_META_LENGTH_KEY);

        return this.SetValue(length_key, length);
    }

    /**
     * Sets whether the key should be hidden from encoding.
     *
     * @param key       Key to set hidden state for.
     * @param hidden    Wheter or not the key should be hidden.
     * @returns         True on success, false otherwise.
     */
    public bool SetKeyHidden(const char[] key, bool hidden)
    {
        int max_size = strlen(key) + strlen(JSON_META_HIDDEN_KEY) + 1;
        char[] hidden_key = new char[max_size];
        FormatEx(hidden_key, max_size, "%s%s", key, JSON_META_HIDDEN_KEY);

        if (hidden) {
            return this.SetValue(hidden_key, hidden);
        } else {
            this.parent.Remove(hidden_key);

            return true;
        }
    }

    /**
     * @section Getters
     */

    // GetValue is implemented natively by StringMap

    // GetString is implemented natively by StringMap

    /**
     * Retrieves the int stored at a key.
     *
     * @param key           Key to retrieve int value for.
     * @param defaultValue  Value to return if the key does not exist.
     * @returns             Value stored at key.
     */
    public int GetInt(const char[] key, int defaultValue = -1)
    {
        int value;
        return (this.GetValue(key, value)) ? value : defaultValue;
    }

    /**
     * Retrieves the float stored at a key.
     *
     * @param key           Key to retrieve float value for.
     * @param defaultValue  Value to return if the key does not exist.
     * @returns             Value stored at key.
     */
    public float GetFloat(const char[] key, float defaultValue = -1.0)
    {
        float value;
        return (this.GetValue(key, value)) ? value : defaultValue;
    }

    /**
     * Retrieves the bool stored at a key.
     *
     * @param key           Key to retrieve bool value for.
     * @param defaultValue  Value to return if the key does not exist.
     * @returns             Value stored at key.
     */
    public bool GetBool(const char[] key, bool defaultValue = false)
    {
        bool value;
        return (this.GetValue(key, value)) ? value : defaultValue;
    }

    /**
     * Retrieves null stored at a key.
     *
     * @param key           Key to retrieve null value for.
     * @param defaultValue  Value to return if the key does not exist.
     * @returns             Value stored at key.
     */
    public Handle GetNull(const char[] key, Handle defaultValue = null)
    {
        Handle value;
        return (this.GetValue(key, value)) ? value : defaultValue;
    }

    /**
     * Retrieves the JSON object stored at a key.
     *
     * @param key           Key to retrieve object value for.
     * @param defaultValue  Value to return if the key does not exist.
     * @returns             Value stored at key.
     */
    public JSON_Object GetObject(const char[] key, JSON_Object defaultValue = null)
    {
        JSON_Object value;
        return this.GetValue(key, value) ? value : defaultValue;
    }

    /**
     * @section Setters
     */

    /**
     * Sets the string stored at a key.
     *
     * @param key       Key to set to string value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetString(const char[] key, const char[] value)
    {
        return this.parent.SetString(key, value)
            && this.SetKeyType(key, Type_String)
            && this.SetKeyLength(key, strlen(value));
    }

    /**
     * Sets the int stored at a key.
     *
     * @param key       Key to set to int value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetInt(const char[] key, int value)
    {
        return this.SetValue(key, value)
            && this.SetKeyType(key, Type_Int);
    }

    /**
     * Sets the float stored at a key.
     *
     * @param key       Key to set to float value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetFloat(const char[] key, float value)
    {
        return this.SetValue(key, value)
            && this.SetKeyType(key, Type_Float);
    }

    /**
     * Sets the bool stored at a key.
     *
     * @param key       Key to set to bool value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetBool(const char[] key, bool value)
    {
        return this.SetValue(key, value)
            && this.SetKeyType(key, Type_Bool);
    }

    /**
     * Sets null at a key.
     *
     * @param key       Key to set to null.
     * @returns         True on success, false otherwise.
     */
    public bool SetNull(const char[] key)
    {
        Handle value = null;
        return this.SetValue(key, value)
            && this.SetKeyType(key, Type_Null);
    }

    /**
     * Sets the JSON object stored at a key.
     *
     * @param key       Key to set to object value.
     * @param value     Value to set.
     * @returns         True on success, false otherwise.
     */
    public bool SetObject(const char[] key, JSON_Object value)
    {
        return this.SetValue(key, value)
            && this.SetKeyType(key, Type_Object);
    }

    /**
     * @section StringMap Overrides
     */

    /**
     * Removes an item from the object by key.
     *
     * @param key       Key of object to remove.
     * @returns         True on success, false if the value was never set.
     */
    public bool Remove(const char[] key) {
        static char meta_keys[][] = {
            JSON_META_TYPE_KEY, JSON_META_LENGTH_KEY, JSON_META_HIDDEN_KEY
        };

        // create a new char[] which will fit the longest meta-key
        int meta_key_size = strlen(key) + 8;
        char[] meta_key = new char[meta_key_size];

        bool success = true;
        for (int i = 0; i < sizeof(meta_keys); ++i) {
            FormatEx(meta_key, meta_key_size, "%s%s", key, meta_keys[i]);

            if (this.HasKey(meta_key)) {
                success = success && this.parent.Remove(meta_key);
            }
        }

        return success && this.parent.Remove(key);
    }

    /**
     * @section Constructor and json.inc Aliases
     */

    /**
     * Creates a new JSON_Object.
     *
     * @returns     A new JSON_Object.
     */
    public JSON_Object()
    {
        return view_as<JSON_Object>(CreateTrie());
    }

    /**
     * Calls the json_encode method, replacing the 'obj' parameter with this instance.
     *
     * @see json_encode
     */
    public void Encode(char[] output, int max_size, bool pretty_print = false, int depth = 0)
    {
        json_encode(this, output, max_size, pretty_print, depth);
    }

    /**
     * Calls the json_decode method, replacing the 'existing' parameter with this instance.
     *
     * @see json_decode
     */
    public void Decode(const char[] buffer)
    {
        json_decode(buffer, this);
    }

    /**
     * Calls the json_merge method, replacing the 'to' parameter with this instance.
     *
     * @see json_merge
     */
    public bool Merge(JSON_Object from, bool replace = true, bool autocleanup = false)
    {
        return json_merge(this, from, replace, autocleanup);
    }

    /**
     * Calls the json_copy_shallow method, replacing the 'obj' parameter with this instance.
     *
     * @see json_copy_shallow
     */
    public JSON_Object ShallowCopy()
    {
        return json_copy_shallow(this);
    }

    /**
     * Calls the json_copy_deep method, replacing the 'obj' parameter with this instance.
     *
     * @see json_copy_deep
     */
    public JSON_Object DeepCopy()
    {
        return json_copy_deep(this);
    }

    /**
     * Calls the json_cleanup method, replacing the 'obj' parameter with this instance.
     *
     * @see json_cleanup
     */
    public void Cleanup()
    {
        json_cleanup(this);
    }
};
