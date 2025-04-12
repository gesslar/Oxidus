#include <simul_efun.h>

/**
 * @file /adm/simul_efun/lpml.c
 *
 * YAML-flavoured parser and decoder for LPC. Provides functionality to parse
 * LPML format into LPC data structures (mappings and arrays).
 *
 * @created 2025-04-06 - Gesslar
 * @last_modified 2025-04-06 - Gesslar
 *
 * @history
 * 2025-04-06 - Gesslar - Created
 */

#define LP_SEQ_ELEMENT          "LP_SEQ_ELEMENT"
#define LP_BLOCK_START          "LP_BLOCK_START"
#define LP_KV_PAIR              "LP_KV_PAIR"
#define LP_COMMENT              "LP_COMMENT"
#define LP_BLANK                "LP_BLANK"
#define LP_MULTILINE_JOIN       "LP_MULTILINE_JOIN"
#define LP_MULTILINE_PRESERVE   "LP_MULTILINE_PRESERVE"
#define LP_MERGE_START          "LP_MERGE_START"
#define LP_MERGE_INLINE         "LP_MERGE_INLINE"
#define LP_UNKNOWN              "LP_UNKNOWN"

#define LP_ML_STRIP             "-"
#define LP_ML_KEEP              "+"
#define LP_ML_CLIP              ([])[0]

#define PAGE_TITLE  0
#define PAGE_SOURCE 1
#define PAGE_RESULT 2

private mixed *detect_line_type(string line);
private int first_non_space(string text);
varargs private mixed *parse_block(string *lines, mixed *pages, int curr_indent, int indent);
private mixed lpml_parse_scalar(string val, mixed *pages);
private string remove_inline_comment(string line);
private mixed lpml_inherit(string text, mixed *pages);
private mixed *paginate_document(string text);
private string multiline_chomp(string text, string kind, string mode);

/**
 * Decodes a LPML string into LPC data structures.
 *
 * This function serves as the main entry point for LPML parsing, converting
 * a LPML formatted string into corresponding LPC mappings and arrays.
 *
 * @public
 * @param {string} text - The LPML text to decode
 * @returns {mixed} The parsed LPC data structure (mapping or array)
 * @example
 * mapping data = lpml_decode("key: value\nlist:\n  - item1\n  - item2");
 */
public mixed lpml_decode(string text) {
  mixed *pages = paginate_document(text);
  int i, sz;

  for(i = 0, sz = sizeof(pages); i< sz; i++) {
    mixed  *page;
    string *lines;
    string *page_result;

    page = pages[i];

    // We need to always include a leading line feed, because
    // explode removes the first one, if present. Don't worry
    // about it. 😏
    lines = explode("\n"+page[PAGE_SOURCE], "\n");
    page_result = parse_block(lines, pages, 0)[0];

    pages[i][PAGE_RESULT] = page_result;
  }

  return pages[<1][PAGE_RESULT];
}

/**
 * Divides a LPML document into pages based on document separators.
 *
 * Pages in LPML are separated by "---" on a line by itself. Each page can
 * optionally have a title prefixed with "#@" on the first line. This function
 * splits the document into pages and prepares them for parsing.
 *
 * @private
 * @param {string} text - The complete LPML document text
 * @returns {mixed*} An array of page data, each containing title, source, and result
 */
private mixed *paginate_document(string text) {
  string *pages;
  mixed *result;
  int i, sz;

  pages = explode(text, "---\n");
  sz = sizeof(pages);
  result = allocate(sz);

  for(; i < sz; i++) {
    string page = pages[i];
    int first_eol;
    string page_title;

    // Append a newline to the end of the page if it doesn't already have one.
    page = page[<1] == '\n' ? page : page + "\n";

    first_eol = strsrch(page, "\n");
    if(sscanf(page[0..first_eol-1], "#@%s", page_title) == 1) {
      page = page[first_eol+1..];
    } else {
      page_title = sprintf("%d", i);
    }

    result[i] = ({ page_title, page, ([])[0] });
  }

  return result;
}

/**
 * Analyzes a line of LPML text to determine its type and structure.
 *
 * This function examines a line of LPML and identifies what kind of construct it
 * represents (e.g., sequence element, key-value pair, comment, etc.). It returns
 * an array containing the line type and any relevant extracted values.
 *
 * @private
 * @param {string} line - The line of LPML text to analyze
 * @param {int} indent - The current indentation level
 * @returns {mixed*} An array where the first element is the line type constant
 *                  and subsequent elements contain extracted values if applicable
 */
private mixed *detect_line_type(string line) {
  string key, val, item;

  line = ltrim(line);

  if(line == "")
    return ({ LP_BLANK });

  // Return comment type for empty lines or lines starting with #
  if(pcre_match(line, "^\\s*#"))
    return ({ LP_COMMENT });

  // Handle merge directives for inheritance/composition
  if(sscanf(line, "- <<: %s", item) == 1)
    return ({ LP_MERGE_INLINE, item });
  if(sscanf(line, "<<: %s", item) == 1)
    return ({ LP_MERGE_INLINE, item });
  if(pcre_match(line, "^(?:(- )?)<<:$"))
    return ({ LP_MERGE_START });

  // Handle sequence elements (list items starting with -)
  if(sscanf(line, "- %s", item) == 1)
    return ({ LP_SEQ_ELEMENT, item });

  { // Multiline string modes block
    string mode;

    // Check for preserved multiline (|) or folded multiline (>)
    // Optional chomping indicators can follow (-, +, or none/clip)
    if(sscanf(line, "%s: |%s", key, mode) >= 1)
      return ({ LP_MULTILINE_PRESERVE, key, mode });

    if(sscanf(line, "%s: >%s", key, mode) >= 1)
      return ({ LP_MULTILINE_JOIN, key, mode });
  }

  // Standard key-value pairs (key: value)
  if(sscanf(line, "%s: %s", key, val) == 2)
    return ({ LP_KV_PAIR, key, val });

  // Block scalar indicators (key: followed by indented block)
  if(sscanf(line, "%s:", key) == 1)
    return ({ LP_BLOCK_START, key });

  // Anything not matching above patterns is considered unknown
  return ({ LP_UNKNOWN, line });
}

/**
 * Finds the position of the first non-space character in a string.
 *
 * @private
 * @param {string} text - The string to examine
 * @returns {int} The position of the first non-space character, or
 *               the length of the string if it contains only spaces
 */
private int first_non_space(string text) {
  int x, len = strlen(text);

  while(text[x++] == ' ' && x < len);

  return x - 1;
}

private int find_next_line(string *lines, int curr) {
  int sz = sizeof(lines);

  while(++curr < sz) {
    string *line_info = detect_line_type(lines[curr]);
    // printf("Testing line %d of %d: %O\n", curr, sz, lines[curr]);
    if(line_info[0] != LP_BLANK && line_info[0] != LP_COMMENT)
      return curr;
  }

  return curr;
}

/**
 * Recursively parses a block of LPML text into LPC data structures.
 *
 * This function processes multiple lines of LPML text, handling indentation
 * and nested structures to build the appropriate LPC data structures.
 *
 * @param {string*} lines - Array of LPML text lines to parse
 * @param {int} curr_indent - The current indentation level
 * @param {int} [indent=0] - The indentation step size
 * @returns {mixed*} An array containing the parsed result and the number of lines processed
 * @private
 */
// Recursive block parsing - this is the heart of the LPML parser
varargs private mixed *parse_block(string *lines, mixed *pages, int curr_indent, int indent) {
  int curr;
  int sz = sizeof(lines);
  mixed result;
  string line, *line_info, line_type;

  if(!sz)
    return ({ ([]), 0 });

  // Find the first non-comment line, we use -1 to start from the beginning
  // printf("Lines 0 = %O\n", lines[0]);
  curr = find_next_line(lines, -1);
  // printf("\n[[  Next line number = %d  ]]\n\n", curr);
  if(curr >= sz)
    return ({ ([]), 0 });

  // We need to determine the shape of our block. These will get overwritten during
  // the loop.
  line = lines[curr];
  line_info = detect_line_type(line);

  // If it's a block, initialize the result as a mapping
  if(line_info[0] == LP_BLOCK_START)
    result = ([]);
  // If it's a sequence element, initialize as an array
  else if(line_info[0] == LP_SEQ_ELEMENT)
    result = ({});
  // For now, at least, let's just be an error. We will
  // update this later to allow primitives. Maybe. Somehow.
  else
    // error("Invalid block type: " + line_info[0]);
    result = ([]);

  do {
    int line_indent;
    string key;
    mixed value;

    // printf("Line %d = %s\n", curr, lines[curr]);

    line = lines[curr];
    line = rtrim(remove_inline_comment(line));
    line_info = detect_line_type(line);
    line_type = line_info[0];
    key = line_info[1];
    line_indent = first_non_space(line);

    // printf("Parsing block at line %d: %s\n", curr, line_info[0]);
    // printf("Current line: %s\n", line);

    // End of block - return the accumulated result
    // printf("Comparing indentation: detected %d < passed %d\n", line_indent, indent);
    if(line_indent < indent) {
      // printf("New block at line %d\n", curr);
      // printf("%O\n", ({ result, curr }));
      break;
      // return ({ result, curr });
    } else if(line_indent > indent) {
      // printf("Indentation increased at line %d from %d to %d\n", curr, indent, line_indent);
      curr_indent++;
      indent = line_indent;
    }

    switch(line_type) {
      case LP_BLOCK_START: {
        mixed *sub;

        // printf("\n");
        // printf("Key %O leads to line: %O\n", key, lines[curr+1]);
        sub = parse_block(lines[curr+1..], pages, curr_indent, line_indent);
        // printf("Result from parse_block = %O\n", sub);
        value = sub[0];
        curr += sub[1];
        // printf("We ended key %O at line %d\n", key, sub[1]);

        // printf("Adding to key %O`: %O\n", key, json_encode(value));

        result[key] = value;
        break;
      }

      // Complex parsing logic for handling merge directives
      case LP_MERGE_START: {
        // Tracks lines in the current merge block for processing
        string *merge_lines = ({});

        // Process each line that's more indented than current level
        while(++curr < sz && first_non_space(lines[curr]) > indent) {
          string val;
          mixed merge_result;

          // Only process lines that start with "- " (list items)
          if(sscanf(trim(lines[curr]), "- %s", val) != 1)
            continue;

          merge_result = lpml_inherit(val, pages);

          // Handle type coercion based on context:
          // Array + Array = Concatenate arrays
          // Array + Non-Array = Append non-array as element
          // Otherwise = Combine mappings
          if(typeof(result) == T_ARRAY && typeof(merge_result) == T_ARRAY) {
            result += merge_result;
          } else if(typeof(result) == T_ARRAY && typeof(merge_result) != T_ARRAY) {
            result += ({ merge_result });
          } else {
            result += merge_result;
          }
        }

        break;
      }

      case LP_MERGE_INLINE: {
        mixed pre;

        pre = lpml_parse_scalar(line_info[1], pages);

        if(!stringp(pre) && !pointerp(pre))
          error("Invalid merge inline value: " + pre);

        if(stringp(pre))
          pre = ({ pre });

        foreach(string inc in pre) {
          mixed merge_result;

          if(!stringp(inc))
            error("Invalid merge inline value: " + inc);

          merge_result = lpml_inherit(inc, pages);
          if(typeof(result) == T_ARRAY && typeof(merge_result) == T_ARRAY) {
            result += merge_result;
          } else if(typeof(result) == T_ARRAY && typeof(merge_result) != T_ARRAY) {
            result += ({ merge_result });
          } else {
            result += merge_result;
          }
        }

        break;
      }

      case LP_SEQ_ELEMENT: {
        // We might have another array here, so, let's see!
        string trimmed = ltrim(line);
        mixed *subdetected = detect_line_type(trimmed[2..]);
        string item_type = subdetected[0];
        string item = subdetected[1];
        mixed *sub;

        if(item_type == LP_SEQ_ELEMENT) {
          string *seq_lines = ({ trimmed[2..] });
          int i = curr + 1;

          while(i < sz && first_non_space(lines[i]) > indent) {
            seq_lines += ({ trim(lines[i]) });
            i++;
          }

          sub = parse_block(seq_lines, pages, curr_indent, indent);
          value = sub[0];
          curr += sub[1];
        } else {
          value = lpml_parse_scalar(item, pages);
        }

        result += ({ value });

        break;
      }

      // Handle key-value pairs (key: value)
      case LP_KV_PAIR: {
        string block;
        string val = line_info[2];

        if(undefinedp(val) || val == "") {
          mixed* sub = parse_block(lines[curr + 1..], pages, curr_indent, indent);
          value = sub[0];
          curr += sub[1];
        } else {
          value = lpml_parse_scalar(val, pages);
        }

        result[key] = value;
        break;
      }

      case LP_MULTILINE_JOIN:
      case LP_MULTILINE_PRESERVE: {
        string block = "";
        string mode = line_info[2];
        string chunk;
        string *inner_info;
        string chunk_type;
        int i = curr + 1;

        do {
          line = lines[i];
          chunk = ltrim(line, " ");
          inner_info = detect_line_type(chunk);
          chunk_type = inner_info[0];

          // printf("> Chunk type is [%s]: %O\n", chunk_type, chunk);

          if(chunk_type == LP_BLANK ||
             chunk_type == LP_COMMENT ||
             chunk_type == LP_UNKNOWN) {
            if(line_type == LP_MULTILINE_PRESERVE) {
              chunk += "\n";
            } else {
              if(chunk_type == LP_BLANK) {
                chunk += " ";
              }
            }
          } else {
            break;
          }

          block += chunk, i++;
        } while(i < sz && first_non_space(lines[i]) > indent);

        // Trim leading blank lines only if mode isn't KEEP
        if(mode != LP_ML_KEEP)
          block = ltrim(block, "\n");

        result[key] = multiline_chomp(block, line_type, mode);

        break;
      }
    }
  } while((curr = find_next_line(lines, curr)) < sz);

  // printf("@@@ Returning at line [%2d] of [%2d] - %O\n", curr, sz, curr == sz ? "DONE" : lines[curr]);
  // printf("@@@ %O\n", ({ result, curr }));
  return ({ result, curr });
}

/**
 * Processes multiline text according to specified chomping rules.
 *
 * This function handles LPML's multiline string handling, applying chomping
 * (trailing whitespace handling) based on the provided mode:
 * - STRIP (-): Remove all trailing whitespace
 * - KEEP (+): Keep all trailing whitespace and add a newline
 * - CLIP (default): Context-dependent handling based on kind
 *
 * @private
 * @param {string} text - The multiline text to process
 * @param {string} kind - The multiline mode (LP_MULTILINE_JOIN or LP_MULTILINE_PRESERVE)
 * @param {string} mode - The chomping mode (LP_ML_STRIP, LP_ML_KEEP, or LP_ML_CLIP)
 * @returns {string} The processed multiline text with appropriate chomping applied
 */
private string multiline_chomp(string text, string kind, string mode) {
  // For joined multiline (>), handle chomping modes:
  // STRIP (-): Remove all trailing whitespace
  // KEEP (+): Keep all whitespace and add newline
  // CLIP (default): Remove trailing space but keep one newline
  if(kind == LP_MULTILINE_JOIN)
    text =   (mode == LP_ML_STRIP) ? rtrim(text)
           : (mode == LP_ML_KEEP)  ? text + "\n"
           : rtrim(text) + "\n"; // default LP_ML_CLIP

  // For preserved multiline (|), handle chomping modes:
  // STRIP (-): Remove all trailing newlines
  // KEEP (+): Keep all newlines and add one more
  // CLIP (default): Keep existing newlines as is
  if(kind == LP_MULTILINE_PRESERVE)
    text =   (mode == LP_ML_STRIP) ? rtrim(text, "\n")
           : (mode == LP_ML_KEEP)  ? text + "\n"
           : text; // default LP_ML_CLIP

  return text;
}

/**
 * Parses a LPML scalar value into its appropriate LPC type.
 *
 * This function handles conversion of LPML scalar values into appropriate
 * LPC data types, including numbers, booleans, strings, null values, and
 * inline arrays/mappings.
 *
 * @param {string} val - The LPML scalar value to parse
 * @returns {mixed} The parsed LPC representation of the scalar
 * @private
 */
// Handle inline arrays and mappings in scalar values
private mixed lpml_parse_scalar(string val, mixed *pages) {
  // Type conversion variables
  int intval;
  float fval;

  // Process inline array notation [item1, item2, ...]
  if(val[0] == '[' && val[<1] == ']') {
    // Transform each item into a sequence element for consistent parsing
    string* parts = map(explode(val[1..<2], ","), (: trim($1) :));
    string* seq_lines = map(parts, (: "- " + $1 :));  // Convert to LPML sequence format

    return parse_block(seq_lines, pages, 0, 0)[0];
  }

  // Handle inline mapping notation {key1: val1, key2: val2, ...}
  // Directly constructs a mapping from the inline format
  if(val[0] == '{' && val[<1] == '}') {
    mapping result = ([]);
    string* pairs = explode(val[1..<2], ",");
    foreach(string pair in pairs) {
      string k, v;
      if(sscanf(trim(pair), "%s:%s", k, v) == 2)
        result[trim(k)] = lpml_parse_scalar(trim(v), pages);
    }
    return result;
  }

  // Null-like
  if(val == "null" || val == "~" || val == "undefined") return ([])[0];

  // Boolean-ish
  if(val == "true" || val == "yes") return 1;
  if(val == "false" || val == "no") return 0;

  // Numbers (int, float, hex)
  if(sscanf(val, "%d", intval) && !nullp(val)) return intval;
  if(sscanf(val, "%f", fval) && !nullp(val)) return fval;
  if(strlen(val) > 2 && val[0..1] == "0x" && sscanf(val, "%x", intval) && !nullp(val)) return intval;

  // Quoted string
  if((val[0] == '"' && val[<1] == '"') || (val[0] == '\'' && val[<1] == '\''))
    return val[1..<2];

  // Fallback as-is
  return val;
}

/**
 * Removes inline comments from a LPML line.
 *
 * This function identifies and removes comment portions of a line (beginning with #),
 * while being careful not to remove # characters that are within quoted strings.
 *
 * @param {string} line - The line to process
 * @returns {string} The line with any inline comments removed
 * @private
 */
// Quote-aware comment detection
private string remove_inline_comment(string line) {
  int pound_pos, left_quote, right_quote;
  string quoted;

  // Early return if no comment marker found
  pound_pos = strsrch(line, "#");
  if(pound_pos == -1)
    return line;

  // Check if comment marker is inside quotes
  // This preserves # characters that are part of string content
  if( (left_quote = strsrch(line, "\"")) != -1 ||
      (left_quote = strsrch(line, "'"))  != -1) {

    right_quote = strsrch(line, line[left_quote], -1);

    // If no matching quote found, assume incomplete quoted string
    if(right_quote == -1)
      return line;

    // Return up to the closing quote
    return line[0..right_quote];
  }

  // Return everything before the comment marker
  return line[0..pound_pos - 1];
}

/**
 * Loads and parses a LPML file for inheritance/inclusion.
 *
 * This function handles the inclusion of external LPML files, allowing for
 * composition and reuse of LPML configurations through inheritance.
 *
 * @private
 * @param {string} file - Path to the LPML file to inherit
 * @param {mixed*} pages - Array of parsed LPML pages
 * @returns {mixed} The parsed contents of the inherited file
 * @errors If the file does not exist or cannot be read
 */
private mixed lpml_inherit(string file, mixed *pages) {
  string text, *lines;
  mixed result;

  if(file[0] == '/') {
    if(!file_exists(file))
      error("No such inherited file: " + file);

    text = read_file(file);
    if(!text)
      error("Could not read inherited file: " + file);
  } else {
    int i, sz;

    for(i = 0, sz = sizeof(pages); i < sz; i++) {
      string page_path = pages[i][PAGE_TITLE];
      if(page_path == file) {
        text = pages[i][PAGE_SOURCE];
        break;
      }
    }

    if(!text)
      error("No such inherited LPML page: " + file);
  }

  lines = explode(text, "\n");
  result = parse_block(lines, pages, 0)[0];

  return result;
}
