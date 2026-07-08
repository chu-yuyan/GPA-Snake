#include "SettlementManager.h"
#include "Renderer.h"

#include "../core/ScoreSystem.h"
#include "../system/Timer.h"

#include <graphics.h>
#include <algorithm>
#include <random>
#include <ctime>


static const wchar_t* SettlementBgPathLocal()
{
	return L"./assets/back/settlement_bg.png";
}

void SettlementManager::reset()
{
	eatCount_.fill(0);
	completedCET4_ = completedSITP_ = completedPROJECT_ = completedCONTEST_ = false;

	generated_ = false;
	win_ = false;

	finalGpa_ = 0.0f;
	finalHappiness_ = 0.0;
	overworkDeaths_ = 0;

	favoriteCourse_ = FoodType::CALCULUS;
	favoriteCourseTimes_ = 0;

	iceTeaTimes_ = 0;
	virusTimes_ = 0;

	completedEventsText_.clear();
	titleLine_.clear();
	bodyLine_.clear();
	favoriteRoast_.clear();
	finalComment_.clear();
}

bool SettlementManager::isCourseFood(FoodType t)
{
	return ScoreSystem::isCourse(t);
}

void SettlementManager::onEatFood(FoodType t)
{
	++eatCount_[(size_t)t];

	if (t == FoodType::ICE_TEA) ++iceTeaTimes_;
	if (t == FoodType::VIRUS) ++virusTimes_;
}

const wchar_t* SettlementManager::eventDisplayName(GameEvent e)
{
	switch (e)
	{
	case GameEvent::CET4: return L"六级";
	case GameEvent::SITP: return L"SITP";
	case GameEvent::PROJECT: return L"大作业";
	case GameEvent::CONTEST: return L"学科竞赛";
	default: return L"";
	}
}

void SettlementManager::onChallengeCompleted(GameEvent e)
{
	switch (e)
	{
	case GameEvent::CET4: completedCET4_ = true; break;
	case GameEvent::SITP: completedSITP_ = true; break;
	case GameEvent::PROJECT: completedPROJECT_ = true; break;
	case GameEvent::CONTEST: completedCONTEST_ = true; break;
	default: break;
	}
}

std::wstring SettlementManager::fixed2(float v)
{
	std::wstring s = std::to_wstring(v);
	auto dot = s.find(L'.');
	if (dot == std::wstring::npos) return s;
	if (dot + 3 < s.size()) s.resize(dot + 3);
	return s;
}

std::wstring SettlementManager::courseRoast(FoodType t)
{
	switch (t)
	{
	case FoodType::CALCULUS: return L"真正的勇士，敢于直面二重积分。";
	case FoodType::LINEAR_ALGEBRA: return L"矩阵没有击垮你，但考试差点做到了。";
	case FoodType::PHYSICS: return L"牛顿看了都想让你补实验报告。";
	case FoodType::CPP: return L"你已经逐渐学会和 bug 共存。";
	case FoodType::ENGLISH: return L"至少这次你没有把CET当成阅读理解模拟器。";
	case FoodType::AI: return L"你开始认真思考未来会不会被AI替代。";
	case FoodType::ELECTIVE: return L"令人震惊的是，你真的去上选修课了。";
	default: return L"这门课你学得挺认真（至少吃得很勤）。";
	}
}

void SettlementManager::generateReport(const ScoreSystem& score, const Timer& /*timer*/, const EventManager& ev)
{
	generated_ = true;

	finalGpa_ = score.gpa();
	finalHappiness_ = score.happinessPreview();
	overworkDeaths_ = score.overworkDeaths();

	// 1) 胜负判定
	win_ = (finalGpa_ >= 3.5f);

	if (win_)
	{
		titleLine_ = L"恭喜你成功活过了这个学期！";
		bodyLine_ = L"你的绩点达到了 3.5 以上。\n虽然过程不一定体面，但至少成绩单还算能看。";
	}
	else
	{
		titleLine_ = L"很遗憾，你没能守住自己的绩点。";
		bodyLine_ = L"这个学期的你在摆烂与挣扎之间反复横跳。\n希望下学期不要再对着 DDL 双手合十了。";
	}

	// 2) 最喜爱课程（只统计课程类食物；选修课仅在启用后才算“有效偏爱”）
	favoriteCourseTimes_ = -1;
	for (int i = 0; i < (int)FoodType::COUNT; ++i)
	{
		FoodType t = (FoodType)i;
		if (!isCourseFood(t)) continue;

		if (t == FoodType::ELECTIVE && !ev.electiveEnabled())
			continue;

		const int c = eatCount_[(size_t)t];
		if (c > favoriteCourseTimes_)
		{
			favoriteCourseTimes_ = c;
			favoriteCourse_ = t;
		}
	}
	if (favoriteCourseTimes_ < 0)
	{
		favoriteCourseTimes_ = 0;
		favoriteCourse_ = FoodType::CALCULUS;
	}

	favoriteRoast_ = courseRoast(favoriteCourse_);

	// 3) 事件总结
	completedEventsText_.clear();
	if (completedCET4_) completedEventsText_.push_back(L"六级");
	if (completedSITP_) completedEventsText_.push_back(L"SITP");
	if (completedPROJECT_) completedEventsText_.push_back(L"大作业");
	if (completedCONTEST_) completedEventsText_.push_back(L"学科竞赛");

	// 4) 随机结算评语（基于 GPA/幸福感/卷死次数）
	//   简单分段 + 随机一句
	enum class CommentBucket { HighGpaHighHappy, HighGpaLowHappy, LowGpaHighHappy, AllLow, OverworkMany };

	CommentBucket bucket = CommentBucket::AllLow;

	const bool highGpa = finalGpa_ >= 3.5f;
	const bool highHappy = finalHappiness_ >= 70.0; // 阈值可调
	const bool overworkMany = overworkDeaths_ >= 3;

	if (overworkMany)
		bucket = CommentBucket::OverworkMany;
	else if (highGpa && highHappy)
		bucket = CommentBucket::HighGpaHighHappy;
	else if (highGpa && !highHappy)
		bucket = CommentBucket::HighGpaLowHappy;
	else if (!highGpa && highHappy)
		bucket = CommentBucket::LowGpaHighHappy;
	else
		bucket = CommentBucket::AllLow;

	std::vector<std::wstring> pool;
	switch (bucket)
	{
	case CommentBucket::HighGpaHighHappy:
		pool = { L"你似乎找到了大学生活的正确打开方式。", L"学习与生活两不误——这种人真的存在。"};
		break;
	case CommentBucket::HighGpaLowHappy:
		pool = { L"你赢了绩点，但不一定赢了人生。", L"成绩单很亮眼，但你的精神状态可能需要缓冲区。"};
		break;
	case CommentBucket::LowGpaHighHappy:
		pool = { L"虽然成绩一般，但至少你活得像个人。", L"你没卷赢，但你没输给生活。"};
		break;
	case CommentBucket::OverworkMany:
		pool = { L"你对自己的压榨甚至让系统都感到害怕。", L"建议下学期把“善待自己”也写进计划表。"};
		break;
	default:
		pool = { L"建议下学期少看手机，多看教学平台。", L"别急，下学期还有更多课会来考验你的心态。"};
		break;
	}

	static std::mt19937 rng{ std::random_device{}() };
	std::uniform_int_distribution<int> d(0, (int)pool.size() - 1);
	finalComment_ = pool[d(rng)];

	// ? 关键：写入游戏记录（只写一次）
	static long long lastWrittenTime = 0;
	GameRecord rec = toRecord();

	// 同一秒内重复调用 generateReport 时避免重复写
	if (rec.unixTime != 0 && rec.unixTime != lastWrittenTime)
	{
		RecordManager rm;
		if (rm.append(rec))
		{
			lastWrittenTime = rec.unixTime;
		}
	}
}

std::wstring SettlementManager::getFavoriteCourseText() const
{
	std::wstring s = L"本学期你最偏爱的课程是：";
	s += ScoreSystem::courseDisplayName(favoriteCourse_);
	return s;
}

std::wstring SettlementManager::getEventSummary() const
{
	if (completedEventsText_.empty())
		return L"本学期你专心刷绩点，\n是一个纯粹的 GPA 战士。";

	std::wstring joined;
	for (size_t i = 0; i < completedEventsText_.size(); ++i)
	{
		if (i) joined += L"、";
		joined += completedEventsText_[i];
	}

	return L"本学期你成功完成了：\n" + joined +
		L"\n\n虽然过程痛苦，但你的能力值似乎确实上涨了。";
}

std::wstring SettlementManager::getFinalComment() const
{
	return finalComment_;
}

GameRecord SettlementManager::toRecord() const
{
	GameRecord r{};
	r.gpa = finalGpa_;
	r.happiness = finalHappiness_;
	r.overworkDeaths = overworkDeaths_;
	r.iceTeaTimes = iceTeaTimes_;
	r.virusTimes = virusTimes_;

	r.cet4 = completedCET4_;
	r.sitp = completedSITP_;
	r.project = completedPROJECT_;
	r.contest = completedCONTEST_;

	r.favoriteCourse = (int)favoriteCourse_;
	r.favoriteCourseTimes = favoriteCourseTimes_;

	r.unixTime = (long long)time(nullptr);
	return r;
}
void SettlementManager::loadFromRecord(const GameRecord& r)
{
	// 用记录直接还原 draw 所需字段
	generated_ = true;

	finalGpa_ = r.gpa;
	finalHappiness_ = r.happiness;
	overworkDeaths_ = r.overworkDeaths;

	iceTeaTimes_ = r.iceTeaTimes;
	virusTimes_ = r.virusTimes;

	completedCET4_ = r.cet4;
	completedSITP_ = r.sitp;
	completedPROJECT_ = r.project;
	completedCONTEST_ = r.contest;

	favoriteCourse_ = (FoodType)r.favoriteCourse;
	favoriteCourseTimes_ = r.favoriteCourseTimes;

	// 重新生成文案（保持一致）
	win_ = (finalGpa_ >= 3.5f);
	if (win_) {
		titleLine_ = L"恭喜你成功活过了这个学期！";
		bodyLine_ = L"你的绩点达到了 3.5 以上。\n虽然过程不一定体面，但至少成绩单还算能看。";
	}
	else {
		titleLine_ = L"很遗憾，你没能守住自己的绩点。";
		bodyLine_ = L"这个学期的你在摆烂与挣扎之间反复横跳。\n希望下学期不要再对着 DDL 双手合十了。";
	}

	favoriteRoast_ = courseRoast(favoriteCourse_);

	completedEventsText_.clear();
	if (completedCET4_) completedEventsText_.push_back(L"六级");
	if (completedSITP_) completedEventsText_.push_back(L"SITP");
	if (completedPROJECT_) completedEventsText_.push_back(L"大作业");
	if (completedCONTEST_) completedEventsText_.push_back(L"学科竞赛");

	// 评语：不强求完全复现随机，给一个确定性版本
	if (overworkDeaths_ >= 3)
		finalComment_ = L"你对自己的压榨甚至让系统都感到害怕。";
	else if (finalGpa_ >= 3.5f && finalHappiness_ >= 70.0)
		finalComment_ = L"你似乎找到了大学生活的正确打开方式。";
	else if (finalGpa_ >= 3.5f)
		finalComment_ = L"你赢了绩点，但不一定赢了人生。";
	else if (finalHappiness_ >= 70.0)
		finalComment_ = L"虽然成绩一般，但至少你活得像个人。";
	else
		finalComment_ = L"建议下学期少看手机，多看教学平台。";
}
void SettlementManager::draw(int screenW, int screenH) const
{
	if (!generated_) return;

	// =========================================================
	// 背景
	// =========================================================
	Renderer::drawImageStretch(
		SettlementBgPathLocal(),
		0, 0,
		screenW, screenH
	);


	// =========================================================
	// 标题区
	// =========================================================
	Renderer::drawTextInRect(
		0, 18, screenW, 50,
		L"学期结算",
		44, RGB(255, 215, 0), true,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER,
		L"微软雅黑"
	);

	std::wstring subtitle = win_ ? L"你成功活过了这个学期" : L"这个学期似乎有点艰难";
	Renderer::drawTextInRect(
		0, 64, screenW, 40,
		subtitle,
		20, RGB(18, 22, 32), false,
		DT_SINGLELINE | DT_CENTER | DT_TOP,
		L"微软雅黑"
	);

	// =========================================================
	// 整体布局参数
	// =========================================================

	const int outerMargin = 30;

	const int topY = 110;

	const int columnGap = 26;

	const int cardGap = 18;

	const int leftW = 590;
	const int rightW = 590;

	const int leftX = outerMargin;
	const int rightX = leftX + leftW + columnGap;

	// =========================================================
	// 卡片绘制函数（更像真正游戏UI）
	// =========================================================
	auto drawCard =
		[&](int x, int y,
			int w, int h,
			const std::wstring& title,
			const std::wstring& content)
		{
			// 外层阴影
			setfillcolor(RGB(18, 22, 32));
			solidroundrect(
				x + 4, y + 5,
				x + w + 4, y + h + 5,
				16, 16
			);

			// 主卡片
			setfillcolor(RGB(28, 34, 48));
			solidroundrect(
				x, y,
				x + w, y + h,
				16, 16
			);

			// 顶部高光条
			setfillcolor(RGB(128, 42, 42));
			solidroundrect(
				x,
				y,
				x + w,
				y + 6,
				16, 16
			);

			// 标题
			Renderer::drawTextStyled(
				x + 20,
				y + 16,
				title,
				24,
				RGB(255, 235, 180),
				true,
				L"微软雅黑"
			);

			// 分割线
			setlinecolor(RGB(70, 78, 95));
			line(x + 18, y + 52, x + w - 18, y + 52);

			// 正文
			Renderer::drawTextInRect(
				x + 22,
				y + 64,
				w - 44,
				h - 82,
				content,
				20,
				RGB(235, 235, 235),
				false,
				DT_WORDBREAK | DT_LEFT | DT_TOP,
				L"微软雅黑"
			);
		};

	// =========================================================
	// 左列
	// =========================================================

	int leftY = topY;

	// ---------- 胜负 ----------
	std::wstring winTitle =
		win_ ? L"游戏胜利" : L"游戏失败";

	std::wstring winContent =
		win_
		?
		L"恭喜你成功活过了这个学期！\n"
		L"你的绩点达到了 3.5 以上。\n"
		L"虽然过程不一定体面，但至少成绩单还算能看。"
		:
		L"很遗憾，你没能守住自己的绩点。\n"
		L"这个学期的你在摆烂与挣扎之间反复横跳。\n"
		L"希望下学期不要再对着 DDL 双手合十了。";

	drawCard(
		leftX,
		leftY,
		leftW,
		150,
		winTitle,
		winContent
	);

	leftY += 150 + cardGap;

	// ---------- 学期事件 ----------
	std::wstring eventSummary;

	if (completedEventsText_.empty())
	{
		eventSummary =
			L"本学期你专心刷绩点，\n"
			L"是一个纯粹的 GPA 战士。";
	}
	else
	{
		eventSummary =
			L"本学期你成功完成了：\n";

		for (size_t i = 0; i < completedEventsText_.size(); ++i)
		{
			if (i) eventSummary += L"、";
			eventSummary += completedEventsText_[i];
		}

		eventSummary +=
			L"\n\n虽然过程痛苦，"
			L"但你的能力值似乎确实上涨了。";
	}

	drawCard(
		leftX,
		leftY,
		leftW,
		185,
		L"学期事件总结",
		eventSummary
	);

	leftY += 185 + cardGap;

	// ---------- 冰红茶 ----------
	std::wstring iceTeaText;

	if (iceTeaTimes_ > 0)
	{
		iceTeaText =
			L"在冰红茶的神秘加持下，\n"
			L"你一共赶完了 ";

		iceTeaText +=
			std::to_wstring(iceTeaTimes_);

		iceTeaText +=
			L" 次DDL。\n\n";
	}
	else
	{
		iceTeaText =
			L"本学期你没有喝过冰红茶。\n"
			L"全靠意志力硬扛。";
	}

	drawCard(
		leftX,
		leftY,
		leftW,
		140,
		L"冰红茶统计",
		iceTeaText
	);

	// =========================================================
	// 右列
	// =========================================================

	int rightY = topY;

	// ---------- 最喜爱课程 ----------
	std::wstring favoriteText;

	favoriteText +=
		L"本学期你最偏爱的课程是：";

	favoriteText +=
		ScoreSystem::courseDisplayName(favoriteCourse_);

	favoriteText +=
		L"\n\n你一共学习了 ";

	favoriteText +=
		std::to_wstring(favoriteCourseTimes_);

	favoriteText +=
		L" 次相关课程内容。\n最终 GPA：";

	favoriteText +=
		fixed2(finalGpa_);

	favoriteText +=
		L"\n\n";

	favoriteText +=
		courseRoast(favoriteCourse_);

	drawCard(
		rightX,
		rightY,
		rightW,
		210,
		L"最喜爱课程",
		favoriteText
	);

	rightY += 210 + cardGap;

	// ---------- 健康状态 ----------
	std::wstring virusText;

	if (virusTimes_ > 0)
	{
		virusText =
			L"你本学期不幸感冒了 ";

		virusText +=
			std::to_wstring(virusTimes_);

		virusText +=
			L" 次。\n"
			L"建议不要边熬夜边说“问题不大”。";
	}
	else
	{
		virusText =
			L"你本学期身体状态良好。\n"
			L"你的作息甚至让室友感到陌生。";
	}

	drawCard(
		rightX,
		rightY,
		rightW,
		135,
		L"健康状态",
		virusText
	);

	rightY += 135 + cardGap;

	// ---------- 卷死记录 ----------
	std::wstring overworkText;

	if (overworkDeaths_ > 0)
	{
		overworkText =
			L"你曾经把自己卷死了 ";

		overworkText +=
			std::to_wstring(overworkDeaths_);

		overworkText +=
			L" 次。\n"
			L"你的蛇身化作了新的课程食物。";
	}
	else
	{
		overworkText =
			L"整个学期里，你居然一次都没把自己卷死。\n"
			L"这在工科里已经算一种天赋。";
	}

	drawCard(
		rightX,
		rightY,
		rightW,
		145,
		L"卷死记录",
		overworkText
	);

	// =========================================================
	// 底部结语栏
	// =========================================================

	const int bottomBarY = 650;

	setfillcolor(RGB(22, 26, 38));
	solidroundrect(
		outerMargin,
		bottomBarY,
		screenW - outerMargin,
		bottomBarY +40,
		14, 14
	);

	Renderer::drawTextStyled(
		outerMargin + 20,
		bottomBarY + 14,
		L"学期结语",
		20,
		RGB(255, 235, 180),
		true,
		L"微软雅黑"
	);

	Renderer::drawTextStyled(
		outerMargin + 140,
		bottomBarY + 14,
		getFinalComment(),
		18,
		RGB(230, 230, 230),
		false,
		L"微软雅黑"
	);

	

}