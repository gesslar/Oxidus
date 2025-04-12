/**
 * @file /ox/lib/adm/daemons/parse.c
 * @description A command parsing and processing system that handles user input,
 * tokenizes commands, and processes them with proper argument resolution.
 * This daemon manages command syntax, alias expansion, and argument handling
 * for player commands.
 *
 * The system supports:
 * - Command tokenization and syntax validation
 * - Object reference resolution (e.g., "the sword", "it")
 * - Contextual command interpretation
 * - Command history and aliasing
 *
 * @created 2024-08-07 - Gesslar
 * @last_modified 2024-08-07 - Gesslar
 *
 * @history
 * 2024-08-07 - Gesslar - Created based on action.c messaging system
 */

inherit STD_DAEMON;

#include <daemons.h>

/**
 * A structure to hold command data for processing.
 *
 * @property {STD_PLAYER} user - The player issuing the command
 * @property {string} supplied - The raw command string
 * @property {string} cmd_path - The path to the command handler
 * @property {STD_CMD} cmd - The command object handling the verb
 * @property {string} verb - The verb being executed
 * @property {string*} verb_pattern_matches - Array of supported grammar patterns
 * @property {string} pattern - The grammar pattern matched
 * @property {mapping} object_matches - Objects matched to grammar positions
 * @property {string} arg - The command argument as a string
 * @property {string*} args - Array of command arguments
 * @property {STD_ITEM*} object_candidates - Array of candidate objects
 * @property {STD_ITEM*} objects - Array of resolved objects
 * @property {mixed} result - The result of the command execution
 */
class CmdData {
  /** @type {STD_PLAYER} */ object  user;
                            string  supplied;
                            string  cmd_path;
  /** @type {STD_CMD}    */ object  cmd;
                            string  verb;
                            string *verb_pattern_matches;
                            mapping object_matches;
                            string  pattern;
                            string  arg;
                            string *args;
  /** @type {STD_ITEM*} */  object *object_candidates;
  /** @type {STD_ITEM*} */  object *objects;
  /** @type {({({string}),({int})*})} */ mixed *errors;
                            mixed   result;
}

// Forward declarations
private string short(mixed x);
string a_short(mixed x);
string the_short(mixed x);
string *tokenize_command(string command);
mixed resolve_object(object player, string reference);
private void process_command(class CmdData cmd_data);
private void find_command(class CmdData cmd_data);
int dispatch_command(object player, mapping cmd_data);
mapping extract_command_data(string command, object player);
private void identify_verb_patterns(class CmdData cmd_data);
object *identify_object_candidates(object player);
private mapping match_pattern(string pattern, class CmdData cmd_data);
mixed check_object_participation(string verb, mapping matches);
mixed execute_command(string verb, string pattern, mapping matches, object player);
private int parse_and_execute(class CmdData cmd_data);
public int handle_command(object player, string command);
mixed check_participation(string verb, mapping matches, object player);
private void record_error(class CmdData cmd_data, mixed result);

// Debug log functions and variables
private nosave int _level = 0;
private void inc() { _level++; }
private void dec() { _level--; }
private varargs void pdebug(string msg, mixed arg...) {
  string indent = repeat_string("  ", _level);

  if(!pointerp(arg) || !sizeof(arg))
    arg = ({});

  msg = sprintf(msg, arg...);

  _debug("%s%s\n", indent, msg);
}

/**
 * Integrates both regular command processing and parser-based processing.
 *
 * This function tries both approaches - first the grammar parser for
 * more complex commands, then falls back to the simpler command processing.
 *
 * @param {object} player - The player issuing the command
 * @param {string} command - The raw command string
 * @returns {int} 1 if command was handled, 0 if not
 */
public int handle_command(string command, object *obs, object player) {
  mapping cmd_data;
  class CmdData work;
  int result;

  // Clear the parser state
  reset();

  // Populate with initial data
  work = new(class CmdData,
    user: player,
    supplied: command,
    cmd_path: null,
    cmd: null,
    verb: "",
    verb_pattern_matches: ({ }),
    object_matches: ([ ]),
    pattern: "",
    arg: "",
    args: ({ }),
    object_candidates: obs,
    objects: ({ }),
    errors: ({ }),
    result: null
  );

  pdebug("handle_command: %s", command);

  // First try the parser-based approaches
  inc();
  parse_and_execute(work);
  dec();

  if(result)
    return result;

  // Fall back to the simpler command processing
  if(!command || command == "")
    return 0;

  // Process the command using the traditional approach
  process_command(work);

  if(!cmd_data)
    return 0;

  // Dispatch the command
  return dispatch_command(player, cmd_data);
}

/**
 * Retrieves a short description for an object or string.
 *
 * For living objects, returns the name. For non-living objects, returns
 * either the name or short description. For strings, returns them unmodified.
 *
 * @param {mixed} x - The item to get a description for
 * @returns {string} The appropriate short description
 */
private string short(mixed x) {
  if(objectp(x)) {
    if(living(x))
      return x->query_name();
    else
      return x->query_name() || x->query_short();
  }

  return x;
}

/**
 * Returns a string with an indefinite article ("a" or "an") added.
 *
 * For objects, returns their short description.
 * For strings, ensures they have an appropriate indefinite article.
 *
 * @param {STD_ITEM|string} x - The object or string to process
 * @returns {string} The description with appropriate article
 */
string a_short(mixed x) {
  if(objectp(x))
    x = x->query_short();

  if(!stringp(x))
    return x;

  // Sanitizes the indefinite article, adding it if it is missing.
  return add_article(x);
}

/**
 * Returns a string with the definite article ("the") added.
 *
 * For objects, returns their short description.
 * For strings, ensures they have the definite article.
 *
 * @param {STD_ITEM|string} x - The object or string to process
 * @returns {string} The description with the definite article
 */
string the_short(mixed x) {
  if(objectp(x))
    x = x->query_short();

  if(!stringp(x))
    return x;

  return add_article(x, 1);
}

/**
 * Parses a command string into tokens based on command grammar.
 *
 * Takes a raw command string and separates it into verb and arguments,
 * handling quoted strings and special syntax.
 *
 * @param {string} command - The raw command string to parse
 * @returns {string*} Array containing [verb, arg1, arg2, ...]
 */
string *tokenize_command(string command) {
  string *tokens = ({});
  string verb, args;
  int i, in_quotes = 0;

  if(!command || command == "")
    return ({});

  // Extract the verb (first word)
  if(sscanf(command, "%s %s", verb, args) != 2) {
    verb = command;
    args = "";
  }

  tokens += ({ verb });

  if(args && args != "") {
    string current_token = "";

    // Parse the remaining arguments
    for(i = 0; i < strlen(args); i++) {
      if(args[i] == '"') {
        in_quotes = !in_quotes;
        continue;
      }

      if(args[i] == ' ' && !in_quotes) {
        if(current_token != "")
          tokens += ({ current_token });
        current_token = "";
      } else {
        current_token += args[i..i];
      }
    }

    if(current_token != "")
      tokens += ({ current_token });
  }

  return tokens;
}

/**
 * Resolves object references in a command.
 *
 * Converts textual references like "the sword" or "it" into actual
 * game objects based on the player's environment and inventory.
 *
 * @param {object} player - The player issuing the command
 * @param {string} reference - The object reference to resolve
 * @returns {object|object*} The object(s) matching the reference
 */
mixed resolve_object(object player, string reference) {
  object *object_candidates = ({});
  object env = environment(player);

  if(!reference || reference == "")
    return 0;

  // Handle pronouns
  if(reference == "it" || reference == "them") {
    // Would return player's last referenced object(s)
    // This would need to track object references in a player attribute
    return player->query_last_objects_referenced();
  }

  // Check for definite article
  if(reference[0..3] == "the ") {
    reference = reference[4..];
  }

  // Check player's inventory
  object_candidates = filter(all_inventory(player), (: $1->id($(reference)) :));

  // If not found, check environment
  if(!sizeof(object_candidates) && env) {
    object_candidates = filter(all_inventory(env), (: $1->id($(reference)) :));
  }

  if(sizeof(object_candidates) == 1)
    return object_candidates[0];
  else if(sizeof(object_candidates) > 1)
    return object_candidates;

  return 0;
}

/**
 * Processes a command from a player.
 *
 * Takes a raw command, tokenizes it, resolves objects, and prepares
 * it for execution by the appropriate command handler.
 *
 * @param {object} player - The player issuing the command
 * @param {string} command - The raw command string
 * @returns {mapping} A mapping containing processed command components
 */
private void process_command(class CmdData cmd_data) {
  mapping result = ([]);
  string *tokens;

  if(falsy(cmd_data.supplied))
    return 0;

  // Extract and process command tokens
  tokens = tokenize_command(cmd_data.supplied);

  if(!sizeof(tokens))
    return 0;

  cmd_data.verb = tokens[0];
  cmd_data.args = tokens[1..];

  // Resolve object references if arguments exist
  if(sizeof(tokens) > 1) {
    foreach(string arg in tokens[1..]) {
      mixed resolved = resolve_object(cmd_data.user, arg);

      if(resolved)
        cmd_data.objects += ({ resolved });
    }
  }
}

/**
 * Finds a command handler for a given verb.
 *
 * Searches through the command path to locate the appropriate command
 * object that can handle the given verb.
 *
 * @param {STD_LIVING} player - The player issuing the command
 * @param {string} verb - The command verb to find a handler for
 * @returns {string} The path to the command handler, or 0 if not found
 */
private void find_command(class CmdData cmd_data) {
  cmd_data.cmd_path = cmd_data.user->find_command_path(cmd_data.verb);

  catch(cmd_data.cmd = load_object(cmd_data.cmd_path));
}

/**
 * Dispatches a processed command to its handler.
 *
 * Takes a processed command mapping and sends it to the appropriate
 * command handler for execution.
 *
 * @param {object} player - The player issuing the command
 * @param {mapping} cmd_data - The processed command data
 * @returns {int} 1 if command was handled, 0 if not
 */
int dispatch_command(class CmdData cmd_data) {
  string cmd_path;
  object cmd_ob;

  if(!cmd_data.cmd)
    return 0;

  return call_other(cmd_data.cmd, "cmd", cmd_data.user, cmd_data.args);
}

/**
 * Tokenizes a command and identifies grammar elements.
 *
 * This function parses a command string and identifies verb patterns
 * according to grammar rules, matching tokens like OBJ (single object),
 * OBS (multiple objects), LIV (living object), etc.
 *
 * @param {string} command - The command string to parse
 * @param {object} player - The player issuing the command
 * @returns {mapping} A mapping containing tokenized command information
 */
private void extract_command_data(class CmdData cmd_data) {
  mapping result = ([]);
  string verb, args;

  if(falsy(cmd_data.supplied))
    return;

  // Extract the verb (first word)
  if(sscanf(cmd_data.supplied, "%s %s", cmd_data.verb, cmd_data.arg) != 2) {
    cmd_data.verb = cmd_data.supplied;
    cmd_data.arg = "";
  }

  cmd_data.args = explode(cmd_data.arg, " ");
}

/**
 * Identifies grammatical patterns for a verb.
 *
 * Scans the command handlers to determine what grammar patterns
 * are supported for a given verb (e.g., OBJ, OBJ with OBJ, etc.).
 *
 * @param {object} player - The player issuing the command
 * @param {string} verb - The verb to check for patterns
 * @returns {string*} Array of supported grammar patterns
 */
private void *identify_verb_patterns(class CmdData cmd_data) {
  string *funcs;

  pdebug("identify_verb_patterns: %s", cmd_data.verb);

  if(!cmd_data.cmd)
    find_command(cmd_data);

  pdebug("identify_verb_patterns: cmd: %O", cmd_data.cmd);

  if(!cmd_data.cmd)
    return;

  // Check for pattern functions like can_verb_obj, can_verb_str_with_obj
  funcs = functions(cmd_data.cmd);

  pdebug("identify_verb_patterns: funcs: %s", identify(funcs));

  foreach(string func in funcs) {
    string v, pattern;

    if(sscanf(func, "can_%s_%s", v, pattern) == 2) {
      if(v == cmd_data.verb)
        cmd_data.verb_pattern_matches += ({ pattern });
    }
  }
}

/**
 * Identifies objects in the environment that can be acted upon.
 *
 * Collects all visible objects in the player's vicinity that could
 * potentially be referenced in commands.
 *
 * @param {object} player - The player issuing the command
 * @returns {object*} Array of visible/accessible objects
 */
void identify_object_candidates(class CmdData cmd_data) {
  object env = environment(cmd_data.user);

  if(!env)
    return;

  // Clear the object candidates list
  cmd_data.object_candidates = ({});

  // Get player's inventory
  cmd_data.object_candidates += all_inventory(cmd_data.user);

  // Get environment's inventory (excluding the player)
  cmd_data.object_candidates += all_inventory(env) - ({ cmd_data.user });
}

/**
 * Asks objects if they're willing to participate in an action.
 *
 * Calls direct_* and indirect_* functions on objects to determine
 * if they will allow the action to proceed.
 *
 * @param {string} verb - The verb being attempted
 * @param {mapping} matches - Objects matched to grammar positions
 * @returns {mixed} 1 if allowed, 0 or string if not allowed
 */
void check_object_participation(class CmdData cmd_data) {
  mixed result;

  // Check direct object participation
  if(cmd_data.object_matches["obj"]) {
    string func;

    if(cmd_data.pattern)
      func = "direct_" + cmd_data.verb + "_" + cmd_data.pattern;
    else
      func = "direct_" + cmd_data.verb;

    if(function_exists(func, cmd_data.object_matches["obj"])) {
      result = call_other(cmd_data.object_matches["obj"], func);
      if(result != 1) {
        if(stringp(result)) {
          push(ref cmd_data.errors, ({ ({ result }), ({ 1 }) }));
        }
      }
    }
  }

  // Check indirect object participation
  if(cmd_data.object_matches["obj2"]) {
    string func;

    if(cmd_data.pattern)
      func = "indirect_" + cmd_data.verb + "_" + cmd_data.pattern;
    else
      func = "indirect_" + cmd_data.verb;

    if(function_exists(func, cmd_data.object_matches["obj2"])) {
      result = call_other(cmd_data.object_matches["obj2"], func, cmd_data.object_matches["obj"]);
      if(!result)
        return result;
    }
  }
}

/**
 * Executes a command after parsing and validation.
 *
 * This is the final step where the command handler's do_* function
 * is called to perform the actual action.
 *
 * @param {string} verb - The verb being executed
 * @param {string} pattern - The grammar pattern that matched
 * @param {mapping} matches - Objects matched to grammar positions
 * @param {object} player - The player issuing the command
 * @returns {mixed} Result of the command execution
 */
mixed execute_command(class CmdData cmd_data) {
  string func;

  if(!cmd_data.verb ||
     !cmd_data.cmd)
    return 0;


  if(cmd_data.pattern)
    func = "do_" + cmd_data.verb + "_" + cmd_data.pattern;
  else
    func = "do_" + cmd_data.verb;

  if(!function_exists(func, cmd_data.cmd))
    return 0;

  // we will have to fix this because we are not passing the right arguments
  return call_other(cmd_data.cmd, func, cmd_data.user, cmd_data.args...);
}

/**
 * Main entry point for handling player commands with parser integration.
 *
 * Processes a command string using the grammatical parser approach,
 * checking object participation and executing the command.
 *
 * @param {object} player - The player issuing the command
 * @param {string} command - The raw command string
 * @returns {int} 1 if command was handled, 0 if not
 */
private void parse_and_execute(class CmdData cmd_data) {
  string pattern;
  string can_func;
  mixed  can_result;
  mixed  participation;

  pdebug("parse_and_execute: %O", cmd_data.supplied);

  inc();
  extract_command_data(cmd_data);
  dec();

  if(!cmd_data.verb)
    return;

  // Get the grammar patterns for this verb
  inc();
  identify_verb_patterns(cmd_data);
  dec();

  inc();
  find_command(cmd_data);
  dec();

  // Try each pattern until one succeeds
  foreach(pattern in cmd_data.verb_pattern_matches) {
    inc();
    match_pattern(pattern, cmd_data);
    dec();

    pdebug("parse_and_execute: pattern: %s, matches: %s", pattern, identify(cmd_data.object_matches));

    if(!sizeof(cmd_data.object_matches))
      continue;

    // Check if the verb handler allows this action
    if(cmd_data.pattern)
      can_func = "can_" + cmd_data.verb + "_" + pattern;
    else
      can_func = "can_" + cmd_data.verb;

    if(!function_exists(can_func, cmd_data.cmd))
      continue;

    // Call the can_* function to see if the action is allowed
    can_result = call_other(cmd_data.cmd, can_func, cmd_data.user, cmd_data.object_matches);

    // We've found one, just do it.
    if(can_result == 1) {
      cmd_data.result = execute_command(cmd_data);
      return;
    }

    if(stringp(can_result))
      push(ref cmd_data.errors, ({ ({ can_result }), ({ 1 }) }));
    else
      push(ref cmd_data.errors, ({ ({ "You can't do that." }), ({ 0 }) }));

    // Check if objects want to participate
    participation = check_object_participation(cmd_data);

    // WRONG
    if(!participation) {
      if(stringp(participation))
        tell_object(cmd_data.user, participation);
      continue;
    }

    // Execute the command
    return execute_command(cmd_data);
  }

  return 0;
}

/**
 * Defines grammar token types used in command parsing patterns.
 *
 * These token types correspond to the FluffOS parser grammar tokens and
 * are used to identify different elements in command sentences.
 *
 * @type {mapping}
 */
private nosave mapping token_types = ([
  "OBJ": "single object",
  "OBS": "multiple objects",
  "LIV": "single living",
  "LVS": "multiple living",
  "STR": "raw string",
  "WRD": "single word"
]);

/**
 * Matches grammar patterns against command input.
 *
 * This is an enhanced version of the match_pattern function that handles
 * all standard FluffOS parser tokens like OBJ, OBS, LIV, etc.
 *
 * @param {string} pattern - Grammar pattern to match
 * @param {string} args - Command arguments to parse
 * @param {object} player - Player issuing the command
 * @returns {mapping|int} Matches mapping or 0 if no match
 */
private mapping match_pattern(string pattern, class CmdData cmd_data) {
  mapping matches = ([]);
  object *object_candidates ;
  string *pattern_parts = explode(pattern, "_");

  identify_object_candidates(player);

  // Parse the pattern into grammar tokens and prepositions
  string *tokens = ({});
  string *prepositions = ({});
  string *arg_parts;

  for(int i = 0; i < sizeof(pattern_parts); i++) {
    if(i % 2 == 0) {
      // Even indices are tokens (OBJ, STR, etc.)
      tokens += ({ pattern_parts[i] });
    } else {
      // Odd indices are prepositions (with, from, etc.)
      prepositions += ({ pattern_parts[i] });
    }
  }

  // Split args based on prepositions
  arg_parts = ({ args });
  if(sizeof(prepositions)) {
    foreach(string prep in prepositions) {
      string current = arg_parts[<1];
      string before, after;

      if(sscanf(current, "%s " + prep + " %s", before, after) == 2) {
        arg_parts[<1] = before;
        arg_parts += ({ after });
      } else {
        // Preposition not found, pattern can't match
        return 0;
      }
    }
  }

  // Now match each token with its corresponding arg part
  for(int i = 0; i < sizeof(tokens); i++) {
    string token = tokens[i];
    string arg = (i < sizeof(arg_parts)) ? arg_parts[i] : "";

    // Skip if we ran out of arguments for tokens
    if(arg == "")
      return 0;

    switch(token) {
      case "OBJ": {
        // Match a single object
        object obj = find_object_in_environment(arg, player);
        if(!obj)
          return 0;
        matches["OBJ" + (i > 0 ? i : "")] = obj;
        break;
      }
      case "OBS": {
        // Match multiple objects
        object *objs = find_objects_in_environment(arg, player);
        if(!sizeof(objs))
          return 0;
        matches["OBS" + (i > 0 ? i : "")] = objs;
        break;
      }
      case "LIV": {
        // Match a single living object
        object liv = find_living_in_environment(arg, player);
        if(!liv)
          return 0;
        matches["LIV" + (i > 0 ? i : "")] = liv;
        break;
      }
      case "LVS": {
        // Match multiple living objects
        object *livs = find_livings_in_environment(arg, player);
        if(!sizeof(livs))
          return 0;
        matches["LVS" + (i > 0 ? i : "")] = livs;
        break;
      }

      case "STR": {
        // Match any string
        matches["STR" + (i > 0 ? i : "")] = arg;
        break;
      }
      case "WRD": {
        // Match a single word
        if(strsrch(arg, " ") != -1)
          return 0;
        matches["WRD" + (i > 0 ? i : "")] = arg;
        break;
      }
      default:
        // Unrecognized token
        return 0;
    }
  }

  return matches;
}

/**
 * Finds a single object matching a description in the player's environment.
 *
 * @param {string} desc - Object description to search for
 * @param {object} player - Player issuing the command
 * @returns {object} Matching object or 0 if none found
 */
object find_object_in_environment(string desc, object player) {
  object *object_candidates = identify_object_candidates(player);

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach(object obj in object_candidates) {
    if(!living(obj) && obj->id(desc))
      return obj;
  }

  return 0;
}

/**
 * Finds multiple objects matching a description in the player's environment.
 *
 * @param {string} desc - Object description to search for
 * @param {object} player - Player issuing the command
 * @returns {object*} Array of matching objects
 */
object *find_objects_in_environment(string desc, object player) {
  object *object_candidates = identify_object_candidates(player);
  object *matches = ({});

  // Handle "all" keyword
  if(desc == "all") {
    foreach(object obj in object_candidates) {
      if(!living(obj))
        matches += ({ obj });
    }
    return matches;
  }

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach(object obj in object_candidates) {
    if(!living(obj) && obj->id(desc))
      matches += ({ obj });
  }

  return matches;
}

/**
 * Finds a single living object matching a description in the player's environment.
 *
 * @param {string} desc - Object description to search for
 * @param {object} player - Player issuing the command
 * @returns {object} Matching living object or 0 if none found
 */
object find_living_in_environment(string desc, object player) {
  object *object_candidates = identify_object_candidates(player);

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach(object obj in object_candidates) {
    if(living(obj) && obj->id(desc))
      return obj;
  }

  return 0;
}

/**
 * Finds multiple living objects matching a description in the player's environment.
 *
 * @param {string} desc - Object description to search for
 * @param {object} player - Player issuing the command
 * @returns {object*} Array of matching living objects
 */
object *find_livings_in_environment(string desc, object player) {
  object *object_candidates = identify_object_candidates(player);
  object *matches = ({});

  // Handle "all" keyword for living objects
  if(desc == "all") {
    foreach(object obj in object_candidates) {
      if(living(obj))
        matches += ({ obj });
    }
    return matches;
  }

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach(object obj in object_candidates) {
    if(living(obj) && obj->id(desc))
      matches += ({ obj });
  }

  return matches;
}

/**
 * Checks if objects will participate in an action using the enhanced FluffOS
 * parser approach with direct_ and indirect_ prefixed methods.
 *
 * @param {string} verb - The verb being attempted
 * @param {mapping} matches - Grammar token matches from match_pattern
 * @param {object} player - The player issuing the command
 * @returns {mixed} 1 if allowed, 0 or error string if not allowed
 */
mixed check_participation(string verb, mapping matches, object player) {
  mixed result;
  string *keys = keys(matches);

  // Check direct object participation
  foreach(string key in keys) {
    if(strsrch(key, "OBJ") == 0 || strsrch(key, "LIV") == 0) {
      object obj = matches[key];
      string func;

      // Skip if not an object
      if(!objectp(obj))
        continue;

      func = "direct_" + verb;
      if(function_exists(func, obj)) {
        // Build arguments for direct_ function based on other matches
        mixed *args = ({ });

        // Add indirect objects if they exist
        foreach(string k in keys) {
          if(strsrch(k, "OBJ") == 0 && k != key ||
              strsrch(k, "LIV") == 0 && k != key) {
            args += ({ matches[k] });
          }
        }

        result = call_other(obj, func, args...);

        if(stringp(result)) {
          tell_object(player, result);
          return 0;
        } else if(!result) {
          return 0;
        }
      }
    }
  }

  // Check indirect object participation
  foreach(string key in keys) {
    if(strsrch(key, "OBJ") == 0 && key != "OBJ" ||
        strsrch(key, "LIV") == 0 && key != "LIV") {
      object obj = matches[key];
      string func;

      // Skip if not an object
      if(!objectp(obj))
        continue;

      func = "indirect_" + verb;
      if(function_exists(func, obj)) {
        // Get the direct object if it exists
        object direct_obj = matches["OBJ"] || matches["LIV"];

        result = call_other(obj, func, direct_obj);

        if(stringp(result)) {
          tell_object(player, result);
          return 0;
        } else if(!result) {
          return 0;
        }
      }
    }
  }

  return 1;
}

private void record_error(class CmdData cmd_data, mixed result) {
  if(stringp(result))
    push(ref cmd_data.errors, ({ ({ result }), ({ 1 }) }));
  else
    push(ref cmd_data.errors, ({ ({ "You can't do that." }), ({ 0 }) }));
}

void reset() {
  _level = 0;
}
