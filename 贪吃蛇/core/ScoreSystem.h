#pragma once

#include "Food.h"
#include <array>

class ScoreSystem
{
public:
    void reset();

    void onEat(FoodType type);
    void onDeath(bool overwork);
    void onAbilityEventCompleted() { ability_ += 3; }
    void setElectiveEnabled(bool enabled);

    int score() const { return score_; }
    float gpa() const { return gpa_; }
    int bestScore() const { return bestScore_; }

    int deathCount() const { return deathCount_; }
    int overworkDeaths() const { return overworkDeaths_; }
    int ability() const { return ability_; }

    double courseGrade(FoodType course) const;
    double totalCredits() const { return totalCredits_; }

    double happinessPreview() const;

    static const wchar_t* courseDisplayName(FoodType course);
    static double courseCredit(FoodType course);
    static bool isCourse(FoodType t);

    static constexpr int CourseCountNoElective = 11;

private:
    void recalcGpa();
    bool isCourseEnabled(FoodType t) const;

private:
    int score_ = 0;
    std::array<double, (size_t)FoodType::COUNT> grade_{};

    double totalCredits_ = 0.0;
    float gpa_ = 0.0f;

    int ability_ = 0;
    int deathCount_ = 0;
    int overworkDeaths_ = 0;

    int bestScore_ = 0;

    bool electiveEnabled_ = false;
};