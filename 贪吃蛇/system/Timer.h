#pragma once

class Timer
{
public:
	void reset()
	{
		ticks_ = 0;
		seconds_ = 0;
	}

	// 每帧调用一次
	void step()
	{
		++ticks_;
		// 用 tickMs 累积出秒（兼容 tickMs=100/50 等任意值）
		const int ms = ticks_ * tickMs_;
		seconds_ = ms / 1000;
	}

	int ticks() const { return ticks_; }
	int seconds() const { return seconds_; } //  真实秒
	int tickMs() const { return tickMs_; }
	void setTickMs(int ms) { tickMs_ = ms; }

private:
	int ticks_ = 0;
	int seconds_ = 0;
	int tickMs_ = 100;
};