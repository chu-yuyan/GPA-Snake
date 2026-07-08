#include "RecordManager.h"

#include <windows.h>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

static std::wstring GetExeDir()
{
	wchar_t buf[MAX_PATH]{};
	GetModuleFileNameW(nullptr, buf, MAX_PATH);
	std::wstring p = buf;
	const auto pos = p.find_last_of(L"\\/");
	if (pos == std::wstring::npos) return L".";
	return p.substr(0, pos);
}

static void DebugLastError(const wchar_t* tag, const std::wstring& path)
{
	const DWORD e = GetLastError();
	wchar_t msgBuf[512]{};
	wsprintfW(msgBuf, L"[RecordManager] %s failed. path=%s, GetLastError=%lu\n", tag, path.c_str(), e);
	OutputDebugStringW(msgBuf);
}

// ===== UTF-8 <-> UTF-16 helpers =====
static std::string WidenToUtf8(const std::wstring& w)
{
	if (w.empty()) return std::string();
	const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
	std::string s;
	s.resize((size_t)n);
	WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), &s[0], n, nullptr, nullptr);
	return s;
}

static std::wstring Utf8ToWiden(const std::string& s)
{
	if (s.empty()) return std::wstring();
	const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
	std::wstring w;
	w.resize((size_t)n);
	MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
	return w;
}

static bool ReadAllTextW(const std::wstring& path, std::wstring& out)
{
	out.clear();

	// 使用 iostream 二进制读取，避免 ReadFile 的 LPVOID/const 兼容问题
	std::ifstream ifs(path, std::ios::binary);
	if (!ifs) return false;

	std::string bytes((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	if (bytes.empty())
		return true;

	// 去掉 UTF-8 BOM（可选）
	if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF)
		bytes.erase(0, 3);

	out = Utf8ToWiden(bytes);
	return true;
}

static bool WriteAllTextW(const std::wstring& path, const std::wstring& text, bool append)
{
	const DWORD disposition = append ? OPEN_ALWAYS : CREATE_ALWAYS;

	HANDLE h = CreateFileW(
		path.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		disposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (h == INVALID_HANDLE_VALUE)
	{
		DebugLastError(L"CreateFileW(write)", path);
		return false;
	}

	if (append)
	{
		LARGE_INTEGER li{};
		li.QuadPart = 0;
		if (!SetFilePointerEx(h, li, nullptr, FILE_END))
			DebugLastError(L"SetFilePointerEx", path);
	}

	// 写入 UTF-8 文本
	const std::string bytes = WidenToUtf8(text);

	DWORD written = 0;
	const DWORD total = (DWORD)bytes.size();
	const BOOL ok = WriteFile(h, bytes.data(), total, &written, nullptr);
	if (!ok)
		DebugLastError(L"WriteFile", path);

	CloseHandle(h);
	return ok && written == total;
}

std::wstring RecordManager::filePath() const
{
	return GetExeDir() + L"\\records.tsv";
}

std::wstring RecordManager::formatLine(const GameRecord& r)
{
	std::wstringstream ss;
	ss << L"v1\t"
		<< r.gpa << L"\t"
		<< r.happiness << L"\t"
		<< r.overworkDeaths << L"\t"
		<< r.iceTeaTimes << L"\t"
		<< r.virusTimes << L"\t"
		<< (r.cet4 ? 1 : 0) << L"\t"
		<< (r.sitp ? 1 : 0) << L"\t"
		<< (r.project ? 1 : 0) << L"\t"
		<< (r.contest ? 1 : 0) << L"\t"
		<< r.favoriteCourse << L"\t"
		<< r.favoriteCourseTimes << L"\t"
		<< r.unixTime;
	return ss.str();
}

static bool NextField(std::wstringstream& ss, std::wstring& out)
{
	return (bool)std::getline(ss, out, L'\t');
}

bool RecordManager::parseLine(const std::wstring& line, GameRecord& out)
{
	if (line.empty()) return false;

	std::wstringstream ss(line);
	std::wstring v;
	if (!NextField(ss, v)) return false;
	if (v != L"v1") return false;

	std::wstring f;

	if (!NextField(ss, f)) return false; out.gpa = (float)_wtof(f.c_str());
	if (!NextField(ss, f)) return false; out.happiness = _wtof(f.c_str());

	if (!NextField(ss, f)) return false; out.overworkDeaths = _wtoi(f.c_str());
	if (!NextField(ss, f)) return false; out.iceTeaTimes = _wtoi(f.c_str());
	if (!NextField(ss, f)) return false; out.virusTimes = _wtoi(f.c_str());

	if (!NextField(ss, f)) return false; out.cet4 = (_wtoi(f.c_str()) != 0);
	if (!NextField(ss, f)) return false; out.sitp = (_wtoi(f.c_str()) != 0);
	if (!NextField(ss, f)) return false; out.project = (_wtoi(f.c_str()) != 0);
	if (!NextField(ss, f)) return false; out.contest = (_wtoi(f.c_str()) != 0);

	if (!NextField(ss, f)) return false; out.favoriteCourse = _wtoi(f.c_str());
	if (!NextField(ss, f)) return false; out.favoriteCourseTimes = _wtoi(f.c_str());

	if (!NextField(ss, f)) return false; out.unixTime = _wtoi64(f.c_str());

	return true;
}

bool RecordManager::append(const GameRecord& r)
{
	const std::wstring line = formatLine(r) + L"\n";
	const bool ok = WriteAllTextW(filePath(), line, true);
	if (!ok)
		DebugLastError(L"append", filePath());
	return ok;
}

std::vector<GameRecord> RecordManager::loadAll() const
{
	std::vector<GameRecord> rs;

	std::wstring content;
	if (!ReadAllTextW(filePath(), content))
		return rs;

	std::wstringstream ss(content);
	std::wstring line;
	while (std::getline(ss, line))
	{
		GameRecord r{};
		if (parseLine(line, r))
			rs.push_back(r);
	}
	return rs;
}

bool RecordManager::removeAt(size_t index)
{
	auto rs = loadAll();
	if (index >= rs.size()) return false;

	rs.erase(rs.begin() + (ptrdiff_t)index);

	std::wstring content;
	for (const auto& r : rs)
		content += formatLine(r) + L"\n";

	return WriteAllTextW(filePath(), content, false);
}

//  与菜单“最新在上”展示对应：
// newestIndex=0 表示删除最新一条（文件末尾那条）
bool RecordManager::removeAtFromNewestIndex(size_t newestIndex)
{
	auto rs = loadAll();
	if (newestIndex >= rs.size()) return false;

	const size_t idx = rs.size() - 1 - newestIndex;
	return removeAt(idx);
}

bool RecordManager::clearAll()
{
	return WriteAllTextW(filePath(), L"", false);
}