/**
 * @file /adm/daemons/modules/virtual/storage.c
 * @description This virtual daemon is responsible for creating virtual
 *              storage objects. If the path passed is /storage/public/...
 *              then the object created will be a shared storage object.
 *              If the path passed is /storage/private/... then the object
 *              created will be a container storage object.
 *
 *              It is important to note that we use load_object() rather
 *              than new() for storage objects. This is because we want the
 *              resulting object file name to be unique, but without any
 *              object oid. Here, we use new() because we want the object
 *              to be returned to the driver.
 *
 * @created 2024-08-13 - Gesslar
 * @last_modified 2024-08-13 - Gesslar
 *
 * @history
 * 2024-08-13 - Gesslar - Created
 */

inherit STD_DAEMON;

public nomask object compile_object(string file) {
  string name;
  object ob;
  string e;

  e = catch(ob = new(STD_STORAGE_OBJECT));

  if(e) {
    log_file("VIRTUAL", e);
    return 0;
  }

  if(!ob)
    return 0;

  return ob;
}
