/**
 * @file /std/mobs/mammal.c
 * @description Virtual mammals
 *
 * @created 2024-08-20 - Gesslar
 * @last_modified 2024-08-20 - Gesslar
 *
 * @history
 * 2024-08-20 - Gesslar - Created
 */

inherit __DIR__ "monster";

void monster_setup(mixed args...) {
  set_race("mammal");
}
