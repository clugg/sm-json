---
Title: Merge
---

Merges in the entries from the specified object, optionally replacing existing entries with the same key.

#- Arguments
- from `JSON_Object`
- Object to merge entries from.
- options `int`
- Bitwise combination of `JSON_MERGE_*` options. *default:* `JSON_MERGE_REPLACE`

#- Returns
- `bool`
- True on success, false otherwise.

#- Throws
- 
- If the object being merged is an array, an error will be thrown.
