#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <ios>
#include <string>
#include <memory.h>

#include <patch/patch.h>
#include <sys/types.h>

template <typename Return, typename... Args>
void calorimeter(std::function<Return(Args...)> function_to_test,
                 Args... arguments_for_function_to_test) {
  // This will invoke the function that we are going to test.
  function_to_test(arguments_for_function_to_test...);
}

extern "C" void time_me(int testing, int one, int two) {
  std::cout << "I am being timed.\n";
  printf("testing: %d, one: %d, two: %d\n", testing, one, two);
}

extern "C" void woah() {
  std::cout << "If this works ...\n";
}

void t() {
  std::cout <<"There's no way this will work!\n";
}

int main() {

  //time_me(5, 6, 7);
  woah();

  hook("woah", (void*)t);

  //time_me(5, 6, 7);
  woah();

  unhook("woah");

  woah();

  return 0;
}