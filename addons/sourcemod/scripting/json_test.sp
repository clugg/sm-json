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
#include <json>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "JSON Tester",
    author = "clug",
    description = "Tests dumping and loading JSON objects.",
    version = "1.0.0",
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
    public Weapon() {
        return view_as<Weapon>(new JSON_Object());
    }

    property int id {
        public get() { return this.GetInt("id"); }
        public set(int value) { this.SetInt("id", value); }
    }

    public bool GetString(char[] buffer, int maxlen) {
        return this.GetString("name", buffer, maxlen);
    }

    public void SetName(const char[] value) {
        this.SetString("name", value);
    }
}

methodmap Player < JSON_Object {
    public Player() {
        return view_as<Player>(new JSON_Object());
    }

    property int id {
        public get() { return this.GetInt("id"); }
        public set(int value) { this.SetInt("id", value); }
    }

    property Weapon weapon {
        public get() { return view_as<Weapon>(this.GetObject("weapon")); }
        public set(Weapon value) { this.SetObject("weapon", view_as<JSON_Object>(value)); }
    }
}

/**
 * @section Helpers
 */

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
    obj.SetString("str", "leet");
    obj.SetString("escaped_str", "\"leet\"");
    obj.SetInt("int", 9001);
    obj.SetInt("negative_int", -9001);
    obj.SetInt("zero", 0);
    obj.SetInt("negative_zero", -0);
    obj.SetFloat("float", 13.37);
    obj.SetFloat("negative_float", -13.37);
    obj.SetFloat("float_zero", 0.0);
    obj.SetFloat("negative_float_zero", -0.0);
    obj.SetBool("true", true);
    obj.SetBool("false", false);
    obj.SetHandle("handle", null);

    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    JSON_Object decoded_obj = json_decode(json_encode_output);
    bool success = decoded_obj != null;
    delete obj;
    delete decoded_obj;

    return success;
}

bool it_should_support_arrays()
{
    JSON_Object arr = new JSON_Object(true);
    arr.PushString("leet");
    arr.PushString("\"leet\"");
    arr.PushInt(9001);
    arr.PushInt(-9001);
    arr.PushInt(0);
    arr.PushInt(-0);
    arr.PushFloat(13.37);
    arr.PushFloat(-13.37);
    arr.PushFloat(0.0);
    arr.PushFloat(-0.0);
    arr.PushBool(true);
    arr.PushBool(false);
    arr.PushHandle(null);

    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);

    JSON_Object decoded_arr = json_decode(json_encode_output);
    bool success = decoded_arr != null;
    delete arr;
    delete decoded_arr;

    return success;
}

bool it_should_reload_an_object()
{
    JSON_Object obj = new JSON_Object();

    obj.SetBool("loaded", true);
    obj.Decode("{\"reloaded\": true}");
    obj.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
    bool success = obj.HasKey("loaded")
        && obj.HasKey("reloaded");
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
        PrintToServer("WARNING: malformed JSON was parsed as valid: %s", json_encode_output);
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

    return true;
}

bool it_should_trim_floats()
{
    JSON_Object arr = new JSON_Object(true);

    arr.PushFloat(0.0);
    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
    bool zero_success = StrEqual(json_encode_output, "[0.0]");

    arr.SetFloatIndexed(0, 1.0);
    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
    bool one_success = StrEqual(json_encode_output, "[1.0]");

    arr.SetFloatIndexed(0, 10.01);
    arr.Encode(json_encode_output, sizeof(json_encode_output));
    PrintToServer("%s", json_encode_output);
    bool fraction_success = StrEqual(json_encode_output, "[10.01]");

    return zero_success && one_success && fraction_success;
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

    PrintToServer("");
    PrintToServer("%d OK, %d FAILED", passed, failed);
}
