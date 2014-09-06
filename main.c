#include <stdio.h>

#define INPUT_SIZE 2048

static char input[INPUT_SIZE];

int main (int args, char *argv[]) {
  puts("Ba-bah scheme, C-c to exit\n");
  while (1) {
    puts("Ba-bah> ");
    fgets(input, sizeof(input), stdin);
    printf("Inputz: %s\n", input)
  }
  return 0;
}







