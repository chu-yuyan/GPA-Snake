#pragma once

enum class GameEvent
{
	NONE,
	ELECTIVE, // 0s
	SITP,     // 30s
	PROJECT,  // 60s
	CET4,     // 110s
	CONTEST   // 140s
};

enum class EventState
{
	IDLE,
	PENDING,
	ACTIVE
};

//  挑战事件结束结果（一次性）
enum class ChallengeResult
{
	None,
	Success,
	Fail
};

class EventManager
{
public:
	void reset();
	void update(int seconds);

	EventState state() const { return state_; }
	GameEvent pendingEvent() const { return pending_; }
	GameEvent activeEvent() const { return active_; }

	int activeRemainingSeconds(int nowSeconds) const;

	void accept(int nowSeconds);
	void skip();

	bool onEatEventFood();

	bool electiveEnabled() const { return electiveEnabled_; }
	int activeProgress() const { return activeProgress_; }

	//  取走上一次挑战事件结果（取一次就清空）
	ChallengeResult consumeLastChallengeResult();

private:
	void tryTriggerPending(int seconds);
	static bool isChallengeEvent(GameEvent e);

private:
	EventState state_ = EventState::IDLE;
	GameEvent pending_ = GameEvent::NONE;
	GameEvent active_ = GameEvent::NONE;

	int activeStartSeconds_ = 0;
	int activeDurationSeconds_ = 10;

	int activeProgress_ = 0;
	bool electiveEnabled_ = false;

	//  上一次挑战事件结果：由EventManager写入，Game读取
	ChallengeResult lastChallengeResult_ = ChallengeResult::None;

	bool firedElective_ = false;
	bool firedSitp_ = false;
	bool firedProject_ = false;
	bool firedCet4_ = false;
	bool firedContest_ = false;
};