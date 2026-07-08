#include "EventManager.h"

bool EventManager::isChallengeEvent(GameEvent e)
{
	switch (e)
	{
	case GameEvent::SITP:
	case GameEvent::PROJECT:
	case GameEvent::CET4:
	case GameEvent::CONTEST:
		return true;
	default:
		return false;
	}
}

void EventManager::reset()
{
	state_ = EventState::IDLE;
	pending_ = GameEvent::NONE;
	active_ = GameEvent::NONE;

	activeStartSeconds_ = 0;
	activeDurationSeconds_ = 10;

	activeProgress_ = 0;
	electiveEnabled_ = false;

	lastChallengeResult_ = ChallengeResult::None;

	firedElective_ = false;
	firedSitp_ = false;
	firedProject_ = false;
	firedCet4_ = false;
	firedContest_ = false;
}

void EventManager::tryTriggerPending(int seconds)
{
	if (state_ != EventState::IDLE)
		return;

	if (!firedElective_ && seconds >= 0) { pending_ = GameEvent::ELECTIVE; state_ = EventState::PENDING; firedElective_ = true; return; }
	if (!firedSitp_ && seconds >= 30) { pending_ = GameEvent::SITP; state_ = EventState::PENDING; firedSitp_ = true; return; }
	if (!firedProject_ && seconds >= 60) { pending_ = GameEvent::PROJECT; state_ = EventState::PENDING; firedProject_ = true; return; }
	if (!firedCet4_ && seconds >= 110) { pending_ = GameEvent::CET4; state_ = EventState::PENDING; firedCet4_ = true; return; }
	if (!firedContest_ && seconds >= 140) { pending_ = GameEvent::CONTEST; state_ = EventState::PENDING; firedContest_ = true; return; }
}

void EventManager::update(int seconds)
{
	if (state_ == EventState::ACTIVE)
	{
		if (seconds - activeStartSeconds_ >= activeDurationSeconds_)
		{
			//  ACTIVE超时结束：挑战事件视为失败
			if (isChallengeEvent(active_))
				lastChallengeResult_ = ChallengeResult::Fail;

			state_ = EventState::IDLE;
			active_ = GameEvent::NONE;
			activeProgress_ = 0;
		}
		return;
	}

	if (state_ == EventState::PENDING)
		return;

	tryTriggerPending(seconds);
}

int EventManager::activeRemainingSeconds(int nowSeconds) const
{
	if (state_ != EventState::ACTIVE) return 0;
	int passed = nowSeconds - activeStartSeconds_;
	int remain = activeDurationSeconds_ - passed;
	return remain < 0 ? 0 : remain;
}

void EventManager::accept(int nowSeconds)
{
	if (state_ != EventState::PENDING) return;

	if (pending_ == GameEvent::ELECTIVE)
	{
		electiveEnabled_ = true;
		pending_ = GameEvent::NONE;
		state_ = EventState::IDLE;
		return;
	}

	active_ = pending_;
	pending_ = GameEvent::NONE;

	activeStartSeconds_ = nowSeconds;
	activeDurationSeconds_ = 10;
	activeProgress_ = 0;

	state_ = EventState::ACTIVE;
}

void EventManager::skip()
{
	if (state_ != EventState::PENDING) return;

	pending_ = GameEvent::NONE;
	state_ = EventState::IDLE;
}

bool EventManager::onEatEventFood()
{
	if (state_ != EventState::ACTIVE) return false;
	if (!isChallengeEvent(active_)) return false;

	++activeProgress_;
	if (activeProgress_ >= 5)
	{
		//  达标结束：记录成功
		lastChallengeResult_ = ChallengeResult::Success;

		state_ = EventState::IDLE;
		active_ = GameEvent::NONE;
		activeProgress_ = 0;
		return true;
	}
	return false;
}

ChallengeResult EventManager::consumeLastChallengeResult()
{
	const ChallengeResult r = lastChallengeResult_;
	lastChallengeResult_ = ChallengeResult::None;
	return r;
}