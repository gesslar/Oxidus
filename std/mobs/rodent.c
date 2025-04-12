/**
 * @file /std/mobs/rodent.c
 * @description Virtual rodents
 *
 * @created 2024-08-20 - Gesslar
 * @last_modified 2024-08-20 - Gesslar
 *
 * @history
 * 2024-08-20 - Gesslar - Created
 */

inherit __DIR__ "mammal";

void monster_setup(mixed args...) {
  ::monster_setup(args...);
  set_race("rodent");
}
