//reboot.c

//Tacitus @ LPUniversity
//01-JULY-05
//Admin command

//Recoded on October 22nd, 2005 by Tacitus
//Last edited on October 22nd, 2005 by Tacitus

#include <logs.h>
#include <shutdown.h>

inherit STD_CMD;

mixed main(object caller, string arg) {
     string status;
     int time;
     status = SHUTDOWN_D->get_status();

     if(!adminp(previous_object())) return notify_fail("Error [reboot]: Access denied.\n");

     if(!arg) {
          if(status) tell_me("Shutdown: " + status);
          else tell_me("There is no shutdown or reboot currently in progress.\n");
          return 1;
     }

     if(arg == "stop") {
          if(!status) return notify_fail("Error: There is no shutdown or reboot currently in progress.\n");
          else SHUTDOWN_D->stop();
          log_file(LOG_SHUTDOWN, capitalize(caller->query_real_name()) + " canceled the sequence (" + time + "m) on " + ctime(time()) + "\n");
          return 1;
     } else {
          if(arg == "now") time = 0;
          else time = to_int(arg);
          if(time == 0 && arg != "now" && arg != "0") return notify_fail("SYNTAX: shutdown [<stop>||<time>/now]\n");
          log_file(LOG_SHUTDOWN, capitalize(caller->query_real_name()) + " started reboot sequence (" + time + "m) on " + ctime(time()) + "\n");
          SHUTDOWN_D->start(time, SYS_REBOOT);
          return 1;
     }
}

string help(object caller) {
    return (" SYNTAX: reboot [<stop>||<time>/now]\n\n" +
    "This command allows you to start a reboot of the mud. You\n"
    "can either supply the ammount of time intill you the reboot\n"
    "is to occur or stop the current shutdown/reboot by giving the\n"
    "argument stop. You may also give the argument now to reboot\n"
    "the mud as soon as the command is given.\n\n"
    "It is important to remember that this command will rebppt the\n"
    "the mud and will come back up. If you wish for the mud to not\n"
    "restart afterwards, then look at the shutdown command.\n\n" +
    "See also: shutdown\n");
}
