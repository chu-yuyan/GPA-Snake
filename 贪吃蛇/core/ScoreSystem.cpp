#include "ScoreSystem.h"
#include <algorithm>

static bool IsBaseCourse(FoodType t)
{
	return ScoreSystem::isCourse(t);
}

void ScoreSystem::reset()
{
	score_ = 0;
	ability_ = 0;
	deathCount_ = 0;
	overworkDeaths_ = 0;
	bestScore_ = 0;

	electiveEnabled_ = false;

	grade_.fill(0.0);

	//  默认初始2.0
	for (int i = 0; i < (int)FoodType::COUNT; ++i)
	{
		FoodType t = (FoodType)i;
		if (IsBaseCourse(t))
			grade_[(size_t)i] = 2.0;
	}

	//  ELECTIVE 初始也按2.0存着，但不启用就不计入GPA
	grade_[(size_t)FoodType::ELECTIVE] = 2.0;

	recalcGpa();
}

bool ScoreSystem::isCourse(FoodType t)
{
	switch (t)
	{
	case FoodType::CALCULUS:
	case FoodType::LINEAR_ALGEBRA:
	case FoodType::PHYSICS:
	case FoodType::ENGLISH:
	case FoodType::PHYSICS_LAB:
	case FoodType::PE:
	case FoodType::CPP:
	case FoodType::MILITARY:
	case FoodType::AI:
	case FoodType::RESEARCH:
	case FoodType::POLITICS:
	case FoodType::ELECTIVE:
		return true;
	default:
		return false;
	}
}

bool ScoreSystem::isCourseEnabled(FoodType t) const
{
	if (!isCourse(t)) return false;
	if (t == FoodType::ELECTIVE) return electiveEnabled_;
	return true;
}

void ScoreSystem::setElectiveEnabled(bool enabled)
{
	if (electiveEnabled_ == enabled) return;
	electiveEnabled_ = enabled;
	recalcGpa();
}

double ScoreSystem::courseCredit(FoodType course)
{
	switch (course)
	{
	case FoodType::CALCULUS: return 6.0;
	case FoodType::LINEAR_ALGEBRA: return 5.0;
	case FoodType::PHYSICS: return 3.0;
	case FoodType::ENGLISH: return 2.0;
	case FoodType::PHYSICS_LAB: return 1.0;
	case FoodType::PE: return 1.0;
	case FoodType::CPP: return 2.0;
	case FoodType::MILITARY: return 2.0;
	case FoodType::AI: return 2.0;
	case FoodType::RESEARCH: return 2.0;
	case FoodType::POLITICS: return 0.5;
	case FoodType::ELECTIVE: return 2.0; //  选修课学分：按你描述给2
	default: return 0.0;
	}
}

const wchar_t* ScoreSystem::courseDisplayName(FoodType course)
{
	switch (course)
	{
	case FoodType::CALCULUS: return L"数分";
	case FoodType::LINEAR_ALGEBRA: return L"线代";
	case FoodType::PHYSICS: return L"物理";
	case FoodType::ENGLISH: return L"英语";
	case FoodType::PHYSICS_LAB: return L"物理实验";
	case FoodType::PE: return L"体育";
	case FoodType::CPP: return L"高程";
	case FoodType::MILITARY: return L"军事理论";
	case FoodType::AI: return L"人工智能";
	case FoodType::RESEARCH: return L"科研前沿";
	case FoodType::POLITICS: return L"形策";
	case FoodType::ELECTIVE: return L"选修课";
	default: return L"";
	}
}

double ScoreSystem::courseGrade(FoodType course) const
{
	if (!isCourse(course)) return 0.0;
	return grade_[(size_t)course];
}

void ScoreSystem::recalcGpa()
{
	double sum = 0.0;
	double credits = 0.0;

	for (int i = 0; i < (int)FoodType::COUNT; ++i)
	{
		FoodType t = (FoodType)i;
		if (!isCourseEnabled(t)) continue;

		double c = courseCredit(t);
		sum += grade_[(size_t)i] * c;
		credits += c;
	}

	totalCredits_ = credits;
	gpa_ = (credits > 0.0) ? (float)(sum / credits) : 0.0f;
}

void ScoreSystem::onEat(FoodType type)
{
	// 你原来的score逻辑可以保留/改造；这里先不强绑定幸福感
	score_ += 10;

	//  只有“启用的课程”才加分（选修课未启用时吃不到它，因为不会掉落）
	if (isCourseEnabled(type))
	{
		double& g = grade_[(size_t)type];
		g = (std::min)(5.0, g + 0.5);
		recalcGpa();
	}
}

double ScoreSystem::happinessPreview() const
{
	double base = (double)gpa_ * 20.0 + (double)ability_;
	double factor = (10.0 - (double)overworkDeaths_) / 10.0;
	if (factor < 0.0) factor = 0.0;
	return base * factor;
}

void ScoreSystem::onDeath(bool overwork)
{
	bestScore_ = (std::max)(bestScore_, score_);

	//  deathCount_ 只统计“撞墙死亡”
	if (!overwork)
		++deathCount_;

	//  卷死次数仍然累计
	if (overwork)
		++overworkDeaths_;
}