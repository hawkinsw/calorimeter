#include <functional>
#include <string>
#include <iostream>
#include <chrono>
template <typename Return, typename ...Args>
void calorimeter(std::function<Return(Args...)> function_to_test, Args... arguments_for_function_to_test) {
    // This will invoke the function that we are going to test.
    auto start = std::chrono::high_resolution_clock::now();
    function_to_test(arguments_for_function_to_test...);
    auto end = std::chrono::high_resolution_clock::now();
    //auto out = end - start;
    std::chrono::duration<double> out = end - start;
    std::cout << out.count();
}

void time_me() {
    std::cout << "I am being timed.\n";
    return;
}

void fish(int a) {
    std::cout << "<>< This is a fish. His favorite number is " << a << ".\n";
    return;
}

/*void fish(int &a){
* std::cout << "<>< This is a fish. His favorite number is " << a << ".\n";
* a++;
* return;
* }
*/


int main() {
    //std::function<void(void)> time_me_fn = time_me;
    //calorimeter(time_me_fn);
    //int num_for_fish{ 4 };
    std::function<void(int)> fish_fn = fish;
    //calorimeter(fish_fn, num_for_fish);
    calorimeter(fish_fn, 3);
    return 0;
}