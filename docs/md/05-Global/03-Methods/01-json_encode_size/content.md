---
Title: json_encode_size
---

Calculates the buffer size required to store an encoded JSON instance.

#- Arguments
- obj `JSON_Object`
- Object to encode.
- options `int`
- Bitwise combination of `JSON_ENCODE_*` options. *default:* `JSON_NONE`
- depth `int`
- The current depth of the encoder. *default:* `0`

#- Returns
- `int`
- The required buffer size.
