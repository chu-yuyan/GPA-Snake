#pragma once

#include <string>
#include <vector>

struct GameRecord
{
	// 基础指标（列表展示）
	float gpa = 0.0f;
	double happiness = 0.0;

	// 详情需要的统计（用于还原结算界面）
	int overworkDeaths = 0;
	int iceTeaTimes = 0;
	int virusTimes = 0;

	// 事件完成情况
	bool cet4 = false;
	bool sitp = false;
	bool project = false;
	bool contest = false;

	// 最喜爱课程
	int favoriteCourse = 0;     // (int)FoodType
	int favoriteCourseTimes = 0;

	// 时间戳（可选，用于排序/显示）
	long long unixTime = 0;
};

class RecordManager
{
public:
	bool append(const GameRecord& r);
	std::vector<GameRecord> loadAll() const;

	//  删除：按loadAll返回顺序的索引删除
	bool removeAt(size_t index);

	//  删除：按“最新在上”(菜单展示)的索引删除（0=最新）
	bool removeAtFromNewestIndex(size_t newestIndex);

	//  清空所有记录
	bool clearAll();

	static std::wstring formatLine(const GameRecord& r);

private:
	std::wstring filePath() const;

	static bool parseLine(const std::wstring& line, GameRecord& out);
};