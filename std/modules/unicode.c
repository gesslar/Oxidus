/**
 * @file /std/modules/unicode.c
 * @description Module for handling unicode characters.
 *
 * Portions of this were delightfully and unabashedly stolen from the Lima base
 * library.
 *
 * @created 2024-07-18 - Gesslar
 * @last_modified 2024-07-18 - Gesslar
 *
 * @history
 * 2024-07-18 - Gesslar - Created
 */

/**
Original Lima header:
/*
** M_WIDGETS (Tsath 2019-2020)
**
** This module returns nicely formatted ANSI/xterm type widgets that can be
** reused all over the MUD. The string return are not sprintf() compatible, so
** you need to tell_me() or out() them.
**
*/

#include <colour.h>

private nomask nosave mapping _c = ([
    "green" : "{{009966}}",
    "blue"  : "{{0033FF}}",
    "purple": "{{990099}}",
    "yellow": "{{CCCC33}}",
    "gold"  : "{{FFCC00}}",
    "black" : "{{444444}}",
    "white" : "{{FFFFFF}}",
]);

private nomask nosave string *slider_colours = ({
    "{{FF0000}}", // red
    "{{FF3300}}", // dark orange
    "{{FF6600}}", // orange
    "{{FF9900}}", // light orange
    "{{FFCC00}}", // gold
    "{{FFFF00}}", // yellow
    "{{CCFF00}}", // light yellow
    "{{99FF00}}", // light green
    "{{66FF00}}", // green
    "{{00FF33}}", // bright green
});

/**
 * Returns true if the this_body() supports unicode
 *
 * @returns {int} 1 if the body supports unicode, 0 otherwise
 */
int supports_unicode() {
  /** @type {STD_BODY} */
  object body = this_body();
  int unicode;

  if(!body)
    return 0;

  if(body->has_screenreader())
    return 0;

  return body->supports_unicode();
}

/**
 * Returns the body's configured word wrap settings as the screen width,
 * or 78, if not set.
 *
 * @returns {int} The configured screen width
 */
int default_user_width() {
  /** @type {STD_BODY} */
  object body = this_body();

  if(body)
    if(body->query_environ("WORD_WRAP"))
      return body->query_environ("WORD_WRAP") - 2;

  return 78;
}

/**
 * Returns the colour in the colour array, cols, to use given the position
 * out of the width (typically screen width).
 *
 * @private
 * @param {string*} cols - List of colours.
 * @param {int} position - The position in the bar.
 * @param {int} width - The total width of the text bar.
 * @returns {string} The colour based on the position within the width.
 *
 * @example
 * use_colour(({RED,ORANGE,BROWN}), 5, 10); // ORANGE
 */
private string use_colour(string *cols, int position, int width) {
  int col_index = floor(sizeof(cols) * ((0.0 + position) / (width || 1)));

  col_index = clamp(0, sizeof(cols) - 1, col_index);

  return cols[col_index];
}

/**
 * Given a string, spread the colour array equally over the string length.
 *
 * @param {string} str - The string to colour.
 * @param {string*} cols - The colours from which to select, left to right.
 *
 * @example
 * gradient_string("Hello world",({RED,ORANGE,GREEN,BLUE}));
 */
string gradient_string(string str, string *cols) {
  string *exstr = explode(str, "");
  int i = strlen(str), len = i;

  while(i--)
    exstr[i] = use_colour(cols, i, len) + exstr[i];

  return implode(exstr, "") + "{{res}}";
}

/**
 * Returns a string for display representing "on" or "off".
 * [On ] (green) or [Off] (red) depending on int on.
 * "On " or "Off" for simplified view.
 *
 * @param {int} on - Whether the string should be "on" or "off".
 * @returns {string} The formatted string.
*/
string on_off_widget(int on) {
  object body = this_body();

  if(body->has_screenreader())
    return on ? "On " : "Off ";

  return on
    ? supports_unicode()
      ? "[ " SYSTEM_OK "✓{{res}} ]"
      : "["  SYSTEM_OK "On{{res}} ]"
    : supports_unicode()
      ? "[ " SYSTEM_ERROR "✕{{res}} ]"
      : "["  SYSTEM_ERROR "Off{{res}}]"
;
}
/**
 * Returns a string that serves as a simple divider either using unicode or
 * not, depending on user settings.
 *
 * The divider is a hyphen or a unicode version.
 *
 * @see simple_double_divider
 * @param {int} [width] - The width of the bar, or the user's screen width.
 * @returns {string} The bar to be displayed to the user.
 */
varargs string simple_divider(int width) {
  object body = this_body();
  string barchar = supports_unicode() ? "─" : "-";

  if(body->has_screenreader())
    return "";

  if(!width)
    width = default_user_width();

  return repeat_string(barchar, width);
}

/**
 * Returns a string that serves as a simple divider either using unicode or
 * not, depending on user settings.
 *
 * The divider is an equals or a unicode version.
 * @see simple_divider
 * @param {int} [width] - The width of the bar, or the user's screen width.
 * @returns {string} The bar to be displayed to the user.
 */
varargs string simple_double_divider(int width) {
    object body = this_body();
    string barchar = supports_unicode() ? "═" : "=";

    if(body->has_screenreader())
        return "";

    if(!width)
        width = default_user_width();

    return repeat_string(barchar, width);
}

/**
 * Returns a string that represents a progress bar that is white for
 * the region representing the percentage of value over max, with
 * the remainder being green.
 *
 * 0 = fully green bar
 *
 * @param {int} value - The value to represent as white.
 * @param {int} max - The maximum value for the bar.
 * @param {int} width - The width of the bar.
 * @returns {string} The coloured string that is the progress bar.
 */
string green_bar(int value, int max, int width) {
  object body = this_body();
  int green, white;
  string barchar = supports_unicode() ? "▅" : "=";
  string nobarchar = supports_unicode() ? "▅" : ".";
  string white_colour;

  if(body->has_screenreader())
    return value + "/" + max;

  if(value > max)
    value = max;

  green = (value * 1.00 / max) * (width)-2;
  white = width - 2 - green;
  white_colour = gradient_hex(_c["green"], -white*1.75);

  return sprintf("[" + (value >= max ? _c["green"] : white_colour) + "%s{{res}}"+_c["black"]+"%s{{res}}]",
    repeat_string(barchar, green), repeat_string(nobarchar, white)
  );
}

//: FUNCTION critical_bar
// A bar that change colour the lower it gets
string critical_bar(int value, int max, int width) {
  object body = this_body();
  int green, white;
  float p;
  string barchar = supports_unicode() ? "▅" : "=";
  string nobarchar = supports_unicode() ? "▅" : ".";
  string bar_colour = "{{009966}}";

  if(body->has_screenreader())
    return value + "/" + max;

    p = value / (max * 1.0);

  if(p < 0.10)
    bar_colour = _c["purple"];
  else if(p < 0.20)
    bar_colour = _c["yellow"];
  else if(p < 0.50)
  bar_colour = _c["gold"];

  if(value > max)
    value = max;

  green = (value * 1.00 / max) * (width)-1;
  if(green < 0)
    green = 0;
  white = width - 1 - green;

  return sprintf("[" + bar_colour + "%s{{res}}"+_c["black"]+"%s{{res}}]",
    repeat_string(barchar, green), repeat_string(nobarchar, white)
  );
}

//: FUNCTION reverse_critical_bar
// A bar that change colour the lower it gets
string reverse_critical_bar(int value, int max, int width) {
  object body = this_body();
  int green, white;
  float p;
  string barchar = supports_unicode() ? "▅" : "=";
  string nobarchar = supports_unicode() ? "▅" : ".";
  string bar_colour = "{{009966}}";

  if(!max)
    return "";

  if(body->has_screenreader())
    return value + "/" + max;

  p = value / (max * 1.0);

  if(p < 0.30)
    bar_colour = _c["green"];
  if(p < 0.60)
    bar_colour = _c["gold"];
  else if(p < 0.80)
    bar_colour = _c["yellow"];

  if(value > max)
    value = max;

  green = (value * 1.00 / max) * (width)-1;
  if(green < 0)
    green = 0;
  white = width - 1 - green;

  return sprintf("[" + bar_colour + "%s{{res}}"+_c["black"]+"%s{{res}}]",
    repeat_string(barchar, green), repeat_string(nobarchar, white)
  );
}

//: FUNCTION slider_red_green
// A slider which is red below the middle and green above, and marks the
// value with a X. 0 is the middle and max is minus on red axis and + on green axis.
string slider_red_green(int value, int max, int width) {
  object body = this_body();
  string return_string;
  int marker;
  string x_char;
  string line_char;

  if(body->has_screenreader())
    return value + "/" + max;

  if(supports_unicode()) {
    x_char = "●";
    line_char = "▬";
  } else {
    x_char = "X";
    line_char = "-";
  }

  width = width - 2; // [, X and ]

  marker = width * ((value + (max / 2.0)) / (max * 1.0));

  return_string = repeat_string(line_char, marker) + x_char + repeat_string(line_char, width - marker);
  return_string = return_string[0..(width / 2)] + return_string[(width / 2 + 1)..];
  return_string = gradient_string(return_string, slider_colours);
  return_string = replace_string(return_string, x_char, _c["white"]+ x_char + "{{res}}");

  return "[" + return_string + "{{res}}] ";
}

//: FUNCTION slider_colours_sum
// A slider with multiple colours and cumulative ranges and a marker.
// The colours mapping should be on the format:
//   ([20:"040",50:"041",100:"042"])
// where each number is bigger and strings are ANSI colours.
string slider_colours_sum(int value, mapping colours, int width) {
  object body = this_body();
  int marker;
  int max = sort_array(keys(colours), -1)[0];
  string return_string;
  int colour_add = 0;
  int next_pos = 0;
  string x_char;
  string line_char;
  string colour_after_marker; // Save this colour to make things easier later.

  if(body->has_screenreader())
    return value + "/" + max;

  if(supports_unicode()) {
    x_char = "●";
    line_char = "▬";
  } else {
    x_char = "X";
    line_char = "-";
  }

  width = width - 3; // [ and ]
  marker = width * (1.0 * value / max);

  if(marker == 0)
    return_string = x_char + repeat_string(line_char, width - 1);
  else if(marker == (width - 1))
    return_string = repeat_string(line_char, width) + x_char;
  else
    return_string = repeat_string(line_char, marker) + x_char + repeat_string(line_char, width - marker);

  foreach(int val in sort_array(keys(colours), 1)) {
    string col = colours[val];
    if(!next_pos)
      return_string = col + return_string;
    if(next_pos)
      return_string = return_string[0..(next_pos + colour_add)] + col +
                      return_string[((next_pos + colour_add) + 1)..];
    colour_add += strlen(col); // TODO: Check this is correct
    next_pos = width * (1.0 * val / max);
    if(next_pos > marker && !colour_after_marker)
      colour_after_marker = col;
  }

  return_string = replace_string(return_string, x_char, _c["white"] + x_char + colour_after_marker);

  return "[" + return_string + "{{res}}] ";
}
