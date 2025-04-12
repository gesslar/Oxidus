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
  /** @type {STD_CMD}    */ object  cmd;
                            string  verb;
                            string *verb_rules;
                            string  pattern;
                            string  arg;
                            string *args;
  /** @type {STD_ITEM*} */  object *object_candidates;
  /** @type {STD_ITEM*} */  object *objects;
  /** @type {({({string}),({int})*})} */ mixed *errors;
                            mixed   result;
}

// Forward declarations
private void reset();
public int handle_command(string command, object *obs, object player);
private void *tokenize_string(string input, string ref *output);
private void find_matching_rule(class CmdData cmd_data);
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
 * Main entry point for handling player commands with parser integration.
 *
 * Processes a command string using the grammatical parser approach,
 * checking object participation and executing the command.
 *
 * @param {STD_CMD} verb_ob - The command object handling the verb
 * @param {string} verb - The verb being executed
 * @param {string} [arg=""] - The raw command string
 * @param {STD_ITEM*} [obs=({})] - Array of candidate objects
 * @param {STD_BODY} [player=this_body()] - The player issuing the command
 * @returns {int} 1 if command was handled, 0 if not
 */
public int handle_command(object verb_ob, string verb, string arg: (:"":), object *obs: (:({}):), object player: (:this_body():)) {
  class CmdData work;
  mixed result;

  if(!objectp(verb_ob) ||
     !strlen(verb) ||
     !objectp(player))
    return 0;

  // Clear the parser state
  reset();

  // Populate with initial data
  work = new(class CmdData,
    user: player,
    cmd: verb_ob,
    verb_rules: verb_ob->query_verb_rules(),
    verb: verb,
    pattern: "",
    arg: trim(arg),
    args: ({ }),
    object_candidates: obs,
    objects: ({ }),
    errors: ({ }),
    result: null
  );

  pdebug("handle_command: %s", sprintf("%s %s", verb, arg));

  // Extract the verb and argument as a string.
  inc();
  pdebug("handle_command: tokenizing: %s", j(arg));
  tokenize_string(work.arg, ref work.args);


  // Find the matching verb rule for the command.
  inc();
  find_matching_rule(work);
  dec();

  dec();
}

private void find_matching_rule(class CmdData work) {
  string rule;

  foreach(rule in work.verb_rules) {
    string *rule_tokens = ({ });

    pdebug("find_matching_rule: tokenizing: %s", j(rule));
    tokenize_string(rule, ref rule_tokens);

    // Check if the rule matches the command)
    // The most basic rule pattern is ""s
    if(sizeof(rule_tokens) == sizeof(work.args)) {
      if(!sizeof(work.args)) {
        work.pattern = rule;
        return;
      }
    }
  }
}

private void *tokenize_string(string input, string ref *output) {
  input = trim(input);

  if(!strlen(input)) {
    output = ({ "" });
  } else {
    output = filter(explode(trim(input), " "), (: strlen :));
  }

  inc();
  pdebug("tokenize_string: %s", j(output));
  dec();
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
