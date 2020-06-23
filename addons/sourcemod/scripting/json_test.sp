/**
 * vim: set ts=4 :
 * =============================================================================
 * sm-json
 * A pure SourcePawn JSON encoder/decoder.
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

#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <testsuite>
#include <json>

public Plugin myinfo = {
    name = "JSON Tester",
    author = "clug",
    description = "Tests dumping and loading JSON objects.",
    version = "3.0.0",
    url = "https://github.com/clugg/sm-json"
};

/**
 * @section Globals
 */

char json_encode_output[1024];

/**
 * @section Helpers
 */

/**
 * Encodes a JSON_Object to a hardcoded output.
 */
void _json_encode(JSON_Object obj, int options = JSON_NONE)
{
    obj.Encode(json_encode_output, sizeof(json_encode_output), options);
}

/**
 * Encodes a JSON_Object and prints it to the test output.
 */
void print_json(JSON_Object obj, int options = JSON_NONE)
{
    _json_encode(obj, options);
    Test_Output("%s", json_encode_output);
}

/**
 * Removes the specified index from the array and confirms
 * that the removed value no longer exists.
 */
bool check_array_remove(JSON_Array arr, int index)
{
    Test_Output("Removing element at index %d", index);

    // get current value at index
    JSONCellType type = arr.GetKeyType(index);
    int str_size = 0;
    if (type == JSON_Type_String) {
        str_size = arr.GetKeyLength(index) + 1;
    }

    any value;
    char[] str = new char[str_size];

    if (type == JSON_Type_String) {
        arr.GetString(index, str, str_size);
    } else {
        arr.GetValue(index, value);
    }

    // remove the index from the array
    arr.Remove(index);
    print_json(arr);

    // confirm that it is gone
    int found = -1;
    if (type == JSON_Type_String) {
        found = arr.IndexOfString(str);
    } else {
        found = arr.IndexOf(value);
    }

    if (found != -1) {
        LogError(
            "json_test: found removed value in array at position %d",
            found
        );

        return false;
    }

    return true;
}

/**
 * @section Methodmaps
 */

methodmap Weapon < JSON_Object
{
    public Weapon()
    {
        return view_as<Weapon>(new JSON_Object());
    }

    property int id
    {
        public get()
        {
            return this.GetInt("id");
        }

        public set(int value)
        {
            this.SetInt("id", value);
        }
    }

    public bool GetString(char[] buffer, int max_size)
    {
        return this.GetString("name", buffer, max_size);
    }

    public void SetName(const char[] value)
    {
        this.SetString("name", value);
    }
}

methodmap Player < JSON_Object
{
    public Player()
    {
        return view_as<Player>(new JSON_Object());
    }

    property int id
    {
        public get()
        {
            return this.GetInt("id");
        }

        public set(int value)
        {
            this.SetInt("id", value);
        }
    }

    property Weapon weapon
    {
        public get()
        {
            return view_as<Weapon>(this.GetObject("weapon"));
        }

        public set(Weapon value)
        {
            this.SetObject("weapon", view_as<JSON_Object>(value));
        }
    }
}

/**
 * @section Tests
 **/

void it_should_decode(char[] data)
{
    JSON_Object obj = json_decode(data);
    if (Test_AssertNotNull("obj", obj)) {
        json_cleanup_and_delete(obj);
    }
}

void it_should_not_decode(char[] data)
{
    JSON_Object obj = json_decode(data);
    if (! Test_AssertNull("obj", obj)) {
        _json_encode(obj);
        LogError(
            "json_test: malformed JSON was parsed as valid: %s",
            json_encode_output
        );

        json_cleanup_and_delete(obj);
    }
}

void it_should_encode_empty_objects()
{
    JSON_Object obj = new JSON_Object();
    _json_encode(obj);
    json_cleanup_and_delete(obj);

    Test_AssertStringsEqual("output", json_encode_output, "{}");
}

void it_should_encode_empty_arrays()
{
    JSON_Array arr = new JSON_Array();
    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("output", json_encode_output, "[]");
}

void it_should_support_objects()
{
    JSON_Object obj = new JSON_Object();
    Test_Assert("set string", obj.SetString("str", "leet"));
    Test_Assert("set escaped string", obj.SetString("escaped_str", "\"leet\""));
    Test_Assert("set int", obj.SetInt("int", 9001));
    Test_Assert("set negative int", obj.SetInt("negative_int", -9001));
    Test_Assert("set int zero", obj.SetInt("int_zero", 0));
    Test_Assert("set float", obj.SetFloat("float", 13.37));
    Test_Assert("set negative float", obj.SetFloat("negative_float", -13.37));
    Test_Assert("set float zero", obj.SetFloat("float_zero", 0.0));
    Test_Assert("set true", obj.SetBool("true", true));
    Test_Assert("set false", obj.SetBool("false", false));
    Test_Assert("set handle", obj.SetObject("handle", null));

    print_json(obj);
    json_cleanup_and_delete(obj);

    JSON_Object decoded = json_decode(json_encode_output);
    if (! Test_AssertNotNull("decoded", decoded)) {
        // if this assertion fails, testing cannot continue
        return;
    }

    char string[32];
    Test_Assert("get decoded string", decoded.GetString("str", string, sizeof(string)));
    Test_AssertStringsEqual("decoded string", string, "leet");
    Test_Assert("get decoded escaped string", decoded.GetString("escaped_str", string, sizeof(string)));
    Test_AssertStringsEqual("decoded escaped string", string, "\"leet\"");
    Test_AssertEqual("decoded int", decoded.GetInt("int"), 9001);
    Test_AssertEqual("decoded negative int", decoded.GetInt("negative_int"), -9001);
    Test_AssertEqual("decoded int zero", decoded.GetInt("int_zero"), 0);
    Test_AssertFloatsEqual("decoded float", decoded.GetFloat("float"), 13.37);
    Test_AssertFloatsEqual("decoded negative float", decoded.GetFloat("negative_float"), -13.37);
    Test_AssertFloatsEqual("decoded float zero", decoded.GetFloat("float_zero"), 0.0);
    Test_AssertTrue("decoded true", decoded.GetBool("true"));
    Test_AssertFalse("decoded false", decoded.GetBool("false"));
    Test_AssertNull("decoded handle", decoded.GetObject("handle"));

    json_cleanup_and_delete(decoded);
}

void it_should_support_arrays()
{
    JSON_Array arr = new JSON_Array();
    Test_AssertNotEqual("push string", arr.PushString("leet"), -1);
    Test_AssertNotEqual("push escaped string", arr.PushString("\"leet\""), -1);
    Test_AssertNotEqual("push int", arr.PushInt(9001), -1);
    Test_AssertNotEqual("push negative int", arr.PushInt(-9001), -1);
    Test_AssertNotEqual("push int zero", arr.PushInt(0), -1);
    Test_AssertNotEqual("push float", arr.PushFloat(13.37), -1);
    Test_AssertNotEqual("push negative float", arr.PushFloat(-13.37), -1);
    Test_AssertNotEqual("push float zero", arr.PushFloat(0.0), -1);
    Test_AssertNotEqual("push true", arr.PushBool(true), -1);
    Test_AssertNotEqual("push false", arr.PushBool(false), -1);
    Test_AssertNotEqual("push handle", arr.PushObject(null), -1);

    print_json(arr);
    json_cleanup_and_delete(arr);

    JSON_Array decoded = view_as<JSON_Array>(
        json_decode(json_encode_output)
    );
    if (! Test_AssertNotNull("decoded", decoded)) {
        // if this assertion fails, testing cannot continue
        return;
    }

    int index = 0;
    char string[32];
    Test_Assert("get decoded string", decoded.GetString(index++, string, sizeof(string)));
    Test_AssertStringsEqual("decoded string", string, "leet");
    Test_Assert("get decoded escaped string",decoded.GetString(index++, string, sizeof(string)));
    Test_AssertStringsEqual("decoded escaped string", string, "\"leet\"");
    Test_AssertEqual("decoded int", decoded.GetInt(index++), 9001);
    Test_AssertEqual("decoded negative int", decoded.GetInt(index++), -9001);
    Test_AssertEqual("decoded int zero", decoded.GetInt(index++), 0);
    Test_AssertFloatsEqual("decoded float", decoded.GetFloat(index++), 13.37);
    Test_AssertFloatsEqual("decoded negative float", decoded.GetFloat(index++), -13.37);
    Test_AssertFloatsEqual("decoded float zero", decoded.GetFloat(index++), 0.0);
    Test_AssertTrue("decoded true", decoded.GetBool(index++));
    Test_AssertFalse("decoded false", decoded.GetBool(index++));
    Test_AssertNull("decoded handle", decoded.GetObject(index++));

    json_cleanup_and_delete(decoded);
}

void it_should_support_objects_nested_in_objects()
{
    JSON_Object nested_obj = new JSON_Object();
    nested_obj.SetBool("nested", true);

    JSON_Object obj = new JSON_Object();
    obj.SetBool("nested", false);
    obj.SetObject("object", nested_obj);

    print_json(obj);
    Test_AssertTrue("object.nested", obj.GetObject("object").GetBool("nested"));

    json_cleanup_and_delete(obj);
}

void it_should_support_objects_nested_in_arrays()
{
    JSON_Object nested_obj = new JSON_Object();
    nested_obj.SetBool("nested", true);

    JSON_Array arr = new JSON_Array();
    arr.PushObject(nested_obj);

    print_json(arr);
    Test_AssertTrue("0.nested", arr.GetObject(0).GetBool("nested"));

    json_cleanup_and_delete(arr);
}

void it_should_support_arrays_nested_in_objects()
{
    JSON_Array nested_arr = new JSON_Array();
    nested_arr.PushBool(true);

    JSON_Object obj = new JSON_Object();
    obj.SetObject("array", nested_arr);

    print_json(obj);
    JSON_Array obj_array = view_as<JSON_Array>(obj.GetObject("array"));
    Test_AssertTrue("array.0", obj_array.GetBool(0));

    json_cleanup_and_delete(obj);
}

void it_should_support_arrays_nested_in_arrays()
{
    JSON_Array nested_arr = new JSON_Array();
    nested_arr.PushBool(true);

    JSON_Array arr = new JSON_Array();
    arr.PushObject(nested_arr);

    print_json(arr);
    JSON_Array arr_array = view_as<JSON_Array>(arr.GetObject(0));
    Test_AssertTrue("0.0", arr_array.GetBool(0));

    json_cleanup_and_delete(arr);
}

void it_should_support_basic_methodmaps()
{
    Player player = new Player();
    player.id = 1;

    print_json(player);
    json_cleanup_and_delete(player);

    Player decoded = view_as<Player>(json_decode(json_encode_output));
    Test_AssertEqual("decoded.id", decoded.id, 1);

    json_cleanup_and_delete(decoded);
}

void it_should_support_nested_methodmaps()
{
    Weapon weapon = new Weapon();
    weapon.id = 1;
    weapon.SetName("ak47");

    Player player = new Player();
    player.id = 1;
    player.weapon = weapon;

    print_json(player);
    Test_AssertEqual("weapon.id", weapon.id, 1);
    Test_Output("changing weapon.id via player");
    player.weapon.id = 2;
    Test_AssertEqual("weapon.id", weapon.id, 2);

    json_cleanup_and_delete(player);
}

void it_should_pretty_print()
{
    JSON_Array child_arr = new JSON_Array();
    child_arr.PushInt(1);
    child_arr.PushObject(new JSON_Array());

    JSON_Object child_obj = new JSON_Object();
    child_obj.SetObject("im_indented", null);
    child_obj.SetObject("second_depth", child_arr);

    JSON_Object parent_obj = new JSON_Object();
    parent_obj.SetBool("pretty_printing", true);
    parent_obj.SetObject("first_depth", child_obj);

    _json_encode(parent_obj, JSON_ENCODE_PRETTY);
    json_cleanup_and_delete(parent_obj);

    Test_AssertStringsEqual("output", json_encode_output, "{\n    \"first_depth\": {\n        \"im_indented\": null,\n        \"second_depth\": [\n            1,\n            []\n        ]\n    },\n    \"pretty_printing\": true\n}");

    JSON_Array empty_arr = new JSON_Array();
    _json_encode(empty_arr, JSON_ENCODE_PRETTY);
    json_cleanup_and_delete(empty_arr);

    Test_AssertStringsEqual("output", json_encode_output, "[]");

    JSON_Object empty_obj = new JSON_Object();
    _json_encode(empty_obj, JSON_ENCODE_PRETTY);
    json_cleanup_and_delete(empty_obj);

    Test_AssertStringsEqual("output", json_encode_output, "{}");
}

void it_should_trim_floats()
{
    JSON_Array arr = new JSON_Array();
    arr.PushFloat(0.0);
    arr.PushFloat(1.0);
    arr.PushFloat(10.01);
    arr.PushFloat(-0.0);
    arr.PushFloat(-1.0);
    arr.PushFloat(-10.01);

    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("output", json_encode_output, "[0.0,1.0,10.01,-0.0,-1.0,-10.01]");
}

void it_should_remove_meta_keys_from_arrays()
{
    JSON_Array arr = new JSON_Array();
    arr.PushString("hello");
    arr.PushInt(0);

    Test_AssertEqual("0 type", arr.GetKeyType(0), JSON_Type_String);
    Test_AssertEqual("0 length", arr.GetKeyLength(0), 5);
    Test_AssertEqual("1 type", arr.GetKeyType(1), JSON_Type_Int);

    arr.Remove(0);

    Test_AssertEqual("0 type", arr.GetKeyType(0), JSON_Type_Int);
    Test_AssertEqual("0 length", arr.GetKeyLength(0), -1);
    Test_AssertEqual("1 type", arr.GetKeyType(1), JSON_Type_Invalid);

    json_cleanup_and_delete(arr);
}

void it_should_remove_meta_keys_from_objects()
{
    JSON_Object obj = new JSON_Object();
    obj.SetString("hello", "world");
    obj.SetInt("zero", 0);

    Test_AssertEqual("hello type", obj.GetKeyType("hello"), JSON_Type_String);
    Test_AssertEqual("hello length", obj.GetKeyLength("hello"), 5);
    Test_AssertEqual("zero type", obj.GetKeyType("zero"), JSON_Type_Int);

    obj.Remove("hello");

    Test_AssertEqual("hello type", obj.GetKeyType("hello"), JSON_Type_Invalid);
    Test_AssertEqual("hello length", obj.GetKeyLength("hello"), -1);
    Test_AssertEqual("zero type", obj.GetKeyType("zero"), JSON_Type_Int);

    json_cleanup_and_delete(obj);
}

void it_should_shift_array_down_after_removed_index()
{
    JSON_Array arr = new JSON_Array();
    arr.PushString("leet");
    arr.PushString("\"leet\"");
    arr.PushInt(9001);
    arr.PushFloat(-13.37);
    arr.PushBool(true);
    arr.PushBool(false);

    print_json(arr);

    Test_Assert("remove first element", check_array_remove(arr, 0));
    Test_Assert("remove last element", check_array_remove(arr, arr.Length - 1));
    int max = arr.Length - 1;
    Test_Assert("remove random element", check_array_remove(arr, GetRandomInt(0, max)));
    Test_AssertEqual("array length", arr.Length, max);

    json_cleanup_and_delete(arr);
}

void it_should_not_merge_array_onto_object()
{
    JSON_Object obj = new JSON_Object();
    JSON_Array arr = new JSON_Array();

    Test_Assert("merge failed", obj.Merge(arr) == false);

    json_cleanup_and_delete(obj);
    json_cleanup_and_delete(arr);
}

void it_should_not_merge_object_onto_array()
{
    JSON_Array arr = new JSON_Array();
    JSON_Object obj = new JSON_Object();

    Test_Assert("merge failed", arr.Merge(obj) == false);

    json_cleanup_and_delete(arr);
    json_cleanup_and_delete(obj);
}

void it_should_merge_arrays()
{
    JSON_Array arr1 = new JSON_Array();
    arr1.PushInt(1);
    arr1.PushBool(true);
    arr1.SetKeyHidden(1, true);

    JSON_Array arr2 = new JSON_Array();
    arr2.PushInt(2);
    arr2.PushBool(false);
    arr2.SetKeyHidden(1, true);

    if (! Test_Assert("merged successfully", arr1.Merge(arr2))) {
        // if this assertion fails, testing cannot continue
        return;
    }

    print_json(arr1);

    Test_AssertEqual("merged length", arr1.Length, 4);
    Test_AssertEqual("merged 2 type", arr1.GetKeyType(2), JSON_Type_Int);
    Test_AssertEqual("merged 2", arr1.GetInt(2), 2);
    Test_AssertEqual("merged 3 type", arr1.GetKeyType(3), JSON_Type_Bool);
    Test_AssertEqual("merged 3", arr1.GetBool(3), false);
    Test_AssertTrue("merged 1 hidden", arr1.GetKeyHidden(1));
    Test_AssertTrue("merged 3 hidden", arr1.GetKeyHidden(3));

    json_cleanup_and_delete(arr1);
    json_cleanup_and_delete(arr2);
}

void it_should_merge_objects_with_replacement()
{
    JSON_Object obj1 = new JSON_Object();
    obj1.SetInt("key1", 1);
    obj1.SetBool("replaced", false);
    obj1.SetKeyHidden("replaced", false);

    JSON_Object obj2 = new JSON_Object();
    obj2.SetInt("key2", 2);
    obj2.SetBool("replaced", true);
    obj2.SetKeyHidden("replaced", true);

    if (! Test_Assert("merged successfully", obj1.Merge(obj2))) {
        // if this assertion fails, testing cannot continue
        return;
    }

    print_json(obj1);

    Test_Assert("merged has key2", obj1.HasKey("key2"));
    Test_AssertEqual("merged key2 type", obj1.GetKeyType("key2"), JSON_Type_Int);
    Test_AssertEqual("merged key2", obj1.GetInt("key2"), 2);
    Test_AssertEqual("merged replaced type", obj1.GetKeyType("replaced"), JSON_Type_Bool);
    Test_AssertTrue("merged replaced", obj1.GetBool("replaced"));
    Test_AssertTrue("merged replaced hidden", obj1.GetKeyHidden("replaced"));

    json_cleanup_and_delete(obj1);
    json_cleanup_and_delete(obj2);
}

void it_should_merge_objects_without_replacement()
{
    JSON_Object obj1 = new JSON_Object();
    obj1.SetInt("key1", 1);
    obj1.SetBool("replaced", false);
    obj1.SetKeyHidden("replaced", false);

    JSON_Object obj2 = new JSON_Object();
    obj2.SetInt("key2", 2);
    obj2.SetBool("replaced", true);
    obj2.SetKeyHidden("replaced", true);

    if (! Test_Assert("merged successfully", obj1.Merge(obj2, JSON_NONE))) {
        // if this assertion fails, testing cannot continue
        return;
    }

    print_json(obj1);

    Test_Assert("merged has key2", obj1.HasKey("key2"));
    Test_AssertEqual("merged key2 type", obj1.GetKeyType("key2"), JSON_Type_Int);
    Test_AssertEqual("merged key2", obj1.GetInt("key2"), 2);
    Test_AssertEqual("merged replaced type", obj1.GetKeyType("replaced"), JSON_Type_Bool);
    Test_AssertFalse("merged replaced", obj1.GetBool("replaced"));
    Test_AssertFalse("merged replaced hidden", obj1.GetKeyHidden("replaced"));

    json_cleanup_and_delete(obj1);
    json_cleanup_and_delete(obj2);
}

void it_should_copy_flat_arrays()
{
    JSON_Array arr = new JSON_Array();
    arr.PushInt(1);
    arr.PushInt(2);
    arr.PushInt(3);

    JSON_Array copy = arr.DeepCopy();
    Test_AssertEqual("copy length", copy.Length, arr.Length);
    Test_AssertEqual("copy 0", copy.GetInt(0), 1);
    Test_AssertEqual("copy 1", copy.GetInt(1), 2);
    Test_AssertEqual("copy 2", copy.GetInt(2), 3);

    arr.PushInt(4);

    Test_AssertNotEqual("copy length", copy.Length, arr.Length);

    json_cleanup_and_delete(arr);
    json_cleanup_and_delete(copy);
}

void it_should_copy_flat_objects()
{
    JSON_Object obj = new JSON_Object();
    obj.SetInt("key1", 1);
    obj.SetInt("key2", 2);
    obj.SetInt("key3", 3);

    JSON_Object copy = obj.DeepCopy();
    Test_AssertEqual("copy length", copy.Length, obj.Length);
    Test_AssertEqual("copy key1", copy.GetInt("key1"), 1);
    Test_AssertEqual("copy key2", copy.GetInt("key2"), 2);
    Test_AssertEqual("copy key3", copy.GetInt("key3"), 3);

    obj.SetInt("key4", 4);

    Test_AssertNotEqual("copy length", copy.Length, obj.Length);

    json_cleanup_and_delete(obj);
    json_cleanup_and_delete(copy);
}

void it_should_shallow_copy_arrays()
{
    JSON_Array arr = new JSON_Array();
    arr.PushObject(new JSON_Array());

    JSON_Array copy = arr.ShallowCopy();

    Test_AssertEqual("copy length", copy.Length, arr.Length);
    Test_Assert("copy 0 == arr 0", copy.GetObject(0) == arr.GetObject(0));

    json_cleanup_and_delete(arr);
    copy.Remove(0);
    json_cleanup_and_delete(copy);
}

void it_should_shallow_copy_objects()
{
    JSON_Object obj = new JSON_Object();
    obj.SetObject("nested", new JSON_Object());

    JSON_Object copy = obj.ShallowCopy();

    Test_AssertEqual("copy length", copy.Length, obj.Length);
    Test_Assert("copy nested == obj nested", copy.GetObject("nested") == obj.GetObject("nested"));

    json_cleanup_and_delete(obj);
    copy.Remove("nested");
    json_cleanup_and_delete(copy);
}

void it_should_deep_copy_arrays()
{
    JSON_Array arr = new JSON_Array();
    arr.PushObject(new JSON_Array());

    JSON_Array copy = arr.DeepCopy();

    Test_AssertEqual("copy length", copy.Length, arr.Length);
    Test_Assert("copy 0 != arr 0", copy.GetObject(0) != arr.GetObject(0));

    json_cleanup_and_delete(arr);
    json_cleanup_and_delete(copy);
}

void it_should_deep_copy_objects()
{
    JSON_Object obj = new JSON_Object();
    obj.SetObject("nested", new JSON_Object());

    JSON_Object copy = obj.DeepCopy();

    Test_AssertEqual("copy length", copy.Length, obj.Length);
    Test_Assert("copy nested != obj nested", copy.GetObject("nested") != obj.GetObject("nested"));

    json_cleanup_and_delete(obj);
    json_cleanup_and_delete(copy);
}

void it_should_allow_single_quotes()
{
    // array
    JSON_Array arr = view_as<JSON_Array>(
        json_decode(
            "['single quotes', \"double quotes\", 'single \\'single\\' quotes', 'single \"double\" quotes', \"double 'single' quotes\", \"double \\\"double\\\" quotes\"]",
            JSON_DECODE_SINGLE_QUOTES
        )
    );

    if (Test_AssertNotNull("array", arr)) {
        print_json(arr);
        Test_AssertEqual("array length", arr.Length, 6);

        json_cleanup_and_delete(arr);
    }

    // object
    JSON_Object obj = json_decode(
        "{'key': \"value\"}",
        JSON_DECODE_SINGLE_QUOTES
    );

    if (Test_AssertNotNull("object", obj)) {
        print_json(obj);
        Test_Assert("object has key", obj.HasKey("key"));

        json_cleanup_and_delete(obj);
    }
}

void it_should_return_default_values_for_missing_elements()
{
    // array
    JSON_Array arr = new JSON_Array();

    Test_AssertEqual("array default int value", arr.GetInt(0, 1), 1);
    Test_AssertFloatsEqual("array default float value", arr.GetFloat(0, 1.0), 1.0);
    Test_AssertTrue("array default bool value", arr.GetBool(0, true));
    Test_AssertNull("array default null value", arr.GetObject(0, null));
    Test_AssertEqual("array default arr value", arr.GetObject(0, arr), arr);

    json_cleanup_and_delete(arr);

    // object
    JSON_Object obj = new JSON_Object();

    Test_AssertEqual("object default int value", obj.GetInt("_", 1), 1);
    Test_AssertFloatsEqual("object default float value", obj.GetFloat("_", 1.0), 1.0);
    Test_AssertTrue("object default bool value", obj.GetBool("_", true));
    Test_AssertNull("object default null value", obj.GetObject("_", null));
    Test_AssertEqual("object default obj value", obj.GetObject("_", obj), obj);

    json_cleanup_and_delete(obj);
}

void it_should_autocleanup_merged_objects()
{
    JSON_Object obj1 = new JSON_Object();
    JSON_Object obj2 = new JSON_Object();
    JSON_Object nested1 = new JSON_Object();
    JSON_Object nested2 = new JSON_Object();

    Test_Output("merging without replacement");
    obj1.SetObject("nested", nested1);
    obj2.SetObject("nested", nested2);
    obj1.Merge(obj2, JSON_MERGE_CLEANUP);

    Test_Assert("nested1 is valid", IsValidHandle(nested1));
    Test_Assert("nested2 is valid", IsValidHandle(nested2));

    Test_Output("merging with replacement and without cleanup");
    obj1.SetObject("nested", nested1);
    obj2.SetObject("nested", nested2);
    obj1.Merge(obj2, JSON_MERGE_REPLACE);

    Test_Assert("nested1 is valid", IsValidHandle(nested1));
    Test_Assert("nested2 is valid", IsValidHandle(nested2));

    Test_Output("merging with replacement and with cleanup");
    obj1.SetObject("nested", nested1);
    obj2.SetObject("nested", nested2);
    obj1.Merge(obj2, JSON_MERGE_REPLACE | JSON_MERGE_CLEANUP);

    Test_Assert("nested1 is invalid", ! IsValidHandle(nested1));
    Test_Assert("nested2 is valid", IsValidHandle(nested2));

    json_cleanup_and_delete(obj1);
    obj2.Remove("nested");
    json_cleanup_and_delete(obj2);
}

void it_should_enforce_types_in_arrays()
{
    JSON_Array arr = new JSON_Array(JSON_Type_Int);

    Test_AssertNotEqual("push int", arr.PushInt(9001), -1);
    Test_AssertNotEqual("push negative int", arr.PushInt(-9001), -1);
    Test_AssertEqual("push string", arr.PushString("leet"), -1);
    Test_AssertEqual("push float", arr.PushFloat(13.37), -1);
    Test_AssertEqual("push bool", arr.PushBool(true), -1);
    Test_AssertEqual("push null", arr.PushObject(null), -1);

    json_cleanup_and_delete(arr);
}

void it_should_not_set_type_on_inconsistent_array()
{
    JSON_Array arr = new JSON_Array();
    arr.PushObject(null);

    Test_Assert("SetType failed", ! arr.SetType(JSON_Type_Int));

    json_cleanup_and_delete(arr);
}

void it_should_import_ints()
{
    int ints[] = {1, 2, 3};
    JSON_Array arr = new JSON_Array();
    Test_Assert("imported successfully", arr.ImportValues(JSON_Type_Int, ints, sizeof(ints)));

    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("output", json_encode_output, "[1,2,3]");
}

void it_should_import_floats()
{
    float floats[] = {1.1, 2.2, 3.3};
    JSON_Array arr = new JSON_Array();
    Test_Assert("imported successfully", arr.ImportValues(JSON_Type_Float, floats, sizeof(floats)));

    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("output", json_encode_output, "[1.1,2.2,3.3]");
}

void it_should_import_bools()
{
    bool bools[] = {true, false};
    JSON_Array arr = new JSON_Array();
    Test_Assert("imported successfully", arr.ImportValues(JSON_Type_Bool, bools, sizeof(bools)));

    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("output", json_encode_output, "[true,false]");
}

void it_should_import_strings()
{
    char strings[][] = {"hello", "world"};
    JSON_Array arr = new JSON_Array();
    Test_Assert("imported successfully", arr.ImportStrings(strings, sizeof(strings)));

    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("output", json_encode_output, "[\"hello\",\"world\"]");
}

void it_should_export_ints()
{
    JSON_Array arr = view_as<JSON_Array>(json_decode("[1,2,3]"));
    int size = arr.Length;
    int[] values = new int[size];
    arr.ExportValues(values, size);

    print_json(arr);
    json_cleanup_and_delete(arr);

    Test_AssertEqual("values[0]", values[0], 1);
    Test_AssertEqual("values[1]", values[1], 2);
    Test_AssertEqual("values[2]", values[2], 3);
}

void it_should_export_floats()
{
    JSON_Array arr = view_as<JSON_Array>(json_decode("[1.1,2.2,3.3]"));
    int size = arr.Length;
    float[] values = new float[size];
    arr.ExportValues(values, size);

    print_json(arr);
    json_cleanup_and_delete(arr);

    Test_AssertFloatsEqual("values[0]", values[0], 1.1);
    Test_AssertFloatsEqual("values[1]", values[1], 2.2);
    Test_AssertFloatsEqual("values[2]", values[2], 3.3);
}

void it_should_export_bools()
{
    JSON_Array arr = view_as<JSON_Array>(json_decode("[true, false]"));
    int size = arr.Length;
    bool[] values = new bool[size];
    arr.ExportValues(values, size);

    print_json(arr);
    json_cleanup_and_delete(arr);

    Test_AssertTrue("values[0]", values[0]);
    Test_AssertFalse("values[1]", values[1]);
}

void it_should_export_strings()
{
    JSON_Array arr = view_as<JSON_Array>(json_decode("[\"hello\",\"world\"]"));
    int size = arr.Length;
    int str_length = arr.MaxStringLength + 1;
    char[][] values = new char[size][str_length];
    arr.ExportStrings(values, size, str_length);

    print_json(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("values[0]", values[0], "hello");
    Test_AssertStringsEqual("values[1]", values[1], "world");
}

void it_should_support_unicode()
{
    JSON_Array arr = view_as<JSON_Array>(json_decode("[\"\\u0073tarts with\",\"ends wit\\u0068\",\"\\u0021\",\"\\u00a1\",\"\\u2122\",\"\\u2b50\"]"));

    char expected_values[][] = {
        "starts with",
        "ends with",
        "!", // 1 byte
        "¡", // 2 bytes
        "™", // 3 bytes
        "⭐" // 4 bytes
    };

    int length = arr.Length;
    char element_name[8];
    for (int i = 0; i < length; i += 1) {
        FormatEx(element_name, sizeof(element_name), "arr[%d]", i);

        int element_length = arr.GetKeyLength(i) + 1;
        char[] element = new char[element_length];
        arr.GetString(i, element, element_length);
        Test_AssertStringsEqual(element_name, element, expected_values[i]);
    }

    _json_encode(arr);
    json_cleanup_and_delete(arr);

    Test_AssertStringsEqual("encoded", json_encode_output, "[\"starts with\",\"ends with\",\"!\",\"\\u00a1\",\"\\u2122\",\"\\u2b50\"]");
}

public void OnPluginStart()
{
    Test_SetBoxWidth(56);
    Test_StartSection("sm-json test suite");

    Test_Run("it_should_encode_empty_objects", it_should_encode_empty_objects);
    Test_Run("it_should_encode_empty_arrays", it_should_encode_empty_arrays);
    Test_Run("it_should_support_objects", it_should_support_objects);
    Test_Run("it_should_support_arrays", it_should_support_arrays);
    Test_Run("it_should_support_objects_nested_in_objects", it_should_support_objects_nested_in_objects);
    Test_Run("it_should_support_objects_nested_in_arrays", it_should_support_objects_nested_in_arrays);
    Test_Run("it_should_support_arrays_nested_in_objects", it_should_support_arrays_nested_in_objects);
    Test_Run("it_should_support_arrays_nested_in_arrays", it_should_support_arrays_nested_in_arrays);
    Test_Run("it_should_support_basic_methodmaps", it_should_support_basic_methodmaps);
    Test_Run("it_should_support_nested_methodmaps", it_should_support_nested_methodmaps);

    // the following tests were acquired from https://www.json.org/JSON_checker/
    // a few additional tests have been added for completeness

    char should_decode[][] = {
        "[\n    \"JSON Test Pattern pass1\",\n    {\"object with 1 member\":[\"array with 1 element\"]},\n    {},\n    [],\n    -42,\n    true,\n    false,\n    null,\n    {\n        \"integer\": 1234567890,\n        \"real\": -9876.543210,\n        \"e\": 0.123456789e-12,\n        \"E\": 1.234567890E+34,\n        \"\":  23456789012E66,\n        \"zero\": 0,\n        \"one\": 1,\n        \"space\": \" \",\n        \"quote\": \"\\\"\",\n        \"backslash\": \"\\\\\",\n        \"controls\": \"\\b\\f\\n\\r\\t\",\n        \"slash\": \"/ & \\/\",\n        \"alpha\": \"abcdefghijklmnopqrstuvwyz\",\n        \"ALPHA\": \"ABCDEFGHIJKLMNOPQRSTUVWYZ\",\n        \"digit\": \"0123456789\",\n        \"0123456789\": \"digit\",\n        \"special\": \"`1~!@#$%^&*()_+-={':[,]}|;.</>?\",\n        \"hex\": \"\\u0123\\u4567\\u89AB\\uCDEF\\uabcd\\uef4A\",\n        \"true\": true,\n        \"false\": false,\n        \"null\": null,\n        \"array\":[  ],\n        \"object\":{  },\n        \"address\": \"50 St. James Street\",\n        \"url\": \"http://www.JSON.org/\",\n        \"comment\": \"// /* <!-- --\",\n        \"# -- --> */\": \" \",\n        \" s p a c e d \" :[1,2 , 3\n\n,\n\n4 , 5        ,          6           ,7        ],\"compact\":[1,2,3,4,5,6,7],\n        \"jsontext\": \"{\\\"object with 1 member\\\":[\\\"array with 1 element\\\"]}\",\n        \"quotes\": \"&#34; \\u0022 %22 0x22 034 &#x22;\",\n        \"\\/\\\\\\\"\\uCAFE\\uBABE\\uAB98\\uFCDE\\ubcda\\uef4A\\b\\f\\n\\r\\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?\"\n: \"A key can be any string\"\n    },\n    0.5 ,98.6\n,\n99.44\n,\n\n1066,\n1e1,\n0.1e1,\n1e-1,\n1e00,2e+00,2e-00\n,\"rosebud\"]",
        "[[[[[[[[[[[[[[[[[[[\"Not too deep\"]]]]]]]]]]]]]]]]]]]",
        "{\n    \"JSON Test Pattern pass3\": {\n        \"The outermost value\": \"must be an object or array.\",\n        \"In this test\": \"It is an object.\"\n    }\n}\n",
        "[\"unicode not high surrogate \\uD7FF\"]"
    };
    for (int i = 0; i < sizeof(should_decode); i += 1) {
        Test_BeforeRun("it_should_decode");
        Test_Output("index %d", i);
        it_should_decode(should_decode[i]);
        Test_AfterRun();
    }

    char should_not_decode[][] = {
        "\"A JSON payload should be an object or array, not a string.\"",
        "-1", "-0", "0", "1",
        "-1.1", "-0.0", "0.0", "1.1",
        "true", "false",
        "null",
        "{\"Extra value after close\": true} \"misplaced quoted value\"",
        "{\"Illegal expression\": 1 + 2}",
        "{\"Illegal invocation\": alert()}",
        "{\"Numbers cannot have leading zeroes\": 013}",
        "{\"Numbers cannot be hex\": 0x14}",
        "[\"Illegal backslash escape: \\x15\"]",
        "[\\naked]",
        "[\"Illegal backslash escape: \\017\"]",
        "{\"Missing colon\" null}",
        "[\"Unclosed array\"",
        "{\"Double colon\":: null}",
        "{\"Comma instead of colon\", null}",
        "[\"Colon instead of comma\": false]",
        "[\"Bad value\", truth]",
        "['single quote']",
        "[\"\ttab\tcharacter\tin\tstring\t\"]",
        "[\"tab\\	character\\	in\\	string\\	\"]",
        "[\"line\nbreak\"]",
        "[\"line\\\nbreak\"]",
        "[0e]",
        "{unquoted_key: \"keys must be quoted\"}",
        "[0e+]",
        "[0e+-1]",
        "{\"Comma instead if closing brace\": true,",
        "[\"mismatch\"}",
        "[\"extra comma\",]",
        "[\"double extra comma\",,]",
        "[   , \"<-- missing value\"]",
        "[\"Comma after the close\"],",
        "[\"Extra close\"]]",
        "{\"Extra comma\": true,}",
        "[\"unicode too short \\uABC\"]",
        "[\"unicode not hex \\uFFFZ\"]",
        "[\"unicode high surrogate \\uD800\"]",
        "[\"unicode high surrogate \\uDFFF\"]"
    };
    for (int i = 0; i < sizeof(should_not_decode); i += 1) {
        Test_BeforeRun("it_should_not_decode");
        Test_Output("index %d", i);
        it_should_not_decode(should_not_decode[i]);
        Test_AfterRun();
    }

    Test_Run("it_should_pretty_print", it_should_pretty_print);
    Test_Run("it_should_trim_floats", it_should_trim_floats);
    Test_Run("it_should_remove_meta_keys_from_arrays", it_should_remove_meta_keys_from_arrays);
    Test_Run("it_should_remove_meta_keys_from_objects", it_should_remove_meta_keys_from_objects);
    Test_Run("it_should_shift_array_down_after_removed_index", it_should_shift_array_down_after_removed_index);
    Test_Run("it_should_not_merge_array_onto_object", it_should_not_merge_array_onto_object);
    Test_Run("it_should_not_merge_object_onto_array", it_should_not_merge_object_onto_array);
    Test_Run("it_should_merge_arrays", it_should_merge_arrays);
    Test_Run("it_should_merge_objects_with_replacement", it_should_merge_objects_with_replacement);
    Test_Run("it_should_merge_objects_without_replacement", it_should_merge_objects_without_replacement);
    Test_Run("it_should_copy_flat_arrays", it_should_copy_flat_arrays);
    Test_Run("it_should_copy_flat_objects", it_should_copy_flat_objects);
    Test_Run("it_should_shallow_copy_arrays", it_should_shallow_copy_arrays);
    Test_Run("it_should_shallow_copy_objects", it_should_shallow_copy_objects);
    Test_Run("it_should_deep_copy_arrays", it_should_deep_copy_arrays);
    Test_Run("it_should_deep_copy_objects", it_should_deep_copy_objects);
    Test_Run("it_should_allow_single_quotes", it_should_allow_single_quotes);
    Test_Run("it_should_return_default_values_for_missing_elements", it_should_return_default_values_for_missing_elements);
    Test_Run("it_should_autocleanup_merged_objects", it_should_autocleanup_merged_objects);
    Test_Run("it_should_enforce_types_in_arrays", it_should_enforce_types_in_arrays);
    Test_Run("it_should_not_set_type_on_inconsistent_array", it_should_not_set_type_on_inconsistent_array);
    Test_Run("it_should_import_ints", it_should_import_ints);
    Test_Run("it_should_import_floats", it_should_import_floats);
    Test_Run("it_should_import_bools", it_should_import_bools);
    Test_Run("it_should_import_strings", it_should_import_strings);
    Test_Run("it_should_export_ints", it_should_export_ints);
    Test_Run("it_should_export_floats", it_should_export_floats);
    Test_Run("it_should_export_bools", it_should_export_bools);
    Test_Run("it_should_export_strings", it_should_export_strings);
    Test_Run("it_should_support_unicode", it_should_support_unicode);

    Test_EndSection();
}
