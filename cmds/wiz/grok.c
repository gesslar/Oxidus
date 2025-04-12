// test grok command.

inherit STD_CMD;

void setup() {
  add_verb_rule("");
  add_verb_rule("WRD");
}

mixed can_grok(object player) {
  return 1;
}

mixed can_grok_str(object player, string str) {
  return 1;
}
