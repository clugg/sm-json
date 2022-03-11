---
Title: json_encode
---

Encodes a JSON instance into its string representation.

#- Arguments
- obj `JSON_Object`
- Object to encode.
- output `char[]`
- String buffer to store output.
- max_size `int`
- Maximum size of string buffer.
- options `int`
- Bitwise combination of `JSON_ENCODE_*` options. *default:* `JSON_NONE`
- depth `int`
- The current depth of the encoder. *default:* `0`

