/**
 * @file /std/modules/loot.c
 * @description Loot module for anything.
 *
 * @created 2024-08-05 - Gesslar
 * @last_modified 2024-08-05 - Gesslar
 *
 * @history
 * 2024-08-05 - Gesslar - Created
 */


private nosave mixed *_loot_table = ({ });
private nosave mixed *_coin_table = ({ });

varargs public void add_loot(mixed item, float chance) {
  if(nullp(item))
    return;

  if(nullp(chance))
    chance = 100.0;
  else if(intp(chance))
    chance = to_float(chance);

  if(!floatp(chance))
    return;

  chance = clamp(0.0, 100.0, chance);

  switch(typeof(item)) {
    case T_STRING :
    case T_MAPPING :
    case T_FUNCTION :
    case T_ARRAY :
      _loot_table += ({ ({ item, chance }) });
      break;
    default :
      return;
  }
}

public varargs void set_loot_table(mixed *table) {
  if(!pointerp(table))
    return;

  _loot_table = table;
}

public mixed *query_loot_table() {
  return copy(_loot_table);
}

varargs public void add_coin(string type, int num, float chance) {
  if(nullp(type) || nullp(num))
    return;

  if(nullp(chance))
    chance = 100.0;
  else if(intp(chance))
    chance = to_float(chance);

  if(!floatp(chance))
    return;

  chance = clamp(0.0, 100.0, chance);

  _coin_table += ({ ({ type, num, chance }) });
}

public varargs void set_coin_table(mixed *table) {
  if(!pointerp(table))
    return;

  _coin_table = table;
}

public mixed *query_coin_table() {
  return copy(_coin_table);
}
