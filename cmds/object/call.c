/*
File     :  _call.c
Syntax   :  int cmd_call(string a);
Purpose  :  allow wizards to call functions in objects
92-06-00 :  Douglas Reay (aka Pallando @ many muds) wrote a bin based wizard
            tool for Ephemeral Dale lpmud (LPC2)
92-10-00 :  Pallando wrote a basic multi-reference tracer for this
93-01-02 :  Pallando changed refs to be stored on player not in daemon
            This file is now part of the TMI-II mudlib distribution.
            Please retain this header if you use any of this code.
93-03-19 :  Pallando changed refs over to get_objects()
93-07-17 :  Robocoder switched call_other to arrays, ie ({ func, args... })
93-08-20 :  Grendel allowed admins to call with any euid
93-08-23 :  Pallando changed it to inherit REF_D
93-10-22 : Mobydick added the ADMIN_ONLY define.
*/
#include <daemons.h>
#include <logs.h>

#define NO_USER_CALLS

inherit STD_CMD;
inherit M_REF;

#define SYNTAX "Syntax: call [-<uid>] <object>;<function>;<arg>;<arg>\n"
#define FUNC_LIST ({ "query_skills", "query_nicknames", "query_aliases" })
#define TAB "\t"

private mixed do_call(object ob, string func, mixed args);

mixed main(object tp, string a) {
    string str, *exp_a;
    mixed objs, funcs, args, tmp, ret, rets;
    object ob;
    int i, s, fi, fs;

    if(!a) { notify_fail(SYNTAX); return 0; }

    exp_a = explode(a, ";");
    s = sizeof(exp_a);
    objs = exp_a[0];
    if(s > 1) funcs = exp_a[1];
    else funcs = FUNC_LIST;
    if(s == 3) args = ({ exp_a[2] });
    if(s > 3) args = exp_a[2..(s-1)];

    objs = resolv_ref(objs);
    if(objs == "users") objs = users();
    else if(objs == "actives") objs = filter(users(), (: query_idle($1) < 180 :));
    else if(objs == "livings") objs = filter(livings(), (: clonep($1) && environment($1) :));
    else if(objs == "monsters") objs = filter(livings(), (: clonep($1) && environment($1) && !userp($1) :));
    if(!pointerp(objs)) objs = ({ objs });
    tmp = ({ });
    s = sizeof(objs);
    for(i = 0 ; i < s ; i++) {
        if(stringp(objs[i])) ob = get_objects(objs[i],0,1);
        else if(objectp(objs[i])) ob = objs[i];
        if(!ob) tell_me("Can't identify " + identify(objs[i]) + " as an object.\n");
        else tmp += ({ ob });
    }
    if(!sizeof(tmp)) return 1;
    objs = tmp;

    funcs = resolv_ref(funcs);
    if(!pointerp(funcs)) funcs = ({ funcs });
    tmp = ({ });
    s = sizeof(funcs);
    for(i = 0 ; i < s ; i ++) {
        if(stringp(funcs[i])) tmp += ({ funcs[i] });
        else tell_me("Can't identify " + identify(funcs[i])+" as a string.\n");
    }
    if(!sizeof(tmp)) return 1;
    funcs = tmp;

    if(pointerp(args) && (s = sizeof(args)))
        for(i = 0 ; i < s ; i++)
            args[i] = resolv_ref(resolv_str(args[i]));

    rets = ({ });
    s = sizeof(objs);
    fs = sizeof(funcs);
    for(i = 0 ; i < s ; i++) {
        str = identify(objs[i]);
        for(fi = 0 ; fi < fs ; fi++) {
            ret = do_call(objs[i], funcs[fi], args);
            if(ret[0]) rets += ({ ret[0] });
            if(fs == 1) str = str + ret[1];
            else str += (fi?"":"\n") + ret[1];
        }
        tell_me(str + "\n");
    }
    switch(sizeof(rets)) {
        case 0: rets = 0; break;
        case 1: rets = rets[0];
    }

    tp->set_ref("default", rets);

    return 1;
}

private mixed do_call(object ob, string func, mixed args) {
    mixed ret, err;
    int i, s;
    string str;
    object shad;

    if(!function_exists(func, ob)) {
        for(shad = shadow(ob, 0) ; shad ; shad = shadow(shad, 0))
            if(function_exists(func, shad)) { ob = shad; break; }
        if(ob != shad) return ({ 0, "- does not contain " + func + "()" });
    }
    str = "->" + func;
    if(pointerp(args) && (s = sizeof(args))) {
        str += "(";
        for(i = 0 ; i < s ; i++) {
            if(i) str += ", ";
            str += identify(args[i]);
        }
        str += ")";
    } else {
        str += "()";
    }

    if(!s) {
      err = catch(ret = call_other(ob, func));
    } else {
        err = catch(ret = call_other(ob, ({ func }) + args));
    }
    if(err) return ({ 0, str + TAB + "\nERR(" + identify(err) + ")" });
    return ({ ret, str + TAB + "\nResult: " + identify(ret) });
}

string help(object caller) {

    tell_me(@text
USAGE:	call [-<uid>] <object>;<function>;<arg>;<arg>

Calls the function <function> in object <object> passing as many arguments
<arg> as you give.

If no function is specified a dump of the object is given.

<object> and <function> can be arrays (e.g. "users")

Any of them can be references (SEE: help refs)

text
);

}
