/**
 * @file /adm/daemons/advance.c
 * @description Daemon to handle XP and leveling
 *
 * @created 2024-07-28 - Gesslar
 * @last_modified 2024-07-28 - Gesslar
 *
 * @history
 * 2024-07-28 - Gesslar - Created
 */

inherit STD_DAEMON;

// Functions
int tnl(float level);

// Variables
float base_tnl;
float tnl_rate;
float overlevel_threshold, overlevel_xp_punish;
float underlevel_threshold, underlevel_xp_bonus;

void setup() {
  base_tnl = mud_config("BASE_TNL");
  tnl_rate = mud_config("TNL_RATE");
  overlevel_threshold = mud_config("OVERLEVEL_THRESHOLD");
  overlevel_xp_punish = mud_config("OVERLEVEL_XP_PUNISH");
  underlevel_threshold = mud_config("UNDERLEVEL_THRESHOLD");
  underlevel_xp_bonus = mud_config("UNDERLEVEL_XP_BONUS");
}

/**
 * @function tnl
 * @description Calculates the total XP needed to reach the next level from the
 *              current level.
 * @param {float} level - The current level to calculate TNL for.
 * @returns {int} - The total XP needed to reach the next level.
 */
int tnl(float level) {
  // Ensure level is at least 1.0
  if(level < 1.0)
    level = 1.0;

  // Calculate TNL
  return to_int(base_tnl * pow(tnl_rate, level - 1.0));
}

/**
 * @function can_advance
 * @description Checks if a character has enough XP to advance to the next level.
 * @param {int} xp - The current XP amount.
 * @param {float} level - The current level.
 * @returns {int} - 1 if the character can advance, 0 otherwise.
 */
int can_advance(int xp, float level) {
  return xp >= tnl(level);
}

/**
 * @function advance
 * @description Advances a character to the next level if they have enough XP.
 *              Deducts the required XP, increments the level, and emits a
 *              signal.
 * @param {STD_PLAYER} tp - The player object to advance.
 * @returns {int} - 1 if the character advanced, 0 if they did not have enough
 *                  XP.
 */
int advance(object tp) {
  int xp = tp->query_xp();
  float level = tp->query_level();
  int tnl = tnl(level);

  if(!can_advance(xp, level))
    return 0;

  xp -= tnl;
  level += 1.0;

  tp->set_level(level);
  tp->set_xp(xp);

  emit(SIG_PLAYER_ADVANCED, tp, level);

  return 1;
}

/**
 * @function earn_xp
 * @description Awards XP to a player and optionally auto-levels them if
 *              enabled in the mud configuration.
 * @param {STD_PLAYER} tp - The player object to award XP to.
 * @param {int} amount - The amount of XP to award.
 * @returns {int} - Always returns 1.
 */
int earn_xp(object tp, int amount) {
  debug("earn_xp: " + amount);
  debug("xp: " + tp->query_xp());
  tp->adjust_xp(amount);
  debug("xp: " + tp->query_xp());


  if(mud_config("PLAYER_AUTOLEVEL"))
    advance(tp);

  return 1;
}

/**
 * @function kill_xp
 * @description Calculates and awards XP to a killer for defeating an opponent.
 *              The XP amount is based on the killed opponent's level with
 *              variance, and adjusted based on level difference between killer
 *              and killed.
 * @param {STD_PLAYER} killer - The object that killed the opponent.
 * @param {STD_NPC | STD_PLAYER} killed - The object that was killed.
 * @returns {int} - The amount of XP awarded, or 0 if either object is null.
 */
int kill_xp(object killer, object killed) {
  int xp, tnl;
  int variance;
  float adjustment_factor;
  int killer_level, killed_level, level_difference;

  if(nullp(killer) || nullp(killed))
    return 0;

  killer_level = killer->query_effective_level();
  killed_level = killed->query_effective_level();

  tnl = tnl(killed_level);
  xp = (tnl / 10);
  variance = xp / 10;
  xp = xp - variance + random(variance);

  // Calculate the level difference
  level_difference = killer_level - killed_level;

  // Initialize the adjustment factor
  adjustment_factor = 1.0;

  // Apply punishments if the player is higher level by more than the monster
  if(level_difference > overlevel_threshold)
    adjustment_factor -= overlevel_xp_punish * (level_difference - overlevel_threshold);
  // Apply bonuses if the player is lower level by more than the monster
  else if(level_difference < underlevel_threshold)
    adjustment_factor += underlevel_xp_bonus * (-level_difference);

  // Apply the adjustment factor to the XP
  xp = to_int(xp * adjustment_factor);

  earn_xp(killer, xp);

  return xp;
}
