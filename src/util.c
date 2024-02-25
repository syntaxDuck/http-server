#include "util.h"
#include <ctype.h>
#include <string.h>

void remove_whitespace(char *str) {
  int len = strlen(str);
  char new_str[len];

  int j = 0;
  for (int i = 0; i < len; i++) {
    if (!isspace(str[i])) {
      new_str[j++] = str[i];
      str[i] = '\0';
    }
  }

  strcpy(str, new_str);
}
