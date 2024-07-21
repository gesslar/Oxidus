#include "/adm/obj/simul_efun.h"

//object.c

//Tacitus @ LPUniversity
//Grouped on October 22nd, 2005

/**
 * @simul_efun getoid
 * @description Retrieves the unique object ID of the given object.
 * @param {object} ob - The object to get the ID of.
 * @returns {int} - The unique object ID.
 */
int getoid(object ob) {
    int id;
    sscanf(file_name(ob || previous_object()), "%*s#%d", id);
    return id;
}

/*
//      get_object() and get_objects()
//      Created by Pallando@Ephemeral Dale   (92-06)
//      Created by Watcher@TMI  (92-09-27)
//      Revised by Watcher and Pallando (92-12-11)
//      Re-written by Pallando (92-12-18)
//      get_objects() added by Pallando@Tabor (93-03-02)
//      "s", "c" & ">" added by Grendel@TMI-2 (93-07-14)
//
//      Use all possible methods to locate an object by the inputed
//      name and return the object pointer if located.
// Ideas for future expansion (please, anyone feel free to add them if they
//                             have the time)
//   "wizards" - the subset of "users" who are wizards.
//   "livings" - all living objects
//   "objects" - all loaded objects
//   check the capitalized and lower_case version of str
//   check wizard's home directories.
*/

/**
 * @simul_efun get_object
 * @description Attempts to locate an object by the given name and returns the
 *              object pointer if found.
 * @param {string} str - The name of the object to locate.
 * @param {object} [player] - The player object to use as a reference for
 *                            searching.
 * @returns {object} - The located object, or 0 if not found.
 */
varargs object get_object( string str, object player ) {
    object what;
    mixed tmp;

    if( !str ) return 0;
    if( !player || !living( player ) ) player = this_body();
    if( sscanf( str, "@%s", tmp ) &&
        ( tmp = get_object( tmp, player ) ) &&
        ( what = environment( tmp ) )) {
            return what;
    }
    if( player ) {   //  Check existance of this_body()
        if( str == "me" ) return player;
        if( what = present( str, player ) ) return what;  // Inventory check
        if( what = environment( player ) ) {              // Environment check
            if(str == "here" || str == "env" || str == "environment")
                return what;
            if( what = present( str, what ) ) return what;
        }
    }

    // Call might be made by a room so make a previous_object() check
    // first just to be sure
    if( what = present( str, previous_object() ) ) return what;

    //  Check to see if a living object matches the name
    if( what = find_player( str ) ) return what;
    if( what = find_living( str ) ) return what;

    // Search for a matching file_name, completing path with
    // user's present path
    if( player ) {
        str = resolve_path( (string)player-> query_env("cwd"), str );
    }

    what = find_object(str);
    if(!what && (file_size(str) >= 0 || file_size(str + ".c") >= 0)) {
        // Make sure the object is loaded into memory, if it exists
        //  Finally return any object found matching the requested name
        catch(what = load_object(str));
    }

    return what;
}

// Created by Pallando@Tabor (93-03-02)
// player - as per get_object()
// no_arr - if specified, only 0 or an object will be returned,
//          otherwise an array of objects may also be returned.
// str - eg
//   "pallando" - returns the object, /std/user#123
//   "pallando:i" - returns pallando's inventory
//   "pallando:e" - returns pallando's environment
//   "pallando:e:d:12" - returns the 12th object in the deep inventory of
//                       the room that pallando is in.
//   "caractacus:e:lady" - finds a lady of the court of King Caractacus 8-)
//   "users:rod" - searches the inventories of all users for a rod.
//   "users:e:guard" - searches the environments of all users for a guard.
//   "/std/ghost:c" - all the children of /std/ghost
//   "users:s" - all the shadows of users
//   "users:>query_user" - all the connection objects in the game
/**
 * @simul_efun get_objects
 * @description Locates objects based on the specified search string, which can
 *              include various search criteria and options.
 * @param {string} str - The search string specifying the objects to locate.
 * @param {object} [player] - The player object to use as a reference for
 *                            searching.
 * @param {int} [no_arr] - If specified, only a single object or 0 will be
 *                         returned, otherwise an array of objects may be
 *                         returned.
 * @returns {mixed} - The located object(s), or 0 if not found.
 */
varargs mixed get_objects( string str, object player, int no_arr ) {
    mixed base, tmp, ret;
    object what;
    int i, s;

    // Hmm.  i and s do several jobs here.  It would be clearer to use different
    // variables (with longer names) for each job.
    // Is it worth slowing the function (using more memory) to do this?

    if( !str ) return 0;
    s = strlen( str );
    i = s;
    while( i-- && ( str[i..i] != ":" ) ); // a reverse sscanf
    if( ( i > 0 ) && ( i < ( s - 1 ) ) ) { // of form "%s:%s"
        base = get_objects( str[0..(i-1)], player );
        str = str[(i+1)..s];
        if( !base ) return 0;
        if( !pointerp( base ) ) base = ({ base });
        s = sizeof( base );
        ret = ({ });
        if( str == "e" ) {
            while( s-- )
                if( tmp = environment( base[s] ) )
                    ret += ({ tmp });
        } else if( str == "i" ) {
            while( s-- )
                if( tmp = all_inventory( base[s] ) )
                    ret += ( pointerp( tmp ) ? tmp : ({ tmp }) );
        } else if( str == "d" ) {
            while( s-- )
                if( tmp = deep_inventory( base[s] ) )
                    ret += ( pointerp( tmp ) ? tmp : ({ tmp }) );
        } else if( sscanf( str, "%d", i ) ) {
            if( ( i > -1 ) && ( i < s ) ) return base[i];
            else return 0;
        } else if( str == "c" ) {
            while( s-- )
                if(tmp = children(base_name(base[s])))
                    ret += tmp;
        } else if( str == "s" ) {
            while( s-- )
                for(tmp=shadow(base[s],0);tmp;tmp=shadow(tmp,0))
                    ret += ({ tmp });
        } else if( strlen(str) && str[0] == '>' ) {
            str=extract(str,1);
            while( s-- )
                if(objectp(tmp=(mixed)call_other(base[s],str)))
                    ret += ({ tmp });
                else if( pointerp(tmp) && sizeof(tmp) && objectp(tmp[0]) )
                    ret += tmp;
        } else {
            // This is the location to add more syntax options if wanted such as
            // ith item in jth base object, all such items in all base objects, etc
            while( s-- )
                if( what = present( str, base[s] ) )
                    return what;
            return 0;
        }
        switch( sizeof( ret ) ) {
            case 0: return 0;
            case 1: return ret[0];
        }
        return( no_arr ? ret[0] : ret );
    }
    if( str == "users" ) {
        ret = users();
        if( !no_arr ) return ret;
        if( sizeof( ret ) ) return ret[0];
        return 0;
    }
    return get_object( str, player );
}

/*
  NB

  It would be fairly simple to combine these two functions into one
  varargs object get_object( string str, object player, int arr_poss )
  which will only return a single object unless the array_possible flag
  is passed.

  I have chosen not to do this however, since some muds may not wish to
  use the more complicated search routines and keeping get_objects() as
  a seperate simul_efun makes it easier to disable.
*/
/**
 * @simul_efun find_ob
 * @description Searches for an object within a container or environment
 *              using the specified criteria.
 * @param {mixed} ob - The object or name of the object to find.
 * @param {mixed} [cont] - The container or environment to search within.
 *                         Defaults to the previous object.
 * @param {function} [f] - An optional function to further filter the search.
 * @returns {object} - The found object, or 0 if not found.
 */
varargs object find_ob(mixed ob, mixed cont, function f) {
    // First let's deal with dest.
    if(nullp(cont))
        cont = previous_object();
    if(stringp(cont))
        cont = load_object(cont);

    if(objectp(ob))
        return present(ob, cont) ? ob : 0;

    if(stringp(ob)) {
        if(ob[0] == '/') {
            object *obs = all_inventory(cont);

            if(strsrch(ob, "#") > -1) {
                foreach(object o in obs) {
                    if(file_name(o) == ob)
                        return o;
                }
            } else {
                ob = chop(ob, ".c", -1) ;
                foreach(object o in obs) {
                    if(base_name(o) == ob)
                        return o;
                }
            }
        }
    }

    return 0;
}

/**
 * @simul_efun top_environment
 * @description Retrieves the top-level environment of the specified object,
 *              traversing up through nested environments.
 * @param {object} ob - The object to get the top-level environment of.
 * @returns {object} - The top-level environment of the object.
 */
object top_environment(object ob) {
    object test, env ;

    if(!objectp(ob))
        ob = previous_object() ;

    if(!objectp(ob))
        error("Missing argument 1 to top_environment()") ;

    if(ob->query_room())
        return ob ;

    if(!environment(ob))
        return ob ;

    while(test = environment(ob)) {
        env = test ;
        ob = env ;
        if(test->is_room())
            break ;
    }

    return env ;
}

/**
 * @simul_efun present_livings
 * @description Retrieves all living objects present in the specified room.
 * @param {object} room - The room to search for living objects in.
 * @returns {object[]} - An array of living objects present in the room.
 */
object *present_livings(object room) {
    if(!room)
        return ({});

    return filter(all_inventory(room), (: living($1) :));
}

/**
 * @simul_efun present_players
 * @description Retrieves all player objects present in the specified room.
 * @param {object} room - The room to search for player objects in.
 * @returns {object[]} - An array of player objects present in the room.
 */
object *present_players(object room) {
    if(!room)
        return ({});

    return filter(present_livings(room), (: userp :));
}

/**
 * @simul_efun get_living
 * @description Locates a living object by the specified name within the
 *              specified room.
 * @param {string} name - The name of the living object to locate.
 * @param {object} room - The room to search for the living object in.
 * @return {object} - The located living object, or 0 if not found.
 */
object get_living(string name, object room) {
    object ob;

    if(!room)
        return 0 ;

    ob = present(name, room);
    if(ob && living(ob))
        return ob;

    return 0;
}

/**
 * @simul_efun get_livings
 * @description Locates living objects by the specified names within the
 *              specified room.
 * @param {string|string[]} names - The name of the living objects to locate.
 * @param {object} room - The room to search for the living objects in.
 * @return {object[]} - An array of located living objects.
 */
object *get_livings(mixed names, object room) {
    object *obs, *ret = ({});

    if(!room)
        return ret ;

    if(!pointerp(names)) names = ({ names });
    names = filter(names, (: stringp :));

    obs = map(names, (: get_living($1, $(room)) :));

    return obs;
}

/**
 * @simul_efun get_player
 * @description Locates a player object by the specified name within the
 *              specified room.
 * @param {string} name - The name of the player to locate.
 * @param {object} room - The room to search for the player in.
 * @return {object} - The located player object, or 0 if not found.
 */
object get_player(string name, object room) {
    object ob;

    ob = get_living(name, room);

    if(ob && userp(ob))
        return ob;

    return 0;
}

/**
 * @simul_efun get_players
 * @description Locates player objects by the specified names within the
 * @param {string|string[]} names - The name of the player objects to locate.
 * @param {object} room - The room to search for the player objects in.
 * @return {object[]} - An array of located player objects.
 */
object *get_players(mixed names, object room) {
    object *obs, *ret = ({});

    ret = get_livings(names, room);
    ret = filter(ret, (: userp :));

    return ret;
}

/**
 * @simul_efun this_body
 * @description This is a simul_efun that will return the body of the current
 *              interactive user. It is used as a replacement for this_player().
 * @returns {object} - The body of the current calling player.
 */
object this_body() {
    return efun::this_player() ;
}

/**
 * @simul_efun this_caller
 * @description This is a simul_efun that will return the object that called
 *              the current operation. This may be this_body(), but it may
 *              also be a shadow another player who initiated the chain.
 *              For example, a wizard using the force command.
 *
 *              Be careful with this one, you don't want to accidentally
 *              perform operations on the wrong object.
 * @returns {object} - The object that called the current operation.
 */
object this_caller() {
    return efun::this_player(1) ;
}
