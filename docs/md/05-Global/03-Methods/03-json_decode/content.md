---
Title: json_decode
---

Decodes a JSON string into a JSON instance.

#- Arguments
- buffer `const char[]`
- Buffer to decode.
- options `int`
- Bitwise combination of `JSON_DECODE_*` options. *default:* `JSON_NONE`
- &pos `int`
- Current position of the decoder as bytes offset into the buffer. *default:* `0`
- depth `int`
- Current nested depth of the decoder. *default:* `0`

#- Returns
- `JSON_Object`
- JSON instance or null if decoding failed becase the buffer didn't contain valid JSON.

#- Throws
- 
- If the buffer does not contain valid JSON, an error will be thrown.
