#pragma once

#include <functional>
#include <iostream>
#include <chrono>

// runs the function <func>, returns its output and prints the elapsed time to the command window
// args: arguments for <func>
template<class ReturnT, class F, class... Args> 
ReturnT executeAndShowElapsedTime(const F& func, Args... args) {
	auto begin = std::chrono::high_resolution_clock::now();
	ReturnT res = func(args...);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
	std::cout << duration << std::endl;
	return res;
}

// runs the function <func> and prints the elapsed time to the command window
template<class F>
void executeAndShowElapsedTime(const F& func) {
	auto begin = std::chrono::high_resolution_clock::now();
	func();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
	std::cout << duration << std::endl;
}