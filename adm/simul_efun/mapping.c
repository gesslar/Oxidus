#include <simul_efun.h>

private string regex = "( \\/\\* sizeof\\(\\) == \\d+ \\*/)";

/**
 * Returns a formatted string representation of a mapping, removing any size
 * annotations.
 *
 * @param {mapping} map - The mapping to format.
 * @returns {string} The formatted string representation of the mapping.
 */
string pretty_map(mapping map) {
  string str = sprintf("%O\n", map);

  while(pcre_match(str, regex))
    str = pcre_replace(str, regex, ({""}));

  return str;
}

/**
 * Finds the first key in a mapping that corresponds to a specific value
 * or satisfies a given condition.
 *
 * When a function is provided as the value parameter, it acts as a predicate
 * that receives (value, key, mapping) as arguments and should return true
 * when the desired key is found.
 *
 * @param {mapping} map - The mapping to search
 * @param {mixed|function} value - The value to find or predicate function
 * @returns {mixed} The first matching key, or 0 if no match found
 * @example
 * // Find key for a specific value
 * mapping scores = (["Alice": 95, "Bob": 87, "Charlie": 95]);
 * string name = find_key(scores, 95); // Returns "Alice"
 *
 * // Find key using a predicate function
 * name = find_key(scores, (: $1 > 90 :)); // Returns first student with score > 90
 */
mixed find_key(mapping map, mixed value) {
  if(valid_function(value)) {
    function f = value;

    foreach(mixed key, mixed val in map) {
      if(f(val,key,map))
        return key;
    }
  } else {
    foreach(mixed key, mixed val in map) {
      if(val == value)
        return key;
    }
  }
}

/**
 * Finds all keys in a mapping that correspond to a specific value
 * or satisfy a given condition.
 *
 * When a function is provided as the value parameter, it acts as a predicate
 * that receives (value, key, mapping) as arguments and should return true
 * for each key that should be included in the result.
 *
 * @param {mapping} map - The mapping to search
 * @param {mixed|function} value - The value to find or predicate function
 * @returns {mixed*} Array of all matching keys, empty array if none found
 * @example
 * // Find all keys for a specific value
 * mapping scores = (["Alice": 95, "Bob": 87, "Charlie": 95]);
 * string* names = find_keys(scores, 95); // Returns ({"Alice", "Charlie"})
 *
 * // Find all keys using a predicate function
 * names = find_keys(scores, (: $1 > 90 :)); // Returns all students with score > 90
 *
 * // Using with evaluate_number for complex conditions
 * string* honor_roll = find_keys(scores, (: evaluate_number($1, ">=95OR(>=80AND%5)") :));
 * // Returns students with scores ≥95 OR (scores ≥80 AND divisible by 5)
 */
mixed *find_keys(mapping map, mixed value) {
  mixed *result = allocate(0);

  if(valid_function(value)) {
    function f = value;

    foreach(mixed key, mixed val in map) {
      if(f(val,key,map))
        push(ref result, key);
    }
  } else {
    foreach(mixed key, mixed val in map) {
      if(val == value)
        push(ref result, key);
    }
  }

  return result;
}
