#include <functional>
#include <string>
#include <iostream>

template <typename Return, typename ...Args>
void calorimeter(std::function<Return(Args...)> function_to_test, Args... arguments_for_function_to_test) {
    // This will invoke the function that we are going to test.
    function_to_test(arguments_for_function_to_test...);
}

void time_me() {
    std::cout << "I am being timed.\n";
    return;
}

int main() {
    std::function<void(void)> time_me_fn = time_me;
    calorimeter(time_me_fn);
    return 0;
}