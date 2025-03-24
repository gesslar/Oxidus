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

// Forward declarations
private string short(mixed x);
string a_short(mixed x);
string the_short(mixed x);
string *tokenize_command(string command);
mixed resolve_object(object player, string reference);
mapping process_command(object player, string command);
string find_command(string verb);
int dispatch_command(object player, mapping cmd_data);
mapping interpret_command(string command, object player);
string *identify_verb_patterns(string verb);
object *identify_candidates(object player);
mapping match_pattern(string pattern, string args, object player);
mixed check_object_participation(string verb, mapping matches);
mixed execute_command(string verb, string pattern, mapping matches, object player);
int parse_and_execute(object player, string command);
int handle_command(object player, string command);
mapping enhanced_match_pattern(string pattern, string args, object player);
object find_object_in_environment(string desc, object player);
object *find_objects_in_environment(string desc, object player);
object find_living_in_environment(string desc, object player);
object *find_livings_in_environment(string desc, object player);
mixed check_participation(string verb, mapping matches, object player);
int improved_parse_and_execute(object player, string command);

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

  if (!command || command == "")
    return ({});

  // Extract the verb (first word)
  if (sscanf(command, "%s %s", verb, args) != 2) {
    verb = command;
    args = "";
  }

  tokens += ({ verb });

  if (args && args != "") {
    string current_token = "";

    // Parse the remaining arguments
    for (i = 0; i < strlen(args); i++) {
      if (args[i] == '"') {
        in_quotes = !in_quotes;
        continue;
      }

      if (args[i] == ' ' && !in_quotes) {
        if (current_token != "")
          tokens += ({ current_token });
        current_token = "";
      } else {
        current_token += args[i..i];
      }
    }

    if (current_token != "")
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
  object *candidates = ({});
  object env = environment(player);

  if (!reference || reference == "")
    return 0;

  // Handle pronouns
  if (reference == "it" || reference == "them") {
    // Would return player's last referenced object(s)
    // This would need to track object references in a player attribute
    return player->query_last_objects_referenced();
  }

  // Check for definite article
  if (reference[0..3] == "the ") {
    reference = reference[4..];
  }

  // Check player's inventory
  candidates = filter(all_inventory(player), (: $1->id($(reference)) :));

  // If not found, check environment
  if (!sizeof(candidates) && env) {
    candidates = filter(all_inventory(env), (: $1->id($(reference)) :));
  }

  if (sizeof(candidates) == 1)
    return candidates[0];
  else if (sizeof(candidates) > 1)
    return candidates;

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
mapping process_command(object player, string command) {
  mapping result = ([]);
  string *tokens;

  if (!command || command == "")
    return 0;

  // Extract and process command tokens
  tokens = tokenize_command(command);

  if (!sizeof(tokens))
    return 0;

  result["verb"] = tokens[0];
  result["args"] = tokens[1..];
  result["full_command"] = command;

  // Resolve object references if arguments exist
  if (sizeof(tokens) > 1) {
    mixed objects = ({});
    foreach (string arg in tokens[1..]) {
      mixed resolved = resolve_object(player, arg);
      if (resolved)
        objects += ({ resolved });
    }

    if (sizeof(objects))
      result["objects"] = objects;
  }

  return result;
}

/**
 * Finds a command handler for a given verb.
 *
 * Searches through the command path to locate the appropriate command
 * object that can handle the given verb.
 *
 * @param {string} verb - The command verb to find a handler for
 * @returns {string} The path to the command handler, or 0 if not found
 */
string find_command(string verb) {
  string *cmd_path = ({
    "/ox/lib/cmds/verbs/",
    "/ox/lib/cmds/wiz/",
    "/ox/lib/cmds/player/"
  });

  foreach (string path in cmd_path) {
    string potential_cmd = path + verb;
    if (file_size(potential_cmd + ".c") > 0)
      return potential_cmd;
  }

  return 0;
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
int dispatch_command(object player, mapping cmd_data) {
  string cmd_path;
  object cmd_ob;

  if (!cmd_data || !cmd_data["verb"])
    return 0;

  cmd_path = find_command(cmd_data["verb"]);

  if (!cmd_path)
    return 0;

  cmd_ob = load_object(cmd_path);

  if (!cmd_ob)
    return 0;

  return cmd_ob->cmd(player, cmd_data["args"], cmd_data);
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
mapping interpret_command(string command, object player) {
  mapping result = ([]);
  string verb, args;

  if (!command || command == "")
    return 0;

  // Extract the verb (first word)
  if (sscanf(command, "%s %s", verb, args) != 2) {
    verb = command;
    args = "";
  }

  result["verb"] = verb;
  result["args"] = args;
  result["full_command"] = command;

  return result;
}

/**
 * Identifies grammatical patterns for a verb.
 *
 * Scans the command handlers to determine what grammar patterns
 * are supported for a given verb (e.g., OBJ, OBJ with OBJ, etc.).
 *
 * @param {string} verb - The verb to check for patterns
 * @returns {string*} Array of supported grammar patterns
 */
string *identify_verb_patterns(string verb) {
  string path = find_command(verb);
  string *patterns = ({});
  object cmd;
  string *funcs;

  if (!path)
    return ({});

  cmd = catch(load_object(path));

  if (!cmd)
    return ({});

  // Check for pattern functions like can_verb_obj, can_verb_str_with_obj
  funcs = functions(cmd);

  foreach (string func in funcs) {
    string v, pattern;

    if (sscanf(func, "can_%s_%s", v, pattern) == 2) {
      if (v == verb)
        patterns += ({ pattern });
    }
  }

  return patterns;
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
object *identify_candidates(object player) {
  object env = environment(player);
  object *candidates = ({});

  if (!env)
    return ({});

  // Get player's inventory
  candidates += all_inventory(player);

  // Get environment's inventory (excluding the player)
  candidates += all_inventory(env) - ({ player });

  return candidates;
}

/**
 * Matches a token pattern against objects in the environment.
 *
 * Attempts to match grammar tokens (OBJ, OBS, LIV, etc.) to actual
 * objects based on the command arguments.
 *
 * @param {string} pattern - The grammar pattern (e.g., "obj", "liv_with_obj")
 * @param {string} args - The command arguments to match
 * @param {object} player - The player issuing the command
 * @returns {mapping} Matched objects for each token position
 */
mapping match_pattern(string pattern, string args, object player) {
  mapping matches = ([]);
  object *candidates = identify_candidates(player);
  string *patterns = explode(pattern, "_");

  // Handle different pattern types
  switch (patterns[0]) {
    case "obj":
      // Try to find a single object match
      foreach (object obj in candidates) {
        if (obj->id(args)) {
          matches["obj"] = obj;
          break;
        }
      }
      break;

    case "str":
      // Just use the string as-is
      matches["str"] = args;
      break;

    case "liv":
      // Try to find a living object
      foreach (object obj in candidates) {
        if (living(obj) && obj->id(args)) {
          matches["liv"] = obj;
          break;
        }
      }
      break;
  }

  // Handle complex patterns with prepositions
  if (sizeof(patterns) > 2) {
    string prep = patterns[1]; // e.g., "with", "in", "on"
    string second_type = patterns[2]; // e.g., "obj", "liv", "str"

    // Try to parse "verb something with something_else"
    string remainder;
    if (sscanf(args, "%*s " + prep + " %s", remainder) == 2) {
      switch (second_type) {
        case "obj":
          foreach (object obj in candidates) {
            if (obj->id(remainder)) {
              matches[second_type] = obj;
              break;
            }
          }
          break;

        case "str":
          matches[second_type] = remainder;
          break;
      }
    }
  }

  return matches;
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
mixed check_object_participation(string verb, mapping matches) {
  mixed result;

  // Check direct object participation
  if (matches["obj"]) {
    string func = "direct_" + verb;

    if (function_exists(func, matches["obj"])) {
      result = call_other(matches["obj"], func);
      if (!result)
        return result;
    }
  }

  // Check indirect object participation
  if (matches["obj2"]) {
    string func = "indirect_" + verb;

    if (function_exists(func, matches["obj2"])) {
      result = call_other(matches["obj2"], func, matches["obj"]);
      if (!result)
        return result;
    }
  }

  return 1;
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
mixed execute_command(string verb, string pattern, mapping matches, object player) {
  string path = find_command(verb);
  object cmd;
  string func;
  mixed args;
  string *patterns;

  if (!path)
    return 0;

  cmd = load_object(path);

  if (!cmd)
    return 0;

  func = "do_" + verb + "_" + pattern;

  if (!function_exists(func, cmd))
    return 0;

  // Call the appropriate do_* function with the matched objects
  args = ({});

  // Build arguments based on pattern
  patterns = explode(pattern, "_");
  foreach (string p in patterns) {
    args += ({ matches[p] });
  }

  return call_other(cmd, func, player, args...);
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
int parse_and_execute(object player, string command) {
  mapping cmd_data = interpret_command(command, player);
  string verb;
  string args;
  string *patterns;

  if (!cmd_data)
    return 0;

  verb = cmd_data["verb"];
  args = cmd_data["args"];

  // Get the grammar patterns for this verb
  patterns = identify_verb_patterns(verb);

  if (!sizeof(patterns))
    return 0;

  // Try each pattern until one succeeds
  foreach (string pattern in patterns) {
    mapping matches = match_pattern(pattern, args, player);
    string can_func;
    string path;
    object cmd;
    mixed can_result;
    mixed participation;

    if (!sizeof(matches))
      continue;

    // Check if the verb handler allows this action
    can_func = "can_" + verb + "_" + pattern;
    path = find_command(verb);

    if (!path)
      continue;

    cmd = catch(load_object(path));

    if (!cmd)
      continue;

    if (!function_exists(can_func, cmd))
      continue;

    // Call the can_* function to see if the action is allowed
    can_result = call_other(cmd, can_func, player, matches...);

    if (!can_result)
      continue;

    // If it returned a string, display it as a message
    if (stringp(can_result)) {
      tell_object(player, can_result);
      // Continue if this is just a message, not an error
      if (intp(can_result) && !can_result)
        continue;
    }

    // Check if objects want to participate
    participation = check_object_participation(verb, matches);

    // WRONG
    if (!participation) {
      if (stringp(participation))
        tell_object(player, participation);
      continue;
    }

    // Execute the command
    return execute_command(verb, pattern, matches, player);
  }

  return 0;
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
int handle_command(object player, string command) {
  mapping cmd_data;
  // First try the parser-based approach
  int result = parse_and_execute(player, command);

  if (result)
    return result;

  // Fall back to the simpler command processing
  if (!command || command == "")
    return 0;

  // Process the command using the traditional approach
  cmd_data = process_command(player, command);

  if (!cmd_data)
    return 0;

  // Dispatch the command
  return dispatch_command(player, cmd_data);
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
mapping enhanced_match_pattern(string pattern, string args, object player) {
  mapping matches = ([]);
  object *candidates = identify_candidates(player);
  string *pattern_parts = explode(pattern, "_");

  // Parse the pattern into grammar tokens and prepositions
  string *tokens = ({});
  string *prepositions = ({});
  string *arg_parts;

  for (int i = 0; i < sizeof(pattern_parts); i++) {
    if (i % 2 == 0) {
      // Even indices are tokens (OBJ, STR, etc.)
      tokens += ({ pattern_parts[i] });
    } else {
      // Odd indices are prepositions (with, from, etc.)
      prepositions += ({ pattern_parts[i] });
    }
  }

  // Split args based on prepositions
  arg_parts = ({ args });
  if (sizeof(prepositions)) {
    foreach (string prep in prepositions) {
      string current = arg_parts[<1];
      string before, after;

      if (sscanf(current, "%s " + prep + " %s", before, after) == 2) {
        arg_parts[<1] = before;
        arg_parts += ({ after });
      } else {
        // Preposition not found, pattern can't match
        return 0;
      }
    }
  }

  // Now match each token with its corresponding arg part
  for (int i = 0; i < sizeof(tokens); i++) {
    string token = tokens[i];
    string arg = (i < sizeof(arg_parts)) ? arg_parts[i] : "";

    // Skip if we ran out of arguments for tokens
    if (arg == "")
      return 0;

    switch (token) {
      case "OBJ": {
        // Match a single object
        object obj = find_object_in_environment(arg, player);
        if (!obj)
          return 0;
        matches["OBJ" + (i > 0 ? i : "")] = obj;
        break;
      }
      case "OBS": {
        // Match multiple objects
        object *objs = find_objects_in_environment(arg, player);
        if (!sizeof(objs))
          return 0;
        matches["OBS" + (i > 0 ? i : "")] = objs;
        break;
      }
      case "LIV": {
        // Match a single living object
        object liv = find_living_in_environment(arg, player);
        if (!liv)
          return 0;
        matches["LIV" + (i > 0 ? i : "")] = liv;
        break;
      }
      case "LVS": {
        // Match multiple living objects
        object *livs = find_livings_in_environment(arg, player);
        if (!sizeof(livs))
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
        if (strsrch(arg, " ") != -1)
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
  object *candidates = identify_candidates(player);

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach (object obj in candidates) {
    if (!living(obj) && obj->id(desc))
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
  object *candidates = identify_candidates(player);
  object *matches = ({});

  // Handle "all" keyword
  if (desc == "all") {
    foreach (object obj in candidates) {
      if (!living(obj))
        matches += ({ obj });
    }
    return matches;
  }

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach (object obj in candidates) {
    if (!living(obj) && obj->id(desc))
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
  object *candidates = identify_candidates(player);

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach (object obj in candidates) {
    if (living(obj) && obj->id(desc))
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
  object *candidates = identify_candidates(player);
  object *matches = ({});

  // Handle "all" keyword for living objects
  if (desc == "all") {
    foreach (object obj in candidates) {
      if (living(obj))
        matches += ({ obj });
    }
    return matches;
  }

  // Remove "the" and other articles
  desc = remove_article(desc);

  foreach (object obj in candidates) {
    if (living(obj) && obj->id(desc))
      matches += ({ obj });
  }

  return matches;
}

/**
 * Checks if objects will participate in an action using the enhanced FluffOS
 * parser approach with direct_ and indirect_ prefixed methods.
 *
 * @param {string} verb - The verb being attempted
 * @param {mapping} matches - Grammar token matches from enhanced_match_pattern
 * @param {object} player - The player issuing the command
 * @returns {mixed} 1 if allowed, 0 or error string if not allowed
 */
mixed check_participation(string verb, mapping matches, object player) {
  mixed result;
  string *keys = keys(matches);

  // Check direct object participation
  foreach (string key in keys) {
    if (strsrch(key, "OBJ") == 0 || strsrch(key, "LIV") == 0) {
      object obj = matches[key];
      string func;

      // Skip if not an object
      if (!objectp(obj))
        continue;

      func = "direct_" + verb;
      if (function_exists(func, obj)) {
        // Build arguments for direct_ function based on other matches
        mixed *args = ({ });

        // Add indirect objects if they exist
        foreach (string k in keys) {
          if (strsrch(k, "OBJ") == 0 && k != key ||
              strsrch(k, "LIV") == 0 && k != key) {
            args += ({ matches[k] });
          }
        }

        result = call_other(obj, func, args...);

        if (stringp(result)) {
          tell_object(player, result);
          return 0;
        } else if (!result) {
          return 0;
        }
      }
    }
  }

  // Check indirect object participation
  foreach (string key in keys) {
    if (strsrch(key, "OBJ") == 0 && key != "OBJ" ||
        strsrch(key, "LIV") == 0 && key != "LIV") {
      object obj = matches[key];
      string func;

      // Skip if not an object
      if (!objectp(obj))
        continue;

      func = "indirect_" + verb;
      if (function_exists(func, obj)) {
        // Get the direct object if it exists
        object direct_obj = matches["OBJ"] || matches["LIV"];

        result = call_other(obj, func, direct_obj);

        if (stringp(result)) {
          tell_object(player, result);
          return 0;
        } else if (!result) {
          return 0;
        }
      }
    }
  }

  return 1;
}

/**
 * Improved parse_and_execute function that uses the enhanced matching.
 *
 * @param {object} player - The player issuing the command
 * @param {string} command - The raw command string
 * @returns {int} 1 if command was handled, 0 if not
 */
int improved_parse_and_execute(object player, string command) {
  string verb, args;
  string path;
  object cmd;
  string *patterns;

  if (!command || command == "")
    return 0;

  // Extract the verb and arguments
  if (sscanf(command, "%s %s", verb, args) != 2) {
    verb = command;
    args = "";
  }

  // Find the command handler
  path = find_command(verb);
  if (!path)
    return 0;

  cmd = catch(load_object(path));
  if (!cmd)
    return 0;

  // Get the grammar patterns for this verb
  patterns = identify_verb_patterns(verb);
  if (!sizeof(patterns))
    return 0;

  // Try each pattern until one succeeds
  foreach (string pattern in patterns) {
    mapping matches = enhanced_match_pattern(pattern, args, player);
    string can_func;
    mixed can_result;
    mixed *call_args;
    string *pattern_parts;
    mixed participation;
    string do_func;

    if (!matches || !sizeof(matches))
      continue;

    // Check if the verb handler allows this action
    can_func = "can_" + verb + "_" + pattern;

    if (!function_exists(can_func, cmd))
      continue;

    // Call the can_* function with appropriate arguments

    // Extract arguments from matches in the order they appear in the pattern
    call_args = ({ player });
    pattern_parts = explode(pattern, "_");

    foreach (string part in pattern_parts) {
      if (matches[part])
        call_args += ({ matches[part] });
    }

    can_result = call_other(cmd, can_func, call_args...);

    if (!can_result)
      continue;

    // If it returned a string, display it as a message
    if (stringp(can_result)) {
      tell_object(player, can_result);
      if (intp(can_result) && !can_result)
        continue;
    }

    // Check if objects want to participate
    participation = check_participation(verb, matches, player);

    if (!participation)
      continue;

    // Execute the command
    do_func = "do_" + verb + "_" + pattern;

    if (!function_exists(do_func, cmd))
      continue;

    // Call the do_* function with the same arguments as can_*
    return call_other(cmd, do_func, call_args...);
  }

  return 0;
}

/**
 * Resolves ambiguous object references when multiple matches are found.
 *
 * When multiple objects match a description, this function helps determine
 * which one the player most likely intended to reference.
 *
 * @param {object*} candidates - Array of matching objects
 * @param {object} player - Player issuing the command
 * @param {string} description - The description used to refer to the object
 * @returns {object} The most likely intended object
 */
object resolve_ambiguity(object *candidates, object player, string description) {
  // ...existing code...

  // Ask the player to clarify which object they meant
  // For now, just return the first match
  return candidates[0];
}

/**
 * Parses numeric prefixes in object descriptions.
 *
 * Handles references like "second sword" or "third potion from the chest".
 *
 * @param {string} desc - Object description to process
 * @returns {mapping} Mapping containing the parsed information
 */
mapping parse_numeric_prefix(string desc) {
  mapping result = ([]);
  string clean_desc = desc;
  int ordinal = 0;

  // Check for ordinal prefixes
  if (regexp(desc, "^(first|second|third|fourth|fifth|[0-9]+[a-z]{2}) ")) {
    string prefix;
    if (sscanf(desc, "%s %s", prefix, clean_desc) == 2) {
      switch(prefix) {
        case "first": ordinal = 1; break;
        case "second": ordinal = 2; break;
        case "third": ordinal = 3; break;
        case "fourth": ordinal = 4; break;
        case "fifth": ordinal = 5; break;
        default:
          if (sscanf(prefix, "%d", ordinal) != 1)
            ordinal = 0;
      }
    }
  }

  result["ordinal"] = ordinal;
  result["description"] = clean_desc;

  return result;
}

/**
 * Extends object finding to handle numeric prefixes.
 *
 * This extends find_object_in_environment to support references
 * like "second sword" or "third book".
 *
 * @param {string} desc - Object description to search for
 * @param {object} player - Player issuing the command
 * @returns {object} Matching object or 0 if none found
 */
object enhanced_find_object(string desc, object player) {
  mapping parsed = parse_numeric_prefix(desc);
  object *matches;

  // Use the clean description without ordinal
  matches = find_objects_in_environment(parsed["description"], player);

  if (!sizeof(matches))
    return 0;

  // If an ordinal was specified, try to return that item
  if (parsed["ordinal"] > 0) {
    if (parsed["ordinal"] <= sizeof(matches))
      return matches[parsed["ordinal"] - 1];
    else
      return 0; // Ordinal out of range
  }

  // If multiple matches and no ordinal, we have ambiguity
  if (sizeof(matches) > 1)
    return resolve_ambiguity(matches, player, desc);

  return matches[0];
}

/**
 * Improves the handling of container-based object references.
 *
 * Handles references like "sword from chest" or "gem in bag".
 *
 * @param {string} desc - Object description with container reference
 * @param {object} player - Player issuing the command
 * @returns {object} The object if found, 0 otherwise
 */
object find_object_in_container(string desc, object player) {
  string item_desc, container_desc, prep;
  object container;
  object *items;

  // Try to parse "item in/from/on container" format
  if (sscanf(desc, "%s in %s", item_desc, container_desc) == 2)
    prep = "in";
  else if (sscanf(desc, "%s from %s", item_desc, container_desc) == 2)
    prep = "from";
  else if (sscanf(desc, "%s on %s", item_desc, container_desc) == 2)
    prep = "on";
  else
    return 0; // Not a container reference

  // Find the container
  container = find_object_in_environment(container_desc, player);

  if (!container)
    return 0;

  // Check if the container can be opened/accessed
  if (function_exists("is_container", container) && !container->is_container())
    return 0;

  // For "from", the container must be open
  if (prep == "from" && function_exists("is_closed", container) &&
      container->is_closed())
    return 0;

  // Look for the item inside the container
  items = filter(all_inventory(container), (: $1->id($(item_desc)) :));

  if (!sizeof(items))
    return 0;

  if (sizeof(items) > 1) {
    // Handle ambiguity
    return resolve_ambiguity(items, player, item_desc);
  }

  return items[0];
}
