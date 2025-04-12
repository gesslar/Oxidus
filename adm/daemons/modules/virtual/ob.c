/**
 * @file /adm/daemons/modules/virtual/ob.c
 * @description Base module for virtual objects
 *
 * @created 2024-08-22 - Gesslar
 * @last_modified 2024-08-22 - Gesslar
 *
 * @history
 * 2024-08-22 - Gesslar - Created
 */

inherit STD_DAEMON;

public nomask object compile_object(string file) {
  string *parts;
  string obj_name, obj_file;
  mapping data;
  string e;
  string module = query_file_name();
  object obj;
  string base_file;

  parts = dir_file(file);
  obj_name = parts[1];

  // Remove the module extension
  sscanf(obj_name, "%s.%*s", obj_name);

  obj_file = sprintf("/obj/%s/%s.lpml", module, obj_name);

  if(file_size(obj_file) < 1)
    return 0;

  e = catch(data = lpml_decode(read_file(obj_file)));
  if(e) {
    log_file("VIRTUAL", e);
    return 0;
  }

  if(!data)
    return 0;

  base_file = sprintf("/obj/%s/%s.c", module, module);
  e = catch(obj = new(base_file, data));
  if(e) {
    log_file("VIRTUAL", e);
    if(obj)
      obj->remove();
    return 0;
  }

  return obj;
}
