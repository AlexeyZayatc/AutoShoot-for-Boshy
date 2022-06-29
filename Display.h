#pragma once
#include <iostream>

void display(const bool& baf, const bool& bgm, int fps) {
	system("cls");
	std::string status[2]{ "DISABLED","ENABLED" };
	std::cout << "Auto-fire is: " << status[baf]
		<< "\nGod-mode is: " << status[bgm]
		<< "\nCurrent max_fps: " << fps;
}