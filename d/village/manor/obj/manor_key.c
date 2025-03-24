/**
 * @file /d/village/manor/obj/manor_key.c
 *
 * The key to open the chest.
 *
 * @created 2025-03-19 - Gesslar
 * @last_modified 2025-03-19 - Gesslar
 *
 * @history
 * 2025-03-19 - Gesslar - Created
 */

inherit LIB_KEY;

void setup() {
  set_name("key");
  set_adj("iron");
  set_short("iron key");
  set_long("This is a large iron key.");
  set_mass(1);
  set_key_id("manor key");
}
