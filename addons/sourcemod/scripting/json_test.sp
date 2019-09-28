/**
 * vim: set ts=4 :
 * =============================================================================
 * sm-json
 * Provides a pure SourcePawn implementation of JSON encoding and decoding.
 * https://github.com/clugg/sm-json
 *
 * sm-json (C)2018 James Dickens. (clug)
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

#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

// Include our own include after setting compiler settings, to ensure we conform.
#include <json>

public Plugin myinfo = {
    name = "JSON Tester",
    author = "clug",
    description = "Tests dumping and loading JSON objects.",
    version = "1.0.2",
    url = "https://github.com/clugg/sm-json"
};

/**
 * @section Globals
 */

char json_encode_output[256];
int passed = 0;
int failed = 0;

/**
 * @section Methodmaps
 */

methodmap Weapon < JSON_Object {
    public Weapon()
    {
        return view_as<Weapon>(new JSON_Object());
    }

    property int id {
        public get()
        {
            return this.GetInt("id");
        }

        public set(int value)
        {
            this.SetInt("id", value);
        }
    }

    public bool GetString(char[] buffer, int maxlen)
    {
        return this.GetString("name", buffer, maxlen);
    }

    public void SetName(const char[] value)
    {
        this.SetString("name", value);
    }
}

methodmap Player < JSON_Object {
    public Player()
    {
        return view_as<Player>(new JSON_Object());
    }

    property int id {
        public get()
        {
            return this.GetInt("id");
        }

        public set(int value)
        {
            this.SetInt("id", value);
        }
    }

    property Weapon weapon {
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

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
    delete obj;

    return StrEqual(json_encode_output, "{}");
}

bool it_should_encode_empty_arrays()
{
    JSON_Object arr = new JSON_Object(true);

    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
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
        && obj.SetHandle("handle", null);

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
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

    if ((hndl = decoded_obj.GetHandle("handle")) != null) {
        LogError("json_test: unexpected value for key handle: %d", view_as<int>(hndl));
        success = false;
    }

    delete decoded_obj;

    return success;
}

bool it_should_support_arrays()
{
    JSON_Object arr = new JSON_Object(true);
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
        && arr.PushHandle(null);

    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
    delete arr;

    if (! success) {
        LogError("json_test: failed while pushing array values");

        return false;
    }

    JSON_Object decoded_arr = json_decode(json_encode_output);
    if (decoded_arr == null) {
        LogError("json_test: unable to decode array");

        return false;
    }

    int index = 0;
    char string[32];
    any value;
    Handle hndl;

    if (! decoded_arr.GetStringIndexed(index++, string, sizeof(string)) || ! StrEqual(string, "leet")) {
        LogError("json_test: unexpected value for index %d: %s", index, string);
        success = false;
    }

    if (! decoded_arr.GetStringIndexed(index++, string, sizeof(string)) || ! StrEqual(string, "\"leet\"")) {
        LogError("json_test: unexpected value for index %d: %s", index, string);
        success = false;
    }

    if ((value = decoded_arr.GetIntIndexed(index++)) != 9001) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetIntIndexed(index++)) != -9001) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetIntIndexed(index++)) != 0) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetIntIndexed(index++)) != -0) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if (! equal_enough((value = decoded_arr.GetFloatIndexed(index++)), 13.37)) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if (! equal_enough((value = decoded_arr.GetFloatIndexed(index++)), -13.37)) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetFloatIndexed(index++)) != 0.0) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetFloatIndexed(index++)) != -0.0) {
        LogError("json_test: unexpected value for index %d: %f", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetBoolIndexed(index++)) != true) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((value = decoded_arr.GetBoolIndexed(index++)) != false) {
        LogError("json_test: unexpected value for index %d: %d", index, value);
        success = false;
    }

    if ((hndl = decoded_arr.GetHandleIndexed(index++)) != null) {
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

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

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

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    bool success = obj.GetObject("object").GetBool("nested");
    obj.Cleanup();
    delete obj;

    return success;
}

bool it_should_support_objects_nested_in_arrays()
{
    JSON_Object nested_obj = new JSON_Object();
    nested_obj.SetBool("nested", true);

    JSON_Object arr = new JSON_Object(true);
    arr.PushObject(nested_obj);

    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    bool success = arr.GetObjectIndexed(0).GetBool("nested");
    arr.Cleanup();
    delete arr;

    return success;
}

bool it_should_support_arrays_nested_in_objects()
{
    JSON_Object nested_arr = new JSON_Object(true);
    nested_arr.PushBool(true);

    JSON_Object obj = new JSON_Object();
    obj.SetObject("array", nested_arr);

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    bool success = obj.GetObject("array").GetBoolIndexed(0);
    obj.Cleanup();
    delete obj;

    return success;
}

bool it_should_support_arrays_nested_in_arrays()
{
    JSON_Object nested_arr = new JSON_Object(true);
    nested_arr.PushBool(true);

    JSON_Object arr = new JSON_Object(true);
    arr.PushObject(nested_arr);

    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    bool success = arr.GetObjectIndexed(0).GetBoolIndexed(0);
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

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

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
    JSON_Object child_arr = new JSON_Object(true);
    child_arr.PushInt(1);

    JSON_Object child_obj = new JSON_Object();
    child_obj.SetHandle("im_indented", null);
    child_obj.SetObject("second_depth", child_arr);

    JSON_Object parent_obj = new JSON_Object();
    parent_obj.SetBool("pretty_printing", true);
    parent_obj.SetObject("first_depth", child_obj);

    parent_obj.Encode(json_encode_output, sizeof(json_encode_output), true);
    PrintToServer("%s", json_encode_output);

    parent_obj.Cleanup();
    delete parent_obj;

    return StrEqual(json_encode_output, "{\n    \"first_depth\": {\n        \"im_indented\": null,\n        \"second_depth\": [\n            1\n        ]\n    },\n    \"pretty_printing\": true\n}");
}

bool it_should_trim_floats()
{
    JSON_Object arr = new JSON_Object(true);
    arr.PushFloat(0.0);
    arr.PushFloat(1.0);
    arr.PushFloat(10.01);
    arr.PushFloat(-0.0);
    arr.PushFloat(-1.0);
    arr.PushFloat(-10.01);

    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    return StrEqual(json_encode_output, "[0.0,1.0,10.01,-0.0,-1.0,-10.01]");
}

bool it_should_remove_meta_keys_from_arrays()
{
    bool success = true;

    JSON_Object arr = new JSON_Object(true);
    arr.PushString("hello");
    arr.PushInt(0);

    if (! arr.HasKey("0:type") || arr.GetKeyTypeIndexed(0) != Type_String
        || ! arr.HasKey("0:length") || arr.GetKeyLengthIndexed(0) != 5
        || ! arr.HasKey("1:type") || arr.GetKeyTypeIndexed(1) != Type_Int) {
        LogError("json_test: array did not properly set meta-keys");

        success = false;
    }

    arr.RemoveIndexed(0);

    if (arr.HasKey("0:type") || arr.HasKey("0:length")) {
        LogError("json_test: array did not properly remove meta-keys");

        success = false;
    }

    if (! arr.HasKey("1:type")) {
        LogError("json_test: array removed incorrect meta-key");

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

    PrintToServer("it_should_support_objects_nested_in_objects");
    check_test(it_should_support_arrays_nested_in_arrays());

    PrintToServer("it_should_support_basic_methodmaps");
    check_test(it_should_support_basic_methodmaps());

    PrintToServer("it_should_support_nested_methodmaps");
    check_test(it_should_support_nested_methodmaps());

    char should_decode[][] = {
        "[]", "{}",
        "[{}]", "[[]]",
        "{\"object\":{}}", "{\"array\":[]}",
        " [\"whitespace_before_array\"]",
        "[\"whitespace_after_array\"] ",
        "{\n\t\"lots_of_whitespace\" : true\n}",
        "[\"nicely\\\"escaped\\\"string\"]",
        "[-1e2]", "[1e2]", "[1e+2]", "[1e-2]", "[-1e-2]",
        "[-1E2]", "[1E2]", "[1E+2]", "[1E-2]", "[-1E-2]",
        "[-0.5e2]", "[0.5e2]", "[0.5e+2]", "[0.5e-2]", "[-0.5e-2]",
        "[-0.5E2]", "[0.5E2]", "[0.5E+2]", "[0.5E-2]", "[-0.5E-2]"
    };
    for (int i = 0; i < sizeof(should_decode); ++i) {
        PrintToServer("it_should_decode %s", should_decode[i]);
        check_test(it_should_decode(should_decode[i]));
    }

    char should_not_decode[][] = {
        "", "\"string\"", "0", "0.0", "true", "false", "null",
        "[", "]",
        "{", "}",
        "{[", "]}",
        "[{", "}]",
        "{\"key_without_value\"}",
        "[\"key_in_array\":true]",
        "{'using_single_quotes':true}",
        "['using_single_quotes']",
        "[\"badly\"escaped\"string\"]",
        "[\"badly\\\\\"escaped\\\\\"string\"]",
        "[0,]", "[,0]", "[,0,]",
        "[0-]",
        "[.1]", "[1.]",
        "[00]", "[-00]",
        "[01]", "[-01]",
        "[00.01]", "[-00.01]",
        "[0e1]", "[0-e1]", "[0.e1]",
        "[1e+-1]", "[-1e-+1]",
        "junk before array[]", "[]junk after array",
        "junk before object{}", "{}junk after object"
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

    PrintToServer("");
    PrintToServer("%d OK, %d FAILED", passed, failed);
}
