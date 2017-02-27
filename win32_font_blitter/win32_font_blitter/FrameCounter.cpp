#include "stdafx.h"
#include "FrameCounter.h"

FrameCounter::FrameCounter() {
	frameCounter = 0.0;
	hiRes0 = Clock::now();
	hiRes1 = Clock::now();
	cumulativeTime = 0.0;
	frames = 0;
	toggle = 0;
	frameRate = 1.0;
}

void FrameCounter::nextFrame() {
	frameCounter += 1.0;

	std::chrono::duration<double> diff;
	if (toggle == 0) {
		hiRes0 = Clock::now();
		diff = hiRes0 - hiRes1;
	}
	else {
		hiRes1 = Clock::now();
		diff = hiRes1 - hiRes0;
	}
	toggle = 1 - toggle;

	double delta = diff.count();
	cumulativeTime += delta;

	if (cumulativeTime > 0.1) {
		frameRate = frameCounter / cumulativeTime;
		cumulativeTime = 0.0;
		frameCounter = 0.0;
		swprintf_s(dbgStr, L"FrameCounter: frameRate=%f\n", frameRate);
		OutputDebugStringW(dbgStr);
	}
}

int FrameCounter::getFrameRate() {
	return (int)frameRate;
}
