string test = "hi  there    this has a lot    of spaces    .";

text = reduce(explode(trim(test), ""), function(string acc, string char, int index, string *chars) {
  if(char == " " && chars[index - 1] == " ")
    return acc;

  return acc + char;

}, "");
