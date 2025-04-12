/**
 * @file /adm/daemons/virtual.c
 * @description Daemon for virtual objects
 *
 * @created 2024-02-04 - Gesslar
 * @last_modified 2024-08-22 - Gesslar
 *
 * @history
 * 2024-02-04 - Gesslar - Created
 * 2024-07-29 - Gesslar - Refactored to handle complex and top-level structures
 * 2024-08-21 - Gesslar - Refactored to use file extensions for virtual object types
 * 2024-08-22 - Gesslar - Updated to use pcre_extract for more robust extension extraction
 */

inherit STD_DAEMON;

public nomask object compile_object(string file) {
  string *path_components, *dir_parts;
  string vclass, module, e;
  object result;
  string extension;
  mixed *matches;

  // Extract file extension using pcre_extract
  matches = pcre_extract(file, "^(.+)\\.([^.]+)$");
  if(sizeof(matches) == 2) {
    file = matches[0];
    extension = matches[1];
    module = sprintf("%smodules/virtual/%s.c", __DIR__, extension);
    if(file_exists(module)) {
      e = catch(load_object(module));
      if(e) {
        log_file("VIRTUAL", e);
        return 0;
      }

      e = catch(result = call_other(module, "compile_object", file));
      if(e) {
        log_file("VIRTUAL", e);
        return 0;
      }

      result->set_virtual_master(base_name(file));

      return result;
    }
  }

  // If no extension or no corresponding module, proceed with existing logic
  path_components = dir_file(file);
  dir_parts = explode(path_components[0], "/") - ({""});

  // First, attempt to handle complex structures using underscore concatenation
  vclass = implode(dir_parts, "_");
  module = sprintf("%smodules/virtual/%s.c", __DIR__, vclass);

  // Check if the module exists and handle accordingly
  if(file_exists(module)) {
    e = catch(load_object(module));
    if(e) {
      log_file("VIRTUAL", e);
      return 0;
    }

    e = catch(result = call_other(module, "compile_object", file));
    if(e) {
      log_file("VIRTUAL", e);
      return 0;
    }

    return result;
  }

  // If complex structure handling fails, try the top-level directory
  if(sizeof(dir_parts) > 0) {
    vclass = dir_parts[0];
    module = sprintf("%smodules/virtual/%s.c", __DIR__, vclass);
    if(file_exists(module)) {
      e = catch(load_object(module));
      if(e) {
        log_file("VIRTUAL", e);
        return 0;
      }

      e = catch(result = call_other(module, "compile_object", file));
      if(e) {
        log_file("VIRTUAL", e);
        return 0;
      }

      return result;
    }
  }

  // Return 0 if no matching virtual daemon is found
  return 0;
}
