inherit STD_DAEMON;

public nomask object compile_object(string file) {
  string *parts;
  string mob_name, mob_file;
  mapping data;
  string e;

  parts = dir_file(file);
  mob_name = parts[1];

  mob_file = "/d/mobs/" + mob_name + ".lpml";
  if(file_size(mob_file) < 1)
    return 0;

  e = catch(data = lpml_decode(read_file(mob_file)));
  if(e) {
    log_file("VIRTUAL", e);
    return 0;
  }

  if(!data)
    return 0;

  { // parse the data and make the monster
    string mob_type = data["type"];
    string object_file = "/std/mobs/%s.c";
    mixed args;
    object mob;

    mob_type = replace_string(mob_type, " ", "_");
    object_file = sprintf(object_file, mob_type);
    if(!file_exists(object_file))
      return 0;

    e = catch(mob = new(object_file, data));
    if(e) {
      log_file("VIRTUAL", e);
      if(mob)
        mob->remove();
      return 0;
    }

    return mob;
  }
}
