#pragma once

#include <string>

class Bgm
{
public:
	// 建议传入 mp3/wav 路径（相对/绝对都可）
	bool playLoop(const std::wstring& filePath);
	void stop();

private:
	bool opened_ = false;
};