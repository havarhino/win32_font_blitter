#pragma once
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::steady_clock::time_point TimePoint;

class FrameCounter {

public:
	FrameCounter();
	void nextFrame();
	int getFrameRate();

private:

	double frameCounter;
	TimePoint hiRes0;
	TimePoint hiRes1;
	double cumulativeTime;
	int frames;
	int toggle;
	double frameRate;
	wchar_t dbgStr[512] = L"";

};