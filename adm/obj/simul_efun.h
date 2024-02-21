#ifndef __SIMUL_EFUN_H__
#define __SIMUL_EFUN_H__

// File: arrays
mixed *distinct_array(mixed *arr) ;

// File: description
string get_short(object ob) ;
string get_long(object ob) ;

// File: directory
int mkdirs(string path) ;
string query_directory(object ob) ;

// File: english
string cap_words(string str) ;
varargs string cap_significant_words(string str, int title) ;
string possessive_noun(mixed ob) ;
string possessive_pronoun(mixed ob) ;
string possessive(mixed ob) ;
string reflexive(mixed ob) ;
string objective(mixed ob) ;
string subjective(mixed ob) ;

// File: exists
int directory_exists(string dirname) ;
int file_exists(string file) ;
int user_exists(string user) ;

// File: file
string file_owner(string file) ;
int file_lines(string file) ;
varargs string tail(string path, int line_count) ;
int log_file(string file, string str) ;
string *explode_file(string file) ;
string query_file_name(object ob) ;

// File: function
varargs string call_trace(int colour) ;

// File: grammar
string int_string (int num) ;
string ordinal (int n) ;

// File: identify
string identify(mixed arg) ;

// File: json
mixed json_decode(string str) ;
mapping json_encode(mixed arg) ;

// File: mappings
string pretty_map(mapping map);

// File: messaging
varargs void tell_up(object ob, string str, int msg_type, mixed exclude) ;
varargs void tell_down(object ob, string str, int msg_type, mixed exclude) ;
varargs void tell_all(object ob, string str, int msg_type, mixed exclude) ;
varargs void tell_direct(object ob, string str, int msg_type, mixed exclude) ;
varargs void tell(object ob, string str, int msg_type, mixed exclude) ;

// File: numbers
float percent_of(float a, float b) ;
float percent(float a, float b) ;

// File: object
int getoid(object ob) ;
varargs mixed get_objects( string str, object player, int no_arr ) ;
varargs object find_ob(mixed ob, mixed cont) ;
varargs object get_object( string str, object player ) ;
object top_environment(object ob) ;

// File: resolve_path
string resolve_path(string Current, string Next) ;

// File: save
string object_save_directory(object ob) ;
string object_save_file(object ob) ;
string assure_object_save_dir(object ob) ;

// File: security
int is_member(string user, string group) ;
int adminp(mixed user) ;
int devp(mixed user) ;
int wizardp(mixed user) ;

// File: socket
string dump_socket_status() ;

// File: string
string append(string source, string to_append) ;
string no_ansi(string str) ;
string prepend(string source, string to_prepend) ;
varargs string chop(string str, string sub, int dir) ;
varargs string extract(string str, int from, int to) ;
varargs string simple_list(mixed *arr, string conj) ;

// File: system
int port() ;
mixed mud_config(string str) ;
string admin_email() ;
string arch() ;
string baselib_name() ;
string baselib_version() ;
string driver_version() ;
string lib_name() ;
string lib_version() ;
string mud_name() ;
string open_status() ;
varargs void debugf(string str, mixed args...) ;
void debug(string str) ;

// File: time
varargs string ldate(int x, int brief) ;
varargs string ltime(int x, int brief) ;

// File: user
string user_data_file(string name) ;
string user_mob_data(string name) ;
string user_data_directory(string priv) ;
string user_core_data_directory(string priv) ;
string user_path(string name) ;

// File: valids
int valid_function(mixed f) ;

#endif // __SIMUL_EFUN_H__
