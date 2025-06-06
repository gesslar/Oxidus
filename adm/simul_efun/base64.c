/**
 * @file /adm/simul_efun/base64.c
 *
 * Simul-efun implementation of Base64 encoding and decoding functions.
 * Provides functionality to encode strings/buffers to Base64 and decode
 * Base64 strings back to their original form.
 *
 * Originally from http://lpmuds.net/smf/index.php?topic=1531.msg8465#msg8465
 * Modified by Gesslar@ThresholdRPG
 *
 * @created Unknown
 * @last_modified 2024-03-11
 *
 * @history
 * 2024-03-11 - Gesslar - Updated documentation and fixed bug in base64_decode.
 */

#include <simul_efun.h>

/**
 * Standard Base64 alphabet for encoding/decoding operations.
 *
 * @type {string}
 */
private nosave string b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * Encodes data into Base64 format.
 *
 * Takes either a string or buffer input and converts it to a Base64 encoded
 * string. When encoding strings, they are first converted to UTF-8 buffers.
 *
 * @param {string|buffer} source_str - Data to encode
 * @returns {string} Base64 encoded string
 * @errors If source_str is invalid type or empty
 * @example
 * string encoded = base64_encode("Hello World!");
 * // Returns: "SGVsbG8gV29ybGQh"
 */
string base64_encode(mixed source_str) {
  string *b;
  string r = "";
  int i;
  int n;
  int n1, n2, n3, n4;
  int slen, plen;
  buffer source;

  if(stringp(source_str)) {
    source = string_encode(source_str, "UTF-8");
  } else if(bufferp(source_str)) {
    source = source_str;
  } else {
    error("Invalid argument 1 to base64encode");
  }

  if(nullp(source_str) || !sizeof(source_str)) {
    error("Missing argument 1 to base64encode");
  }

  slen = sizeof(source);
  plen = slen % 3;
  b = explode(b64chars, "");

  for(i = 0; i < slen; i += 3) {
    n = source[i] << 16;

    if((i + 1) < slen)
      n += source[i + 1] << 8;

    if((i + 2) < slen)
      n += source[i + 2];

    n1 = (n >> 18) & 63;
    n2 = (n >> 12) & 63;
    n3 = (n >> 6) & 63;
    n4 = n & 63;

    r += b[n1] + b[n2];

    if((i + 1) < slen)
      r += b[n3];

    if((i + 2) < slen)
      r += b[n4];
  }

  if(plen > 0)
    for(; plen < 3; plen++) r += "=";

  return r;
}

/**
 * Decodes a Base64 encoded string.
 *
 * Converts a Base64 encoded string back to its original form. Handles padding
 * and whitespace in the input string. The result is decoded as UTF-8.
 *
 * @param {string} source - Base64 encoded string to decode
 * @returns {string} Decoded string in UTF-8 encoding
 * @errors If source is null/empty or contains invalid Base64 characters
 * @example
 * string decoded = base64_decode("SGVsbG8gV29ybGQh");
 * // Returns: "Hello World!"
 */
string base64_decode(string source) {
  string *b;
  string f = "";
  int i, j;
  int c;
  int n;
  int plen = 0;
  buffer result;

  if(nullp(source) || !strlen(source))
    error("Missing argument 1 to base64decode");

  b = explode(b64chars, "");

  for(i = 0; i < sizeof(source); i++) {
    c = strsrch(b64chars, source[i]);
    if(c == -1) {
      // not found
      if(source[i] == 61) {
        // We found an "=", meaning we hit the padding.
        // For decoding purposes, "A" is a zero pad value here.
        f += "A";
        plen++;
        continue;
      } else if(source[i] == 32 || source[i] == 10 || source[i] == 9 || source[i] == 13) {
        // We found whitespace, skip it
        continue;
      } else {
        // invalid character
        return "Invalid input.";
      }
    } else {
      f += b[c];
    }
  }

  if(sizeof(f) % 4)
    return "Invalid input.";

  result = allocate_buffer(sizeof(f) / 4 * 3);
  j = 0;
  for(i = 0; i < sizeof(f); i += 4) {
    c = strsrch(b64chars, f[i]);
    n = c << 18;
    c = strsrch(b64chars, f[i+1]);
    n += c << 12;
    c = strsrch(b64chars, f[i+2]);
    n += c << 6;
    c = strsrch(b64chars, f[i+3]);
    n += c;

    result[j++] = (n >> 16) & 0xFF;
    result[j++] = (n >> 8) & 0xFF;
    result[j++] = n & 0xFF;
  }

  return string_decode(result, "UTF-8");
}
