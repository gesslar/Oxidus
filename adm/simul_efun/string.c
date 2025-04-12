#include <simul_efun.h>

/**
 * @simul_efun append
 * @description Appends a string to another string if it is not already there.
 *              If the string is already present, the original string is returned.
 * @param {string} source - The string to append to.
 * @param {string} to_append - The string to append.
 * @returns {string} - The original string with the appended string if it was not
 *                     already present.
 */
string append(string source, string to_append) {
    int source_len, to_append_len;

    source_len = strlen(source);
    to_append_len = strlen(to_append);

    if(source[<to_append_len..] != to_append) {
        source += to_append;
    }

    return source;
}

/**
 * @simul_efun prepend
 * @description Prepends a string to another string if it is not already there.
 *              If the string is already present, the original string is returned.
 * @param {string} source - The string to prepend to.
 * @param {string} to_prepend - The string to prepend.
 * @returns {string} - The original string with the prepended string if it was not
 *                     already present.
 */
string prepend(string source, string to_prepend) {
    int source_len, to_prepend_len;

    source_len = strlen(source);
    to_prepend_len = strlen(to_prepend);

    if(source[0..to_prepend_len-1] != to_prepend) {
        source = to_prepend + source;
    }

    return source;
}

/**
 * @simul_efun chop
 * @description Chops a substring off the end or beginning of another string if
 *              it is present. If the substring is not present, the original
 *              string is returned.
 * @param {string} str - The string to chop from.
 * @param {string} sub - The substring to chop.
 * @param {int} [dir=-1] - The direction to chop: 1 for left, -1 for right.
 * @returns {string} - The string with the substring chopped off if it was present.
 */
varargs string chop(string str, string sub, int dir) {
    int sub_len;

    if(nullp(str)) error("chop: Missing argument 1 for chop");
    if(nullp(sub)) error("chop: Missing argument 2 for chop");

    if(dir != -1)
        dir = 1;

    sub_len = strlen(sub);
    if(dir == 1) {
        if(sub[0..sub_len-1] == sub) {
            str = str[0..sub_len];
        }
    } else if(dir == -1) {
        if(str[<sub_len..] == sub) {
            str = str[0..<sub_len+1];
        }
    }

    return str;
}

/**
 * @simul_efun extract
 * @description Extracts a substring from a string.
 * @param {string} str - The string to extract from.
 * @param {int} from - The starting position to extract from.
 * @param {int} [to] - The ending position to extract to.
 * @returns {string} - The extracted substring.
 */
varargs string extract(string str, int from, int to) {
    if(nullp(to)) return str[from ..];
    else return str[from .. to];
}

/**
 * @simul_efun no_ansi
 * @description Returns a string with all colour removed.
 * @param {string} str - The string to remove colour from.
 * @returns {string} - The string without colour.
 */
string no_ansi(string str) {
    return COLOUR_D->substitute_colour(str, "plain");
}

/**
 * @simul_efun simple_list
 * @description Returns a string that is a simple list of the elements of an array,
 *              joined by a conjunction.
 * @param {string[]} arr - The array to make a list from.
 * @param {string} [conjunction="and"] - The word to join the last two elements
 *                                       of the list.
 * @returns {string} - The simple list string.
 */
varargs string simple_list(string *arr, string conjunction) {
  assert_arg(pointerp(arr) && uniform_array(arr, T_STRING), 1, "Invalid or missing array.");

  conjunction = conjunction || "and";

  if(sizeof(arr) == 1)
    return arr[0];
  else if(sizeof(arr) == 2)
    return arr[0] + " " + conjunction + " " + arr[1];
  else
    return implode(arr[0..<2], ", ") + ", " + conjunction + " " + arr[<1];
}

/**
 * @simul_efun substr
 * @description Returns a substring of a string, starting from 0 and ending at the
 *              first occurrence of another string within it. If the reverse flag
 *              is set, the substring will start at the last occurrence of the
 *              substring within the string.
 * @param {string} str - The string to extract from.
 * @param {string} sub - The substring to extract to.
 * @param {int} [reverse=0] - If set, the substring will start at the last occurrence.
 * @returns {string} - The extracted substring.
 */
varargs string substr(string str, string sub, int reverse) {
    int sub_len;
    int pos;

    if(nullp(str)) error("substr: Missing argument 1 for substr");
    if(nullp(sub)) error("substr: Missing argument 2 for substr");

    reverse = !!reverse;
    sub_len = strlen(sub);
    pos = strsrch(str, sub, reverse);

    if(pos == -1) return "";

    if(reverse)
        return str[pos+sub_len..];
    else
        return str[0..pos-1];
}

// This simul_efun blatantly (but lovingly) ripped off from:
// https://github.com/fluffos/dead-souls/blob/09a74caa87d8aadbfe303c294cc0bebb25fdb4db/lib/secure/sefun/strings.c#L104C1-L242C1
/**
 * @simul_efun from_string
 * @description Converts a string representation of an LPC value to the
 *              corresponding LPC value.
 * @param {string} str - The string to convert.
 * @param {int} [flag=0] - If set, returns an array with the value and the
 *                         remaining string.
 * @returns {mixed} - The LPC value represented by the string.
 */
varargs mixed from_string(string str, int flag) {
    mixed *ret = ({ 0, "" });

    if(!str || !sizeof(str) || str == "")
        return 0;

    if((str = trim(str)) == "")
        return 0;

    str = implode(map(explode(str, "\n"), function(string line) {
        return trim(line);
    }), "\n");

    str = replace_string(str, "\n", "");
    str = replace_string(str, "\"\"", "");

    if(str[0] == '(') {
        switch(str[1]) {
            case '{':
                ret[0] = ({});
                str = ltrim(str[2..]);
                if(str[0] == '}') {
                    // Empty array case
                    str = str[1..];
                    str = ltrim(str);
                    if(str[0] != ')')
                        error("Illegal array terminator.\n");
                    ret[1] = str[1..];
                    return flag ? ret : ret[0];
                }
                while(str[0] != '}') {
                    mixed *tmp;

                    tmp = from_string(str, 1);
                    ret[0] += ({ tmp[0] });
                    str = tmp[1];
                    while(str[0] == ' ' || str[0] == '\t')
                        str = str[1..];
                    if(str[0] != ',' && str[0] != '}') {
                        error("Improperly formatted array: " + str + "\n");
                    } else if(str[0] == ',') {
                        str = str[1..];
                        while(str[0] == ' ' || str[0] == '\t')
                            str = str[1..];
                    }
                }

                if(str[1] != ')') {
                    str = str[2..];
                    while(str[0] == ' ' || str[0] == '\t')
                        str = str[1..];
                    if(str[0] != ')')
                        error("Illegal array terminator.\n");
                    else
                        ret[1] = str[1..];
                }
                else
                    ret[1] = str[2..];

                if(!flag)
                    return ret[0];

                while(ret[1][0] == ' ' || ret[1][0] == '\t')
                    ret[1] = ret[1][1..];

                return ret;
            case '[':
                ret[0] = ([]);
                str = str[2..];
                while(str[0] != ']') {
                    mixed *tmp;
                    mixed cle;

                    tmp = from_string(str, 1);
                    str = tmp[1];
                    while(str[0] == ' ' || str[0] == '\t') str = str[1..];
                    if(str[0] != ':')
                        error("Illegally formatting mapping: " + str + "\n");
                    cle = tmp[0];
                    tmp = from_string(str[1..], 1);
                    ret[0][cle] = tmp[0];
                    str = tmp[1];
                    while(str[0] == ' ' || str[0] == '\t')
                        str = str[1..];
                    if(str[0] != ',' && str[0] != ']')
                        error("Illegally formatted mapping: " + str + "n");
                    else if(str[0] != ']') {
                        str = str[1..];
                        while(str[0] == ' ' || str[0] == '\t') str = str[1..];
                    }
                }
                if(str[1] != ')') {
                    str = str[2..];
                    while(str[0] == ' ' || str[0] == '\t')
                        str = str[1..];
                    if(str[0] != ')')
                        error("Illegal array terminator.\n");
                    else
                        ret[1] = str[1..];
                }
                else
                    ret[1] = str[2..];
                if(!flag)
                    return ret[0];
                while(ret[1][0] == ' ' || ret[1][0] == '\t')
                    ret[1] = ret[1][1..];
                return ret;
        }
    } else if(str[0] == '"') {
        string tmp;

        tmp = "";
        while(str[1] != '"' || (str[1] == '"' && str[0] == '\\')) {
            if(str[1] == '"')
                tmp = tmp[0..<2] + "\"";
            else
                tmp += str[1..1];
            str = str[1..];
        }
        if(!flag)
            return tmp;
        if(strlen(str) > 2)
            str = trim(str[2..]);
        return ({ tmp, str });
    } else if(str[0] >= '0' && str[0] <= '9' || str[0] == '-') {
        string tmp;
        int y;

        if(strlen(str) > 1 && str[0] == '-') {
            tmp = str[0..0];
            str = str[1..];
        } else {
            tmp = "";
        }

        if(strlen(str) > 1 && str[0..1] == "0x") {
            tmp = "0x";
            str = str[2..];
            while(str != "" && (str[0] >= '0' && str[0] <= '9')) {
                tmp += str[0..0];
                if(strlen(str) > 1) str = str[1..];
                else str = "";
            }
            sscanf(tmp, "%x", y);
        } else {
            int decimal_found = 0;
            while(str != "" && (str[0] == '.' || (str[0] >= '0' && str[0] <= '9'))) {
                if(str[0] == '.') {
                    if(decimal_found)
                        error("Improperly formatted number: " + str + "\n");
                    decimal_found = 1;
                }

                tmp += str[0..0];
                if(strlen(str) > 1) str = str[1..];
                else str = "";
            }
            if(decimal_found)
                sscanf(tmp, "%f", y);
            else
                sscanf(tmp, "%d", y);
        }
        if(!flag)
            return y;
        if(str != "")
            str = trim(str);
        return ({ y, str });
    } else {
        string tmp;

        tmp = "";
        while(strlen(str) && ((str[0] >= 'a' && str[0] <= 'z') ||
                (str[0] >= 'A' && str[0] <= 'Z') ||
                (str[0] >= '0' && str[0] <= '9') ||
                (str[0] == '_'))) {
            tmp += str[0..0];
            if(strlen(str) > 1)
                str = str[1..];
            else str = "";
        }
        if(!flag)
            return get_object(tmp);
        else
        return ({ get_object(tmp), str });
    }
    error("Gobbledygook in string.\n");
}

/**
 * @simul_efun stringify
 * @description Converts an LPC value to its string representation.
 * @param {mixed} val - The value to convert.
 * @returns {string} - The string representation of the value.
 */
string stringify(mixed val) {
    if(nullp(val))
        return "0";
    if(intp(val))
        return sprintf("%d", val);
    if(floatp(val))
        return sprintf("%f", val);
    if(stringp(val))
        return val;
    if(objectp(val))
        return file_name(val);
    if(mapp(val))
        return save_variable(val);
    if(pointerp(val))
        return save_variable(val);
    return "";

}

// TODO: maybe use a define?
// private nosave string decimal = mud_config("DECIMAL");
// private nosave string thousands = mud_config("THOUSANDS");
private nosave string decimal = ".";
private nosave string thousands = ",";

/**
 * @simul_efun add_commas
 * @description Returns a string with commas added to the number.
 * @param {mixed} number - The number to add commas to.
 * @returns {string} - The number with commas added as a string.
 */
string add_commas(mixed number) {
    string num_str;
    string result = "";
    int len;
    int dot_index;
    int insert_position;
    int is_negative = 0;

    if(typeof(number) == T_INT) {
        num_str = sprintf("%d", number);
    } else if(typeof(number) == T_FLOAT) {
        num_str = sprintf("%f", number);
    } else if(typeof(number) == T_STRING) {
        if((dot_index = strsrch(number, decimal)) > -1) {
            string int_part = number[0..dot_index-1];
            return sprintf("%s.%s", add_commas(to_int(int_part)), number[dot_index+1..]);
        }
        else
            number = to_int(number);
        if(nullp(number))
            error("add_commas: Argument 1 must be an number, or a string that can be converted to a number.");
        return add_commas(number);
    } else {
        error("add_commas: Argument 1 must be an number, or a string that can be converted to a number.");
    }

    // Check if the number is negative
    if(num_str[0] == '-') {
        is_negative = 1;
        num_str = num_str[1..]; // Remove the negative sign for processing
    }

    len = strlen(num_str);
    dot_index = strsrch(num_str, decimal, -1); // Search for decimal point from the end

    // If there's a decimal point, handle the fractional part separately
    if(dot_index != -1) {
        result = add_commas(to_int(num_str[0..dot_index-1])); // Recurse on the integer part
        result = sprintf("%s%s%s", result, decimal, num_str[dot_index+1..]);
        return is_negative ? "-" + result : result;
    }

    // Calculate where to start inserting commas
    insert_position = (len % 3) || 3;

    for(int i = 0; i < len; i++) {
        if(i == insert_position && i != 0) {
            result = result + thousands + num_str[i..i]; // Insert comma then the character
            insert_position += 3;
        } else {
            result += num_str[i..i]; // Just add the character
        }
    }

    return is_negative ? "-" + result : result;
}

/**
 * @simul_efun reverse_string
 * @description Reverses a string.
 * @param {string} str - The string to reverse.
 * @returns {string} - The reversed string.
 */
string reverse_string(string str) {
    return implode(reverse_array(explode(str, "")), "");
}

/**
 * @simul_efun reverse_strsrch
 * @description Searches for a substring in a string starting from a given position
 *              and moving backwards.
 * @param {string} str - The string to search in.
 * @param {string} sub - The substring to search for.
 * @param {int} [start=-1] - The starting position to search from.
 * @returns {int} - The position of the substring in the string, or -1 if not found.
 */
varargs int reverse_strsrch(string str, string sub, int start) {
    int sub_len, str_len;
    int i;

    if(!str) error("reverse_strsrch: Missing argument 1 for reverse_strsrch");
    if(!sub) error("reverse_strsrch: Missing argument 2 for reverse_strsrch");
    if(!start) start = 0;

    str_len = strlen(str);
    sub_len = strlen(sub);

    if(str_len < 1) return -1;
    if(sub_len < 1) return -1;
    if(sub_len > str_len) return -1;
    if(start < 0) return -1;
    // Ensure start does not exceed string length
    if(start >= str_len) start = str_len - 1;

    for(i = str_len - start - 1; i >= 0; i--) {
        if(i + sub_len > str_len) continue;
        if(str[i..i+sub_len-1] == sub)
            return i;
    }

    return -1;
}
#include <colour.h>
/**
 * @simul_efun pcre_strsrch
 * @description Searches for the position of a substring in a string using a
 *              regular expression.
 * @param {string} str - The string to search in.
 * @param {string} substr - The regular expression to search for.
 * @param {int} [reverse=0] - If set, the search will start from the end of the string.
 * @returns {int} - The position of the substring in the string, or -1 if not found.
 */
varargs int pcre_strsrch(string str, string substr, int reverse) {
    int pos;
    string *matches;

    matches = pcre_extract(str, substr);
    if(sizeof(matches) == 0) return -1;
    pos = strsrch(str, matches[0], reverse);
    return pos;
}

/**
 * @simul_efun colourp
 * @description Returns 1 if the string contains colour codes, 0 if not.
 * @param {string} str - The string to check.
 * @returns {int} - 1 if the string contains colour codes, otherwise 0.
 */
int colourp(string str) {
    return pcre_match(str, COLOUR_REGEX);
}

string all_caps(string str) {
  int len;

  len = strlen(str);

  while(len--)
    str[len..len] = capitalize(str[len..len]);

  return str;
}

/**
 * Checks if `str` is a numeric literal (integer or float). It can optionally
 * start with '-' for negatives. Uses a regex to confirm the format.
 *
 * @param {string} str The string to check
 * @return {int} 1 if it matches a numeric pattern, 0 otherwise
 *
 * @example
 * ```c
 * if(is_numeric("3")) {
 *   write("It's a number.\n");
 * }
 *
 * if(is_numeric("3.14159", 1)) {
 *   write("It's a number.\n");
 * }
 * ```
 */
varargs int is_numeric(string str, int allow_float: (: 0 :)) {
  if(allow_float)
    return pcre_match(str, "^-?\\d+(\\.\\d+)?$");
  else
    return pcre_match(str, "^-?\\d+$");
}
