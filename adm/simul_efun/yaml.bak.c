// @simul_efun

private string test_yaml = @YAML_TEST
name: "Gesslar"
description: >
  This is a long
  wrapped string
  that should become
  one single line.
inventory:
  - sword
  - shield
  - potion
stats:
  strength: 10
  agility: 7
  magic: 3
YAML_TEST;

private int detect_block_type(string* lines);
private mixed* parse_block(string* lines, int start, int indent);
private mixed yaml_parse_scalar(string val);

mixed yaml_decode(string text) {
  string* lines = explode(text, "\n");
  return parse_block(lines, 0, 0)[0];
}

#define YML_SEQ        1
#define YML_MAP        2
#define YML_KV_PAIR    3
#define YML_COMMENT    4
#define YML_MULTILINE  5
#define YML_UNKNOWN   -1

private int detect_block_type(string* lines) {
  int curr = 0, sz = sizeof(lines);
  string line;

  printf(" detect_block_type: sz = %d\n", sz);

  if(curr >= sz)
    return YML_UNKNOWN;

  while(curr < sz) {
    line = lines[curr];

    printf("  Scanning line %d: %O\n", curr, line);
    printf("   seq test: %d\n", sscanf(line, "- %*s"));
    printf("   map test: %d\n", sscanf(line, "%*s: %*s"));
    printf("   comment test: %d\n", sscanf(line, "%*s#%*s"));

    if(line == "")
      continue;

    if(curr == 0 && sscanf(line, "%*s#%*s") == 2)
      return YML_COMMENT;

    if(sscanf(line, "- %*s") == 1)
      return YML_SEQ;

    if(sscanf(line, "%*s: %*s") >= 1)
      return YML_MAP;

    curr++;
  }

  return YML_UNKNOWN;
}

private mixed* parse_block(string* lines, int start, int indent_level) {
  mixed result;
  string trimmed, item, key, val;
  int curr = start;
  int is_sequence;
  int next, line_indent;
  int main_block_type;

  if(!sizeof(lines) || start >= sizeof(lines))
    return null;

  // Initialise the current status markers
  is_sequence = 0;

  printf("Detecting main block type...\n");

  // Determine the main block type for this document.
  main_block_type = detect_block_type(lines);
  if(main_block_type == YML_SEQ) {
    is_sequence = 1;
    result = ({});
    printf("Main block type: SEQUENCE\n");
  } else if(main_block_type == YML_MAP) {
    is_sequence = 0;
    result = ([]);
    printf("Main block type: MAP\n");
  } else if(main_block_type == YML_COMMENT) {
    printf("Main block type: COMMENT\n");
    return null;
  } else {
    printf("Main block type: UNKNOWN\n");
    return null;
  }

  next = start;

  while(curr < sizeof(lines)) {
    string line = lines[curr];
    int block_type;
    string leading_spaces;
    mixed inner_result;

    printf("line %d = %O\n", curr, line);

    block_type = detect_block_type(lines[curr..]);

    if(block_type == YML_UNKNOWN) {
      printf("  Found unknown block type\n");
      curr++;
      continue;
    }

    printf(" block_type = %d\n", block_type);

    if(block_type == YML_COMMENT) {
      printf("  Found comment\n");
      curr++;
      continue;
    }

    if(block_type == YML_SEQ) {
      is_sequence = 1;
      inner_result = ({});
    } else if(block_type == YML_MAP) {
      is_sequence = 0;
      inner_result = ([]);
    } else {
      // we're a key-value pair
      is_sequence = 0;
      inner_result = ([]);
    }

    printf(" Result so far: %O\n", j(result));

    // Identify the indentation level of the current line
    // and check if it is less than the expected indentation
    // level. If it is, we are done with this block.
    sscanf(line, "%(\\s*)%*s", leading_spaces);

    line_indent = strlen(leading_spaces);

    printf(" line_indent = %d, indent = %d\n", line_indent, indent_level);

    if(line_indent < indent_level) {
      printf("  Found end of block\n");
      break;
    } else {
      printf("  Found line with indent %d\n", line_indent);
    }

    trimmed = ltrim(line);

    if(is_sequence) {
      string seq_label;

      // This should be the start of a sequence item, let's see if
      // it is and if so, let's grab the label;
      if(sscanf(trimmed, "%s:", item) != 1) {
        // Uhm... this is not a sequence item label, something
        // went wrong. BAIL!
        printf("  Found non-sequence item label: %O\n", trimmed);
        return null;
      }

      // Whew! We found a sequence item label. Let's grab it!
      printf("  Found sequence item label: %O\n", item);

      seq_label = item;
      inner_result = ({});
      curr++;
      line = lines[curr];
      trimmed = ltrim(line);


      if(item == "") {
        mixed* nested = parse_block(lines, curr + 1, line_indent + 1);
        inner_result = ({ nested[0] });
        curr = nested[1];
        continue;
      } else {
        result += ({ yaml_parse_scalar(item) });
        curr++;
        continue;
      }
    }

    if(!is_sequence && sscanf(trimmed, "%s: %s", key, val) >= 1) {
      mixed value;

      if(!undefinedp(val)) {
        if(val == ">" || val == "|") {
          string block = "";
          string next_line;

          next = curr + 1;
          while(next < sizeof(lines)) {
            next_line = lines[next];
            if(strlen(next_line) - strlen(ltrim(next_line)) <= line_indent)
              break;
            block += ltrim(next_line) + (val == ">" ? " " : "\n");
            next++;
          }
          value = val == ">" ? rtrim(block) : block;
          result[key] = value;
          curr = next;
        } else {
          value = yaml_parse_scalar(val);
          result[key] = value;
          curr++;
        }
      } else {
        mixed* nested;
        printf("   nested line = %O\n", lines[curr]);
        nested = parse_block(lines, curr + 1, line_indent + 1);
        value = nested[0];
        result[key] = value;
        curr = nested[1];
      }
      printf("   key: %s, value: %s\n", j(key), j(value));
      printf("   typeof key: %O, value: %O\n", typeof(key), typeof(value));
      printf("   > Result so far: %O\n", j(result));
      continue;
    }
    curr++;
  }

  return ({ result, curr });
}

private mixed yaml_parse_scalar(string val) {
  int intval;
  float fval;

  if(val == "null" || val == "~") return ([])[0];
  if(val == "true") return 1;
  if(val == "false") return 0;
  if(sscanf(val, "%d", intval)) return intval;
  if(sscanf(val, "%f", fval)) return fval;
  if((val[0] == '"' && val[<1] == '"') || (val[0] == '\'' && val[<1] == '\''))
    return val[1..<2];
  return val;
}

public runit() {
  mixed result = yaml_decode(test_yaml);
  printf("%O\n", result);
  return result;
}
