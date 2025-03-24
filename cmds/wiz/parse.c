/**
 * @file /ox/lib/cmds/test/parse.c
 *
 * A test command for the parser system.
 *
 * @created 2024-08-07 - Gesslar
 * @last_modified 2024-08-07 - Gesslar
 */

inherit STD_CMD;

/**
 * Tests the parser with various grammar patterns.
 *
 * @param {object} actor - Player using the command
 * @param {string} args - Command arguments
 * @returns {int} 1 if successful, 0 otherwise
 */
mixed main(object actor, string args) {
  string test_str = args;
  mapping result;

  _info("Testing parser with: " + test_str + "\n");

  result = PARSE_D->interpret_command(test_str, actor);
  if(!result) {
    _error("Failed to interpret command.\n");
    return 0;
  }

  _info("Results %O\n", result);

  _info("Verb: " + identify(result["verb"]) + "\n");
  _info("Args: " + identify(result["args"]) + "\n");

  _info("Testing enhanced object finding...\n");

  if (sizeof(args) > 0) {
    object obj = PARSE_D->find_object_in_environment(result["args"], actor);

    if (obj)
      _ok("Found object: " + obj->query_short() + "\n");
    else
      _error("No object found matching: " + identify(args) + "\n");
  }

  return 1;
}
