---
Title: json_write_to_file
---

Encodes the object with the options provided and writes the output to the file at the path specified.

#- Arguments
- obj `JSON_Object`
- Object to encode/write to file.
- path `const char[]`
- Path of file to write to.
- options `int`
- Options to pass to `json_encode`. *default:* `JSON_NONE`

#- Returns
- `bool`
- True on success, false otherwise.
