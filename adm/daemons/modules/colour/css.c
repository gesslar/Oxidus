/**
* @file /adm/daemons/modules/colour/css.c
* @description CSS colour codes for colour, forward and reverse
*
* @created 2024-07-14 - Gesslar
* @last_modified 2024-07-14 - Gesslar
*
* @history
* 2024-07-14 - Gesslar - Created
*/

mapping colour_to_hex = ([
  "000": "#000000", "001": "#800000", "002": "#008000", "003": "#808000",
  "004": "#000080", "005": "#800080", "006": "#008080", "007": "#C0C0C0",
  "008": "#808080", "009": "#FF0000", "010": "#00FF00", "011": "#FFFF00",
  "012": "#0000FF", "013": "#FF00FF", "014": "#00FFFF", "015": "#FFFFFF",
  "016": "#000000", "017": "#00005F", "018": "#000087", "019": "#0000AF",
  "020": "#0000D7", "021": "#0000FF", "022": "#005F00", "023": "#005F5F",
  "024": "#005F87", "025": "#005FAF", "026": "#005FD7", "027": "#005FFF",
  "028": "#008700", "029": "#00875F", "030": "#008787", "031": "#0087AF",
  "032": "#0087D7", "033": "#0087FF", "034": "#00AF00", "035": "#00AF5F",
  "036": "#00AF87", "037": "#00AFAF", "038": "#00AFD7", "039": "#00AFFF",
  "040": "#00D700", "041": "#00D75F", "042": "#00D787", "043": "#00D7AF",
  "044": "#00D7D7", "045": "#00D7FF", "046": "#00FF00", "047": "#00FF5F",
  "048": "#00FF87", "049": "#00FFAF", "050": "#00FFD7", "051": "#00FFFF",
  "052": "#5F0000", "053": "#5F005F", "054": "#5F0087", "055": "#5F00AF",
  "056": "#5F00D7", "057": "#5F00FF", "058": "#5F5F00", "059": "#5F5F5F",
  "060": "#5F5F87", "061": "#5F5FAF", "062": "#5F5FD7", "063": "#5F5FFF",
  "064": "#5F8700", "065": "#5F875F", "066": "#5F8787", "067": "#5F87AF",
  "068": "#5F87D7", "069": "#5F87FF", "070": "#5FAF00", "071": "#5FAF5F",
  "072": "#5FAF87", "073": "#5FAFAF", "074": "#5FAFD7", "075": "#5FAFFF",
  "076": "#5FD700", "077": "#5FD75F", "078": "#5FD787", "079": "#5FD7AF",
  "080": "#5FD7D7", "081": "#5FD7FF", "082": "#5FFF00", "083": "#5FFF5F",
  "084": "#5FFF87", "085": "#5FFFAF", "086": "#5FFFD7", "087": "#5FFFFF",
  "088": "#870000", "089": "#87005F", "090": "#870087", "091": "#8700AF",
  "092": "#8700D7", "093": "#8700FF", "094": "#875F00", "095": "#875F5F",
  "096": "#875F87", "097": "#875FAF", "098": "#875FD7", "099": "#875FFF",
  "100": "#878700", "101": "#87875F", "102": "#878787", "103": "#8787AF",
  "104": "#8787D7", "105": "#8787FF", "106": "#87AF00", "107": "#87AF5F",
  "108": "#87AF87", "109": "#87AFAF", "110": "#87AFD7", "111": "#87AFFF",
  "112": "#87D700", "113": "#87D75F", "114": "#87D787", "115": "#87D7AF",
  "116": "#87D7D7", "117": "#87D7FF", "118": "#87FF00", "119": "#87FF5F",
  "120": "#87FF87", "121": "#87FFAF", "122": "#87FFD7", "123": "#87FFFF",
  "124": "#AF0000", "125": "#AF005F", "126": "#AF0087", "127": "#AF00AF",
  "128": "#AF00D7", "129": "#AF00FF", "130": "#AF5F00", "131": "#AF5F5F",
  "132": "#AF5F87", "133": "#AF5FAF", "134": "#AF5FD7", "135": "#AF5FFF",
  "136": "#AF8700", "137": "#AF875F", "138": "#AF8787", "139": "#AF87AF",
  "140": "#AF87D7", "141": "#AF87FF", "142": "#AFAF00", "143": "#AFAF5F",
  "144": "#AFAF87", "145": "#AFAFAF", "146": "#AFAFD7", "147": "#AFAFFF",
  "148": "#AFD700", "149": "#AFD75F", "150": "#AFD787", "151": "#AFD7AF",
  "152": "#AFD7D7", "153": "#AFD7FF", "154": "#AFFF00", "155": "#AFFF5F",
  "156": "#AFFF87", "157": "#AFFFAF", "158": "#AFFFD7", "159": "#AFFFFF",
  "160": "#D70000", "161": "#D7005F", "162": "#D70087", "163": "#D700AF",
  "164": "#D700D7", "165": "#D700FF", "166": "#D75F00", "167": "#D75F5F",
  "168": "#D75F87", "169": "#D75FAF", "170": "#D75FD7", "171": "#D75FFF",
  "172": "#D78700", "173": "#D7875F", "174": "#D78787", "175": "#D787AF",
  "176": "#D787D7", "177": "#D787FF", "178": "#D7AF00", "179": "#D7AF5F",
  "180": "#D7AF87", "181": "#D7AFAF", "182": "#D7AFD7", "183": "#D7AFFF",
  "184": "#D7D700", "185": "#D7D75F", "186": "#D7D787", "187": "#D7D7AF",
  "188": "#D7D7D7", "189": "#D7D7FF", "190": "#D7FF00", "191": "#D7FF5F",
  "192": "#D7FF87", "193": "#D7FFAF", "194": "#D7FFD7", "195": "#D7FFFF",
  "196": "#FF0000", "197": "#FF005F", "198": "#FF0087", "199": "#FF00AF",
  "200": "#FF00D7", "201": "#FF00FF", "202": "#FF5F00", "203": "#FF5F5F",
  "204": "#FF5F87", "205": "#FF5FAF", "206": "#FF5FD7", "207": "#FF5FFF",
  "208": "#FF8700", "209": "#FF875F", "210": "#FF8787", "211": "#FF87AF",
  "212": "#FF87D7", "213": "#FF87FF", "214": "#FFAF00", "215": "#FFAF5F",
  "216": "#FFAF87", "217": "#FFAFAF", "218": "#FFAFD7", "219": "#FFAFFF",
  "220": "#FFD700", "221": "#FFD75F", "222": "#FFD787", "223": "#FFD7AF",
  "224": "#FFD7D7", "225": "#FFD7FF", "226": "#FFFF00", "227": "#FFFF5F",
  "228": "#FFFF87", "229": "#FFFFAF", "230": "#FFFFD7", "231": "#FFFFFF",
  "232": "#080808", "233": "#121212", "234": "#1C1C1C", "235": "#262626",
  "236": "#303030", "237": "#3A3A3A", "238": "#444444", "239": "#4E4E4E",
  "240": "#585858", "241": "#606060", "242": "#666666", "243": "#767676",
  "244": "#808080", "245": "#8A8A8A", "246": "#949494", "247": "#9E9E9E",
  "248": "#A8A8A8", "249": "#B2B2B2", "250": "#BCBCBC", "251": "#C6C6C6",
  "252": "#D0D0D0", "253": "#DADADA", "254": "#E4E4E4", "255": "#EEEEEE"
]);

mapping hex_to_colour = ([
  "#000000": "000", "#800000": "001", "#008000": "002", "#808000": "003",
  "#000080": "004", "#800080": "005", "#008080": "006", "#C0C0C0": "007",
  "#808080": "008", "#FF0000": "009", "#00FF00": "010", "#FFFF00": "011",
  "#0000FF": "012", "#FF00FF": "013", "#00FFFF": "014", "#FFFFFF": "015",
  "#000000": "016", "#00005F": "017", "#000087": "018", "#0000AF": "019",
  "#0000D7": "020", "#0000FF": "021", "#005F00": "022", "#005F5F": "023",
  "#005F87": "024", "#005FAF": "025", "#005FD7": "026", "#005FFF": "027",
  "#008700": "028", "#00875F": "029", "#008787": "030", "#0087AF": "031",
  "#0087D7": "032", "#0087FF": "033", "#00AF00": "034", "#00AF5F": "035",
  "#00AF87": "036", "#00AFAF": "037", "#00AFD7": "038", "#00AFFF": "039",
  "#00D700": "040", "#00D75F": "041", "#00D787": "042", "#00D7AF": "043",
  "#00D7D7": "044", "#00D7FF": "045", "#00FF00": "046", "#00FF5F": "047",
  "#00FF87": "048", "#00FFAF": "049", "#00FFD7": "050", "#00FFFF": "051",
  "#5F0000": "052", "#5F005F": "053", "#5F0087": "054", "#5F00AF": "055",
  "#5F00D7": "056", "#5F00FF": "057", "#5F5F00": "058", "#5F5F5F": "059",
  "#5F5F87": "060", "#5F5FAF": "061", "#5F5FD7": "062", "#5F5FFF": "063",
  "#5F8700": "064", "#5F875F": "065", "#5F8787": "066", "#5F87AF": "067",
  "#5F87D7": "068", "#5F87FF": "069", "#5FAF00": "070", "#5FAF5F": "071",
  "#5FAF87": "072", "#5FAFAF": "073", "#5FAFD7": "074", "#5FAFFF": "075",
  "#5FD700": "076", "#5FD75F": "077", "#5FD787": "078", "#5FD7AF": "079",
  "#5FD7D7": "080", "#5FD7FF": "081", "#5FFF00": "082", "#5FFF5F": "083",
  "#5FFF87": "084", "#5FFFAF": "085", "#5FFFD7": "086", "#5FFFFF": "087",
  "#870000": "088", "#87005F": "089", "#870087": "090", "#8700AF": "091",
  "#8700D7": "092", "#8700FF": "093", "#875F00": "094", "#875F5F": "095",
  "#875F87": "096", "#875FAF": "097", "#875FD7": "098", "#875FFF": "099",
  "#878700": "100", "#87875F": "101", "#878787": "102", "#8787AF": "103",
  "#8787D7": "104", "#8787FF": "105", "#87AF00": "106", "#87AF5F": "107",
  "#87AF87": "108", "#87AFAF": "109", "#87AFD7": "110", "#87AFFF": "111",
  "#87D700": "112", "#87D75F": "113", "#87D787": "114", "#87D7AF": "115",
  "#87D7D7": "116", "#87D7FF": "117", "#87FF00": "118", "#87FF5F": "119",
  "#87FF87": "120", "#87FFAF": "121", "#87FFD7": "122", "#87FFFF": "123",
  "#AF0000": "124", "#AF005F": "125", "#AF0087": "126", "#AF00AF": "127",
  "#AF00D7": "128", "#AF00FF": "129", "#AF5F00": "130", "#AF5F5F": "131",
  "#AF5F87": "132", "#AF5FAF": "133", "#AF5FD7": "134", "#AF5FFF": "135",
  "#AF8700": "136", "#AF875F": "137", "#AF8787": "138", "#AF87AF": "139",
  "#AF87D7": "140", "#AF87FF": "141", "#AFAF00": "142", "#AFAF5F": "143",
  "#AFAF87": "144", "#AFAFAF": "145", "#AFAFD7": "146", "#AFAFFF": "147",
  "#AFD700": "148", "#AFD75F": "149", "#AFD787": "150", "#AFD7AF": "151",
  "#AFD7D7": "152", "#AFD7FF": "153", "#AFFF00": "154", "#AFFF5F": "155",
  "#AFFF87": "156", "#AFFFAF": "157", "#AFFFD7": "158", "#AFFFFF": "159",
  "#D70000": "160", "#D7005F": "161", "#D70087": "162", "#D700AF": "163",
  "#D700D7": "164", "#D700FF": "165", "#D75F00": "166", "#D75F5F": "167",
  "#D75F87": "168", "#D75FAF": "169", "#D75FD7": "170", "#D75FFF": "171",
  "#D78700": "172", "#D7875F": "173", "#D78787": "174", "#D787AF": "175",
  "#D787D7": "176", "#D787FF": "177", "#D7AF00": "178", "#D7AF5F": "179",
  "#D7AF87": "180", "#D7AFAF": "181", "#D7AFD7": "182", "#D7AFFF": "183",
  "#D7D700": "184", "#D7D75F": "185", "#D7D787": "186", "#D7D7AF": "187",
  "#D7D7D7": "188", "#D7D7FF": "189", "#D7FF00": "190", "#D7FF5F": "191",
  "#D7FF87": "192", "#D7FFAF": "193", "#D7FFD7": "194", "#D7FFFF": "195",
  "#FF0000": "196", "#FF005F": "197", "#FF0087": "198", "#FF00AF": "199",
  "#FF00D7": "200", "#FF00FF": "201", "#FF5F00": "202", "#FF5F5F": "203",
  "#FF5F87": "204", "#FF5FAF": "205", "#FF5FD7": "206", "#FF5FFF": "207",
  "#FF8700": "208", "#FF875F": "209", "#FF8787": "210", "#FF87AF": "211",
  "#FF87D7": "212", "#FF87FF": "213", "#FFAF00": "214", "#FFAF5F": "215",
  "#FFAF87": "216", "#FFAFAF": "217", "#FFAFD7": "218", "#FFAFFF": "219",
  "#FFD700": "220", "#FFD75F": "221", "#FFD787": "222", "#FFD7AF": "223",
  "#FFD7D7": "224", "#FFD7FF": "225", "#FFFF00": "226", "#FFFF5F": "227",
  "#FFFF87": "228", "#FFFFAF": "229", "#FFFFD7": "230", "#FFFFFF": "231",
  "#080808": "232", "#121212": "233", "#1C1C1C": "234", "#262626": "235",
  "#303030": "236", "#3A3A3A": "237", "#444444": "238", "#4E4E4E": "239",
  "#585858": "240", "#606060": "241", "#666666": "242", "#767676": "243",
  "#808080": "244", "#8A8A8A": "245", "#949494": "246", "#9E9E9E": "247",
  "#A8A8A8": "248", "#B2B2B2": "249", "#BCBCBC": "250", "#C6C6C6": "251",
  "#D0D0D0": "252", "#DADADA": "253", "#E4E4E4": "254", "#EEEEEE": "255"
]);

varargs mixed colour_to_hex(int colour) {
  if(nullp(colour))
    return colour_to_hex;

  if(colour < 0 || colour > 255 )
    return "";

  return colour_to_hex[sprintf("%03d", colour)];
}

varargs mixed hex_to_colour(string hex) {
  if(nullp(hex))
    return hex_to_colour;

  hex = upper_case(hex);
  if(hex[0] == '#')
    hex = hex[1..];

  return hex_to_colour[hex];
}
