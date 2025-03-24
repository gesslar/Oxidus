/**
 * @file /obj/mudlib/key.c
 *
 * Base for keys.
 *
 * @created 2025-03-19 - Gesslar
 * @last_modified 2025-03-19 - Gesslar
 *
 * @history
 * 2025-03-19 - Gesslar - Created
 */

inherit STD_ITEM;

public void set_key_id(string str);
public string query_key_id();

private nosave string _key_id;

public void set_key_id(string str) {
  assert_arg(stringp(str) && truthy(str), 1, "Invalid key id.");

  if(stringp(_key_id))
    remove_id("#"+_key_id);

  _key_id = str;

  add_id("#"+_key_id);
}

public string query_key_id() {
  return _key_id;
}

public int is_key() { return 1; }
