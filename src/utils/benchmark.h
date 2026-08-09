#pragma once

#include <functional>
#include <iostream>
#include <chrono>
#include <QDebug>

// runs the function <func>, returns its output and prints the elapsed time to the command window
// args: arguments for <func>
template<class ReturnT, class F, class... Args>
ReturnT executeAndShowElapsedTime(const F& func, const std::string_view text, Args... args) {
	auto begin = std::chrono::high_resolution_clock::now();
	ReturnT res = func(args...);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
	qInfo() << text << ":" << duration.count() << "ms";
	return res;
}

// runs the function <func> and prints the elapsed time to the command window
template<class F>
void executeAndShowElapsedTime(const F& func, const std::string_view text) {
    auto begin = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    qInfo() << text << ":" << duration.count() << "ms";
}
