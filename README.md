# sm-json

**This project is currently unstable. I am currently refactoring the parser, so things may not work as expected. I recommend using [SMJansson](https://forums.alliedmods.net/showthread.php?t=184604) or [REST in Pawn](https://forums.alliedmods.net/showthread.php?t=298024) for the time being.**

Provides a pure SourcePawn implementation of JSON encoding and decoding. Also offers a nice way of implementing objects using `StringMap` inheritance coupled with `methodmap`s.

Follows the JSON specification ([RFC7159](https://tools.ietf.org/html/rfc7159)) almost perfectly. Currently, the following is not supported:
* Scientific notation for floating point numbers
* Escaping/unescaping unicode values in strings (\uXXXX)

## Requirements
* SourceMod 1.7 and up

## Usage

### Creating an Object, Nesting Objects & Basic Encoding

A JSON_Object is used to represent the values within any JSON. It can be either an array or an object, which can be checked with `object.IsArray`.

```c
char output[JSON_BUFFER_SIZE];

JSON_Object empty_array = new JSON_Object(true);  // true denotes array

JSON_Object array = new JSON_Object(true);
array.PushString("my string");
array.PushInt(1234);
array.PushFloat(13.37);
array.PushBool(true);
array.PushHandle(null);
array.PushObject(empty_array);

array.Encode(output, sizeof(output));
// ["my string",1234,13.370000,true,null,[]]

JSON_Object object = new JSON_Object();
object.SetString("strkey", "your string");
object.SetInt("intkey", -1234);
object.SetFloat("floatkey", -13.37);
object.SetBool("boolkey", false);
object.SetHandle("handlekey", null);
object.SetObject("array", array);

object.Encode(output, sizeof(output));
// {"intkey":-1234,"array":["my string",1234,13.370000,true,null,[]],"floatkey":-13.370000,"boolkey":false,"strkey":"your string","handlekey":null}
```

### Creating a 'Class'

JSON_Objects can be inherited once you understand a little bit about methodmaps. They can be abused to create pseudo-classes with properties.

```c
methodmap YourClass < JSON_Object {
    property int myint {
        public get() { return this.GetInt("myint"); }
        public set(int value) { this.SetInt("myint", value); }
    }

    property float myfloat {
        public get() { return this.GetFloat("myfloat"); }
        public set(float value) { this.SetFloat("myfloat", value); }
    }

    property bool mybool {
        public get() { return this.GetBool("mybool"); }
        public set(bool value) { this.SetBool("mybool", value); }
    }

    property Handle myhandle {
        public get() { return this.GetHandle("myhandle"); }
        public set(Handle value) { this.SetHandle("myhandle", value); }
    }

    property JSON_Object myobject {
        public get() { return this.GetObject("myobject"); }
        public set(JSON_Object value) { this.SetObject("myobject", value); }
    }

    public bool SetName(const char[] value) {
        return this.SetString("name", value);
    }

    public bool GetName(char[] buffer, int maxlen) {
        return this.GetString("name", buffer, maxlen);
    }

    public YourClass() {
        YourClass obj = view_as<YourClass>(new JSON_Object());
        obj.myint = 9001;
        obj.myfloat = 73.57;
        obj.mybool = false;
        obj.myhandle = null;
        obj.myobject = new JSON_Object(true);  // store an array
        obj.SetName("my class");
        return obj;
    }

    public void increment_int() {
        this.myint += 1;  // sample usage of properties
    }
}

YourClass instance = new YourClass();
instance.Encode(output, sizeof(output));
// {"myfloat":73.570000,"myobject":[],"myhandle":null,"mybool":false,"name":"my class","myint":9001}
```

Additionally, a class may contain properties directly pointing to an instance of another. For example:

```c
methodmap OtherClass < JSON_Object {
    property YourClass instance {
        public get() { return view_as<YourClass>(this.GetObject("instance")); }
        public set(YourClass value) { this.SetObject("instance", view_as<JSON_Object>(value)); }
    }
}
```

After creating an instance of OtherClass and setting it's instance property to an existing YourClass instance, encoding will produce the expected nested JSON.

### Decoding
```
JSON_Object obj = json_decode("{\"myfloat\":73.570000,\"myobject\":[],\"myhandle\":null,\"mybool\":false,\"name\":\"my class\",\"myint\":9001}");
// obj now contains the setup as defined in the Creating a 'Class' example
// obj can also be coerced to an OtherClass so that its properties are available
OtherClass obj_coerced = view_as<OtherClass>(obj);
```

### Iterating Objects
You can iterate through the keys in a JSON_Object due to the fact that it's a `StringMap` and as such you can fetch a `StringMapSnapshot` from it.
```c
char key[JSON_BUFFER_LENGTH];
bool is_array = obj.IsArray;
StringMapSnapshot snap = obj.Snapshot();
for (int i = 0; i < obj.Length; ++i) {
    if (is_array) {
        obj.GetIndexString(key, sizeof(key), i);
    } else {
        snap.GetKey(i, key, sizeof(key));
        if (json_is_meta_key(key)) continue;  // skip meta-keys
    }

    JSON_CELL_TYPE type = obj.GetKeyType(key);
    // do whatever you want with type, key information
}
```

### Other Stuff
A key can be hidden from the encoder, but still used for data storage. This is useful for 'secret' information.
```c
obj.SetKeyHidden("my secret key", true);
```
In the case of needing to set integer-based keys (without using array-only functionality), you can use SetStringIndexed (and so on), which accepts an int as the key and converts it to a string internally.
```c
obj.SetStringIndexed(5, "hello");
```

## Testing
A number of common tests have been written [here](addons/sourcemod/scripting/json_test.sp).

## License
[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
