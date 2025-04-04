/**
 * @file /d/village/field/zone.c
 * @description Zone for the field
 *
 * @created 2024-02-04 - Gesslar
 * @last_modified 2024-02-04 - Gesslar
 *
 * @history
 * 2024-02-04 - Gesslar - Created
 */


inherit STD_VIRTUAL_SERVER;

object generate_object(string file) {
  object result;

  if(pcre_match(file, "^\\d,\\d,\\d$")) {
    result = new(__DIR__ "field_base", file);
    return result;
  }

  return 0;
}
