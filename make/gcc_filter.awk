#!/usr/bin/gawk -f

/^In file included from/ {
  l1 = $0;
  idx = 0;
  #print "--started on " l1;
  while (1) {
    getline line;
    if (!(line ~ /^[ \t]* from /)) {
      break;
    }
    #print "--saving " line;
    lines[idx] = line;
    idx++
  }

  #print "--broke on " line;

  if (!(line ~ /This file contains more/)) {
    print l1;
    for (i = 0; i < idx; i++)
      print lines[i];
  }
  next;
}

{
  print;
}
