#include "/adm/obj/simul_efun.h"
#include <modules.h>

//system.c

//Tacitus @ LPUniversity
//08-APR-05
//Simul-efuns

#include <config.h>

/* Please keep the following as is */
/**
 * @simul_efun baselib_name
 * @description Returns the name of the base library.
 * @returns {string} - The base library name.
 */
string baselib_name() {
    return "LPUniversity";
}

/**
 * @simul_efun baselib_version
 * @description Returns the version of the base library.
 * @returns {string} - The base library version.
 */
string baselib_version() {
    return "A-0610-R7" ;
}
/* Thanks */

/**
 * @simul_efun mud_name
 * @description Returns the name of the MUD.
 * @returns {string} - The name of the MUD.
 */
string mud_name() {
    return MUD_NAME ;
}

/**
 * @simul_efun lib_name
 * @description Returns the name of the library being used by the MUD.
 * @returns {string} - The library name.
 */
string lib_name() {
    return mud_config("LIB_NAME") ;
}

/**
 * @simul_efun lib_version
 * @description Returns the version of the library being used by the MUD.
 * @returns {string} - The library version.
 */
string lib_version() {
    return mud_config("LIB_VERSION") ;
}

/**
 * @simul_efun open_status
 * @description Returns the open status of the MUD.
 * @returns {string} - The open status of the MUD.
 */
string open_status() {
    return mud_config("OPEN_STATUS") ;
}

/**
 * @simul_efun admin_email
 * @description Returns the admin email address for the MUD.
 * @returns {string} - The admin email address.
 */
string admin_email() {
    return mud_config("ADMIN_EMAIL") ;
}

/**
 * @simul_efun port
 * @description Returns the port number the MUD is running on.
 * @returns {int} - The port number.
 */
int port() {
    return __PORT__;
}

/**
 * @simul_efun driver_version
 * @description Returns the version of the MUD driver.
 * @returns {string} - The driver version.
 */
string driver_version() {
    string version = __VERSION__ ;
    string name, rest ;

    sscanf(version, "%s %s", name, rest) ;
    if(strsrch(rest, "uncommited") != -1)
        sscanf(rest, "%s-uncommited", rest) ;

    return sprintf("%s %s", name, rest) ;
}

/**
 * @simul_efun arch
 * @description Returns the architecture of the system the MUD is running on.
 * @returns {string} - The system architecture.
 */
string arch() {
    return __ARCH__;
}

/**
 * @simul_efun mud_config
 * @description Retrieves a specific configuration value from the MUD config.
 * @param {string} str - The configuration key to retrieve.
 * @returns {mixed} - The configuration value.
 */
mixed mud_config(string str) {
    return (mixed)CONFIG_D->get_mud_config(str) ;
}

/**
 * @simul_efun log_dir
 * @description Returns the directory where log files are stored.
 * @returns {string} - The log directory.
 */
string log_dir() {
    return mud_config("LOG_DIR") ;
}

/**
 * @simul_efun tmp_dir
 * @description Returns the directory where temporary files are stored.
 * @returns {string} - The temporary directory.
 */
string tmp_dir() {
    return mud_config("TMP_DIR") ;
}

/**
 * @simul_efun doc_dir
 * @description Returns the directory where documentation files are stored.
 * @returns {string} - The documentation directory.
 */

string doc_dir() {
    return mud_config("DOC_DIR") ;
}

/**
 * @simul_efun debug
 * @description Logs a debug message, optionally formatted with arguments.
 * @param {string} str - The debug message.
 * @param {mixed} [args] - Optional arguments to format the message.
 */
varargs void _debug(mixed str, mixed args...) {
    if(stringp(str)) {
        if(sizeof(args))
            str = sprintf(str, args...) ;
    } else {
        str = sprintf("%O", str) ;
    }

    str = COLOUR_D->substitute_colour(str, "xterm") ;

    debug_message(str) ;
}

private nosave string _brief_circle = "\u25CB" ;

/**
 * @function _format_message
 * @description Formats a message with a color and the name of the previous
                object.
 * @param {string} color - The color to use for the message.
 * @param {string} str - The message to format.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {string} - The formatted message or null if there is no previous
                       object.
 */
private string _format_message(string color, string str, mixed args...) {
    object body = this_body() ;
    string tag ;

    if(body) {
        if(body->has_screenreader())
            tag = "" ;
        else
            if(M_UNICODE->supports_unicode())
                tag = _brief_circle ;
            else
                tag = "o" ;

            tag = color + tag + "\eres\e " ;
    }

    str = tag + str ;
    str = sprintf(str, args...) ;

    return str ;
}

/**
 * @simul_efun _ok
 * @def varargs int _ok(object tp, string str, mixed args...)
 * @def varargs int _ok(string str, mixed args...)
 * @description Provides an confirmation message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              sent to the debug log.
 * @param {string} str - The confirmation message.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {int} - Always returns 1, unless there is no previous object.
 */
varargs int _ok(mixed args...) {
    object tp ;
    string str ;
    string mess ;

    if(!sizeof(args))
        return 0 ;

    if(objectp(args[0])) {
        tp = args[0] ;
        str = args[1] ;
        args = args[2..] ;
    } else if(stringp(args[0])) {
        str = args[0] ;
        args = args[1..] ;
    } else {
        return 0 ;
    }

    if(!tp)
        tp = this_body() ;

    mess = _format_message("\e0036\e", str, args...) ;
    if(nullp(mess))
        return 0 ;

    if(tp)
        tell(tp, mess + "\n") ;
    else
        _debug(mess) ;

    return 1 ;
}

/**
 * @simul_efun _error
 * @def varargs int _error(object tp, string str, mixed args...)
 * @def varargs int _error(string str, mixed args...)
 * @description Provides an error message, optionally formatted with
 *              arguments. If no object is provided, the message will be sent
 *              to this_body(). If no object is found, the message will be
 *              sent to the debug log.
 * @param {string} str - The error message.
 * @param {mixed} [args] - Optional arguments to format the message.
 * @returns {int} - Always returns 1, unless there is no previous object.
 */
varargs int _error(mixed args...) {
    object tp ;
    string str ;
    string mess ;

    if(!sizeof(args))
        return 0 ;

    if(objectp(args[0])) {
        tp = args[0] ;
        str = args[1] ;
        args = args[2..] ;
    } else if(stringp(args[0])) {
        str = args[0] ;
        args = args[1..] ;
    } else {
        return 0 ;
    }

    if(!tp)
        tp = this_body() ;

    mess = _format_message("\e0160\e", str, args...) ;
    if(nullp(mess))
        return 0 ;

    if(tp)
        tell(tp, mess + "\n") ;
    else
        _debug(mess) ;

    return 1 ;
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
    object tp ;
    string str ;
    string mess ;

    if(!sizeof(args))
        return 0 ;

    if(objectp(args[0])) {
        tp = args[0] ;
        str = args[1] ;
        args = args[2..] ;
    } else if(stringp(args[0])) {
        str = args[0] ;
        args = args[1..] ;
    } else {
        return 0 ;
    }

    if(!tp)
        tp = this_body() ;

    mess = _format_message("\e0214\e", str, args...) ;
    if(nullp(mess))
        return 0 ;

    if(tp)
        tell(tp, mess + "\n") ;
    else
        _debug(mess) ;

    return 1 ;
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
    object tp ;
    string str ;
    string mess ;

    if(!sizeof(args))
        return 0 ;

    if(objectp(args[0])) {
        tp = args[0] ;
        str = args[1] ;
        args = args[2..] ;
    } else if(stringp(args[0])) {
        str = args[0] ;
        args = args[1..] ;
    } else {
        return 0 ;
    }

    if(!tp)
        tp = this_body() ;

    mess = _format_message("\e0228\e", str, args...) ;
    if(nullp(mess))
        return 0 ;

    if(tp)
        tell(tp, mess + "\n") ;
    else
        _debug(mess) ;

    return 1 ;
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
    object tp ;
    string str ;
    string mess ;

    if(!sizeof(args))
        return 0 ;

    if(objectp(args[0])) {
        tp = args[0] ;
        str = args[1] ;
        args = args[2..] ;
    } else if(stringp(args[0])) {
        str = args[0] ;
        args = args[1..] ;
    } else {
        return 0 ;
    }

    if(!tp)
        tp = this_body() ;
    if(!tp)
        return 0 ;

    mess = _format_message("\e0033\e", str, args...) ;
    if(nullp(mess))
        return 0 ;

    tell(tp, mess) ;

    return 1 ;
}
