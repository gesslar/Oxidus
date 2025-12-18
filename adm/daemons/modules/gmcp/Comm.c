/**
 * @file /adm/daemons/modules/gmcp/Comm.c
 * @description GMCP module to handle Comm.* packages
 *
 * @created 2024-10-10 - Gesslar
 * @last_modified 2024-10-10 - Gesslar
 *
 * @history
 * 2024-10-10 - Gesslar - Created
 */


// /adm/daemons/gmcp_modules/Core.c
//
// Created:     2024/02/23: Gesslar
// Last Change: 2024/02/23: Gesslar
//
// 2024/02/23: Gesslar - Created

#include <gmcp_defines.h>

inherit STD_DAEMON;

/**
 * Handle incoming GMCP `Comm.*` packages and forward them to the
 * recipient player's GMCP handler.
 *
 * This daemon currently recognises the `Text` sub-package. For
 * `Text` it expects `data` to be a mapping containing at least the
 * `channel` and `text` keys. If the mapping does not include a
 * `talker` key, it will be defaulted to the string `"system"`.
 *
 * @public
 *
 * @param {STD_PLAYER} who - The player object which should receive the
 *   forwarded GMCP package.
 * @param {string} sub - The GMCP sub-package name (for example,
 *   `"Text"`).
 * @param {mapping | undefined} data - The payload mapping for the
 *   sub-package; expected keys depend on `sub` (see description).
 * @returns {void}
 * @errors The function uses `assert()` to validate the structure of
 *   `data` for the `Text` sub-package; failing validation will raise an
 *   assertion error.
 * @example
 * // Forward a text message to the player's GMCP handler
 * Channel(player, "Text", ([ "channel": "say", "text": "Hi" ]));
 */
varargs void Channel(object who, string sub, mapping data) {
  switch(sub) {
    case "Text": {
      assert((:
            mapp($(data))
        && !nullp($(data)["channel"])
        && !nullp($(data)["text"])
      :));

      if(nullp(data["talker"]))
        data["talker"] = "system";

      who->do_gmcp(GMCP_PKG_COMM_CHANNEL_TEXT, data);
      break;
    }
  }
}
