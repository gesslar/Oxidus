/**
 * @file /adm/daemons/config.c
 *
 * Configuration management daemon that provides a centralized system for
 * game settings. Uses a cascading configuration pattern where default values
 * are loaded from default.lpml and can be overridden by local settings in
 * config.lpml.
 *
 * The two-file system enables:
 * - Default values stored in git (/adm/etc/default.lpml)
 * - Local overrides in /adm/etc/config.lpml (not in git)
 * - Easy upgrades without losing custom settings
 * - Environment-specific configurations
 *
 * @created 2024-02-03 - Gesslar
 * @last_modified 2024-02-03 - Gesslar
 *
 * @history
 * 2024-02-03 - Gesslar - Created
 */

inherit STD_DAEMON;

// Forward declarations
public void rehash_config();
public mixed get_mud_config(string key);

private nosave string DEFAULT_CONFIG = "/adm/etc/default.lpml";
private nosave string CONFIG_FILE = "/adm/etc/config.lpml";
private nosave mapping config = ([ ]);
private nosave int loaded = false;
/**
 * Initializes the configuration daemon.
 *
 * Sets the daemon to persist and loads the initial configuration.
 */
void setup() {
  set_no_clean(1);
  rehash_config();

  loaded = true;
}

/**
 * Retrieves a configuration value by key.
 *
 * @param {string} key - The configuration key to look up
 * @returns {mixed} The value associated with the key
 * @errors If config is null, key is missing, or key is invalid
 */
public mixed get_mud_config(string key) {
  if(!loaded)
    return null;

  if(nullp(config))
    error("get_mud_config: No configuration found.");

  if(!key)
    error("get_mud_config: Missing key.");

  if(nullp(config[key]))
    // return null;
    error("get_mud_config: Invalid key: " + key + ".");

  return config[key];
}

/**
 * Reloads configuration from both default and override files.
 *
 * Loads and merges configurations in this order:
 * 1. /adm/etc/default.lpml - Base configuration
 * 2. /adm/etc/config.lpml - Local overrides
 *
 * Later values override earlier ones for the same keys.
 */
public void rehash_config() {
  mapping temp;

  debug_message(call_trace(false));

  if(file_exists(DEFAULT_CONFIG)) {
    temp = lpml_decode(read_file(DEFAULT_CONFIG));

    if(mapp(temp))
      config += temp;
  }

  if(file_exists(CONFIG_FILE)) {
    temp = lpml_decode(read_file(CONFIG_FILE));

    if(mapp(temp))
      config += temp;
  }
}

/**
 * Returns a copy of the entire configuration mapping.
 *
 * @returns {mapping} A copy of the current configuration
 */
mapping get_all_config() {
  return copy(config);
}
