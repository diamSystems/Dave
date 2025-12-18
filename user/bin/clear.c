#include "types.h"
#include "user.h"

int
main(void)
{
  // ANSI escape sequence: clear screen and move cursor home.
  printf(1, "\033[2J\033[H");
  exit();
}
