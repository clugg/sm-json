# sm-json
![Build Status](https://github.com/clugg/sm-json/workflows/Compile%20with%20SourceMod/badge.svg) [![Latest Release](https://img.shields.io/github/v/release/clugg/sm-json?include_prereleases&sort=semver)](https://github.com/clugg/sm-json/releases)

**This README covers documentation for v3.x. If you're looking for v2.x docs, please use the [v2.x branch](../../tree/v2.x).**

A pure SourcePawn JSON encoder/decoder. Also offers a nice way of implementing pseudo-classes with properties and methods.

Follows the JSON specification ([RFC7159](https://tools.ietf.org/html/rfc7159)) almost perfectly. Singular values not contained within a structure (e.g. `"string"`, `1`, `0.1`, `true`, `false`, `null`, etc.) are not supported.

Table of Contents
=================

* [Requirements](#requirements)
* [Installation](#installation)
* [Usage](#usage)
  * [Creating & Encoding](#creating--encoding)
  * [Decoding](#decoding)
  * [Iteration](#iteration)
  * [Cleaning Up](#cleaning-up)
  * [Pseudo-Classes](#pseudo-classes)
* [API](#api)
  * [Getters & Setters](#getters--setters)
  * [Metadata](#metadata)
  * [Removing Elements](#removing-elements)
  * [Array Helpers](#array-helpers)
  * [Array Type Enforcement](#array-type-enforcement)
  * [Array Importing](#array-importing)
  * [Merging](#merging)
  * [Copying](#copying)
  * [Working with Unknowns](#working-with-unknowns)
  * [Accessing Super Methods](#accessing-super-methods)
  * [Global Helper Functions](#global-helper-functions)
* [Testing](#testing)
* [Contributing](#contributing)
* [License](#license)

## Requirements
* SourceMod 1.8 or later

## Installation
Download the source code for the [latest release](https://github.com/clugg/sm-json/releases/latest) and move all files and directories from the [`addons/sourcemod/scripting/include`](addons/sourcemod/scripting/include) directory to your existing `addons/sourcemod/scripting/include` directory.

## Usage
All of the following examples implicitly begin with the following code snippet.

```c
// include the library
#include <json>

// this is where our encoding results will go
char output[1024];
```

### Creating & Encoding
#### Arrays
```c
JSON_Array arr = new JSON_Array();
arr.PushString("my string");
arr.PushInt(1234);
arr.PushFloat(13.37);
arr.PushBool(true);
arr.PushObject(null);
arr.PushObject(new JSON_Array());
arr.PushObject(new JSON_Object());

arr.Encode(output, sizeof(output));
// output now contains ["my string",1234,13.37,true,null,[],{}]
arr.Cleanup();
delete arr;
```

#### Objects
```c
JSON_Object obj = new JSON_Object();
obj.SetString("strkey", "your string");
obj.SetInt("intkey", -1234);
obj.SetFloat("floatkey", -13.37);
obj.SetBool("boolkey", false);
obj.SetObject("nullkey", null);
obj.SetObject("array", new JSON_Array());
obj.SetObject("object", new JSON_Object());

obj.Encode(output, sizeof(output));
// output now contains {"object":{},"floatkey":-13.37,"boolkey":false,"intkey":-1234,"array":[],"nullkey":null,"strkey":"your string"}
obj.Cleanup();
delete obj;
```

#### Options
Options which modify how the encoder works can be passed as the third parameter (or fourth in `json_encode`).
* `JSON_ENCODE_PRETTY`: enables pretty printing. You can customise pretty printing by overriding (i.e. strcopy) the `JSON_PP_*` strings which are declared in [`addons/sourcemod/scripting/include/json/definitions.inc`](addons/sourcemod/scripting/include/json/definitions.inc#L62-L71). Please do note that these are `char[32]`s. **Example:**

```c
JSON_Array child_arr = new JSON_Array();
child_arr.PushInt(1);

JSON_Object child_obj = new JSON_Object();
child_obj.SetObject("im_indented", null);
child_obj.SetObject("second_depth", child_arr);

JSON_Object parent_obj = new JSON_Object();
parent_obj.SetBool("pretty_printing", true);
parent_obj.SetObject("first_depth", child_obj);

parent_obj.Encode(output, sizeof(output), JSON_ENCODE_PRETTY);
parent_obj.Cleanup();
delete parent_obj;
```

`output` will contain the following:
```json
{
    "first_depth": {
        "im_indented": null,
        "second_depth": [
            1
        ]
    },
    "pretty_printing": true
}
```

Using the same parent object as last time (pretending we didn't just clean it up!):
```c
strcopy(JSON_PP_AFTER_COLON, sizeof(JSON_PP_AFTER_COLON), " ");
strcopy(JSON_PP_INDENT, sizeof(JSON_PP_AFTER_COLON), "");
strcopy(JSON_PP_NEWLINE, sizeof(JSON_PP_NEWLINE), " ");

parent_obj.Encode(output, sizeof(output), JSON_ENCODE_PRETTY);
```

`output` will contain the following:
```json
{ "first_depth": { "im_indented": null, "second_depth": [ 1, [] ] }, "pretty_printing": true }
```

### Decoding
#### Arrays
```c
JSON_Array arr = view_as<JSON_Array>(json_decode("[\"my string\",1234,13.37,true,null,[],{}]"));
char strval[32];
arr.GetString(0, strval, sizeof(strval));
int intval = arr.GetInt(1);
float floatval = arr.GetFloat(2);
bool boolval = arr.GetBool(3);
Handle nullval = arr.GetObject(4);
JSON_Array arrval = view_as<JSON_Array>(arr.GetObject(5));
JSON_Object objval = arr.GetObject(6);

arr.Cleanup();
delete arr;
```

#### Objects
```c
JSON_Object obj = json_decode("{\"object\":{},\"floatkey\":-13.37,\"boolkey\":false,\"intkey\":-1234,\"array\":[],\"nullkey\":null,\"strkey\":\"your string\"}");
char strval[32];
obj.GetString("strkey", strval, sizeof(strval));
int intval = obj.GetInt("intkey");
float floatval = obj.GetFloat("floatkey");
bool boolval = obj.GetBool("boolkey");
Handle nullval = obj.GetObject("nullkey");
JSON_Array arrval = view_as<JSON_Array>(obj.GetObject("array"));
JSON_Object objval = obj.GetObject("object");

obj.Cleanup();
delete obj;
```

#### Options
Options which modify how the parser works can be passed as the second parameter (e.g. `json_decode("[]", JSON_DECODE_SINGLE_QUOTES)`).
* `JSON_DECODE_SINGLE_QUOTES`: accepts `'single quote strings'` as valid. A mixture of single and double quoted strings can be used in a structure (e.g. `['single', "double"]`) as long as quotes are matched correctly. *Note: encoded output will still use double quotes, and unescaping of single quotes in double quoted strings does not occur.*

### Iteration
#### Arrays
```c
int length = arr.Length;
for (int i = 0; i < length; i += 1) {
    JSONCellType type = arr.GetKeyType(i);
    // do whatever you want with the index and type information
}
```

#### Objects
```c
int length = obj.Length;
int key_length = 0;
StringMapSnapshot snap = obj.Snapshot();
for (int i = 0; i < length; i += 1) {
    key_length = snap.KeyBufferSize(i);
    char[] key = new char[key_length];

    snap.GetKey(i, key, key_length);

    // skip meta-keys
    if (json_is_meta_key(key)) {
        continue;
    }

    JSONCellType type = obj.GetKeyType(key);
    // do whatever you want with the key and type information
}
delete snap;
```

### Cleaning Up
Since this library uses `StringMap` under the hood, you need to make sure you manage your memory properly by cleaning up instances with the `delete` keyword when you're done with them.

If an instance contains nested instance(s) (e.g. `[{}]`), they will not be automatically cleaned up upon deletion. A helper function `Cleanup()` has been provided which recursively cleans up and deletes all nested instances before deleting the parent instance.

Additionally, there is a global helper function `json_cleanup_and_delete()` which will first call `Cleanup()`, then `delete`, then set the variable to null.

```c
arr.Cleanup();
delete arr;
arr = null;
// or
json_cleanup_and_delete(arr);

obj.Cleanup();
delete obj;
obj = null;
// or
json_cleanup_and_delete(obj);
```

This may trip you up if you have multiple references to one shared instance, because cleaning up the first will invalidate the handle for the second. For example:

```c
JSON_Array shared = new JSON_Array();

JSON_Object obj1 = new JSON_Object();
obj1.SetObject("shared", shared);

JSON_Object obj2 = new JSON_Object();
obj2.SetObject("shared", shared);

// this will clean up the nested "shared" array
obj1.Cleanup();
delete obj1;

// this will throw an Invalid Handle exception because "shared" no longer exists
obj2.Cleanup();
delete obj2;
```

You can avoid this by removing known shared instances from other instances before cleaning them up.

```c
obj1.Remove("shared");
obj1.Cleanup();
delete obj1;

obj2.Remove("shared");
obj2.Cleanup();
delete obj2;

shared.Cleanup();
delete shared;
```

### Pseudo-Classes
#### Creating & Encoding
```c
methodmap Player < JSON_Object
{
    public bool SetAlias(const char[] value)
    {
        return this.SetString("alias", value);
    }

    public bool GetAlias(char[] buffer, int max_size)
    {
        return this.GetString("alias", buffer, max_size);
    }

    property int Score
    {
        public get()
        {
            return this.GetInt("score");
        }

        public set(int value)
        {
            this.SetInt("score", value);
        }
    }

    property float Height
    {
        public get()
        {
            return this.GetFloat("height");
        }

        public set(float value)
        {
            this.SetFloat("height", value);
        }
    }

    property bool Alive
    {
        public get()
        {
            return this.GetBool("alive");
        }

        public set(bool value)
        {
            this.SetBool("alive", value);
        }
    }

    property Handle Handle
    {
        public get()
        {
            return view_as<Handle>(this.GetObject("handle"));
        }

        public set(Handle value)
        {
            this.SetObject("handle", value);
        }
    }

    property JSON_Object Object
    {
        public get()
        {
            return this.GetObject("object");
        }

        public set(JSON_Object value)
        {
            this.SetObject("object", value);
        }
    }

    property JSON_Array Array
    {
        public get()
        {
            return view_as<JSON_Array>(this.GetObject("array"));
        }

        public set(JSON_Array value)
        {
            this.SetObject("array", value);
        }
    }

    public Player()
    {
        Player self = view_as<Player>(new JSON_Object());
        self.SetAlias("clug");
        self.Score = 9001;
        self.Height = 1.8;
        self.Alive = true;
        self.Handle = null;
        self.Object = new JSON_Object();
        self.Array = new JSON_Array();

        return self;
    }

    public void IncrementScore()
    {
        this.Score += 1;
    }
}

Player player = new Player();
player.Encode(output, sizeof(output));
// output now contains {"score":9001,"alive":true,"object":{},"handle":null,"height":1.8,"alias":"clug","array":[]}
```

You are also free to nest classes within one another (a continuation from the previous snippet).

```c
methodmap Weapon < JSON_Object
{
    property Player Owner
    {
        public get()
        {
            return view_as<Player>(this.GetObject("owner"));
        }

        public set(Player value)
        {
            this.SetObject("owner", value);
        }
    }

    property int Id
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

    public Weapon()
    {
        Weapon self = view_as<Weapon>(new JSON_Object());
        self.Owner = new Player();
        self.Id = 1;

        return self;
    }
}

Weapon weapon = new Weapon();
weapon.Encode(output, sizeof(output));
// output now contains {"id":1,"owner":{"score":9001,"alive":true,"object":{},"handle":null,"height":1.8,"alias":"clug","array":[]}}
```

#### Decoding
You can take any JSON_Object or JSON_Array and coerce it to a custom class in order to access its properties and methods.

```
Weapon weapon = view_as<Weapon>(json_decode("{\"id\":1,\"owner\":{\"score\":9001,\"alive\":true,\"object\":{},\"handle\":null,\"height\":1.8,\"alias\":\"clug\",\"array\":[]}}"));
weapon.Owner.IncrementScore();
int score = weapon.Owner.Score; // 9002
```

## API
All of the following examples assume access to an existing `JSON_Array` and `JSON_Object` instance.

```c
JSON_Array arr = new JSON_Array();
JSON_Object obj = new JSON_Object();
```

In every case where a method denotes that it accepts a `key/index`, it means the following:
* `JSON_Object` methods will accept a `const char[] key`
* `JSON_Array` methods will accept an `int index`

### Getters & Setters

`JSON_Array` and `JSON_Object` contain the following getters. These getters also accept a second parameter specifying a default value to return if the key/index was not found. Sensible default values have been set and are listed below.

* `obj/arr.GetString(key/index, buffer, max_size)`, which will place the string in the buffer provided and return true, or false if it fails.
* `obj/arr.GetInt(key/index)`, which will return the value or -1 if it was not found.
* `obj/arr.GetFloat(key/index)`, which will return the value or -1.0 if it was not found.
* `obj/arr.GetBool(key/index)`, which will return the value or false if it was not found.
* `obj/arr.GetObject(key/index)`, which will return the value or null if it was not found. You should typecast objects to arrays if you know the contents to be an array: `view_as<JSON_Array>(obj.GetObject("array"))`.

`JSON_Array` and `JSON_Object` contain the following setters. These methods will return true if setting was successful, or false otherwise.

* `obj/arr.SetString(key/index, value)`
* `obj/arr.SetInt(key/index, value)`
* `obj/arr.SetFloat(key/index, value)`
* `obj/arr.SetBool(key/index, value)`
* `obj/arr.SetObject(key/index, value)`: value can be a `JSON_Array`, a `JSON_Object` or `null`

`JSON_Array` also contains push methods, which will push a value to the end of the array and return its index, or -1 if pushing failed.

* `arr.PushString(value)`
* `arr.PushInt(value)`
* `arr.PushFloat(value)`
* `arr.PushBool(value)`
* `arr.PushObject(value)`: value can be a `JSON_Array`, a `JSON_Object` or `null`

### Metadata
* `obj/arr.HasKey(key/index)`: returns true if the key exists, false otherwise.
* `obj/arr.GetKeyType(key/index)`: returns the [JSONCellType](addons/sourcemod/scripting/include/json/definitions.inc#LL96-L106) stored at the key.
* `obj/arr.GetKeyLength(key/index)`: if the key contains a string, returns the exact length of the string (not including NULL terminator). **Example:**

```c
int len = arr.GetKeyLength(0) + 1;
char[] val = new char[len];
arr.GetString(0, val, len);
```

It is possible to mark a key as 'hidden' so that it does not appear in encoder output. **WARNING:** When calling `Clear()` or `Remove()`, the hidden flag will be removed.
* `obj/arr.SetKeyHidden(key/index, true/false)`: sets the specified key to be hidden (or not hidden).
* `obj/arr.GetKeyHidden(key/index)`: returns whether or not the key is hidden.
**Example:**
```c
obj.SetKeyHidden("secret_key", true);
obj.SetString("secret_key", "secret_value");
obj.SetString("public_key", "public_value");
obj.Encode(output, sizeof(output));
// output now contains {"public_key":"public_value"}

// Clear() example, assuming key is still hidden
obj.Clear();
obj.GetKeyHidden("secret_key"); // returns false

// Remove() example, assuming key is still hidden
obj.Remove("secret_key");
obj.GetKeyHidden("secret_key"); // returns false
```

### Removing Elements
`obj/arr.Remove(key/index)`

Removing an element will also remove all metadata associated with it (i.e. type, string length and hidden flag). When removing from an array, all following elements will be shifted down an index to ensure that all indexes fall within [0, `arr.Length`) and that there are no gaps in the array.

### Array Helpers
There are a few functions which make working with `JSON_Array`s a bit nicer.

* `arr.IndexOf(value)`: returns the index of the value in the array if it is found, -1 otherwise.
* `arr.IndexOfString(value)`: as above, but works exclusively with strings.
* `arr.Contains(value)`: returns true if the value is found in the array, false otherwise.
* `arr.ContainsString(value)`: as above, but works exclusively with strings.

Please note that due to how the `any` type works in SourcePawn, `Contains` may return false positives for values that are stored the same in memory. For example, `0`, `null` and `false` are all stored as `0` in memory and `1` and `true` are both stored as `1` in memory. Because of this, `view_as<JSON_Array>(json_decode("[0]")).Contains(null)` will return true, and so on. You may use `Contains` in conjunction with `GetKeyType` to typecheck the returned index and ensure it matches what you expected.

### Array Type Enforcement
It is possible to enforce an array to only accept a single type. You can either do this when first creating the array, or later on.

```c
JSON_Array ints = new JSON_Array(JSON_Type_Int);
ints.PushObject(null); // fails and returns -1
ints.PushInt(1); // returns 0
json_cleanup_and_delete(ints);

JSON_Array values = new JSON_Array();
values.PushObject(null);
values.PushInt(1);
values.SetType(JSON_Type_Int); // fails and returns false, array doesn't only contain ints
values.Remove(0);
values.SetType(JSON_Type_Int); // returns true
json_cleanup_and_delete(values);
```

### Array Importing
It is possible to import any native array of values into a `JSON_Array`. The following code snippet works for every native type except char[]s.

```c
int ints[] = {1, 2, 3};
JSON_Array arr = new JSON_Array();
arr.ImportValues(JSON_Type_Int, ints, sizeof(ints));

arr.Encode(output, sizeof(output)); // output now contains [1,2,3]
json_cleanup_and_delete(arr);
```

For strings, you need to use a separate function.

```c
char strings[][] = {"hello", "world"};
JSON_Array arr = new JSON_Array();
arr.ImportStrings(strings, sizeof(strings));

arr.Encode(output, sizeof(output)); // output now contains [\"hello\",\"world\"]
json_cleanup_and_delete(arr);
```

### Array Exporting
It is possible to export a `JSON_Array`'s values to a native array. The following code snippet works for every native type except char[]s. *Note: there is no type checking done during export - it is entirely up to you to ensure that your array only contains the type that you expect (see [Array Type Enforcement](#array-type-enforcement)).*

```c
JSON_Array arr = view_as<JSON_Array>(json_decode("[1,2,3]"));
int size = arr.Length;
int[] values = new int[size];
arr.ExportValues(values, size);
json_cleanup_and_delete(arr);
// values now contains {1, 2, 3}
```

For strings, you need to use a separate function.

```c
JSON_Array arr = view_as<JSON_Array>(json_decode("[\"hello\",\"world\"]"));
int size = arr.Length;
int str_length = arr.MaxStringLength + 1;
char[][] values = new char[size][str_length];
arr.ExportStrings(values, size, str_length);
json_cleanup_and_delete(arr);
// values now contains {"hello", "world"}
```

### Merging
`JSON_Array`s can be merged with one another, and `JSON_Object`s can too. For obvious reasons, an array cannot be merged with an object (and vice versa).

Merging is shallow, which means that if the second object has child objects, the reference will be maintained to the existing object when merged, as opposed to copying the children.

Merged keys will respect their previous hidden state when merged on to the first object.

#### Options
* `JSON_MERGE_REPLACE`: active by default. Tells the merger to replace any existing keys on the first object with the values from the second. For example, if you have two objects both containing key `x`, with replacement on, the value of `x` will be taken from the second object, and with replacement off, from the first object. You can explicitly disable this by passing `JSON_NONE` as an option.
* `JSON_MERGE_CLEANUP`: tells merge to clean up any nested instances before they are replaced. Since this only has an effect while replacement is enabled, you will need to pass `JSON_MERGE_REPLACE | JSON_MERGE_CLEANUP` as options.

```c
JSON_Array arr1 = new JSON_Array();
arr1.PushInt(1);
arr1.PushInt(2);
arr1.PushInt(3);

JSON_Array arr2 = new JSON_Array();
arr2.PushInt(4);
arr2.PushInt(5);
arr2.PushInt(6);

arr1.Merge(arr2); // arr1 is now equivocally [1,2,3,4,5,6], arr2 remains unchanged
```

```c
JSON_Object obj1 = new JSON_Object();
obj1.SetInt("x", 1);
obj2.SetInt("y", 2);

JSON_Object obj2 = new JSON_Object();
obj2.SetInt("y", 3);
obj2.SetInt("z", 4)

obj1.Merge(obj2); // obj1 is now equivocally {"x":1,"y":3,"z":4}, obj2 remains unchanged
// alternatively, without replacement
obj1.Merge(obj2, JSON_NONE); // obj1 is now equivocally {"x":1,"y":2,"z":4}, obj2 remains unchanged
```

### Copying
#### Shallow
A shallow copy will maintain the original reference to nested instances within the instance.

```c
arr.PushInt(1);
arr.PushInt(2);
arr.PushInt(3);
arr.PushObject(new JSON_Array());
// arr is now equivocally [1,2,3,[]]

JSON_Array copied = arr.ShallowCopy();
JSON_Array nested = view_as<JSON_Array>(copied.GetObject(3));
nested.PushInt(4);
copied.PushInt(5);
// copied is now equivocally [1,2,3,[4],5] and arr is now equivocally [1,2,3,[4]]
```

```c
obj.SetString("hello", "world");
obj.SetObject("nested", new JSON_Object());
// obj is now equivocally {"hello":"world","nested":{}}

JSON_Object copied = obj.ShallowCopy();
JSON_Object nested = copied.GetObject("nested");
nested.SetString("key", "value");
copied.SetInt("test", 1);
// copied is now equivocally {"hello":"world","nested":{"key":"value"},"test":1} and obj is now equivocally {"hello":"world","nested":{"key":"value"}}
```

#### Deep
A deep copy will recursively copy all nested instances, yielding an entirely unrelated structure with all of the same values.
```c
JSON_Array copied = arr.DeepCopy();
JSON_Array nested = view_as<JSON_Array>(copied.GetObject(3));
nested.PushInt(4);
copied.PushInt(5);
// copied is now equivocally [1,2,3,[4],5] but arr does not change
```

```c
JSON_Object copied = obj.DeepCopy();
JSON_Object nested = copied.GetObject("nested");
nested.SetString("key", "value");
copied.SetInt("test", 1);
// copied is now equivocally {"hello":"world","nested":{"key":"value"},"test":1} but obj does not change
```

### Working with Unknowns
In some cases, you may receive JSON which you do not know the structure of. It may contain an object or an array. This is possible to handle using the `IsArray` property, although it can result in some messy code.

```c
JSON_Object obj = json_decode(SOME_UNKNOWN_JSON);
JSON_Array arr = view_as<JSON_Array>(obj);

if (obj.IsArray) {
    arr.PushString("ok");
} else {
    obj.SetString("result", "ok");
}
```

### Accessing Super Methods
`JSON_Array` inherits `JSON_Object` and `JSON_Object` inherits `StringMap`. There may be rare cases where you need to access an instance's superclass methods. A `Super` property has been provided which views an instance as its superclass.

```c
JSON_Object arr_super = arr.Super;
StringMap arr_super_super = arr.Super.Super; // or arr_super.Super

StringMap obj_super = obj.Super;
```

### Global Helper Functions
A few of the examples in this documentation use object-oriented syntax, while in reality, they are wrappers for global functions. A complete list of examples can be found below.

```c
obj/arr.Encode(output, sizeof(output) /*, options */);
// is equivalent to
json_encode(obj/arr, output, sizeof(output) /*, options */);

obj/arr.Merge(other /*, options */);
// is equivalent to
json_merge(obj/arr, other /*, options */);

obj/arr.ShallowCopy();
// is equivalent to
json_copy_shallow(obj/arr);

obj/arr.DeepCopy();
// is equivalent to
json_copy_deep(obj/arr);

obj/arr.Cleanup();
// is equivalent to
json_cleanup(obj/arr);
```

If you prefer this style you may wish to use it instead.

## Testing
A number of common tests have been written [here](addons/sourcemod/scripting/json_test.sp). These tests include library-specific tests (which can be considered examples of how the library can be used) as well as every relevant test from the [json.org test suite](https://www.json.org/JSON_checker/).

The test plugin uses the [sm-testsuite](https://github.com/clugg/sm-testsuite) library, which is included as a submodule to this repository. If you wish to run the tests yourself, follow these steps:
1. run `git submodule update --init` on your command line inside the `sm-json` directory
2. compile the plugin using `spcomp json_test.sp -O2 -t4 -v2 -w234 -i../../../dependencies/sm-testsuite/addons/sourcemod/scripting/include`
3. place the plugin in your sourcemod installation
4. run srcds if it's not already running
5. `sm plugins load json_test` (or `reload` if already loaded)
6. take note of output and ensure that all tests pass

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please ensure that all tests pass before making a pull request. A description of how to compile the test plugin can be seen in the [testing](#Testing) section.

If you are fixing a bug, please add a regression test to ensure that the bug does not sneak back in. If you are adding a feature, please add tests to ensure that it works as expected.

## License
[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
