/*
 wms.c

 WMS -- Wizard Management System

 Tacitus  @ ImperialExpansion
 28-MAR-2005
 Wizard Management System

*/


/* Updates:

Fixed bug related to flags :: 29-MAR-05
Fixed edit profile bug     :: 29-MAR-05
Modified history msg       :: 30-MAR-05
Initial implementation     :: 31-MAR-05
Added file logging         :: 31-MAR-05
Added auto-add code        :: 01-APR-05
Ported to LPUniversity     :: 06-MAY-06

*/

inherit STD_CMD;

#define DATAFILE "/data/tools/wms"
#define WRITEDIR "/open/"
#define WMS_LOG "adm/wms.log"

object restore_user(string name);
int auto_add(string str);
int auto_add2();
void add_user();
void add_user2(string str);
void add_user3();
void edit_user();
void edit_user2(string name);
void edit_user3();
void note_user();
void note_user2(string name);
void note_user3(string str);
void note_user4(string str, int i);
void note_user5();
void flag_user();
void flag_user2(string str);
void flag_user3(string str);
void remove_user();
void remove_user2(string str);
void read_note();
void read_note2(string str);
void filter_list();
void filter_list2(string str);
void disp_user(string str);
void summary();
string datafile();
string query_flags(string str);
string help(object caller);

mapping users, notes, types;
string current;
string current_file;
int idn;
int did_write;

void setup() {
    users = ([]);
    notes = ([]);
    types = ([]);
    restore_object(datafile());
}

string datafile() {
    return DATAFILE;
}

mixed main(object caller, string str) {
    if(!adminp(previous_object())) return notify_fail("Error [wms]: Access denied.\n");

    if(!str) {
        summary();
        return 1;
    }

    switch(str) {
        case "add" : add_user(); break;
        case "edit" : edit_user(); break;
        case "note" : note_user(); break;
        case "flag" : flag_user(); break;
        case "remove" : remove_user(); break;
        case "read" : read_note(); break;
        case "list" : filter_list(); break;
        default : disp_user(str); break;
    }

    return 1;
}

void add_user() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("Please enter the user you wish to add []: ");
    input_to("add_user2");
}

void add_user2(string str) {
    int count;
    mapping data;
    object user;

    if(!adminp(this_body()))
        return 0;

    str = lower_case(str);
    data = ([]);

    if(!str) {
        if(count) {
            tell_me("Now exiting WMS...\n");
            return;
        }

        tell_me("Error: No name supplied.\nPlease enter the user you wish to add [quit]: ");
        count = 1;
        input_to("add_user2");
        return;
    }

    if(str == "quit") {
        tell_me("Now exiting WMS...\n");
        return;
    }

    user = restore_user(str);

    if(!user) {
        tell_me("Error: No such user.\nPlease enter the user you wish to add [quit]: ");
        count = 1;
        input_to("add_user2");
        return;
    }

    if(users[str]) {
        tell_me("Error: User already in database... Aborting.\n");
        return;
    }

    data += (["history" : ({"User created -- User Management - " + capitalize(this_body()->query_real_name()) + " - " + (string)ctime(time()) }) ]);
    data += (["notes" : ({}) ]);
    data += (["flags" : ({}) ]);
    data += (["profile" : ""]);
    data += (["added" : time() ]);
    data += (["email" : user->query_pref("email") ]);
    users += ([str : data]);
    current = str;
    tell_me("Data structure created for user " + capitalize(str) + ".\n");
    tell_me("Now entering editor... Please enter profile/user history.\n");

    current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();
    while(file_exists(current_file)) current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();

    write_file(current_file, "");
    ed(current_file, "callback_write", "add_user3", 0);

    log_file(WMS_LOG, capitalize(str) + " was added to the WMS database manually by " + capitalize(this_body()->query_real_name()) + " on " + ctime(time()) + "\n");
    return;
}

int callback_write(string fname, int flag) {
    if(!master()->valid_read(fname, this_object(), "callback_exit"))
        return 0;
    current_file = fname;
    did_write = 1;
    return 1;
}

void add_user3() {
    mapping data;
    string file;

    if(!adminp(this_body()))
        return 0;

    file = read_file(current_file);
    data = users[current];
    data["profile"] = file;
    users[current] = data;
    current = "";
    rm(current_file);
    current_file = "";
    tell_me("Success: User addition to database complete.\n");
    save_object(datafile());
    return;
}

int auto_add(string str) {
    mapping data;
    object user;

    if(!adminp(this_body())) return 0;

    user = restore_user(str);
    if(!user) return 1;
    if(users[str]) return 1;

    data = ([]);
    data += (["history" : ({"User created -- User Management - " + capitalize(this_body()->query_real_name()) + " - " + (string)ctime(time()) }) ]);
    data += (["notes" : ({}) ]);
    data += (["flags" : ({}) ]);
    data += (["profile" : ""]);
    data += (["added" : time() ]);
    data += (["email" : user->query_pref("email") ]);
    users += ([str : data]);
    tell_me("WMS: Data structure created for user " + capitalize(str) + ".\n");
    log_file(WMS_LOG, capitalize(str) + " was added to the WMS database automaticaly on " + ctime(time()) + "\n");
    tell_me("Now entering editor... Please enter profile/user history.\n");
    current = str;



    current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();
    while(file_exists(current_file)) current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();

    write_file(current_file, "");
    ed(current_file, "callback_write", "add_user3", 0);

    return 1;
}

void edit_user() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("User you wish to edit: ");
    input_to("edit_user2");
    return;
}

void edit_user2(string name) {
    int count;

    if(!adminp(this_body()))
        return 0;

     if(!name) {
        if(count) {
            tell_me("Now exiting WMS...\n");
            return;
        }

        tell_me("Error: No name supplied.\nPlease enter the user you wish to edit [quit]: ");
        count = 1;
        input_to("edit_user2");
        return;
    }
    if(name == "quit") {
        tell_me("Now exiting WMS...\n");
            return;
    }

    if(!users[name]) {
        tell_me("Error: User not found in database.\nPlease enter the user you wish to edit [quit]: ");
        count = 1;
        input_to("edit_user2");
        return;
    }

    current = name;

    tell_me("\nPlease update the user's profile using ed:\n\n");

    current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();
    while(file_exists(current_file)) current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();

    write_file(current_file, "");
    ed(current_file, "callback_write", "edit_user3", 0);

    log_file(WMS_LOG, capitalize(this_body()->query_real_name()) + " edited " + capitalize(name) + "'s profile on " + ctime(time()) + ".\n");
}

void edit_user3() {
    string file;

    if(!adminp(this_body()))
        return 0;

    file = read_file(current_file);

    users[current]["history"] += ({ "Profile edited -- User management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
    users[current]["profile"] = file;

    current = "";
    rm(current_file);
    current_file = "";
    tell_me("Success: Profile edit complete.\n");
    save_object(datafile());
    return;
}

void note_user() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("Please enter the name of the user who you wish to add a note to: ");
    input_to("note_user2");
    return;
}

void note_user2(string name) {
    int count;

    if(!adminp(this_body()))
        return 0;

    if(!name) {
        if(count) {
            tell_me("Now exiting WMS...\n");
            return;
        }

        tell_me("Error: No name supplied.\nPlease enter the user you wish to note [quit]: ");
        count = 1;
        input_to("note_user2");
        return;
    }
    if(name == "quit") {
        tell_me("Now exiting WMS...\n");
            return;
    }

if(!users[name]) {
        tell_me("Error: User not found in database.\nPlease enter the user you wish to note [quit]: ");
        count = 1;
        input_to("note_user2");
        return;
    }

    current = name;
    tell_me("Please select a type of note you wish to make: \n");
    message("note", "Note types:\n", this_body());
    tell_me("1 : Warning\n");
    tell_me("2 : Praise\n");
    tell_me("3 : Action\n");
    tell_me("4 : Project success\n");
    tell_me("5 : Project failure\n");
    tell_me("6 : Promoted\n");
    tell_me("7 : Demoted\n");
    tell_me("8 : Loss of privilage\n");
    tell_me("9 : Gain of privilage\n");
    input_to("note_user3");
}

void note_user3(string str) {
    int count;

    if(!adminp(this_body()))
        return 0;

    if(!str) {
        if(count) {
            tell_me("Now exiting WMS...\n");
            return;
        }

        tell_me("Error: No type supplied.\nPlease enter the type of note you wish to add [quit]: ");
        count = 1;
        input_to("note_user3");
        return;
    }

    if(str == "quit") {
        tell_me("Now exiting WMS...\n");
            return;
    }

    if(!to_int(str)) {
        tell_me("Error: Invalid type supplied.\nPlease enter the type of note you wish to add [quit]: ");
        count = 1;
        input_to("notes_user3");
        return;
    }

    switch(to_int(str)) {
        case 1..9 : break;
        default : {
            tell_me("Error: Invalid type supplied.\nPlease enter the type of note you wish to add [quit]: ");
            count = 1;
            input_to("notes_user3");
            return;
        }
    }

    tell_me("Is this note to be public (viewable by other admins)? [yes] : ");
    input_to("note_user4", 0, to_int(str));
}

void note_user4(string str, int i) {
    int *ids;
    mapping data, note, new_note;
    string *keys, type;

    if(!adminp(this_body()))
        return 0;

    keys = keys(notes);
    data = ([]);
    new_note = ([]);
    note = notes;

    if(str == "quit") {
        tell_me("Now exiting WMS...\n");
            return;
    }

    if(str != "no" || str == "y" || str == "yes" || str == "public") {
        str = "public";
    } else {
        str = this_body()->query_real_name();
    }

    data = users[current];
    ids = data["notes"];
    idn = sizeof(keys) + 1;
    ids += ({idn });
    data["notes"] = ids;
    users[current] = data;

    switch(i) {
        case 1 : type = "Warning"; break;
        case 2 : type = "Praise"; break;
        case 3 : type = "Action"; break;
        case 4 : type = "PSuccess"; break;
        case 5 : type = "PFailure"; break;
        case 6 : type = "Promoted"; break;
        case 7 : type = "Demoted"; break;
        case 8 : type = "LOP"; break;
        case 9 : type = "GOP"; break;
        default : type = "Type Unknown"; break;
    }

    new_note += (["perm" : str, "type" : type, "admin" : this_body()->query_real_name(), "date" : time(), "for" : current]);
    note += ([idn : new_note]);
    tell_me("\nNow entering editor... Please enter note contents.\n");


    current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();
    while(file_exists(current_file)) current_file = "/tmp/" + random(9999999) + "." + this_body()->query_real_name();

    write_file(current_file, "");
    ed(current_file, "callback_write", "note_user5", 0);

    log_file(WMS_LOG, capitalize(this_body()->query_real_name()) + " made a " + type + " note for " + capitalize(current) + " on " + ctime(time()) + ".\n");
}

void note_user5() {
    mapping data, user;
    string file, *arr;

    if(!adminp(this_body()))
    return 0;

    file = read_file(current_file);
    data = notes[idn];
    user = ([]);
    user = users[current];
    arr = user["history"];
    arr += ({ "Note added -- Note Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
    user["history"] = arr;
    users[current] = user;
    current = "";
    data["msg"] = file;
    rm(current_file);
    current_file = "";
    idn = 0;
    tell_me("Success: Note content added.\n");
    save_object(datafile());
}

void flag_user() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("Please enter the name of the user who you wish to flag: ");
    input_to("flag_user2");
}

void flag_user2(string str) {
    mapping data;
    int count;
    string *flags;

    if(!adminp(this_body()))
        return 0;

    if(!str) {
        if(count) {
            tell_me("Now exiting WMS...\n");
            return;
        }

        tell_me("Error: No name supplied.\nPlease enter the type of note you wish to add [quit]: ");
        count = 1;
        input_to("flag_user2");
        str = "quit";
        return;
    }

    if(str == "quit") {
        tell_me("Now exiting WMS...\n");
            return;
    }

    if(!users[str]) {
        tell_me("Error: User not found in database.\nPlease enter the type of note you wish to add [quit]: ");
        count = 1;
        input_to("flag_user2");
        return;
    }

    data = users[str];
    current = str;
    tell_me(capitalize(str) + " currently has flags toggled as follows:\n\n");
    flags = data["flags"];

    if(member_array("excellent performance", flags) != -1) tell_me("1 ) Excellent performance : Toggled\n");
    else tell_me("1 ) Excellent performance : Not Toggled\n");
    if(member_array("inactive", flags) != -1) tell_me("2 ) Inactive : Toggled.\n");
    else tell_me("2 ) Inactive : Not Toggled.\n");
    if(member_array("suspended", flags) != -1) tell_me("3 ) Suspended : Toggled.\n");
    else tell_me("3 ) Suspended : Not Toggled.\n");
    if(member_array("discipline problem", flags) != -1) tell_me("4 ) Discipline Problem: Toggled.\n");
    else tell_me("4 ) Discipline Problem: Not Toggled.\n");
    if(member_array("recruiter", flags) != -1) tell_me("5 ) Recruiter : Toggled.\n");
    else tell_me("5 ) Recruiter : Not Toggled.\n");

    tell_me("\n");
    tell_me("Please enter the number of the item you wish to toggle/untoggle [quit]: ");
    input_to("flag_user3");
}

void flag_user3(string str) {
     int count;
     mapping data;
     string *flags, *hist;

     if(!adminp(this_body()))
        return 0;

     data = users[current];
     flags = ({});
     hist = ({});
     flags = data["flags"];
     hist = data["history"];
     tell_me("\n");

     if(!str || str == "quit") {
           tell_me("Now exiting WMS...\n");
           return;
     }

     if(!to_int(str)) {
          if(count) {
               tell_me("Now exiting WMS...\n");
               return;
          }

          tell_me("Error: Invalid argument supplied.\nPlease enter the number of the item you wish to toggle/untoggle [quit]: ");
          input_to("flag_user3");
          count = 1;
          return;
     }

    switch(to_int(str)) {
        case 1 :
            if(member_array("excellent performance", flags) != -1) {
                flags -= ({ "excellent performance" });
                hist += ({"Untoggled E -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            } else {
                flags += ({ "excellent performance" });
                hist += ({"Toggled E -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            }

        case 2 :
            if(member_array("inactive", flags) != -1) {
                flags -= ({ "inactive" });
                hist += ({"Untoggled I -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            } else {
                flags += ({ "inactive" });
                hist += ({"Toggled I -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            }

        case 3 :
            if(member_array("suspended", flags) != -1) {
                flags -= ({ "suspended" });
                hist += ({"Untoggled S -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            } else {
                flags += ({"suspended" });
                hist += ({"Toggled I -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            }

        case 4 :
            if(member_array("discipline problem", flags) != -1) {
                flags -= ({"discipline problem"});
                hist += ({"Untoggled D -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            } else {
                flags += ({"discipline problem"});
                hist += ({"Toggled D -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            }

        case 5 :
            if(member_array("recruiter", flags) != -1) {
                flags -= ({"recruiter"});
                hist += ({"Untoggled R -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            } else {
                flags += ({"recruiter"});
                hist += ({"Toggled R -- Flag Management - " + capitalize(this_body()->query_real_name()) + " - " + ctime(time()) });
                break;
            }

        default :
            if(count) {
                tell_me("Now exiting WMS...\n");
                return;
            }

            tell_me("Error : Invalid argument.\nPlease enter the number of the item you wish to toggle/untoggle [quit]: ");
            count = 1;
            input_to("flag_user3");
            return;
    }

    data["history"] = hist;
    data["flags"] = flags;
    users[current] = data;
    count = 0;
    tell_me("Flags updated for " + capitalize(current) + "\n");
    log_file(WMS_LOG, capitalize(this_body()->query_real_name()) + " modified " + capitalize(current) + "'s flags on " + ctime(time()) + "\n");
    current = "";
    save_object(datafile());
}

void filter_list() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("Filter Options:\n");
    tell_me("1 : Alphabetical Order\n");
    tell_me("2 : E flag only\n");
    tell_me("3 : D flag only\n");
    tell_me("4 : I flag only\n");
    tell_me("5 : S flag only\n");
    tell_me("6 : R flag only\n");
    tell_me("7 : None\n");
    tell_me("\n" + "Please select the filter option you wish to use [quit]: ");
    input_to("filter_list2");
}

void filter_list2(string str) {
    string *arr, flag, *flags, *keys;
    int i;
    mapping data, temp;

    if(!adminp(this_body()))
        return 0;

    arr = ({});
    temp = ([]);
    data = ([]);
    keys = ({});

    if(!str) str = "7";

    if(!to_int(str)) {
        tell_me("Error: Invalid argument supplied.\nNow exiting WMS...\n");
        return;
    }

    arr = keys(users);
    keys = keys(users);

    switch(to_int(str)) {
        case 1 :
            keys = sort_array(arr, 1);
            break;

          case 2 :
            for(i = 0; i < sizeof(arr); i++) {
                temp = users[arr[i]];
                flags = temp["flags"];
                if(member_array("excellent performance", flags) == -1) {
                        keys -= ({arr[i]});
                }
            }
            break;

        case 3 :
            for(i = 0; i < sizeof(arr); i++) {
                temp = users[arr[i]];
                flags = temp["flags"];
                if(member_array("discipline problem", flags) == -1) {
                        keys -= ({arr[i]});
                }
            }

            break;

        case 4 :
            for(i = 0; i < sizeof(arr); i++)  {
                temp = users[arr[i]];
                flags = temp["flags"];
                if(member_array("inactive", flags) == -1) {
                    keys -= ({arr[i]});
                }
            }

            break;

        case 5 :
            for(i = 0; i < sizeof(arr); i++) {
                temp = users[arr[i]];
                flags = temp["flags"];
                if(member_array("suspended", flags) == -1) {
                    keys -= ({arr[i]});
                }
            }
            break;

        case 6 :
            for(i = 0; i < sizeof(arr); i++) {
                temp = users[arr[i]];
                flags = temp["flags"];
                if(member_array("recruiter", flags) == -1) {
                    keys -= ({arr[i]});
                }
            }

            break;

        case 7 :
            break;

        default :
            tell_me("Error: Invalid argument passed.\n\n");
            filter_list();
            return;
    }

    tell_me("Welcome to WMS...\n\n");
    tell_me(sprintf("%-10s %s -- %s -- %s\n", "Name", ":: Date Added", "Flags", "# of Notes"));
    tell_me("---------------------------------------------------\n");

    for(i = 0; i < sizeof(keys); i ++) {
        data = users[keys[i]];
        flag = query_flags(keys[i]);
        if(!flag || flag == "") flag = "N/A";
        printf("%-10s %s -- %s -- %d\n", capitalize(keys[i]), " :: " + ctime(data["added"]), flag, sizeof(data["notes"]));
    }
}

void remove_user() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("Note: You may not undo this action. There is no confirmation.\n");
    tell_me("Please enter the name of the user you wish to remove from the database: ");
    input_to("remove_user2");
}

void remove_user2(string str) {
    mapping data;
    string *ids;
    int i;

    if(!adminp(this_body()))
        return 0;

    if(!str) {
        tell_me("Error: No name supplied... Aborting.\n");
        return;
    }

    if(!users[str]) {
        tell_me("Error: User not found in database... Aborting.\n");
        return;
    }

    data = users[str];
    ids = data["notes"];

    for(i = 0; i < sizeof(ids); i ++) {
        notes[ids[i]] = "removed";
    }

    map_delete(users, str);
    log_file(WMS_LOG, capitalize(str) + " was deleted manually from the WMS database by " + capitalize(this_body()->query_real_name()) + " on " + ctime(time()) + ".\n");
    tell_me("User " + capitalize(str) + " has been deleted from the database.\n");
}


void read_note() {
    if(!adminp(this_body()))
        return 0;

    tell_me("Welcome to WMS...\n\n");
    tell_me("Please enter the note id you wish to read: ");
    input_to("read_note2");
}

void read_note2(string str) {
    int count;
    int note_id;
    mapping data;

    if(!adminp(this_body()))
        return 0;

    if(!str) {
        if(count) {
            tell_me("Now exiting WMS...\n");
            return;
        }
        tell_me("Error: No id supplied.\nPlease enter the id of the note you wish to read [quit]: ");
        count = 1;
        input_to("read_note2");
        return;
    }

    if(!to_int(str)) {
        tell_me("Error: Invalid ID supplied.\nPlease enter the id of the note you wish to read [quit]: ");
        count = 1;
        input_to("read_note2");
        return;
    }

    note_id = to_int(str);

    if(!notes[note_id]) {
        tell_me("Error: Note not found in database... Now exiting WMS.\n");
        return;
    }

    data = notes[note_id];

    if(!mapp(data)) {
    tell_me("Error: Note not found in database... Now exiting WMS.\n");
    return;
    }

    if(data["perm"] != "public" && data["perm"] != this_body()->query_real_name()) {
        tell_me("Error: Note not found in database... Now exiting WMS.\n");
        return;
    }

    if(!data["msg"]) {
        tell_me("Error: No Content in Note... Now exiting WMS.\n");
        return;
    }

    if(data["msg"] == "removed") {
        tell_me("Error: Note not found in database... Now exiting WMS.\n");
        return;
    }
    tell_me("\nNote Id: " + note_id + "\n");
    printf("For: %-10s Author: %-10s\n", capitalize(data["for"]),
        capitalize(data["admin"]));
    printf("Type: %-12s Permissions: %-10s\n", capitalize(data["type"]),
        capitalize(data["perm"]));
    tell_me("Date: " + ctime(data["date"]) + "\n\n");
    tell_me(data["msg"]);
}

void disp_user(string str) {
    mapping data, note;
    string *hist, note_ids, *flags;
    int i, n;

    if(!adminp(this_body()))
        return 0;

    data = ([]);
    note = ([]);
    flags = ({});
    n = 0;
    if(!str)
        return;

    if(!users[str]) {
        tell_me(help(this_body()));
        return;
    }

    data = users[str];
    flags = data["flags"];
    tell_me("Welcome to WMS...\n\n");
    tell_me("Username   : " + capitalize(str) + "\n");
    tell_me("Date Added : " + ctime(data["added"]) + "\n");
    tell_me("E-mail     : " + data["email"] + "\n");

    if(!sizeof(flags))
        tell_me("Flags      : " + "N/A\n");
    else
        tell_me("Flags      :\n");

    for(i = 0; i < sizeof(flags); i ++) {
        tell_me(" " + capitalize(flags[i]) + "\n");
    }

    tell_me("History    :\n");
    hist = data["history"];

    for(i = 0; i < sizeof(hist); i ++) {
        tell_me(" " + hist[i] + "\n");
    }

    tell_me("Notes      :\n");
    note_ids = data["notes"];

    for(i = 0; i < sizeof(note_ids); i++) {
        note = notes[note_ids[i]];
        if(note["perm"] != "public" && note["perm"] != this_body()->query_real_name()) continue;
        tell_me(" #" + note_ids[i] + " - " + note["type"] + " - " + capitalize(note["admin"]) + "\n");
    }

    tell_me("Profile    :\n");
    if(!data["profile"] || data["profile"] == 0)
        tell_me(" No data\n");
    else
        tell_me(" " + data["profile"]);
}

void summary() {
    string *keys, flags;
    int i;
    mapping data;

    if(!adminp(this_body()))
        return 0;

    keys = keys(users);
    tell_me("Welcome to WMS...\n\n");
    tell_me(sprintf("%-10s %s -- %s -- %s\n", "Name", ":: Date Added", "Flags", "# of Notes"));
    tell_me("---------------------------------------------------\n");

    for(i = 0; i < sizeof(keys); i ++) {
        data = users[keys[i]];
        flags = query_flags(keys[i]);
        if(!flags || flags == "") flags = "N/A";
        printf("%-10s %s -- %s -- %d\n", capitalize(keys[i]), " :: " + ctime(data["added"]), flags, sizeof(data["notes"]));
    }
}

string query_flags(string str) {
    string tmp, *flags, ret;
    mapping data;
    int i;

    if(!adminp(this_body()))
        return 0;

    if(!str)
        return "";

    if(!users[str])
        return "";

    data = users[str];
    flags = data["flags"];
    ret = "";

    for(i = 0; i < sizeof(flags); i++) {
        tmp = capitalize(flags[i]);
        ret += tmp[0..0];
    }

    if(!ret)
        ret = "N/A";

    return ret;
}

object restore_user(string name) {
    object link;

    if(!adminp(this_body())) return 0;

    if(find_player(name) && interactive(find_player(name))) {
        return (object)find_player(name);
    }

    link = FINGER_D->get_user(name);

    return link;
}

string help(object caller) {
    return("SYNTAX: wms [<username>||add||remove||list||note||read||edit||flag]\n\n"
    "Wms is a wizard management system that allows admins to quickly and easily\n"
    "manage their wizards (staff). To make use of the wms command, you must\n"
    "have adminaccess or above.\n"
    "If you do not supply any argument, wms will return a summary of all users\n"
    "in the wms database. If you supply a username, it will return the data on\n"
    "the user if it is in the wms database. The add argument allows you to\n"
    "manually add users to the database. The remove arugment allows you to\n"
    "manually remove wizards from the database. The arugment list allows you\n"
    "to filter the summary list based on criteria. The note arugment allows\n"
    "you to add a note to a user. The read arugment allows you to read notes.\n"
    "The edit argument allows you to edit a wizard's profile. The flag\n"
    "argument allows you to toggle flags on a user.\n");
}
