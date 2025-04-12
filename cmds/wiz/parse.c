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
mixed main(object actor, string arg) {
  mapping result;

  _info("Parsing command: %s\n", arg);

  result = PARSE_D->handle_command(
    this_object(),
    query_file_name(),
    arg,
    deep_inventory(environment(actor)), actor
  );

  _info("Result: %O\n", result);

  return 1;
}
