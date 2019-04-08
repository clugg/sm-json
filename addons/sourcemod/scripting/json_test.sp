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

#pragma dynamic 16384
#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "JSON Tester",
    author = "clug",
    description = "Tests dumping and loading JSON objects.",
    version = "1.0.0",
    url = "http://intradark.com/"
};


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


void malformed_error(JSON_Object obj, int line = 0) {
    if (obj != null) {
        char output[JSON_BUFFER_SIZE];
        obj.Encode(output, sizeof(output));
        PrintToServer("json-malformed-tests: WARNING malformed JSON #%d was parsed as valid: %s", line, output);
    }
}


public void OnPluginStart()
{
    char output[JSON_BUFFER_SIZE];

    /* empty-object-test */
    JSON_Object test_obj = new JSON_Object();
    // encode
    test_obj.Encode(output, sizeof(output));
    PrintToServer("json-empty-object-test: %s", output);

    /* object-test */
    test_obj.SetString("test_str", "leet");
    test_obj.SetInt("test_str", 9001);
    test_obj.SetFloat("test_float", 13.37);
    test_obj.SetBool("test_bool", true);
    test_obj.SetHandle("test_handle", null);
    // encode
    test_obj.Encode(output, sizeof(output));
    PrintToServer("json-object-test: %s", output);

    /* object-indexed-test */
    test_obj.SetStringIndexed(1, "teel");
    test_obj.SetIntIndexed(2, -9001);
    test_obj.SetFloatIndexed(4, -13.47);
    test_obj.SetBoolIndexed(8, false);
    test_obj.SetHandleIndexed(16, null);
    // encode
    test_obj.Encode(output, sizeof(output));
    PrintToServer("json-object-indexed-test: %s", output);

    /* object-reload-test */
    test_obj.Decode("{\"reloaded\": true}");
    // encode
    test_obj.Encode(output, sizeof(output));
    PrintToServer("json-object-reload-test: %s", output);

    /* empty-array-test */
    JSON_Object test_arr = new JSON_Object(true);
    // encode
    test_arr.Encode(output, sizeof(output));
    PrintToServer("json-empty-array-test: %s", output);

    /* array-test */
    test_arr.PushString("leet");
    test_arr.PushInt(9001);
    test_arr.PushFloat(13.37);
    test_arr.PushBool(true);
    test_arr.PushHandle(null);
    // encode
    test_arr.Encode(output, sizeof(output));
    PrintToServer("json-array-test: %s", output);

    /* nested-object-object-test */
    JSON_Object nest_obj = new JSON_Object();
    nest_obj.SetBool("nested", true);
    test_obj.SetObject("object", nest_obj);
    // encode
    test_obj.Encode(output, sizeof(output));
    PrintToServer("json-nested-object-object-test: %s", output);
    test_obj.Remove("object");

    /* nested-object-array-test */
    test_obj.SetObject("array", test_arr);
    // encode
    test_obj.Encode(output, sizeof(output));
    PrintToServer("json-nested-object-array-test: %s", output);
    test_obj.Remove("array");

    /* nested-array-object-test */
    test_arr.PushObject(test_obj);
    // encode
    test_arr.Encode(output, sizeof(output));
    PrintToServer("json-nested-array-object-test: %s", output);

    /* nested-array-array-test */
    JSON_Object nest_arr = new JSON_Object(true);
    nest_arr.PushString("nested");
    test_arr.PushObject(nest_arr);
    // encode
    test_arr.Encode(output, sizeof(output));
    PrintToServer("json-nested-array-array-test: %s", output);

    // cleanup
    test_arr.Cleanup();  // will clean up test_obj, nest_obj and nest_arr too as they are children
    delete test_arr;

    /* methodmap-test */
    Player player = new Player();
    player.id = 1;
    // encode
    player.Encode(output, sizeof(output));
    PrintToServer("json-methodmap-test: %s", output);

    /* nested-methodmap-test */
    Weapon weapon = new Weapon();
    weapon.SetName("ak47");

    player.weapon = weapon;
    player.weapon.id = 5;  // demonstrating nested property setters
    // encode
    player.Encode(output, sizeof(output));
    PrintToServer("json-methodmap-test: %s", output);

    // cleanup
    player.Cleanup();
    delete player;

    /* malformed-tests */
    PrintToServer("Running json-malformed-tests...");
    JSON_Object invalid_obj;
    invalid_obj = json_decode(""); malformed_error(invalid_obj, 1);
    invalid_obj = json_decode("{]"); malformed_error(invalid_obj, 2);
    invalid_obj = json_decode("[}"); malformed_error(invalid_obj, 3);
    invalid_obj = json_decode("{\"test\"}"); malformed_error(invalid_obj, 4);
    invalid_obj = json_decode("[\"test\":true]"); malformed_error(invalid_obj, 5);
    invalid_obj = json_decode("{'test':true}"); malformed_error(invalid_obj, 6);
    invalid_obj = json_decode("[\"test\"data\"]"); malformed_error(invalid_obj, 7);
    invalid_obj = json_decode("[\"test\\\\\"data\"]"); malformed_error(invalid_obj, 8);
}
