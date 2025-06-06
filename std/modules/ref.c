/*
File     : ref_d.c
Purpose  : inherited by commands wanting to use refs (qv help refs)
93-02-05 : Pallando fixed resolv_str() to handle the cases ([]) and ({})
93-08-23 : Pallando changed /cmds/refs/ref_funcs.c to a daemon
93-12-27 : Inspiral added support for floats.
*/

#include "/std/living/include/env.h"

private nomask nosave mapping _ref = ([]);

mixed resolv_ref(mixed a) {
    mixed ret;
    string Ref;

    if(!stringp(a)) return a;
    if(!sscanf(a, "#%s", Ref)) return a;
    if(Ref == "#") return query_env("cwf");
    ret = _ref[Ref];
    if(!mapp(ret)) return a;
    ret = ret[Ref];
    if(nullp(ret)) return a;
    return ret;
}

void set_ref(string ref_name, mixed ref_contents) {
    _ref[ref_name] = ref_contents;
}

// NB arrays or mappings containing arrays, mappings or
// functions are liable to confuse this function.
mixed resolv_str(string a) {
    mapping v_m;
    mixed v_o; //object or array of objects
    mixed *v_a, tmp1, tmp2;
    string v_s, sa, sb;
    int v_i, ia, ib;
    float v_f;

    if(!a) return a;
    if(a == "" || a == " " || a == "  ") return a;
    if(sscanf(a, " %s", sa)) a = sa;
    ib = strlen(a) - 1;
    if(a[ib..ib] == " ") a = a[0..(ib-1)];
    if(sscanf(a, "\"%s\"", v_s)) return v_s;
    tmp1 = resolv_ref(a);
    if(tmp1 != a) return tmp1;
    if(pcre_match(a, "^[+-]*[0-9]*\\.[0-9]+$")) if(sscanf(a, "%f", v_f)) return v_f;
    if(sscanf(a, "f%f", v_f)) return v_f;
    if(sscanf(a, "%d", v_i)) return v_i;
    if(v_o = get_objects(a)) return v_o;
    if(sscanf(a, "(:%s,%s:)", sa, sb) == 2) {
        tmp1 = resolv_str(sa);
        tmp2 = resolv_str(sb);
        if(objectp(tmp1) && stringp(tmp2)) return call_other(tmp1, tmp2);
    }
    if(sscanf(a, "({%s})", sa)) {
        v_a = explode(sa, ",");
        ib = sizeof(v_a);
        if(ib && !v_a[0]) return ({ });
        for(ia = 0 ; ia < ib ; ia++)
            v_a[ia] = resolv_str(v_a[ia]);
        return v_a;
    }
    if(sscanf(a, "([%s])", sa)) {
        v_m = ([]);
        v_a = explode(sa, ",");
        ib = sizeof(v_a);
        if(ib && !v_a[0]) return ([]);
        for(ia = 0 ; ia < ib ; ia++)
            if(sscanf(v_a[ia], "%s:%s", sa, sb) == 2)
                v_m[resolv_str(sa)] = resolv_str(sb);
        return v_m;
    }
    return a;
}

mixed ref_ob(mixed a) {
    mixed tmp;

    a = resolv_ref(a);
    if(stringp(a) && (tmp = get_objects(a, 0, 1))) a = tmp;
    return a;
}
