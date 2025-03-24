/**
 * @file /std/object/contents.c
 * @description Handles the capacity and contents of an object, providing
 *              inventory management and mass/capacity tracking.
 *
 * @created 2024-02-18 - Gesslar
 * @last_modified 2025-03-16 - GitHub Copilot
 *
 * @history
 * 2024-02-18 - Gesslar - Created
 * 2025-03-16 - GitHub Copilot - Added documentation
 */

#include <contents.h>
#include <gmcp_defines.h>
#include <daemons.h>
#include <container.h>

#include "/std/living/include/wealth.h"

private int _capacity;
private nosave int _fill;

/**
 * Removes all objects from this container recursively.
 *
 * Calls clean_contents() on all contained objects before removing them.
 */
void clean_contents() {
  object *obs = deep_inventory();

  obs->clean_contents();
  obs->remove();
}

/**
 * Sets the total capacity of this container.
 *
 * Updates fill levels and broadcasts status via GMCP if applicable.
 * Has no effect if USE_MASS mud configuration is disabled.
 *
 * @param {int} x - The new capacity value
 */
void set_capacity(int x) {
  if(!mud_config("USE_MASS"))
    return;

  _capacity = x;

  rehash_capacity();

  GMCP_D->send_gmcp(this_object(),
    GMCP_PKG_CHAR_STATUS, ([
      GMCP_LBL_CHAR_STATUS_FILL    : query_fill(),
      GMCP_LBL_CHAR_STATUS_CAPACITY: query_capacity()
    ])
  );
}

/**
 * Adjusts the capacity of this container by a relative amount.
 *
 * Updates fill levels and broadcasts status via GMCP if applicable.
 * Has no effect if USE_MASS mud configuration is disabled.
 *
 * @param {int} x - The amount to adjust capacity by (positive or negative)
 */
void adjust_capacity(int x) {
  if(!mud_config("USE_MASS"))
    return;

  _capacity += x;

  rehash_capacity();

  GMCP_D->send_gmcp(this_object(),
    GMCP_PKG_CHAR_STATUS, ([
      GMCP_LBL_CHAR_STATUS_FILL    : query_fill(),
      GMCP_LBL_CHAR_STATUS_CAPACITY: query_capacity()
    ])
  );
}

/**
 * Returns the total capacity of this container.
 *
 * @returns {int} The total capacity, or null if USE_MASS is disabled
 */
int query_capacity() {
  if(!mud_config("USE_MASS"))
    return null;

  return _capacity;
}

/**
 * Returns the current fill level of this container.
 *
 * @returns {int} The current fill level, or null if USE_MASS is disabled
 */
int query_fill() {
  if(!mud_config("USE_MASS"))
    return null;

  return _fill;
}

/**
 * Attempts to adjust the fill level by a relative amount.
 *
 * Will not allow adjustments that would cause fill to go below 0 or above capacity.
 * Updates GMCP status if successful.
 *
 * @param {int} x - The amount to adjust fill by (positive or negative)
 * @returns {int} 1 if successful, 0 if adjustment would exceed limits
 */
int adjust_fill(int x) {
  if(!mud_config("USE_MASS"))
    return null;

  if(_fill + x < 0 || _fill + x > _capacity)
    return 0;

  _fill += x;

  GMCP_D->send_gmcp(this_object(),
    GMCP_PKG_CHAR_STATUS, ([
      GMCP_LBL_CHAR_STATUS_FILL    : query_fill(),
      GMCP_LBL_CHAR_STATUS_CAPACITY: query_capacity()
    ])
  );

  return 1;
}

/**
 * Checks if this container can hold a specific object.
 *
 * Will always return 1 if USE_MASS is disabled.
 *
 * @param {object} ob - The object to check
 * @returns {int} 1 if the object can be held, 0 if not
 */
int can_hold_object(object ob) {
  int use_mass;
  int mass;

  use_mass = mud_config("USE_MASS");

  if(use_mass) {
    mass = ob->query_mass();

    return can_hold_mass(mass);
  }

  return 1;
}

/**
 * Checks if this container can hold the specified mass.
 *
 * @param {int} mass - The mass to check
 * @returns {int} 1 if the mass can be held, 0 if not
 */
int can_hold_mass(int mass) {
  return _fill + mass <= _capacity;
}

/**
 * Recalculates the total fill level based on contained objects.
 *
 * Recursively calls rehash_capacity() on all contained objects.
 * For living objects, includes the mass of carried coins.
 * Updates GMCP status for user players.
 */
void rehash_capacity() {
  object ob, *obs;
  int total;

  if(!mud_config("USE_MASS"))
    return;

  if(ignore_capacity())
    return;

  if(living() && !query_capacity())
    set_capacity(1000);

  total = 0;
  obs = all_inventory();
  foreach(ob in obs) {
    ob->rehash_capacity();
    total += ob->query_mass();
  }

  if(living())
    total += query_total_coins();

  _fill = total;

  if(userp()) {
    GMCP_D->send_gmcp(this_object(),
      GMCP_PKG_CHAR_STATUS, ([
        GMCP_LBL_CHAR_STATUS_FILL    : query_fill(),
        GMCP_LBL_CHAR_STATUS_CAPACITY: query_capacity()
      ])
    );
  }
}
