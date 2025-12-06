# gLPU True Colour Code Reference for LLMs

This document provides a comprehensive reference for the true colour (24-bit RGB) system used in gLPU. This reference will help LLMs understand how colours are referenced in code and accurately use them in documentation or new code.

## Basic Syntax

Colour codes in gLPU use a double-brace format with hexadecimal RGB values:

- `{{HEXCODE}}` - Foreground colour (3 or 6 digit hex)
- `{{RES}}` or `{{res}}` - Reset all formatting back to default
- Background colours are handled via mode parameter in the daemon

### Examples

```c
// 6-digit hex (full RGB precision)
"{{FF0000}}This is red text{{res}}"
"{{00FF00}}This is green text{{res}}"
"{{0000FF}}This is blue text{{res}}"

// 3-digit hex (shorthand, converted to 6-digit internally)
"{{F00}}This is also red{{res}}"
"{{0F0}}This is also green{{res}}"
"{{00F}}This is also blue{{res}}"
```

## 16 Base System Colours

These are the standard terminal colours defined in the colour daemon:

| Hex Code | Colour Name     | Description |
|----------|----------------|-------------|
| {{000000}} | black          | Pure black |
| {{800000}} | red            | Dark red |
| {{008000}} | green          | Dark green |
| {{808000}} | orange         | Dark yellow/orange |
| {{000080}} | blue           | Dark blue |
| {{800080}} | magenta        | Dark magenta |
| {{008080}} | cyan           | Dark cyan |
| {{C0C0C0}} | white          | Light grey/silver |
| {{808080}} | bright black   | Medium grey |
| {{FF0000}} | bright red     | Bright red |
| {{00FF00}} | bright green   | Bright green |
| {{FFFF00}} | bright orange  | Bright yellow |
| {{0000FF}} | bright blue    | Bright blue |
| {{FF00FF}} | bright magenta | Bright magenta |
| {{00FFFF}} | bright cyan    | Bright cyan |
| {{FFFFFF}} | bright white   | Pure white |

## Hexadecimal Colour System

gLPU uses true colour (24-bit RGB) via hexadecimal codes, giving access to 16.7 million colours.

### Hex Format

- **6-digit hex**: `RRGGBB` where RR, GG, BB are hexadecimal values 00-FF
- **3-digit hex**: `RGB` shorthand (automatically expanded to RRGGBB)

### Colour Examples

```c
// Pure colours
"{{FF0000}}" // Pure red (R=255, G=0, B=0)
"{{00FF00}}" // Pure green (R=0, G=255, B=0)
"{{0000FF}}" // Pure blue (R=0, G=0, B=255)

// Greyscale (R=G=B)
"{{000000}}" // Black
"{{808080}}" // Medium grey
"{{C0C0C0}}" // Light grey
"{{FFFFFF}}" // White

// Mixed colours
"{{FF8000}}" // Orange (R=255, G=128, B=0)
"{{8000FF}}" // Purple (R=128, G=0, B=255)
"{{00FF80}}" // Spring green (R=0, G=255, B=128)
```

## Luminance and "Too Dark" Handling

The colour daemon includes accessibility features to prevent colours that are too dark to read:

### Minimum Luminance

- **Configuration**: `COLOUR_MININUM_LUMINANCE` (default: 50.0)
- **Formula**: Luminance = 0.2126 × R + 0.7152 × G + 0.0722 × B

### Automatic Scaling

Colours below the minimum luminance are automatically brightened by applying a scaling multiplier to all RGB components:

```c
// Example: Dark blue {{000040}} might be too dark
// Daemon calculates: luminance = 0.2126×0 + 0.7152×0 + 0.0722×64 ≈ 4.6
// If minimum is 50.0, multiplier = 50.0 / 4.6 ≈ 10.9
// New RGB: (0, 0, min(255, 64×10.9)) = (0, 0, 255)
// Result: {{0000FF}} (bright blue)
```

### Base Colour Exemptions

The 16 base system colours are exempt from "too dark" scaling to preserve standard terminal colour behaviour.

## Text Formatting Codes

All formatting codes use the double-brace syntax:

| Code | Function | ANSI Escape | Description |
|------|----------|-------------|-------------|
| `{{res}}` or `{{RES}}` | Reset all | \e[0m | Reset all formatting |
| `{{bl0}}` | Bold off | \e[22m | Disable bold |
| `{{bl1}}` | Bold on | \e[1m | Enable bold |
| `{{di0}}` | Dim off | \e[22m | Disable dim |
| `{{di1}}` | Dim on | \e[2m | Enable dim |
| `{{it0}}` | Italic off | \e[23m | Disable italic |
| `{{it1}}` | Italic on | \e[3m | Enable italic |
| `{{ul0}}` | Underline off | \e[24m | Disable underline |
| `{{ul1}}` | Underline on | \e[4m | Enable underline |
| `{{fl0}}` | Flash off | \e[25m | Disable blink/flash |
| `{{fl1}}` | Flash on | \e[5m | Enable blink/flash |
| `{{re0}}` | Reverse off | \e[27m | Disable reverse video |
| `{{re1}}` | Reverse on | \e[7m | Enable reverse video |
| `{{st0}}` | Strikethrough off | \e[29m | Disable strikethrough |
| `{{st1}}` | Strikethrough on | \e[9m | Enable strikethrough |
| `{{ol0}}` | Overline off | \e[55m | Disable overline |
| `{{ol1}}` | Overline on | \e[53m | Enable overline |

### Formatting Examples

```c
"{{bl1}}Bold text{{bl0}} normal text"
"{{it1}}Italic text{{it0}} normal text"
"{{ul1}}Underlined text{{ul0}} normal text"
"{{FF0000}}{{bl1}}Bold red text{{res}}" // Reset clears both colour and bold
```

### Important Notes

- It is **not required** to reset colours after each use - the system switches colours as needed
- Use `{{res}}` to reset **all** formatting (colours and text attributes) back to terminal default
- Attributes can be combined: `{{bl1}}{{ul1}}Bold and underlined{{res}}`

## Useful Functions

When working with colour in code, these simul_efuns and daemon functions are available:

- `no_ansi(string)` - Strip all colour codes from a string
- `colourp(string)` - Check if a string contains colour codes
- `body_colour_replace(body, text, type)` - Apply player-specific colour preferences

See [adm/daemons/colour.c](adm/daemons/colour.c) for implementation details.

## Usage in Code

### String Literals

```c
// In LPC string literals
string message = "{{FF0000}}Error: {{res}}{{FFFF00}}Warning level exceeded{{res}}";
string prompt = "{{00FF00}}> {{res}}";
string title = "{{bl1}}{{FFFFFF}}Chapter One{{res}}";
```

### With Simul Efuns

```c
// Remove colours for length calculations
string plain = no_ansi(coloured_string);
int length = strlen(plain);

// Check if string has colours
if(colourp(text)) {
  // Contains colour codes
}
```

### Message Output

```c
// Body-specific colour preferences (combat messages)
string msg = body_colour_replace(player, damage_text, MSG_COMBAT_HIT);
```

## Best Practices

1. **Always reset**: End coloured text with `{{res}}` to avoid colour bleed
2. **Use base colours**: Prefer the 16 base colours for common uses (consistency)
3. **Test visibility**: Very dark or very bright colours may be hard to read
4. **Consider accessibility**: The daemon's luminance system helps, but choose wisely
5. **Cache-friendly**: Reuse common colour codes (they're cached automatically)
6. **Canadian spelling**: Use "colour" not "color" in code and documentation
