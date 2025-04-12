// @simul_efun

#define YML_SEQ_ELEMENT 1
#define YML_BLOCK_START 2
#define YML_KV_PAIR     3
#define YML_COMMENT     4
#define YML_MULTILINE   5
#define YML_UNKNOWN    -1

private nosave mapping YML = ([
  YML_SEQ_ELEMENT: "SEQUENCE",
  YML_BLOCK_START: "BLOCK_START",
  YML_KV_PAIR:     "KEY_VALUE_PAIR",
  YML_COMMENT:     "COMMENT",
  YML_MULTILINE:   "MULTILINE",
  YML_UNKNOWN:     "UNKNOWN",
]);

private string test_yaml = @YAML_TEST
# This is a comment
name: "Gesslar"
description: >
  This is a long
  wrapped string
  that should become
  one single line.
# This is another comment
inventory:
  - sword
  - shield
  - potion
stats:
  strength: 10
  agility: 7
  magic: 3
YAML_TEST;

private mixed* detect_line_type(string line, int indent);
private int first_non_space(string text);
varargs private mixed* parse_block(string* lines, int curr_indent, int indent);
private mixed yaml_parse_scalar(string val);

mixed yaml_decode(string text) {
  string* lines = explode(text, "\n");
  return parse_block(lines, 0)[0];
}

private mixed* detect_line_type(string line, int indent) {
  string key, val, item;

  _debug("  detect_line_type: %O\n", line);
  _debug("  detect_line_type: indent = %d\n", indent);

  line = ltrim(line);

  _debug("  detect_line_type: trimmed = %O\n", line);

  if(line == "" || sscanf(line, "%*s#%*s") == 2)
    return ({ YML_COMMENT });

  if(sscanf(line, "- %s", item) == 1)
    return ({ YML_SEQ_ELEMENT, item });

  if(sscanf(line, "%s: %*([>|])", key) == 2)
    return ({ YML_MULTILINE, key });

  if(sscanf(line, "%s: %s", key, val) == 2)
    return ({ YML_KV_PAIR, key, val });

  if(sscanf(line, "%s:", key) == 1)
    return ({ YML_BLOCK_START, key });

  return ({ YML_UNKNOWN });
}

public int first_non_space(string text) {
  for(int i = 0; i < strlen(text); i++) {
    if(text[i] != ' ') return i;
  }
  return strlen(text);
}

varargs private mixed* parse_block(string* lines, int curr_indent, int indent) {
  mixed result;
  int curr = 0;
  int sz = sizeof(lines);
  int is_sequence = 0;
  string item, key, val, line, trimmed;
  int indented, i;
  int block_type;
  mixed* nested;

  key = val = "";

  line = lines[0][(curr_indent * indent)..];
  block_type = detect_line_type(lines[0], curr_indent * indent)[0];

  _debug("Main block type: %s\n", YML[block_type]);

  if(block_type == YML_COMMENT)
    return parse_block(lines[1..], curr_indent, indent);
  else if(block_type == YML_SEQ_ELEMENT)
    result = ({}), is_sequence = 1;
  else
    result = ([]);

  _debug("Structure decided = %s\n", j(result));

  while(curr < sz) {
    mixed* line_info, value;
    int kind;

    _debug("> Result so far (start): %O\n", j(result));

    line = lines[curr];

    _debug(" parsing line %d: %O\n", curr, line);

    trimmed = trim(line);
    indented = first_non_space(line);

    _debug("  indented: %d\n", indented);
    _debug("  curr_indent: %d\n", curr_indent);
    _debug("  indent: %d\n", indent);
    _debug("  num spaces: %d\n", indented - curr_indent * indent);

    if(indented < curr_indent * indent) {
      _debug("  indented < curr_indent * indent\n");
      break;
    } else {
      _debug("  indented >= curr_indent * indent\n");
    }

    line_info = detect_line_type(trimmed, curr_indent * indent);
    kind = line_info[0];

    _debug("  line_info: %s [%s]\n", j(line_info), YML[kind]);

    switch(kind) {
      case YML_COMMENT: {
        curr++;
        break;
      }

      case YML_SEQ_ELEMENT: {
        result += ({ yaml_parse_scalar(line_info[1]) });
        curr++;
        break;
      }

      case YML_KV_PAIR: {
        string block;

        key = line_info[1];
        val = line_info[2];

        if(undefinedp(val) || val == "") {
          mixed* sub = parse_block(lines[curr + 1..], curr_indent + 1, indent);
          value = sub[0];
          curr += sub[1] + 1;
        } else {
          value = yaml_parse_scalar(val);
          curr++;
        }
        result[key] = value;
        break;
      }

      case YML_MULTILINE: {
        string block;

        key = line_info[1];
        block = "";
        i = curr + 1;
        while(i < sz && first_non_space(lines[i]) > indented) {
          block += trim(lines[i]) + "\n";
          i++;
        }
        value = block;
        result[key] = value;
        curr = i;
        break;
      }

      case YML_BLOCK_START:
        key = line_info[1];
        nested = parse_block(lines[curr + 1..], curr_indent + 1, indent);

        _debug("  typeof(result): %O\n", typeof(result));
        _debug("  typeof(nested): %O\n", typeof(nested));
        _debug("  key: %O\n", key);
        _debug("  nested: %O\n", j(nested));

        result[key] = nested[0];
        curr += nested[1] + 1;
        break;

      default:
        curr++;
        break;
    }

    _debug("< Result so far (end): %O\n", j(result));
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
