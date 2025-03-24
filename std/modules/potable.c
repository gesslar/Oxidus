/**
 * @file /std/modules/potable.c
 * @description This module is inherited in order to make something potable.
 *
 * @created 2024-08-06 - Gesslar
 * @last_modified 2024-08-06 - Gesslar
 *
 * @history
 * 2024-08-06 - Gesslar - Created
 */

#include "include/potable.h"

inherit M_USES;

private nomask int _potable = null;
private nomask mapping _default_actions = ([
    "drink": "$N $vdrink a $o.",
    "sip"  : "$N $vsip from a $o.",
]);

private nomask mapping _actions = ([
  "drink" : ([
    "action": null,
    "self"  : null,
    "room"  : null
  ]),
  "sip"   : ([
    "action": null,
    "self"  : null,
    "room"  : null
  ]),
]);

public void set_drink_action(string action) {
  _actions["drink"]["action"] = action;
}

public void set_self_drink_action(string action) {
  _actions["drink"]["self"] = action;
}

public void set_room_drink_action(string action) {
  _actions["drink"]["room"] = action;
}

public void set_sip_action(string action) {
  _actions["sip"]["action"] = action;
}

public void set_self_sip_action(string action) {
  _actions["sip"]["self"] = action;
}

public void set_room_sip_action(string action) {
  _actions["sip"]["room"] = action;
}

public int set_potable(int potable) {
  _potable = potable;

  return _potable;
}

public int is_potable() {
  return _potable;
}

/**
 * Drink the object.
 *
 * @param {STD_BODY} user - The user drinking the object.
 * @returns {mixed} 1 if the object was successfully drank, otherwise a failure message.
 */
protected mixed drink(object user) {
  if(!_potable)
    return "You can't drink that.";

  if(nullp(adjust_uses(-query_uses())))
    return "There is nothing left to drink.";

  if(_actions["drink"]["action"]) {
    user->simple_action(_actions["drink"]["action"], this_object());
  } else {
    if(!_actions["drink"]["self"] && !_actions["drink"]["room"]) {
      user->simple_action(_default_actions["drink"], this_object());
    } else {
      if(_actions["drink"]["self"])
        user->simple_action(_actions["drink"]["self"], this_object());
      else
        user->simple_action(_default_actions["drink"], this_object());
      if(_actions["drink"]["room"])
        user->simple_action(_actions["drink"]["room"], this_object());
      else
        user->simple_action(_default_actions["drink"], this_object());
    }
  }

  return 1;
}

/**
 * Sip the object.
 *
 * @param {STD_BODY} user - The user sipping the object.
 * @param {int} amount - The amount to sip.
 * @returns {mixed} 1 if the object was successfully sipped, otherwise a failure message.
 */
protected mixed sip(object user, int amount) {
  if(!_potable)
    return "You can't sip that.";

  if(nullp(adjust_uses(-amount)))
    return "There is nothing left to sip.";

  if(_actions["sip"]["action"]) {
    user->simple_action(_actions["sip"]["action"], this_object());
  } else {
    if(!_actions["sip"]["self"] && !_actions["sip"]["room"]) {
        user->simple_action(_default_actions["sip"], this_object());
    } else {
      if(_actions["sip"]["self"])
        user->simple_action(_actions["sip"]["self"], this_object());
      else
        user->simple_action(_default_actions["sip"], this_object());
      if(_actions["sip"]["room"])
        user->simple_action(_actions["sip"]["room"], this_object());
      else
        user->simple_action(_default_actions["sip"], this_object());
    }
  }

  return 1;
}

public void reset_potable() {
  reset_uses();
}
