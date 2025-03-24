/**
 * @file /std/object/id.c
 * @description Manages object identifiers and adjectives for item recognition
 *              and command parsing.
 *
 * @created 2024-01-31 - Gesslar
 * @last_modified 2025-03-16 - GitHub Copilot
 *
 * @history
 * 2024-01-31 - Gesslar - Created
 * 2025-03-16 - GitHub Copilot - Added documentation
 */

#include <id.h>
#include <object.h>

private nosave string *_ids = ({}), *_base_ids = ({});
private nosave string *_plural_ids = ({});
private nosave string *_adj = ({}), *_base_adj = ({});

/**
 * Sets the primary identifiers for this object, replacing any existing IDs.
 *
 * @param {string|string*} str - Single ID or array of IDs
 * @errors If argument is not a string or array of strings
 */
void set_id(mixed str) {
  _ids = ({});
  _base_ids = ({});

  add_id(str);
}

/**
 * Sets the adjectives for this object, replacing any existing adjectives.
 *
 * These adjectives can be combined with IDs for identification.
 *
 * @param {string|string*} str - Single adjective or array of adjectives
 * @errors If argument is not a string or array of strings
 */
void set_adj(mixed str) {
  _adj = ({});
  _base_adj = ({});

  add_adj(str);
}

/**
 * Adds additional identifiers to this object.
 *
 * @param {string|string*} str - Single ID or array of IDs to add
 * @errors If argument is not a string or array of strings
 */
void add_id(mixed str) {
  if(stringp(str))
    str = ({ str });

  if(!pointerp(str))
    error("Bad argument 1 to add_id().");

  if(!uniform_array(str, T_STRING))
    error("Bad argument 1 to add_id().");

  _ids = distinct_array(_ids + str);
  _base_ids += str;

  rehash_ids();
}

/**
 * Adds additional adjectives to this object.
 *
 * @param {string|string*} str - Single adjective or array of adjectives to add
 * @errors If argument is not a string or array of strings
 */
void add_adj(mixed str) {
  if(stringp(str))
    str = ({ str });

  if(!pointerp(str))
    error("Bad argument 1 to add_adj().");

  if(!uniform_array(str, T_STRING))
    error("Bad argument 1 to add_adj().");

  _adj = distinct_array(_adj + str);
  _base_adj += str;

  rehash_ids();
}

/**
 * Removes identifiers from this object.
 *
 * @param {string|string*} str - Single ID or array of IDs to remove
 * @errors If argument is not a string or array of strings
 */
void remove_id(mixed str) {
  if(stringp(str))
    str = ({ str });

  if(!pointerp(str))
    error("Bad argument 1 to remove_id().");

  if(!uniform_array(str, T_STRING))
    error("Bad argument 1 to remove_id().");

  _ids = distinct_array(_ids - str);
  _base_ids -= str;

  rehash_ids();
}

/**
 * Removes adjectives from this object.
 *
 * @param {string|string*} str - Single adjective or array of adjectives to remove
 * @errors If argument is not a string or array of strings
 */
void remove_adj(mixed str) {
  if(stringp(str))
    str = ({ str });

  if(!pointerp(str))
    error("Bad argument 1 to remove_adj().");

  if(!uniform_array(str, T_STRING))
    error("Bad argument 1 to remove_adj().");

  _adj = distinct_array(_adj - str);
  _base_adj -= str;

  rehash_ids();
}

/**
 * Returns all identifiers for this object.
 *
 * @returns {string*} Array of identifiers
 */
string *query_ids() {
  return _ids;
}

/**
 * Returns all plural identifiers for this object.
 *
 * @returns {string*} Array of plural identifiers
 */
string *query_plural_ids() {
  return _plural_ids;
}

/**
 * Returns all adjectives for this object.
 *
 * @returns {string*} Array of adjectives
 */
string *query_adjs() {
  return _adj;
}

/**
 * Driver apply to checks if a string matches one of this object's
 * identifiers.
 *
 * @param {string} arg - The string to check against identifiers
 * @returns {int} 1 if the string matches, 0 if not
 */
int id(string arg) {
  if(!arg || !stringp(arg))
    return 0;

  if(sizeof(_ids) < 1)
    _ids = ({query_name()});

  if(member_array(arg, _ids) != -1)
    return 1;

  return 0;
}

/**
 * Regenerates the full set of identifiers.
 *
 * Includes plurals and adjective combinations in the identifier list.
 * Called when identifiers or adjectives change.
 */
void rehash_ids() {
  string *tmp = ({});
  string  name;
  string *base_id, *base_adj;

  name = query_name();

  base_id = copy(_base_ids);
  base_adj = copy(_base_adj);

  tmp += base_id;

  foreach(string id in tmp) {
    tmp += map(base_adj, function(string t, string id, string *curr) {
      string result = sprintf("%s %s", t, id);

      if(includes(curr, result))
        return 0;

      return result;
    }, id, tmp);
  }

  if(!includes(tmp, name))
    tmp += ({ name });

  _ids = filter(distinct_array(tmp), (: truthy :));
  _plural_ids = map(_ids, (: pluralize :));
}

string *parse_command_id_list() {
  // if(test_flag(INVIS))
  //   return ({});

  return _ids - ({0});
}

nomask string *parse_command_plural_id_list() {
  // if(test_flag(INVIS))
  //   return ({});

  return _plural_ids - ({0});
}

nomask string *parse_command_adjectiv_id_list() {
  // if(test_flag(INVIS))
  //     return ({});

  return _adj - ({0});
}

string id_info() {
  return
    "Short: " + get_short() + "\n" + "IDs: " + implode(parse_command_id_list(), ", ") + "\n" +
    "Plurals: " + implode(parse_command_plural_id_list(), ", ") + "\n" +
    "Adjectives: " + implode(parse_command_adjectiv_id_list(), ", ") + "\n" +
    "Long: \n" + get_long() + "\n";
}
