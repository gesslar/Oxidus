#include <simul_efun.h>
#include <modules.h>

//system.c

//Tacitus @ LPUniversity
//08-APR-05
//Simul-efuns

/* Please keep the following as is */
/**
 * Returns the name of the base library.
 *
 * @returns {string} The base library name.
 */
string baselib_name() {
  return "LPUniversity";
}

/**
 * Returns the version of the base library.
 *
 * @returns {string} The base library version.
 */
string baselib_version() {
  return "A-0610-R7";
}
/* Thanks */

/**
 * Returns the name of the MUD.
 *
 * @returns {string} The name of the MUD.
 */
string mud_name() {
  return MUD_NAME;
}

/**
 * Returns the name of the library being used by the MUD.
 *
 * @returns {string} The library name.
 */
string lib_name() {
  return mud_config("LIB_NAME");
}

/**
 * Returns the version of the library being used by the MUD.
 *
 * @returns {string} The library version.
 */
string lib_version() {
  return mud_config("LIB_VERSION");
}

/**
 * Returns the open status of the MUD.
 *
 * @returns {string} The open status of the MUD.
 */
string open_status() {
  return mud_config("OPEN_STATUS");
}

/**
 * Returns the admin email address for the MUD.
 *
 * @returns {string} The admin email address.
 */
string admin_email() {
  return mud_config("ADMIN_EMAIL");
}

/**
 * Returns the port number the MUD is running on.
 *
 * @returns {int} The port number.
 */
int port() {
  return __PORT__;
}

/**
 * Returns the simul_efun object.
 *
 * @returns {object "adm/obj/simul_efun"} The simul_efun object.
 */
object simul_efun() {
  return this_object();
}

/**
 * Returns the version of the MUD driver.
 *
 * @returns {string} The driver version.
 */
string driver_version() {
  string version = __VERSION__;
  string name, rest;

  sscanf(version, "%s %s", name, rest);
  if(strsrch(rest, "uncommited") != -1)
    sscanf(rest, "%s-uncommited", rest);

  return sprintf("%s %s", name, rest);
}

/**
 * Returns the architecture of the system the MUD is running on.
 *
 * @returns {string} The system architecture.
 */
string arch() {
  return __ARCH__;
}

/**
 * Retrieves a specific configuration value from the MUD config.
 *
 * @param {string} str - The configuration key to retrieve.
 * @returns {mixed} The configuration value.
 */
mixed mud_config(string str) {
  return CONFIG_D->get_mud_config(str);
}

/**
 * Returns the directory where log files are stored as configured
 * in the MUD configuration.
 *
 * @returns {string} The log directory.
 */
string log_dir() {
  return mud_config("LOG_DIR");
}

/**
 * Returns the directory where temporary files are stored.
 *
 * @returns {string} The temporary directory.
 */
string tmp_dir() {
  return mud_config("TMP_DIR");
}

/**
 * Returns the directory where documentation files are stored.
 *
 * @returns {string} The documentation directory.
 */

string doc_dir() {
  return mud_config("DOC_DIR");
}

/**
 * @property {string} type - The type of the message.
 * @property {message} message - The message to send to the player.
 *
 * A class representing a message to be sent to a player.
*/
class SystemMessage {
  string type;
  string message;
}

/**
 * Logs a debug message, optionally formatted with arguments.
 *
 * If the first argument is not a string, it will be printed using %O
 * notation using sprintf.
 *
 * If the first argument is a string, it will be printed according to
 * sprintf, using any following additional arguments as substitutions.
 *
 * @param {string} str - The debug message.
 * @param {...mixed} [args] - Optional arguments to format the message.
 */
varargs void debug(mixed str, mixed args...) {
  if(stringp(str)) {
    if(sizeof(args))
      str = sprintf(str, args...);
  } else {
    str = sprintf("%O", str);
  }

  str = COLOUR_D->substitute_colour(str, "on");

  debug_message(str);
}

private nosave mapping _symbols = ([
  "ok"      : ({ ({ SYSTEM_OK,      "" }), ({ "\u2022 ", "o ", "" }) }), // • = bullet point
  "error"   : ({ ({ SYSTEM_ERROR,   "" }), ({ "\u25CF ", "o ", "" }) }), // ● = black circle
  "warn"    : ({ ({ SYSTEM_WARNING, "" }), ({ "\u25B2 ", "o ", "" }) }), // ▲ = black up-pointing triangle
  "info"    : ({ ({ SYSTEM_INFO,    "" }), ({ "\u25A0 ", "o ", "" }) }), // ■ = black square
  "question": ({ ({ SYSTEM_QUERY,   "" }), ({ "\u25C6 ", "o ", "" }) }), // ◆ = black diamond
  "debug"   : ({ ({ SYSTEM_DEBUG,   "" }), ({ "\u25A1 ", "o ", "" }) }), // □ = white square
]);

private string _format_message(string type, string str, mixed args...) {
  /** @type {STD_BODY} */
  object body = this_body();
  mixed *symbol = _symbols[type];
  int colour_index, tag_index;
  string final;

  if(body) {
    if(body->has_screenreader()) {
      colour_index = 1;
      tag_index = 2;
    } else {
      colour_index = 0;

      if(M_UNICODE->supports_unicode())
        tag_index = 0;
      else
        tag_index = 1;
    }
  } else {
    colour_index = 1;
    tag_index = 2;
  }

  final = symbol[0][colour_index] + symbol[1][tag_index] + "{{res}}";
  str = final + str;
  str = sprintf(str, args...);

  return str;
}

private varargs class SystemMessage constructMessageFromArgs(string type, mixed args...) {
  string str;
  class SystemMessage system_message;

  if(!sizeof(args))
    return 0;

  if(!stringp(args[0]) && !objectp(args[0]))
    return 0;

  if(objectp(args[0])) {
    str = args[1];
    args = args[2..];
  } else if(stringp(args[0])) {
    str = args[0];
    args = args[1..];
  }

  system_message = new(class SystemMessage,
    type: type,
    message: _format_message(type, str, args...)
  );

  return system_message.message
    ? system_message
    : 0;
}

private int _feedback(string type, mixed args...) {
  class SystemMessage result;
  object tp;
  string str;
  string mess;

  result = constructMessageFromArgs(type, args...);
  if(!result)
    return 0;

  tp = objectp(args[0])
    ? args[0]
    : this_body()
  ;

  if(tp)
    tell(tp, append(result.message, "\n"));
  else
    debug(mess);

  return 1;
}

/**
 * Sends a message to a player using accessible notation to indicate
 * that it is a confirmation/success system message.
 *
 * @override varargs int _ok(object tp, string str, mixed... args)
 * @override varargs int _ok(string str, mixed... args)
 * @description Provides a confirmation message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              sent to the debug log.
 * @param {STD_PLAYER} [tp] - The target object to receive the message.
 * @param {string} str - The confirmation message.
 * @param {mixed...} [args] - Optional arguments to format the message.
 * @returns {int} Always returns 1, unless there is no previous object.
 */
varargs int _ok(mixed args...) {
  return _feedback("ok", args...);
}

/**
 * Sends a message to a player using accessible notation to indicate
 * that it is a error/failure system message.
 *
 * @override varargs int _error(object tp, string str, mixed... args)
 * @override varargs int _error(string str, mixed... args)
 * @description Provides a confirmation message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              sent to the debug log.
 * @param {STD_PLAYER} [tp] - The target object to receive the message.
 * @param {string} str - The confirmation message.
 * @param {mixed...} [args] - Optional arguments to format the message.
 * @returns {int} Always returns 1, unless there is no previous object.
 */
varargs int _error(mixed args...) {
  return _feedback("error", args...);
}

/**
 * @simul_efun _warn
 * @def varargs int _warn(object tp, string str, mixed args...)
 * @def varargs int _warn(string str, mixed args...)
 * @description Provides an warning message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              sent to the debug log.
 * @param {string} str - The warning message.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {int} - Always returns 1, unless there is no previous object.
 */
varargs int _warn(mixed args...) {
  return _feedback("warn", args...);
}

/**
 * @simul_efun _info
 * @def varargs int _info(object tp, string str, mixed args...)
 * @def varargs int _info(string str, mixed args...)
 * @description Provides an informational message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              sent to the debug log.
 * @param {string} str - The informational message.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {int} - Always returns 1, unless there is no previous object.
 */
varargs int _info(mixed args...) {
  return _feedback("info", args...);
}

/**
 * @simul_efun _question
 * @def varargs int _question(object tp, string str, mixed args...)
 * @def varargs int _question(string str, mixed args...)
 * @description Provides a question message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              discarded.
 * @param {string} str - The question message.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {int} - Always returns 1, unless there is no body object.
 */
varargs int _question(mixed args...) {
  return _feedback("question", args...);
}

/**
 * @def varargs int _debug(object tp, string str, mixed args...)
 * @def varargs int _debug(string str, mixed args...)
 * @description Provides a question message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              discarded.
 * @param {string} str - The question message.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {int} - Always returns 1, unless there is no body object.
 */
varargs int _debug(mixed args...) {
  return _feedback("debug", args...);
}

/**
 * @simul_efun boot_number
 * @description Returns the boot number of the MUD. This is the number of
 *              times the MUD has been booted.
 * @returns {int} - The current boot number.
 */
int boot_number() {
  return BOOT_D->query_boot_number();
}
