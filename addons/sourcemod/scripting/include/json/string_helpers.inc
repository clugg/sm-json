#if defined _json_string_helpers_included
    #endinput
#endif
#define _json_string_helpers_included

/**
 * Checks if a string starts with another string.
 *
 * @param haystack       String to check that starts with needle.
 * @param maxlen         Maximum size of string buffer.
 * @param needle         String to check that haystack starts with.
 * @return               True if haystack begins with needle, false otherwise.
 */
stock bool String_StartsWith(const char[] haystack, const char[] needle) {
    int haystack_length = strlen(haystack);
    int needle_length = strlen(needle);
    if (needle_length > haystack_length) return false;

    for (int i = 0; i < needle_length; ++i) {
        if (haystack[i] != needle[i]) return false;
    }

    return true;
}

/**
 * Checks if a string ends with another string.
 *
 * @param haystack       String to check that ends with needle.
 * @param maxlen         Maximum size of string buffer.
 * @param needle         String to check that haystack ends with.
 * @return               True if haystack ends with needle, false otherwise.
 */
stock bool String_EndsWith(const char[] haystack, const char[] needle) {
    int haystack_length = strlen(haystack);
    int needle_length = strlen(needle);
    if (needle_length > haystack_length) return false;

    for (int i = 0; i < needle_length; ++i) {
        if (haystack[haystack_length - needle_length + i] != needle[i]) return false;
    }

    return true;
}