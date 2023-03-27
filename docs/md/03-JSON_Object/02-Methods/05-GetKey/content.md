---
Title: GetKey
---

Gets the key stored at an index. If an array, will convert the index to its string value. If an array, will return false if the index is not between [0, length].

#- Arguments
- index `int`
- Index of key.
- value `char[]`
- Buffer to store key at.
- max_size `int`
- Maximum size of value buffer.

#- Returns
- `bool`
- True on success, false otherwise.
