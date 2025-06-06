###############################################################################
#                   Oxidus Mudlib Runtime Configuation File                   #
###############################################################################
# NOTE: All paths specified here are relative to the mudlib directory except  #
#       for mudlib directory and the log directory.                           #
# Lines beginning with a # or a newline are ignored.                          #
###############################################################################

# name of this mud
name : Oxidus

# port number to accept users on
external_port_1: telnet 1336

# Restrict IP binding, if omitted, bind to all addresses.
#mud ip : 0.0.0.0

# absolute pathname of mudlib
mudlib directory : /home/gesslar/Oxidus

# debug.log and author/domain stats are stored here
log directory : /home/gesslar/Oxidus/log

# the directories which are searched by #include <...>
# for multiple dirs, separate each path with a ':'
include directories : /include:/include/driver:/adm/include

# the file which defines the master object
master file : /adm/obj/master

# the file where all global simulated efuns are defined.
simulated efun file : /adm/obj/simul_efun

# alternate debug.log file name (assumed to be in specified 'log directory')
debug log file : debug.log

# This is an include file which is automatically #include'd in all objects
global include file : <global.h>

# if an object is left alone for a certain time, then the
# function clean_up will be called. This function can do anything,
# like destructing the object. If the function isn't defined by the
# object, then nothing will happen.
# This time should be substantially longer than the swapping time.
time to clean up : 120000

# How long time until an unused object is swapped out.
# Machine with too many players and too little memory: 900 (15 minutes)
# Machine with few players and lot of memory: 10000
# Machine with infinite memory: 0 (never swap).
time to swap : 10000

# How many seconds until an object is reset again.
time to reset : 600

# Maximum number of bits in a bit field. They are stored in printable
# strings, 6 bits per byte.
maximum bits in a bitfield : 1200

# Max number of local variables in a function.
maximum local variables : 100

# Maximum amount of 'eval cost' per thread - execution is halted when
# it is exceeded.
maximum evaluation cost : 500000

# This is the maximum array size allowed for one single array.
maximum array size : 15000

# This is the maximum allowed size of a variable of type 'buffer'.
maximum buffer size : 400000

# Max size for a mapping
maximum mapping size : 1500000

# Max inherit chain size
inherit chain size : 30

# maximum length of a string variable
maximum string length : 200000

# Max size of a file allowed to be read by 'read_file()'.
maximum read file size : 200000

# max number of bytes you allow to be read and written with read_bytes
# and write_bytes
maximum byte transfer : 10000

# Define the size of the shared string hash table.  This number should
# a prime, probably between 1000 and 30000; if you set it to about 1/5
# of the number of distinct strings you have, you will get a hit ratio
# (number of comparisons to find a string) very close to 1, as found strings
# are automatically moved to the head of a hash chain.  You will never
# need more, and you will still get good results with a smaller table.
hash table size : 7001

# Object hash table size.
# Define this like you did with the strings; probably set to about 1/4 of
# the number of objects in a game, as the distribution of accesses to
# objects is somewhat more uniform than that of strings.
object table size : 1501

# default no-matching-action message
default fail message : What?

# default message when error() occurs (optional)
default error message : Something went wrong.

# Number of milliseconds between game ticks, affects call_out() granularity
gametick msec : 100

# Number of milliseconds between heartbeats
heartbeat interval msec : 2000

# explode():
#
# The old behavior (#undef both of the below) strips any number of
# delimiters at the start of the string, and one at the end.  So
# explode("..x.y..z..", ".") gives ({ "x", "y", "", "z", "" })
#
# SANE_EXPLODE_STRING strips off at most one leading delimiter, and
# still strips off one at the end, so the example above gives
# ({ "", "x", "y", "", "z", "" }).
#
# REVERSIBLE_EXPLODE_STRING overrides SANE_EXPLODE_STRING, and makes
# it so that implode(explode(x, y), y) is always x; i.e. no delimiters
# are ever stripped.  So the example above gives
# ({ "", "", "x", "y", "", "z", "", "" }).
#
sane explode string : 1
reversible explode string : 0

# SANE_SORTING: Use system provided fastest sorting routine for various
# sorting, including sort_array EFUN.
#
# This replace the old internal version qsort which only sorts to one
# direction repetitively. so following LPC code:
#
#    sort_array(({4,3,2,1}), (: -($1<$2) :));
#
# can still return ({1,2,3,4}), even though it only returns -1 and 0.
#
# It is recommended to fix your LPC code to not rely on this behavior.
#
# Your LPC code should return 1, 0, -1 for situation where first argument
# is less than, equal to, or greater than the second argument. This will
# will work with both implementation.
#
# Old code should work fine with this added, easy to inspect by searching
# for sort_array.
#
sane sorting : 1

# WARN_TAB: Some versions of the editor built in indent function use
#   tabs for indenting. This options turns on a warning message for
#   files indented with tabs instead of spaces.
#
warn tab : 0

# WOMBLES: don't allow spaces between start/end of array/mapping/functional
# token chars so ({1,2,3}) still works, but ( { 1 , 2 , 3 } ) doesn't
# and ({ 1 , 2 , 3 }) does.
wombles : 0

# CALL_OTHER_TYPE_CHECK: enable type checking for call_other()
# (-> operator on objects)
#
call other type check : 0

# CALL_OTHER_WARN, make it warning instead of errors
call other warn : 0

# MUDLIB_ERROR_HANDLER: If you define this, the driver doesn't do any
#   handling of runtime errors, other than to turn the heartbeats of
#   objects off.  Information about the error is passed in a mapping
#   to the error_handler() function in the master object.  Whatever is
#   returned is put in the debug.log.
#
# A good mudlib error handler is one of the best tools for tracking down
# errors.  Unfortunately, you need to have one.  Check the testsuite or
# other libs for an example.
#
mudlib error handler : 1

# NO_RESETS: completely disable the periodic calling of reset()
no resets : 0

# LAZY_RESETS: if this is defined, an object will only have reset()
#   called in it when it is touched via call_other() or move_object()
#   (assuming enough time has passed since the last reset).  If LAZY_RESETS
#   is #undef'd, then reset() will be called as always (which guaranteed that
#   reset would always be called at least once).  The advantage of lazy
#   resets is that reset doesn't get called in an object that is touched
#   once and never again (which can save memory since some objects won't get
#   reloaded that otherwise would).
#
lazy resets : 1

# RANDOMIZED_RESETS: if this is defined, then reset() will be called in
#   a randomized time interval, varying from between TIME_TO_RESET/2 to
#   TIME_TO_RESET-1.  If RANDOMIZED_RESETS is undefined, then reset()
#   will be called upon all objects at the same time using the interval
#   TIME_TO_RESET.
#
randomized resets : 1

# NO_ANSI: define if you wish to disallow users from typing in commands that
#   contain ANSI escape sequences.  Defining NO_ANSI causes all escapes
#   (ASCII 27) to be replaced with a space ' ' before the string is passed
#   to the action routines added with add_action.
#
# STRIP_BEFORE_PROCESS_INPUT allows the location where the stripping is
# done to be controlled.  If it is defined, then process_input() doesn't
# see ANSI characters either; if it is undefined ESC chars can be processed
# by process_input(), but are stripped before add_actions are called.
# Note that if NO_ADD_ACTION is defined, then #define NO_ANSI without
# #define STRIP_BEFORE_PROCESS_INPUT is the same as #undef NO_ANSI.
#
# If you anticipate problems with users intentionally typing in ANSI codes
# to make your terminal flash, etc define this.
#
no ansi : 1
strip before process input: 1

# THIS_PLAYER_IN_CALL_OUT: define this if you wish this_player() to be
#   usable from within call_out() callbacks.
#
this_player in call_out : 1

# TRACE: define this to enable the trace() and traceprefix() efuns.
#   (keeping this undefined will cause the driver to run faster).
#
trace : 1

# TRACE_CODE: define this to enable code tracing (the driver will print
#   out the previous lines of code to an error) eval_instruction() runs about
#   twice as fast when this is not defined (for the most common eoperators).
#
trace code : 0

# TRACE: set this to 1 to collect context information during LPC traces.
trace lpc execution context : 0
g
# TRACE: set this to 1 to collect LPC instruction information during LPC traces.
trace lpc instructions : 0

# INTERACTIVE_CATCH_TELL: define this if you want catch_tell called on
#   interactives as well as NPCs.  If this is defined, user.c will need a
#   catch_tell(msg) method that calls receive(msg);
#
interactive catch tell : 0

# RECEIVE_SNOOP: define this if you want snoop text to be sent to
#   the receive_snoop() function in the snooper object (instead of being
#   sent directly via add_message()).  This is useful if you want to
#  build a smart client that does something different with snoop messages.
#
receive snoop : 1

# SNOOP_SHADOWED: define this if you want snoop to report what is
#   sent to the player even in the event that the player's catch_tell() is
#   shadowed and the player may not be seeing what is being sent.  Messages
#   of this sort will be prefixed with $$.
#
snoop shadowed : 0

#  REVERSE_DEFER:
#
#  If not defined executes defer functions in LIFO mode.
#
#  If defined executes defer functions in FIFO mode.
#
reverse defer : 1

# OLD_TYPE_BEHAVIOR: reintroduces a bug in type-checking that effectively
#  renders compile time type checking useless.  For backwards compatibility.
#
#  Compat status: dealing with all the resulting compile errors can be
#  a huge pain even if they are correct, and the impact on the code is
#  small.
#
old type behavior : 0

# OLD_RANGE_BEHAVIOR: define this if you want negative indexes in string
# or buffer range values (not lvalue, i.e. x[-2..-1]; for e.g. not
# x[-2..-1] = foo, the latter is always illegal) to mean counting from the
# end
#
# Compat status: Not horribly difficult to replace reliance on this, but not
# trivial, and cannot be simulated.
#
old range behavior : 0

# define to get a warning for code that might use the old range behavior
# when you're not actually using the old range behavior
warn old range behavior : 1

# NONINTERACTIVE_STDERR_WRITE: if defined, all writes/tells/etc to
#   noninteractive objects will be written to stderr prefixed with a ']'
#   (old behavior).
#
# Compat status: Easy to support, and also on the "It's a bug!  No, it's
# a feature!" religious war list.
#
noninteractive stderr write : 1

# supress warnings about unused arguments; only warn about unused local
# variables.  Makes older code (where argument names were required) compile
# more quietly.
#
suppress argument warnings : 0

# call_out(0) loop prevention:
#
# This is the number of call_out(.., 0) call can be scheduled on the same
# gametick.
#
call_out(0) nest level : 10

# HAS_CONSOLE: If defined, the driver can take the argument -C
#   which will give the driver an interactive console (you can type
#   commands at the terminal.)  Backgrounding the driver will turn off
#   the console, but sending signal SIGTTIN (kill -21) to the driver can
#   turn it back on.  Typing 'help' will display commands available.
#   The intent is to allow the inspection of things that are difficult
#   to inspect from inside the mud.
#
has console : 1

# sprintf: Make format like "%10s" ignore ANSI escape code length, as they
# are not visible in the client.
#
sprintf add_justified ignore ANSI colors : 1

# add_action: Make enable_commands() always re-call init(). same as
# calling enable_commands(1).
#
enable_commands call init : 0

# TRAP_CRASHES:  define this if you want MudOS to call crash() in master.c
#   and then shutdown when signals are received that would normally crash the
#   driver.
#
trap crashes : 1

# telnet extensions support
enable mxp : 0
enable gmcp : 1
enable zmp : 0
enable mssp : 1
enable msp : 0
