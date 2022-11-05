#pragma once
#include <iostream>

const void display(const bool& baf, const bool& bgm, const int& fps) noexcept {
	system("cls");
	const std::string status[2]{ "DISABLED","ENABLED" };
	std::cout << "Auto-fire is: " << status[baf]
		<< "\nGod-mode is: " << status[bgm]
		<< "\nCurrent max_fps: " << fps;
}