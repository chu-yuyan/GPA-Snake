#include "Bgm.h"
#include <windows.h>
#include <mmsystem.h>
#include <string>

#pragma comment(lib, "winmm.lib")

static std::wstring ToAbsPath(const std::wstring& path)
{
	wchar_t buf[MAX_PATH]{};
	DWORD n = GetFullPathNameW(path.c_str(), MAX_PATH, buf, nullptr);
	if (n == 0 || n >= MAX_PATH) return path;
	return std::wstring(buf);
}

bool Bgm::playLoop(const std::wstring& filePath)
{
	stop();

	const std::wstring abs = ToAbsPath(filePath);

	const BOOL ok = PlaySoundW(abs.c_str(), nullptr,
		SND_FILENAME | SND_ASYNC | SND_LOOP | SND_NODEFAULT);

	opened_ = (ok != FALSE);

	if (!opened_)
	{
		MessageBoxW(nullptr,
			(L"播放BGM失败：\n" + abs +
				L"\n\n提示：请确认 WAV 为 PCM 16-bit / 44.1kHz / Stereo，并且文件未被占用。").c_str(),
			L"BGM", MB_OK);
		return false;
	}

	return true;
}

void Bgm::stop()
{
	PlaySoundW(nullptr, nullptr, 0);
	opened_ = false;
}