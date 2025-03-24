/**
 * @file /std/room/door.c
 *
 * Room door management system that handles door states, synchronization between
 * connected rooms, and player interactions. Uses the Door class to track door
 * properties and state.
 *
 * @created 2024-09-13 - Gesslar
 * @last_modified 2025-03-16 - GitHub Copilot
 *
 * @history
 * 2024-09-13 - Gesslar - Created
 * 2025-03-16 - GitHub Copilot - Enhanced documentation
 */

#include <classes.h>
#include <exits.h>
#include <door.h>
#include <gmcp_defines.h>

inherit CLASS_DOOR;

private nosave mapping _doors = ([ ]);

/**
 * Adds a door to a room in the specified direction.
 *
 * Validates the door configuration and sets sensible defaults for missing values.
 * Door IDs must be unique and the direction must be a valid exit.
 *
 * @param {class Door} door - Door configuration object
 * @returns {int} 1 on success, null if:
 *   - Door lacks required direction
 *   - Direction isn't a valid room exit
 *   - Direction already has a door
 *   - Door ID is neither null, pointer nor string
 */
int add_door(class Door door) {
  // Absolutely necessary in order to be able to add a door to a room.
  if(!stringp(door.direction))
    return null;

  if(!valid_exit(door.direction))
    return null;

  if(!nullp(_doors[door.direction]))
    return null;

  // Fill in missing values.
  if(nullp(door.id))
    door.id = ({sprintf("%s door", door.direction)});
  if(!pointerp(door.id))
    door.id = ({door.id});
  door.id = distinct_array(door.id);

  if(!door.status)
    door.status = "closed";

  if(!door.type)
    door.type = "door";

  if(!door.name)
    door.name = sprintf("%s %s", door.direction, door.type);

  door.id += ({ door.name, door.type, lower_case(door.short) });

  _doors[door.direction] = door;

  GMCP_D->broadcast_gmcp(this_object(), GMCP_PKG_ROOM_INFO, this_object());

  return 1;
}

/**
 * Checks if a valid door exists in the specified direction.
 *
 * @param {string} direction - The direction to check
 * @returns {int} 1 if door exists, null if not
 */
int valid_door(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  return 1;
}

/**
 * Removes a door from the specified direction.
 *
 * Updates GMCP room info after removing the door.
 *
 * @param {string} direction - The direction of door to remove
 * @returns {int} 1 on success, null if door doesn't exist
 */
int remove_door(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  map_delete(_doors, direction);

  GMCP_D->broadcast_gmcp(this_object(), GMCP_PKG_ROOM_INFO, this_object());

  return 1;
}

/**
 * Sets a door's open status and broadcasts the change.
 *
 * @param {string} direction - The direction of the door
 * @param {int} bool - 1 to open door, 0 to close door
 * @param {int} [silent=0] - If 1, suppresses messages about state changes
 * @returns {int} The new state (1=open, 0=closed), or null on failure
 */
varargs int set_door_open(string direction, int bool, int silent) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  if(nullp(bool))
    return null;

  bool = !!bool;
  door.status = bool ? "open" : "closed";

  _doors[direction] = door;

  if(!silent)
    tell_down(this_object(),
      sprintf("The %s is now %s.\n", door.name, door.status));

  GMCP_D->broadcast_gmcp(this_object(), GMCP_PKG_ROOM_INFO, this_object());

  return bool;
}

/**
 * Checks if a door in the specified direction is open.
 *
 * @param {string} direction - The direction to check
 * @returns {int} 1 if door is open, 0 if closed or locked, null if door doesn't exist
 */
int is_door_open(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  return door.status == "open";
}

/**
 * Sets a door's lock status.
 *
 * Cannot lock an open door. Updates GMCP room info after state change.
 *
 * @param {string} direction - The direction of the door
 * @param {int} bool - 1 to lock door, 0 to unlock door
 * @param {int} [silent=0] - If 1, suppresses messages about state changes
 * @returns {int} The new lock state (1=locked, 0=unlocked), or null on failure
 */
int set_door_locked(string direction, int bool, int silent) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  if(nullp(bool))
    return null;

  if(door.status == "open")
    return 0;

  if(nullp(silent))
    silent = 0;

  silent = !!silent;
  bool = !!bool;

  door.status = bool ? "locked" : "closed";

  if(!silent)
    tell_down(this_object(),
      sprintf("There is a click from the %s.\n", door.name));

  _doors[direction] = door;

  GMCP_D->broadcast_gmcp(this_object(), GMCP_PKG_ROOM_INFO, this_object());

  return bool;
}

/**
 * Checks if a door in the specified direction is locked.
 *
 * @param {string} direction - The direction to check
 * @returns {int} 1 if door is locked, 0 if not locked, null if door doesn't exist
 */
int is_door_locked(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  return door.status == "locked";
}

/**
 * Adds an ID to a door in the specified direction.
 *
 * @param {string} direction - The direction of the door
 * @param {string} id - The ID to add
 * @returns {int} 1 on success, null if door doesn't exist
 */
int add_door_id(string direction, string id) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  door.id += ({id});
  door.id = distinct_array(door.id);

  _doors[direction] = door;

  return 1;
}

/**
 * Removes an ID from a door in the specified direction.
 *
 * @param {string} direction - The direction of the door
 * @param {string} id - The ID to remove
 * @returns {int} 1 on success, null if door doesn't exist
 */
int remove_door_id(string direction, string id) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  door.id = remove_array_element(door.id, id);

  _doors[direction] = door;

  return 1;
}

/**
 * Returns all directions that have doors.
 *
 * @returns {string*} Array of direction strings
 */
string *query_door_ids() {
  return keys(_doors);
}

/**
 * Sets a door's short description.
 *
 * @param {string} direction - The direction of the door
 * @param {string} short - The short description
 * @returns {int} 1 on success, null if door doesn't exist
 */
int set_door_short(string direction, string short) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  door.short = short;

  _doors[direction] = door;

  return 1;
}

/**
 * Returns a door's short description.
 *
 * @param {string} direction - The direction of the door
 * @returns {string} The door's short description, or null if door doesn't exist
 */
string query_door_short(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  return door.short;
}

/**
 * Sets a door's long description.
 *
 * @param {string} direction - The direction of the door
 * @param {string} long - The long description
 * @returns {int} 1 on success, null if door doesn't exist
 */
int set_door_long(string direction, string long) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  door.long = long;

  _doors[direction] = door;

  return 1;
}

/**
 * Returns a door's long description.
 *
 * @param {string} direction - The direction of the door
 * @returns {string} The door's long description, or null if door doesn't exist
 */
string query_door_long(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  return door.long;
}

/**
 * Returns a copy of all doors in the room.
 *
 * @returns {mapping} Mapping of direction to door object
 */
mapping query_doors() {
  return copy(_doors);
}

/**
 * Returns a door's name.
 *
 * @param {string} direction - The direction of the door
 * @returns {string} The door's name, or null if door doesn't exist
 */
string query_door_name(string direction) {
  class Door door;

  if(nullp(door = _doors[direction]))
    return null;

  return door.name;
}

/**
 * Returns all directions of doors that match the given ID.
 *
 * @param {string} id - The ID to match
 * @returns {string*} Array of directions with doors matching the ID
 */
string *id_door(string id) {
  return map(
    filter(values(_doors), (: includes($1.id, $(id)) :)),
    (: $1.direction :)
  );
}

/**
 * Synchronizes door states with connected rooms.
 *
 * Ensures doors have the same state on both sides of an exit.
 * Removes doors if their exit no longer exists. Should be called
 * during room reset.
 */
void reset_doors() {
  string *exits = query_exit_ids();

  foreach(string dir, class Door door in _doors) {
    string other_room_file;

    // Sanity check.
    if(member_array(dir, exits) == -1) {
      remove_door(dir);
      continue;
    }

    if(other_room_file = query_exit(dir)) {
      object other_room;

      other_room = find_object(other_room_file);

      if(!objectp(other_room))
        continue;

      foreach(string other_dir in other_room->query_exit_ids()) {
        if(other_room->query_exit_dest(other_dir, true) == this_object()) {
          if(other_room->is_door_open(other_dir)) {
            set_door_open(dir, true, true);
            break;
          } else if(other_room->is_door_locked(other_dir)) {
            set_door_locked(dir, true, true);
            break;
          }
        }
      }
    }
  }
}

/**
 * Returns a door's status.
 *
 * @param {string} direction - The direction of the door
 * @param {int} [as_number=0] - If true, returns status as a number (1=open, 2=closed, 3=locked)
 * @returns {mixed} The door's status as string or number, or null if door doesn't exist
 */
varargs mixed query_door_status(string direction, int as_number) {
  class Door door;

  direction = resolve_direction_from_input(direction);

  if(nullp(direction) || nullp(door = _doors[direction]))
    return null;

  if(as_number) {
    switch(door.status) {
      case "open": return 1;
      case "closed": return 2;
      case "locked": return 3;
      default: return 0;
    }
  }

  return door.status;
}

/**
 * Handles attempted door opening commands.
 *
 * Provides user-friendly messages for various door states and handles
 * cases where multiple doors exist in a room.
 *
 * @param {string} direction - The direction or "door" for auto-detection
 * @returns {mixed} 1 if can open, error message string if not
 */
mixed can_open_door(string direction) {
  if(!stringp(direction) || falsy(direction))
    return 0;

  if(direction == "door") {
    if(sizeof(_doors) > 1)
      return "Which door?";

    direction = keys(_doors)[0];
  }

  switch(query_door_status(direction)) {
    case "open":
      return "That door is already open.";
    case "closed":
      return 1;
    case "locked":
      return "That door is locked.";
    default:
      return 0;
  }
}

/**
 * Handles attempted door closing commands.
 *
 * @param {string} direction - The direction or "door" for auto-detection.
 * @returns {mixed} 1 if can close, error message string if not.
 */
mixed can_close_door(string direction) {
  if(!stringp(direction) || falsy(direction))
    return 0;

  if(direction == "door") {
    if(sizeof(_doors) > 1)
      return "Which door?";

    direction = keys(_doors)[0];
  }

  switch(query_door_status(direction)) {
    case "open":
      return 1;
    case "closed":
    case "locked":
      return "That door is already closed.";
    default:
      return 0;
  }
}

/**
 * Handles attempted door locking commands.
 *
 * @param {string} direction - The direction or "door" for auto-detection
 * @returns {mixed} 1 if can lock, error message string if not
 */
mixed can_lock_door(string direction) {
  if(!stringp(direction) || falsy(direction))
    return 0;

  if(direction == "door") {
    if(sizeof(_doors) > 1)
      return "Which door?";

    direction = keys(_doors)[0];
  }

  switch(query_door_status(direction)) {
    case "open":
      return "You consider the futility of locking an open door.";
    case "closed":
      return 1;
    case "locked":
      return "That door is already locked.";
    default:
      return 0;
  }
}

public string resolve_direction_from_input(string direction) {
  string *directions;

  // First, let's try to find an exit directly by its name.
  directions = find_keys(_doors, (: $2 == $(direction) :));
  if(sizeof(directions))
    return directions[0];

  // Okay, let's try to find a door by its id.
  directions = id_door(direction);
  if(sizeof(directions))
    return directions[0];

  return null;
}
