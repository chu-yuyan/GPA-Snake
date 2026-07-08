#pragma once

#include "../core/Food.h"
#include "../core/EventManager.h"
#include "../system/RecordManager.h"

#include <array>
#include <string>
#include <vector>

class ScoreSystem;
class Timer;

class SettlementManager
{
public:
	void reset();

	// 每吃到一个食物时调用
	void onEatFood(FoodType t);

	// 每次挑战事件成功时调用（CET4/SITP/PROJECT/CONTEST）
	void onChallengeCompleted(GameEvent e);

	// 生成结算报告（文本/结构化结果）
	void generateReport(const ScoreSystem& score, const Timer& timer, const EventManager& ev);

	// 绘制结算界面（阻塞式外部循环里每帧调用即可）
	void draw(int screenW, int screenH) const;

	// 文案拼装（便于单元测试/后续复用）
	std::wstring getFavoriteCourseText() const;
	std::wstring getEventSummary() const;
	std::wstring getFinalComment() const;

	//  新增：导出/导入记录
	GameRecord toRecord() const;
	void loadFromRecord(const GameRecord& r);

private:
	// 统计
	std::array<int, (size_t)FoodType::COUNT> eatCount_{};
	bool completedCET4_ = false;
	bool completedSITP_ = false;
	bool completedPROJECT_ = false;
	bool completedCONTEST_ = false;

	// 报告缓存（generateReport 生成）
	bool generated_ = false;
	bool win_ = false;

	float finalGpa_ = 0.0f;
	double finalHappiness_ = 0.0;
	int overworkDeaths_ = 0;

	FoodType favoriteCourse_ = FoodType::CALCULUS;
	int favoriteCourseTimes_ = 0;

	int iceTeaTimes_ = 0;
	int virusTimes_ = 0;

	std::vector<std::wstring> completedEventsText_;
	std::wstring titleLine_;
	std::wstring bodyLine_;

	std::wstring favoriteRoast_;
	std::wstring finalComment_;

private:
	static bool isCourseFood(FoodType t);
	static const wchar_t* eventDisplayName(GameEvent e);
	static std::wstring fixed2(float v);
	static std::wstring courseRoast(FoodType t);
};