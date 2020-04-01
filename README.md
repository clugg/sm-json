# sm-json
![Build Status](https://github.com/clugg/sm-json/workflows/Compile%20with%20SourceMod/badge.svg) [![Latest Release](https://img.shields.io/github/v/release/clugg/sm-json)](https://github.com/clugg/sm-json/releases/latest)

A pure SourcePawn JSON encoder/decoder. Also offers a nice way of implementing objects using `StringMap` inheritance coupled with `methodmap`s.

Follows the JSON specification ([RFC7159](https://tools.ietf.org/html/rfc7159)) almost perfectly. The following are not supported and likely never will be:
* Any singular value not contained with a structure (e.g. `"string"`, `1`, `0.1`, `true`, `false`, `null`, etc.)
* Escaping/unescaping unicode values in strings (\uXXXX)

Additionally, users may opt to allow for `'single quote strings'` during decoding by setting `JSON_ALLOW_SINGLE_QUOTES = true` in their code (it is already declared by the library, defaulting to `false`). A mixture of single and double quoted strings can be used in a structure (e.g. `['single', "double"]`) as long as quotes are matched correctly. Note that encoded output will still use double quotes, and unescaping of single quotes in double quoted strings does not occur.

Table of Contents
=================

* [Requirements](#requirements)
* [Installation](#installation)
* [Usage](#usage)
  * [Creating and Encoding Arrays & Objects](#creating-and-encoding-arrays--objects)
  * [Decoding Arrays & Objects](#decoding-arrays--objects)
  * [Creating a 'Class'](#creating-a-class)
  * [Decoding 'Classes'](#decoding-classes)
  * [Iteration](#iteration)
  * [Pretty Printing](#pretty-printing)
  * [Cleaning Up](#cleaning-up)
* [API](#api)
  * [Getters and Setters](#getters-and-setters)
  * [Accessing super methods](#accessing-super-methods)
  * [Checking if a key exists](#checking-if-a-key-exists)
  * [Getting a key's type](#getting-a-keys-type)
  * [Getting a string's length](#getting-a-strings-length)
  * [Removing a key](#removing-a-key)
  * [Hiding Keys](#hiding-keys)
  * [Searching Arrays](#searching-arrays)
  * [Merging Instances](#merging-instances)
  * [Copying Instances](#copying-instances)
  * [Decoding Over Existing Instances](#decoding-over-existing-instances)
  * [Working with Unknowns](#working-with-unknowns)
  * [Global Helper Functions](#global-helper-functions)
* [Testing](#testing)
* [Contributing](#contributing)
* [License](#license)

## Requirements
* SourceMod 1.7 or later

## Installation
Download the source code for the [latest release](https://github.com/clugg/sm-json/releases/latest) and move all files and directories from the [`addons/sourcemod/scripting/include`](addons/sourcemod/scripting/include) directory to your existing `addons/sourcemod/scripting/include` directory.

## Usage
All of the following examples assume that the library has been included and an output char array is available.

```c
#include <json>

char output[1024];
```

### Creating and Encoding Arrays & Objects
```c
JSON_Array arr = new JSON_Array();
arr.PushString("my string");
arr.PushInt(1234);
arr.PushFloat(13.37);
arr.PushBool(true);
arr.PushNull();
arr.PushObject(new JSON_Array());
arr.PushObject(new JSON_Object());

arr.Encode(output, sizeof(output));
// output now contains ["my string",1234,13.37,true,null,[],{}]
arr.Cleanup();
delete arr;
```

```c
JSON_Object obj = new JSON_Object();
obj.SetString("strkey", "your string");
obj.SetInt("intkey", -1234);
obj.SetFloat("floatkey", -13.37);
obj.SetBool("boolkey", false);
obj.SetNull("nullkey");
obj.SetObject("array", new JSON_Array());
obj.SetObject("object", new JSON_Object());

obj.Encode(output, sizeof(output));
// output now contains {"object":{},"floatkey":-13.37,"boolkey":false,"intkey":-1234,"array":[],"nullkey":null,"strkey":"your string"}
obj.Cleanup();
delete obj;
```

### Decoding Arrays & Objects
```c
char strval[32];
int intval;
float floatval;
bool boolval;
Handle nullval;
JSON_Array arrval;
JSON_Object objval;

JSON_Array arr = view_as<JSON_Array>(json_decode("[\"my string\",1234,13.37,true,null,[],{}]"));
arr.GetString(0, strval, sizeof(strval));
intval = arr.GetInt(1);
floatval = arr.GetFloat(2);
boolval = arr.GetBool(3);
nullval = arr.GetNull(4);
arrval = view_as<JSON_Array>(arr.GetObject(5));
objval = arr.GetObject(6);

arr.Cleanup();
delete arr;
```

```c
JSON_Object obj = json_decode("{\"object\":{},\"floatkey\":-13.37,\"boolkey\":false,\"intkey\":-1234,\"array\":[],\"nullkey\":null,\"strkey\":\"your string\"}");
obj.GetString("strkey", strval, sizeof(strval));
intval = obj.GetInt("intkey");
floatval = obj.GetFloat("floatkey");
boolval = obj.GetBool("boolkey");
nullval = obj.GetNull("nullkey");
arrval = view_as<JSON_Array>(obj.GetObject("array"));
objval = obj.GetObject("object");

obj.Cleanup();
delete obj;
```

### Creating a 'Class'
`JSON_Object`s and `JSON_Array`s can be inherited once you understand a little bit about methodmaps. This can be abused to create pseudo-classes with properties. Since these use StringMaps under the hood, they will probably not be as efficient as arrays or enum structs, but in most cases they should be more than fine.

```c
methodmap YourClass < JSON_Object
{
    public bool SetName(const char[] value)
    {
        return this.SetString("name", value);
    }

    public bool GetName(char[] buffer, int max_size)
    {
        return this.GetString("name", buffer, max_size);
    }

    property int myint
    {
        public get()
        {
            return this.GetInt("myint");
        }

        public set(int value)
        {
            this.SetInt("myint", value);
        }
    }

    property float myfloat
    {
        public get()
        {
            return this.GetFloat("myfloat");
        }

        public set(float value)
        {
            this.SetFloat("myfloat", value);
        }
    }

    property bool mybool
    {
        public get()
        {
            return this.GetBool("mybool");
        }

        public set(bool value)
        {
            this.SetBool("mybool", value);
        }
    }

    property Handle mynull
    {
        public get()
        {
            return this.GetNull("mynull");
        }

        public set(Handle value)
        {
            this.SetNull("mynull");
        }
    }

    property JSON_Object myobject
    {
        public get()
        {
            return this.GetObject("myobject");
        }

        public set(JSON_Object value)
        {
            this.SetObject("myobject", value);
        }
    }

    property JSON_Array myarray
    {
        public get()
        {
            return view_as<JSON_Array>(this.GetObject("myarray"));
        }

        public set(JSON_Array value)
        {
            this.SetObject("myarray", value);
        }
    }

    public YourClass()
    {
        YourClass self = view_as<YourClass>(new JSON_Object());
        self.SetName("my class");
        self.myint = 9001;
        self.myfloat = 73.57;
        self.mybool = false;
        self.mynull = null;
        self.myobject = new JSON_Object();
        self.myarray = new JSON_Array();

        return self;
    }

    public void increment_int()
    {
        this.myint += 1;
    }
}

YourClass instance = new YourClass();
instance.Encode(output, sizeof(output));
// output now contains {"myarray":[],"mybool":false,"myint":9001,"myfloat":73.57,"name":"my class","myobject":{},"mynull":null}
```

You are also free to nest classes within one another.

```c
methodmap OtherClass < JSON_Object
{
    property YourClass instance
    {
        public get()
        {
            return view_as<YourClass>(this.GetObject("instance"));
        }

        public set(YourClass value)
        {
            this.SetObject("instance", value);
        }
    }

    property int otherint
    {
        public get()
        {
            return this.GetInt("otherint");
        }

        public set(int value)
        {
            this.SetInt("otherint", value);
        }
    }

    public OtherClass()
    {
        OtherClass self = view_as<OtherClass>(new JSON_Object());
        self.instance = new YourClass();
        self.otherint = -1;

        return self;
    }
}

OtherClass other = new OtherClass();
other.Encode(output, sizeof(output));
// output now contains {"otherint":-1,"instance":{"myarray":[],"mybool":false,"myint":9001,"myfloat":73.57,"name":"my class","myobject":{},"mynull":null}}
```

### Decoding 'Classes'
You can take any JSON_Object or JSON_Array and coerce it to a custom class in order to access its properties and methods.

```
OtherClass other = view_as<OtherClass>(json_decode("{\"otherint\":-1,\"instance\":{\"myarray\":[],\"mybool\":false,\"myint\":9001,\"myfloat\":73.57,\"name\":\"my class\",\"myobject\":{},\"mynull\":null}}"));
other.instance.increment_int();
int myint = other.instance.myint; // 9002
```

### Iteration
You can iterate through a JSON_Array because it is indexed numerically. You can also iterate through the keys in a JSON_Object because it is a `StringMap`, and as such you can fetch a `StringMapSnapshot` from it.

```c
JSON_Array arr = new JSON_Array();
for (int i = 0; i < arr.Length; i += 1) {
    JSON_CELL_TYPE type = arr.GetKeyType(i);
    // do whatever you want with the index and type information
}
```

```c
JSON_Object obj = new JSON_Object();
int key_length = 0;
StringMapSnapshot snap = obj.Snapshot();
for (int i = 0; i < obj.Length; i += 1) {
    key_length = snap.KeyBufferSize(i);
    char[] key = new char[key_length];

    snap.GetKey(i, key, key_length);

    // skip meta-keys
    if (json_is_meta_key(key)) {
        continue;
    }

    JSON_CELL_TYPE type = obj.GetKeyType(key);
    // do whatever you want with the key and type information
}
delete snap;
```

### Pretty Printing
You can enable pretty printed encoded JSON by passing `true` as a parameter. You can customise pretty printing by manually updating the `JSON_PP_*` constants in [`addons/sourcemod/scripting/include/json/definitions.inc`](addons/sourcemod/scripting/include/json/definitions.inc#L49-L51).

```c
JSON_Array child_arr = new JSON_Array();
child_arr.PushInt(1);

JSON_Object child_obj = new JSON_Object();
child_obj.SetNull("im_indented");
child_obj.SetObject("second_depth", child_arr);

JSON_Object parent_obj = new JSON_Object();
parent_obj.SetBool("pretty_printing", true);
parent_obj.SetObject("first_depth", child_obj);

parent_obj.Encode(output, sizeof(output), /* pretty print: */ true);
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

### Cleaning Up
Since this library uses `StringMap` under the hood, you need to make sure you manage your memory properly by cleaning up JSON instances when you're done with them.

You can use the `delete` keyword to directly delete an instance.

If an instance contains nested JSON instances (e.g. `[{}]`), the nested instances will not be cleaned up. A helper function `Cleanup()` has been provided which recursively cleans up and deletes all nested instances before deleting the parent instance.

```c
arr.Cleanup();
delete arr;

obj.Cleanup();
delete obj;
```

This may trip you up if you have multiple instances referring to one shared instance, because cleaning up the first will invalidate the handle for the second. For example:

```c
JSON_Array shared = new JSON_Array();

JSON_Object obj1 = new JSON_Object();
obj1.SetObject("shared", shared);

JSON_Object obj2 = new JSON_Object();
obj2.SetObject("shared", shared);

// this will clean up the "shared" array
obj1.Cleanup();
delete obj1;

// this will throw an Invalid Handle exception because "shared" has already been cleaned up
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

## API
All of the following examples assume access to an existing `JSON_Array` and `JSON_Object` instance.

```c
JSON_Array arr = new JSON_Array();
JSON_Object obj = new JSON_Object();
```

### Getters and Setters

`JSON_Array` and `JSON_Object` contain the following getters. `JSON_Array` getters will accept an int index and `JSON_Object` getters will accept a string key. Getters also accept a second parameter specifying what value to return if the key/index was not found. Sensible default values have been set and are listed below.

* `GetInt`, which will return the value or -1 if it was not found.
* `GetFloat`, which will return the value or -1.0 if it was not found.
* `GetBool`, which will return the value or false if it was not found.
* `GetNull`, which will return null.
* `GetObject`, which will return the value or null if it was not found.

It is recommended that you typecast objects to arrays if you believe the contents will be an array: `view_as<JSON_Array>(obj.GetObject("array"))`.

`JSON_Object` contains the following setters, which will accept a string key and a value and return true if setting was successful, or false otherwise.

* `SetString`
* `SetInt`
* `SetFloat`
* `SetBool`
* `SetNull`, which does not accept a value (since the value MUST be null).
* `SetObject`

`JSON_Array` contains the following setters, which will accept a value and return true if setting was successful, or false otherwise.

* `PushString`
* `PushInt`
* `PushFloat`
* `PushBool`
* `PushNull`, which does not accept a value (since the value MUST be null).
* `PushObject`

### Accessing super methods
`JSON_Array` inherits `JSON_Object` and `JSON_Object` inherits `StringMap`. There may be rare cases where you need to access underlying methods of a class. A `Super` property has been provided which views an instance as its superclass.

```c
JSON_Object arr_super = arr.Super;
StringMap obj_super = obj.Super;
```

### Checking if a key exists
Returns true or false depending on whether the key exists.

```c
arr.HasKey(0);
obj.HasKey("my_key");
```

### Getting a key's type
Returns a [JSON_CELL_TYPE](addons/sourcemod/scripting/include/json/definitions.inc#L81-L89). Useful if you are unsure what type of value a key contains.

```c
arr.GetKeyType(0);
obj.GetKeyType("my_key");
```

### Getting a string's length
If a key contains a string, you can get it's exact length, not including a NULL terminator. Useful if you want perfectly sized buffers.

```c
arr.GetKeyLength(0);
obj.GetKeyLength("my_string");

// example
int len = arr.GetKeyLength(0);
char[] val = new char[len];
arr.GetString(0, val, len);
```

### Removing a key
Removing a key will also remove all metadata associated with it (i.e. type, string length and hidden flag). When removing from an array, all following elements will be shifted down to ensure that all indexes fall within [0, `arr.Length`) and that there are no gaps in the array.

```c
arr.Remove(0); // index 1 becomes index 0 and so on
obj.Remove("my_key");
```

### Hiding Keys
Hiding a key will prevent the encoder from outputting it. Useful for when you wish to store 'secret' data without accidentally exposing it. **WARNING:** When calling `Clear()` or `Remove()`, the hidden flag is removed as well.

```c
arr.SetKeyHidden(0, true);
obj.SetKeyHidden("my_secret_key", true);

// encoding example
obj.SetString("my_secret_key", "my_secret_value");
obj.SetString("my_public_key", "my_public_value");
obj.Encode(output, sizeof(output));
// output now contains {"my_public_key":"my_public_value"}

// clear() example
obj.Clear();
obj.GetKeyHidden("my_secret_key"); // returns false

// remove() example
obj.Remove("my_secret_key");
obj.GetKeyHidden("my_secret_key"); // returns false
```

### Searching Arrays
There are a few functions which make working with `JSON_Array`s a bit nicer.

* `IndexOf`, which will return the index of a value in the array, or -1 if it is not found.
* `IndexOfString`, as above, but working with string values only.
* `Contains`, which will return true if a value is found in the array, false otherwise.
* `ContainsString`, as above, but working with string values only.

Please note that due to how the `any` type works in SourcePawn, `Contains` may return false positives for values that are stored the same in memory. For example, `0`, `null` and `false` are all stored as `0` in memory and `1` and `true` are both stored as `1` in memory. Because of this, `view_as<JSON_Array>(json_decode("[0]")).Contains(null)` will return true, and so on. You may use `Contains` in conjunction with `GetKeyType` to typecheck the returned index and ensure it matches what you expected.

### Merging Instances
`JSON_Array`s can be merged with one another, and `JSON_Object`s can too. For obvious reasons, an array cannot be merged with an object (and vice versa). Other combinations will log an error and fail.

Merging is shallow, which means that if the second object has child objects, the reference will be maintained to the existing object when merged, as opposed to copying the children. You can also disable key replacement by passing `false` as a parameter. For example, if you have two objects both containing key `x`, with replacement on (default behaviour), `x` will be taken from the second object, and with replacement off, from the first object.

Merged keys will respect their previous hidden state when merged on to the first object.

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
obj1.Merge(obj2, false); // obj1 is now equivocally {"x":1,"y":2,"z":4}, obj2 remains unchanged
```

### Copying Instances
`JSON_Array`s and `JSON_Object`s can both be copied either shallowly or deeply. A shallow copy will maintain reference to nested instances within the instance, while a deep copy will also copy all nested instances, yielding an entirely unrelated structure with all of the same values.

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

// alternatively, deep copying

JSON_Array copied = arr.DeepCopy();
JSON_Array nested = view_as<JSON_Array>(copied.GetObject(3));
nested.PushInt(4);
copied.PushInt(5);
// copied is now equivocally [1,2,3,[4],5] but arr does not change
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

// alternatively, deep copying

JSON_Object copied = obj.DeepCopy();
JSON_Object nested = copied.GetObject("nested");
nested.SetString("key", "value");
copied.SetInt("test", 1);
// copied is now equivocally {"hello":"world","nested":{"key":"value"},"test":1} but obj does not change
```

### Decoding Over Existing Instances
It is possible that you already have a `JSON_Array` or `JSON_Object` instance which you wish to decode over the top of. As with [Merging Instances](#merging-instances), only array/array and object/object decoding is possible.

```c
arr.PushInt(1);
arr.PushInt(2);
arr.PushInt(3);
// arr is now equivocally [1,2,3]
arr.Decode("[4,5,6]");
// arr is now equivocally [1,2,3,4,5,6]
```

```c
obj.SetBool("loaded", true);
// obj is now equivocally {"loaded":true}
obj.Decode("{\"hello\":\"world\"}");
// obj is now equivocally {"hello":"world","loaded":true}
```

### Working with Unknowns
In some rare cases, you may receive JSON which you do not know the structure of. It may contain an object or an array. This is possible to handle using the `IsArray` property, although it can result in some messy code.

```c
JSON_Object obj = json_decode(SOME_UNKNOWN_JSON);
JSON_Array arr = view_as<JSON_Array>(obj);

if (obj.IsArray) {
    arr.PushString("ok");
} else {
    obj.SetString("result", "ok");
}
```

### Global Helper Functions
All of the examples that you have seen in this documentation use object-oriented syntax. In reality, these are wrappers for global functions. A complete list of examples can be found below.

```c
arr.Encode(output, sizeof(output));
// is equivalent to
json_encode(arr, output, sizeof(output));

arr.Decode("[1,2,3]");
// is equivalent to
json_decode("[1,2,3]", arr);

arr.Merge(other_arr);
// is equivalent to
json_merge(arr, other_arr);

arr.ShallowCopy();
// is equivalent to
json_copy_shallow(arr);

arr.DeepCopy();
// is equivalent to
json_copy_deep(arr);

arr.Cleanup();
// is equivalent to
json_cleanup(arr);
```

If you prefer this style you may wish to use it instead.

## Testing
A number of common tests have been written [here](addons/sourcemod/scripting/json_test.sp). These tests include library-specific tests (which can be considered examples of how the library can be used) as well as almost every test from the [json.org test suite](https://www.json.org/JSON_checker/). Tests regarding unsupported features such as Unicode handling have been excluded.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please ensure that all tests pass before making a pull request.

If you are fixing a bug, please add a regression test to ensure that the bug does not sneak back in. If you are adding a feature, please add tests to ensure that it works as expected.

## License
[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
