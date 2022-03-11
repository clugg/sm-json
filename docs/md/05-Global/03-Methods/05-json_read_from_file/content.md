---
Title: json_read_from_file
---

Reads and decodes the contents of a JSON file.

#- Arguments
- path `const char[]`
- Path of file to read from.
- options `int`
- Options to pass to `json_decode`. *default:* `JSON_NONE`

#- Returns
- `JSON_Object`
- The decoded object on success, null otherwise.
