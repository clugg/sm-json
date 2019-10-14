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

#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <json>

public Plugin myinfo = {
    name = "JSON Tester",
    author = "clug",
    description = "Tests dumping and loading JSON objects.",
    version = "2.0.0",
    url = "https://github.com/clugg/sm-json"
};

/**
 * @section Globals
 */

char json_encode_output[1024];
int passed = 0;
int failed = 0;

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
 * @section Helpers
 */

/**
 * Checks if floats are "equal enough", to account for floating point errors.
 * @see https://en.wikipedia.org/wiki/Floating_point_error_mitigation
 *
 * @param float x           First value to compare.
 * @param float y           Second value to compare.
 * @param float tolerance   Maximum allowed tolerance for floats to be considered equal.
 * @returns True if the floats are equal within the distance, false otherwise.
 */
bool equal_enough(float x, float y, float tolerance = 0.0005)
{
    float difference = x / y;
    return difference > (1 - tolerance)
        && difference < (1 + tolerance);
}

void print_json(JSON_Object obj, bool pretty = false)
{
    obj.Encode(json_encode_output, sizeof(json_encode_output), pretty);
    PrintToServer("%s", json_encode_output);
}

bool check_array_remove(JSON_Array arr, int index)
{
    PrintToServer("Removing element at index %d", index);

    // get current value at index
    JSON_CELL_TYPE type = arr.GetKeyType(index);
    int str_size = 0;
    if (type == Type_String) {
        str_size = arr.GetKeyLength(index) + 1;
    }

    any value;
    char[] str = new char[str_size];

    if (type == Type_String) {
        arr.GetString(index, str, str_size);
    } else {
        arr.GetValue(index, value);
    }

    // remove the index from the array
    arr.Remove(index);
    print_json(arr);

    // confirm that it is gone
    int found = -1;
    if (type == Type_String) {
        found = arr.IndexOfString(str);
    } else {
        found = arr.IndexOf(value);
    }

    if (found != -1) {
        LogError("json_test: found value at position %d after removing it from array", found);
    }

    return found == -1;
}

void check_test(bool result)
{
    if (result) {
        PrintToServer("\tOK");
        ++passed;
    } else {
        PrintToServer("\tFAILED");
        ++failed;
    }

    PrintToServer("");
}

/**
 * @section Tests
 **/

bool it_should_encode_empty_objects()
{
    JSON_Object obj = new JSON_Object();

    print_json(obj);
    delete obj;

    return StrEqual(json_encode_output, "{}");
}

bool it_should_encode_empty_arrays()
{
    JSON_Array arr = new JSON_Array();

    print_json(arr);
    delete arr;

    return StrEqual(json_encode_output, "[]");
}

bool it_should_support_objects()
{
    JSON_Object obj = new JSON_Object();
    bool success = obj.SetString("str", "leet")
        && obj.SetString("escaped_str", "\"leet\"")
        && obj.SetInt("int", 9001)
        && obj.SetInt("negative_int", -9001)
        && obj.SetInt("int_zero", 0)
        && obj.SetInt("negative_int_zero", -0)
        && obj.SetFloat("float", 13.37)
        && obj.SetFloat("negative_float", -13.37)
        && obj.SetFloat("float_zero", 0.0)
        && obj.SetFloat("negative_float_zero", -0.0)
        && obj.SetBool("true", true)
        && obj.SetBool("false", false)
        && obj.SetNull("handle");

    print_json(obj);
    delete obj;

    if (! success) {
        LogError("json_test: failed while setting object values");

        return false;
    }

    JSON_Object decoded_obj = json_decode(json_encode_output);
    if (decoded_obj == null) {
        LogError("json_test: unable to decode object");

        return false;
    }

    char string[32];
    any value;
    Handle hndl;

    if (! decoded_obj.GetString("str", string, sizeof(string)) || ! StrEqual(string, "leet")) {
        LogError("json_test: unexpected value for key str: %s", string);
        success = false;
    }

    if (! decoded_obj.GetString("escaped_str", string, sizeof(string)) || ! StrEqual(string, "\"leet\"")) {
        LogError("json_test: unexpected value for key escaped_str: %s", string);
        success = false;
    }

    if ((value = decoded_obj.GetInt("int")) != 9001) {
        LogError("json_test: unexpected value for key int: %d", value);
        success = false;
    }

    if ((value = decoded_obj.GetInt("negative_int")) != -9001) {
        LogError("json_test: unexpected value for key negative_int: %d", value);
        success = false;
    }

    if ((value = decoded_obj.GetInt("int_zero")) != 0) {
        LogError("json_test: unexpected value for key int_zero: %d", value);
        success = false;
    }

    if ((value = decoded_obj.GetInt("negative_int_zero")) != -0) {
        LogError("json_test: unexpected value for key negative_int_zero: %d", value);
        success = false;
    }

    if (! equal_enough((value = decoded_obj.GetFloat("float")), 13.37)) {
        LogError("json_test: unexpected value for key float: %f", value);
        success = false;
    }

    if (! equal_enough((value = decoded_obj.GetFloat("negative_float")), -13.37)) {
        LogError("json_test: unexpected value for key negative_float: %f", value);
        success = false;
    }

    if ((value = decoded_obj.GetFloat("float_zero")) != 0.0) {
        LogError("json_test: unexpected value for key float_zero: %f", value);
        success = false;
    }

    if ((value = decoded_obj.GetFloat("negative_float_zero")) != -0.0) {
        LogError("json_test: unexpected value for key negative_float_zero: %f", value);
        success = false;
    }

    if ((value = decoded_obj.GetBool("true")) != true) {
        LogError("json_test: unexpected value for key true: %d", value);
        success = false;
    }

    if ((value = decoded_obj.GetBool("false")) != false) {
        LogError("json_test: unexpected value for key false: %d", value);
        success = false;
    }

    if ((hndl = decoded_obj.GetNull("handle")) != null) {
        LogError("json_test: unexpected value for key handle: %d", view_as<int>(hndl));
        success = false;
    }

    delete decoded_obj;

    return success;
}

bool it_should_support_arrays()
{
    JSON_Array arr = new JSON_Array();
    bool success = arr.PushString("leet")
        && arr.PushString("\"leet\"")
        && arr.PushInt(9001)
        && arr.PushInt(-9001)
        && arr.PushInt(0)
        && arr.PushInt(-0)
        && arr.PushFloat(13.37)
        && arr.PushFloat(-13.37)
        && arr.PushFloat(0.0)
        && arr.PushFloat(-0.0)
        && arr.PushBool(true)
        && arr.PushBool(false)
        && arr.PushNull();

    print_json(arr);
    delete arr;

    if (! success) {
        LogError("json_test: failed while pushing array values");

        return false;
    }

    JSON_Array decoded_arr = view_as<JSON_Array>(json_decode(json_encode_output));
    if (decoded_arr == null) {
        LogError("json_test: unable to decode array");

        return false;
    }

    int index = 0;
    char string[32];
    any value;
    Handle hndl;

    if (! decoded_arr.GetString(index++, string, sizeof(string)) || ! StrEqual(string, "leet")) {
        LogError("json_test: unexpected value for index %d: %s", index, string);
        success = false;
    }

    if (! decoded_arr.GetString(index++, string, sizeof(string)) || ! StrEqual(string, "\"leet\"")) {
        LogError("json_test: unexpected value for index %d: %s", index, string);
        success = false;
    }

    if ((value = decoded_arr.GetInt(index++)) != 9001) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetInt(index++)) != -9001) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetInt(index++)) != 0) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetInt(index++)) != -0) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if (! equal_enough((value = decoded_arr.GetFloat(index++)), 13.37)) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if (! equal_enough((value = decoded_arr.GetFloat(index++)), -13.37)) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetFloat(index++)) != 0.0) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetFloat(index++)) != -0.0) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetBool(index++)) != true) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetBool(index++)) != false) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((hndl = decoded_arr.GetNull(index++)) != null) {
        LogError("json_test: unexpected value for index %d: %d", index, view_as<int>(hndl));
        success = false;
    }

    delete decoded_arr;

    return success;
}

bool it_should_reload_an_object()
{
    JSON_Object obj = new JSON_Object();
    obj.SetBool("loaded", false);
    obj.Decode("{\"reloaded\": true}");

    print_json(obj);

    bool success = obj.HasKey("loaded") && obj.GetBool("loaded") == false
        && obj.HasKey("reloaded") && obj.GetBool("reloaded") == true;

    delete obj;

    return success;
}

bool it_should_support_objects_nested_in_objects()
{
    JSON_Object nested_obj = new JSON_Object();
    nested_obj.SetBool("nested", true);

    JSON_Object obj = new JSON_Object();
    obj.SetBool("nested", false);
    obj.SetObject("object", nested_obj);

    print_json(obj);

    bool success = obj.GetObject("object").GetBool("nested");
    obj.Cleanup();
    delete obj;

    return success;
}

bool it_should_support_objects_nested_in_arrays()
{
    JSON_Object nested_obj = new JSON_Object();
    nested_obj.SetBool("nested", true);

    JSON_Array arr = new JSON_Array();
    arr.PushObject(nested_obj);

    print_json(arr);

    bool success = arr.GetObject(0).GetBool("nested");
    arr.Cleanup();
    delete arr;

    return success;
}

bool it_should_support_arrays_nested_in_objects()
{
    JSON_Array nested_arr = new JSON_Array();
    nested_arr.PushBool(true);

    JSON_Object obj = new JSON_Object();
    obj.SetObject("array", nested_arr);

    print_json(obj);

    bool success = view_as<JSON_Array>(obj.GetObject("array")).GetBool(0);
    obj.Cleanup();
    delete obj;

    return success;
}

bool it_should_support_arrays_nested_in_arrays()
{
    JSON_Array nested_arr = new JSON_Array();
    nested_arr.PushBool(true);

    JSON_Array arr = new JSON_Array();
    arr.PushObject(nested_arr);

    print_json(arr);

    bool success = view_as<JSON_Array>(arr.GetObject(0)).GetBool(0);
    arr.Cleanup();
    delete arr;

    return success;
}

bool it_should_support_basic_methodmaps()
{
    Player player = new Player();
    player.id = 1;

    player.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    delete player;

    return true;
}

bool it_should_support_nested_methodmaps()
{
    Weapon weapon = new Weapon();
    weapon.SetName("ak47");

    Player player = new Player();
    player.id = 1;
    player.weapon = weapon;
    player.weapon.id = 2;  // demonstrating nested property setters

    player.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    bool success = player.weapon.id == 2;
    player.Cleanup();
    delete player;

    return success;
}

bool it_should_decode(char[] data)
{
    JSON_Object obj = json_decode(data);
    if (obj == null) {
        return false;
    }

    print_json(obj);

    return true;
}

bool it_should_not_decode(char[] data)
{
    JSON_Object obj = json_decode(data);
    if (obj != null) {
        obj.Encode(json_encode_output, sizeof(json_encode_output));
        LogError("json_test: malformed JSON was parsed as valid: %s", json_encode_output);
        return false;
    }

    return true;
}

bool it_should_pretty_print()
{
    JSON_Array child_arr = new JSON_Array();
    child_arr.PushInt(1);
    child_arr.PushObject(new JSON_Array());

    JSON_Object child_obj = new JSON_Object();
    child_obj.SetNull("im_indented");
    child_obj.SetObject("second_depth", child_arr);

    JSON_Object parent_obj = new JSON_Object();
    parent_obj.SetBool("pretty_printing", true);
    parent_obj.SetObject("first_depth", child_obj);

    print_json(parent_obj, true);
    parent_obj.Cleanup();
    delete parent_obj;

    bool success = StrEqual(json_encode_output, "{\n    \"first_depth\": {\n        \"im_indented\": null,\n        \"second_depth\": [\n            1,\n            []\n        ]\n    },\n    \"pretty_printing\": true\n}");

    JSON_Array empty_arr = new JSON_Array();
    print_json(empty_arr, true);
    delete empty_arr;

    success = success && StrEqual(json_encode_output, "[]");

    JSON_Object empty_object = new JSON_Object();
    print_json(empty_object, true);
    delete empty_object;

    success = success && StrEqual(json_encode_output, "{}");

    return success;
}

bool it_should_trim_floats()
{
    JSON_Array arr = new JSON_Array();
    arr.PushFloat(0.0);
    arr.PushFloat(1.0);
    arr.PushFloat(10.01);
    arr.PushFloat(-0.0);
    arr.PushFloat(-1.0);
    arr.PushFloat(-10.01);

    print_json(arr);

    return StrEqual(json_encode_output, "[0.0,1.0,10.01,-0.0,-1.0,-10.01]");
}

bool it_should_remove_meta_keys_from_arrays()
{
    bool success = true;

    JSON_Array arr = new JSON_Array();
    arr.PushString("hello");
    arr.PushInt(0);

    if (! arr.parent.HasKey("0:type") || arr.GetKeyType(0) != Type_String
        || ! arr.parent.HasKey("0:length") || arr.GetKeyLength(0) != 5
        || ! arr.parent.HasKey("1:type") || arr.GetKeyType(1) != Type_Int) {
        LogError("json_test: array did not properly set meta-keys");

        success = false;
    }

    arr.Remove(0);

    if (arr.parent.HasKey("1:type") || arr.parent.HasKey("0:length") || arr.GetKeyType(0) != Type_Int) {
        LogError("json_test: array did not properly remove meta-keys");

        success = false;
    }

    delete arr;

    return success;
}

bool it_should_remove_meta_keys_from_objects()
{
    bool success = true;

    JSON_Object obj = new JSON_Object();
    obj.SetString("hello", "world");
    obj.SetInt("zero", 0);

    if (! obj.HasKey("hello:type") || obj.GetKeyType("hello") != Type_String
        || ! obj.HasKey("hello:length") || obj.GetKeyLength("hello") != 5
        || ! obj.HasKey("zero:type") || obj.GetKeyType("zero") != Type_Int) {
        LogError("json_test: object did not properly set meta-keys");

        success = false;
    }

    obj.Remove("hello");

    if (obj.HasKey("hello:type") || obj.HasKey("hello:length")) {
        LogError("json_test: object did not properly remove meta-keys");

        success = false;
    }

    if (! obj.HasKey("zero:type")) {
        LogError("json_test: object removed incorrect meta-key");

        success = false;
    }

    delete obj;

    return success;
}

bool it_should_shift_array_down_after_removed_index()
{
    JSON_Array arr = new JSON_Array();
    bool success = arr.PushString("leet")
        && arr.PushString("\"leet\"")
        && arr.PushInt(9001)
        && arr.PushFloat(-13.37)
        && arr.PushBool(true)
        && arr.PushBool(false);

    print_json(arr);

    if (! success) {
        LogError("json_test: failed while pushing array values");

        success = false;
    }

    success = success && check_array_remove(arr, 0);
    success = success && check_array_remove(arr, arr.Length - 1);
    int max = arr.Length - 1;
    success = success && check_array_remove(arr, GetRandomInt(0, max));

    if (arr.Length != max) {
        LogError("json_test: array did not properly shift down indexes");

        success = false;
    }

    delete arr;

    return success;
}


public void OnPluginStart()
{
    PrintToServer("Running tests...");
    PrintToServer("");

    PrintToServer("it_should_encode_empty_objects");
    check_test(it_should_encode_empty_objects());

    PrintToServer("it_should_encode_empty_arrays");
    check_test(it_should_encode_empty_arrays());

    PrintToServer("it_should_support_objects");
    check_test(it_should_support_objects());

    PrintToServer("it_should_support_arrays");
    check_test(it_should_support_arrays());

    PrintToServer("it_should_reload_an_object");
    check_test(it_should_reload_an_object());

    PrintToServer("it_should_support_objects_nested_in_objects");
    check_test(it_should_support_objects_nested_in_objects());

    PrintToServer("it_should_support_objects_nested_in_arrays");
    check_test(it_should_support_objects_nested_in_arrays());

    PrintToServer("it_should_support_arrays_nested_in_objects");
    check_test(it_should_support_arrays_nested_in_objects());

    PrintToServer("it_should_support_arrays_nested_in_arrays");
    check_test(it_should_support_arrays_nested_in_arrays());

    PrintToServer("it_should_support_basic_methodmaps");
    check_test(it_should_support_basic_methodmaps());

    PrintToServer("it_should_support_nested_methodmaps");
    check_test(it_should_support_nested_methodmaps());

    // the following tests were acquired from https://www.json.org/JSON_checker/
    // a few additional tests have been added for completeness

    char should_decode[][] = {
        "[\n    \"JSON Test Pattern pass1\",\n    {\"object with 1 member\":[\"array with 1 element\"]},\n    {},\n    [],\n    -42,\n    true,\n    false,\n    null,\n    {\n        \"integer\": 1234567890,\n        \"real\": -9876.543210,\n        \"e\": 0.123456789e-12,\n        \"E\": 1.234567890E+34,\n        \"\":  23456789012E66,\n        \"zero\": 0,\n        \"one\": 1,\n        \"space\": \" \",\n        \"quote\": \"\\\"\",\n        \"backslash\": \"\\\\\",\n        \"controls\": \"\\b\\f\\n\\r\\t\",\n        \"slash\": \"/ & \\/\",\n        \"alpha\": \"abcdefghijklmnopqrstuvwyz\",\n        \"ALPHA\": \"ABCDEFGHIJKLMNOPQRSTUVWYZ\",\n        \"digit\": \"0123456789\",\n        \"0123456789\": \"digit\",\n        \"special\": \"`1~!@#$%^&*()_+-={':[,]}|;.</>?\",\n        \"hex\": \"\\u0123\\u4567\\u89AB\\uCDEF\\uabcd\\uef4A\",\n        \"true\": true,\n        \"false\": false,\n        \"null\": null,\n        \"array\":[  ],\n        \"object\":{  },\n        \"address\": \"50 St. James Street\",\n        \"url\": \"http://www.JSON.org/\",\n        \"comment\": \"// /* <!-- --\",\n        \"# -- --> */\": \" \",\n        \" s p a c e d \" :[1,2 , 3\n\n,\n\n4 , 5        ,          6           ,7        ],\"compact\":[1,2,3,4,5,6,7],\n        \"jsontext\": \"{\\\"object with 1 member\\\":[\\\"array with 1 element\\\"]}\",\n        \"quotes\": \"&#34; \\u0022 %22 0x22 034 &#x22;\",\n        \"\\/\\\\\\\"\\uCAFE\\uBABE\\uAB98\\uFCDE\\ubcda\\uef4A\\b\\f\\n\\r\\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?\"\n: \"A key can be any string\"\n    },\n    0.5 ,98.6\n,\n99.44\n,\n\n1066,\n1e1,\n0.1e1,\n1e-1,\n1e00,2e+00,2e-00\n,\"rosebud\"]",
        "[[[[[[[[[[[[[[[[[[[\"Not too deep\"]]]]]]]]]]]]]]]]]]]",
        "{\n    \"JSON Test Pattern pass3\": {\n        \"The outermost value\": \"must be an object or array.\",\n        \"In this test\": \"It is an object.\"\n    }\n}\n"
    };
    for (int i = 0; i < sizeof(should_decode); ++i) {
        PrintToServer("it_should_decode %s", should_decode[i]);
        check_test(it_should_decode(should_decode[i]));
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
        // test case disabled due to lack of escaping support
        // "[\"Illegal backslash escape: \\x15\"]",
        "[\\naked]",
        // test case disabled due to lack of escaping support
        // "[\"Illegal backslash escape: \\017\"]",
        // test case disabled due to lack of depth limiting
        // "[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]",
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
        "{\"Extra comma\": true,}"
    };
    for (int i = 0; i < sizeof(should_not_decode); ++i) {
        PrintToServer("it_should_not_decode %s", should_not_decode[i]);
        check_test(it_should_not_decode(should_not_decode[i]));
    }

    PrintToServer("it_should_pretty_print");
    check_test(it_should_pretty_print());

    PrintToServer("it_should_trim_floats");
    check_test(it_should_trim_floats());

    PrintToServer("it_should_remove_meta_keys_from_arrays");
    check_test(it_should_remove_meta_keys_from_arrays());

    PrintToServer("it_should_remove_meta_keys_from_objects");
    check_test(it_should_remove_meta_keys_from_objects());

    PrintToServer("it_should_shift_array_down_after_removed_index");
    check_test(it_should_shift_array_down_after_removed_index());

    PrintToServer("");
    PrintToServer("%d OK, %d FAILED", passed, failed);
}
