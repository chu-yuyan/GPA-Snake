#ifndef DEBUG_TOTAL_TIME_SECONDS
#define DEBUG_TOTAL_TIME_SECONDS 180   //调试用
#endif
#include "Game.h"
#include "ui/FoodSprite.h"
#include "ui/SnakeHeadSprite.h"
#include "system/RecordManager.h"

#include <graphics.h>
#include <windows.h>
#include <random>
#include <algorithm>

static int GetFoodWeight(FoodType t, bool electiveEnabled)
{
	switch (t)
	{
	case FoodType::CALCULUS:        return 12;   // 数分 6学分
	case FoodType::LINEAR_ALGEBRA:  return 10;   // 线代 5学分
	case FoodType::PHYSICS:         return 8;    // 物理 3学分
	case FoodType::ENGLISH:         return 7;    // 英语 2学分
	case FoodType::CPP:             return 7;    // 高程 2学分
	case FoodType::AI:              return 6;    // 人工智能 2学分
	case FoodType::MILITARY:        return 6;    // 军理 2学分
	case FoodType::RESEARCH:        return 5;    // 科研前沿 2学分
	case FoodType::PHYSICS_LAB:     return 4;    // 物理实验 1学分
	case FoodType::PE:              return 4;    // 体育 1学分
	case FoodType::POLITICS:        return 3;    // 形策 0.5学分

	case FoodType::ICE_TEA:         return 2;
	case FoodType::VIRUS:           return 2;

		// 选修课
	case FoodType::ELECTIVE:        return electiveEnabled ? 5 : 0;

		// 事件
	case FoodType::CET4:
	case FoodType::SITP:
	case FoodType::PROJECT:
	case FoodType::CONTEST:
	default:
		return 0;
	}
}

static FoodType WeightedRandomFood(bool electiveEnabled)
{
	static std::mt19937 rng{ std::random_device{}() };

	std::vector<std::pair<FoodType, int>> candidates;
	for (int i = 0; i < (int)FoodType::COUNT; ++i)
	{
		FoodType t = (FoodType)i;
		int w = GetFoodWeight(t, electiveEnabled);
		if (w > 0)
			candidates.emplace_back(t, w);
	}

	if (candidates.empty())
		return FoodType::CALCULUS; // fallback

	int total = 0;
	for (auto& p : candidates) total += p.second;

	std::uniform_int_distribution<int> dist(1, total);
	int r = dist(rng);
	int accum = 0;
	for (auto& p : candidates)
	{
		accum += p.second;
		if (r <= accum)
			return p.first;
	}
	return FoodType::CALCULUS;
}
static bool IsEventOnlyFood(FoodType t)
{
	switch (t)
	{
	case FoodType::SITP:
	case FoodType::PROJECT:
	case FoodType::CET4:
	case FoodType::CONTEST:
		return true;
	default:
		return false;
	}
}

Game::Game(int windowW_, int windowH_, int cellSize_, SnakeSkin skin)
	: windowW(windowW_), windowH(windowH_), cellSize(cellSize_)
{
	timer.setTickMs(100);
	FoodSprite::init(cellSize);
	SnakeHeadSprite::init(cellSize);

	playOffsetX = uiPanelW;
	playW = windowW - uiPanelW;
	playH = windowH;

	gridW = playW / cellSize;
	gridH = playH / cellSize;

	wall.buildBorder(gridW, gridH);

	score.reset();
	eventMgr.reset();
	timer.reset();

	snake.setSkin(skin);

	resetRound();
}

int Game::calcSleepMs() const
{
	const int baseMs = timer.tickMs();
	const int now = timer.seconds();

	const bool virusActive = now < virusUntilSec_;
	const bool teaActive = now < iceTeaUntilSec_;
	const bool spaceDown = lastInput_.spaceDown;

	double mul = 1.0;

	// 病毒
	if (virusActive)
	{
		mul = 0.5;
	}
	else
	{
		// 默认：空格1.5倍
		if (spaceDown)
			mul = 1.5;

		// 冰红茶）`
		if (teaActive && spaceDown)
			mul = 3.0;
	}

	int ms = (int)(baseMs / mul);
	if (ms < 1) ms = 1;
	return ms;
}

void Game::run()
{
	BeginBatchDraw();

	//  进入游戏后的开场动画（阻塞播放）
	anim_.playGameIntroBlocking(windowW, windowH);

	while (running)
	{
		input();
		update();
		render();

		Sleep(calcSleepMs());

		if (eventMgr.state() != EventState::PENDING)
			timer.step();
	}

	//  先播放退出动画，再进入结算
	const bool win = (score.gpa() >= 3.5f);
	anim_.playGameOutroBlocking(windowW, windowH, win);

	//  游戏结束总结窗口（点击关闭后才返回main退出）
	settle_.generateReport(score, timer, eventMgr);

	//  写入一条记录到 records.tsv（关键：之前没有调用 append）
	{
		RecordManager rm;
		const GameRecord rec = settle_.toRecord();
		const bool ok = rm.append(rec);
		if (!ok)
			OutputDebugStringW(L"[Game] RecordManager::append failed\n");
	}

	anim_.showGameSummaryBlocking(windowW, windowH, score, timer, eventMgr, settle_);

	EndBatchDraw();
}

void Game::input()
{
	lastInput_ = inputSys.poll();

	if (lastInput_.quit)
	{
		running = false;
		return;
	}

	if (lastInput_.dirChanged)
		snake.setDirection(lastInput_.dir);
}

FoodType Game::mapEventToFood(GameEvent e) const
{
	switch (e)
	{
	case GameEvent::ELECTIVE: return FoodType::ELECTIVE;
	case GameEvent::SITP: return FoodType::SITP;
	case GameEvent::PROJECT: return FoodType::PROJECT;
	case GameEvent::CET4: return FoodType::CET4;
	case GameEvent::CONTEST: return FoodType::CONTEST;
	default: return FoodType::CALCULUS;
	}
}

void Game::spawnFoods(int count)
{
	static std::mt19937 rng{ std::random_device{}() };

	const int minX = 1;
	const int minY = 1;
	const int maxX = (gridW - 2) - (Food::Size - 1);
	const int maxY = (gridH - 2) - (Food::Size - 1);
	if (maxX < minX || maxY < minY) return;

	std::uniform_int_distribution<int> dx(minX, maxX);
	std::uniform_int_distribution<int> dy(minY, maxY);

	auto canPlaceFood = [&](const Position& topLeft) -> bool
		{
			if (topLeft.x < minX || topLeft.x > maxX || topLeft.y < minY || topLeft.y > maxY)
				return false;

			Food tmp;
			tmp.pos = topLeft;
			for (const auto& c : tmp.cells())
			{
				if (isCellBlocked(c)) return false;
			}
			return true;
		};

	for (int i = 0; i < count; ++i)
	{
		Position p{};
		int guard = 0;

		do
		{
			p = { dx(rng), dy(rng) };
			++guard;
			if (guard > 4000) return;
		} while (!canPlaceFood(p));

		Food f;
		f.pos = p;
		// 关键修改：按权重随机选择食物类型，并传入选修课是否启用
		f.type = WeightedRandomFood(eventMgr.electiveEnabled());
		f.spawnTick = timer.ticks();
		f.ttlTicks = 80;
		foods.push_back(f);
	}
}

bool Game::isCellBlocked(const Position& p) const
{
	if (p.x < 0 || p.x >= gridW || p.y < 0 || p.y >= gridH)
		return true;

	// 边界墙占用
	if (p.x == 0 || p.x == gridW - 1 || p.y == 0 || p.y == gridH - 1)
		return true;

	if (wall.contains(p)) return true;
	if (snake.occupies(p)) return true;

	for (const auto& f : foods)
	{
		if (f.contains(p)) return true;
	}

	return false;
}

void Game::spawnEventFoodsOnly(int count, FoodType eventFoodType)
{
	static std::mt19937 rng{ std::random_device{}() };

	const int minX = 1;
	const int minY = 1;
	const int maxX = (gridW - 2) - (Food::Size - 1);
	const int maxY = (gridH - 2) - (Food::Size - 1);
	if (maxX < minX || maxY < minY) return;

	std::uniform_int_distribution<int> dx(minX, maxX);
	std::uniform_int_distribution<int> dy(minY, maxY);

	auto canPlaceFood = [&](const Position& topLeft) -> bool
		{
			if (topLeft.x < minX || topLeft.x > maxX || topLeft.y < minY || topLeft.y > maxY)
				return false;

			Food tmp;
			tmp.pos = topLeft;
			tmp.type = eventFoodType;

			for (const auto& c : tmp.cells())
			{
				if (isCellBlocked(c)) return false;
			}
			return true;
		};

	for (int i = 0; i < count; ++i)
	{
		Position p{};
		int guard = 0;
		do
		{
			p = { dx(rng), dy(rng) };
			++guard;
			if (guard > 4000) return;
		} while (!canPlaceFood(p));

		Food f;
		f.pos = p;
		f.type = eventFoodType;
		f.spawnTick = timer.ticks();
		f.ttlTicks = 80;
		foods.push_back(f);
	}
}

void Game::resetRound()
{
	eatCount = 0;
	deathFoodDebt = 0;

	iceTeaUntilSec_ = 0;
	virusUntilSec_ = 0;

	midtermUntilSec_ = 0;
	finalUntilSec_ = 0;

	settle_.reset(); //  新增

	//  清空期中/期末周
	midtermUntilSec_ = 0;
	finalUntilSec_ = 0;

	snake.reset({ gridW / 2, gridH / 2 }, 3, Direction::Right);
	foods.clear();

	if (eventMgr.state() == EventState::ACTIVE)
	{
		const FoodType ef = mapEventToFood(eventMgr.activeEvent());
		spawnEventFoodsOnly(10, ef);
	}
	else
	{
		spawnFoods(10);
	}
	midtermTriggered_ = false;
	finalTriggered_ = false;
}

void Game::render()
{
	cleardevice();

	// 1) 先画游戏区（右侧）
	setorigin(playOffsetX, 0);
	ui.drawBackgroundGrid(playW, playH, cellSize);

	wall.draw(cellSize);

	for (const auto& f : foods)
		f.draw(cellSize);

	snake.draw(cellSize);

	// 2) 再画左侧UI栏（不偏移）
	setorigin(0, 0);
	ui.drawLeftPanel(uiPanelW, windowH, score, timer, eventMgr, snake.length());

	// 3) 只在等待选择时显示事件窗口
	if (eventMgr.state() == EventState::PENDING)
		anim_.drawEventConfirmDialog(windowW, windowH, timer.seconds(), eventMgr);

	// 道具10秒动画（overlay）
	anim_.drawIceTeaOverlay(windowW, windowH, timer.seconds(), iceTeaUntilSec_);
	anim_.drawVirusOverlay(windowW, windowH, timer.seconds(), virusUntilSec_);

	// 期中/期末周 overlay（掉落翻倍提示）
	anim_.drawMidtermWeekOverlay(windowW, windowH, timer.seconds(), midtermUntilSec_);
	anim_.drawFinalWeekOverlay(windowW, windowH, timer.seconds(), finalUntilSec_);

	// 事件结果toast
	anim_.drawEventResultToast(windowW, windowH, timer.seconds());

	FlushBatchDraw();
}

void Game::update()
{
	setorigin(0, 0);

	eventMgr.update(timer.seconds());

	const int nowSec = timer.seconds();

	if (!midtermTriggered_ && nowSec >= 90 && nowSec < 100) {
		midtermUntilSec_ = nowSec + 10;
		midtermTriggered_ = true;
	}

	// 期末周（160 秒时触发，持续 20 秒）
	if (!finalTriggered_ && nowSec >= 160 && nowSec < 180) {
		finalUntilSec_ = nowSec + 20;
		finalTriggered_ = true;
	}

	//  同步选修课启用状态到计分系统（启用后计入GPA）
	score.setElectiveEnabled(eventMgr.electiveEnabled());

	// ===== 鼠标点击检测（用于事件弹窗）=====
	dbgMouseX = -1;
	dbgMouseY = -1;
	dbgMouseClick = false;

	POINT pt{};
	if (GetCursorPos(&pt))
	{
		HWND hwnd = GetHWnd();
		if (hwnd && ScreenToClient(hwnd, &pt))
		{
			dbgMouseX = pt.x;
			dbgMouseY = pt.y;

			static bool prevDown = false;
			const bool downNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			dbgMouseClick = (downNow && !prevDown);
			prevDown = downNow;
		}
	}

	AnimDialogAction act =
		anim_.hitTestEventConfirmDialog(windowW, windowH, eventMgr, dbgMouseX, dbgMouseY, dbgMouseClick);

	if (eventMgr.state() == EventState::PENDING && act != AnimDialogAction::None)
	{
		if (act == AnimDialogAction::Accept)
		{
			eventMgr.accept(timer.seconds());

			if (eventMgr.state() == EventState::ACTIVE)
			{
				foods.clear();
				spawnEventFoodsOnly(10, mapEventToFood(eventMgr.activeEvent()));
			}
		}
		else if (act == AnimDialogAction::Skip)
		{
			eventMgr.skip();
		}
	}

	//  等待选择期间：蛇不动，食物不刷新，不判定死亡
	if (eventMgr.state() == EventState::PENDING)
		return;

	// ===== 过期食物处理 =====
	const int nowTick = timer.ticks();

	// 卷死债务未清时：不让食物自动消失
	if (deathFoodDebt <= 0)
	{
		foods.erase(
			std::remove_if(foods.begin(), foods.end(),
				[&](const Food& f) { return f.expired(nowTick); }),
			foods.end());
	}

	// ===== 保持食物数量：10个 =====
		// ===== 保持食物数量：10个（期中/期末周可能>10；结束后>10时不补）=====
	const int targetFoodCount = 10;

	if (deathFoodDebt <= 0)
	{
		const bool inMidterm = (nowSec < midtermUntilSec_);
		const bool inFinal = (nowSec < finalUntilSec_);
		const bool inDoubleDrop = inMidterm || inFinal;

		//  不再把 foods 强行裁剪到 10（否则翻倍没意义）
		// if ((int)foods.size() > targetFoodCount) ...

		if ((int)foods.size() < targetFoodCount)
		{
			// 如果不在翻倍窗口，并且当前>10：先不补（等吃回来）
			// （这里其实 size<10 已经成立，不会>10；规则保留给逻辑完整性）
			bool allowRefill = true;
			if (!inDoubleDrop && (int)foods.size() > targetFoodCount)
				allowRefill = false;

			if (allowRefill)
			{
				int need = targetFoodCount - (int)foods.size();

				//  翻倍窗口：普通掉落补充数量*2（只影响普通池）
				if (inDoubleDrop && eventMgr.state() != EventState::ACTIVE)
					need *= 2;

				if (eventMgr.state() == EventState::ACTIVE)
					spawnEventFoodsOnly(need, mapEventToFood(eventMgr.activeEvent()));
				else
					spawnFoods(need);
			}
		}
	}

	// ===== 蛇移动与碰撞 =====
	snake.move();

	const Position h = snake.head();
	const bool hitWall = wall.contains(h);
	const bool hitSelf = snake.checkSelfCollision();

	if (hitWall || hitSelf)
	{
		const auto& body = snake.bodyCells();

		if (hitWall)
		{
			// 撞墙：蛇身变墙
			std::vector<Position> ps;
			ps.reserve(body.size());
			for (const auto& p : body) ps.push_back(p);
			wall.addBlocks(ps);
		}
		else  // hitSelf
		{
			for (const auto& p : body)
			{
				Food f;
				f.pos = p;
				f.type = WeightedRandomFood(eventMgr.electiveEnabled());
				f.spawnTick = timer.ticks();
				f.ttlTicks = 80;
				foods.push_back(f);
			}
			deathFoodDebt = (int)body.size();
		}

		score.onDeath(hitSelf);
		save.writeBest(score.bestScore());

		eatCount = 0;
		snake.reset({ gridW / 2, gridH / 2 }, 3, Direction::Right);
		return;
	}

	// ===== 吃食物 =====
	for (size_t i = 0; i < foods.size(); ++i)
	{
		if (!foods[i].contains(h))
			continue;

		const FoodType eaten = foods[i].type;

		// 道具
		const int nowSec = timer.seconds();
		if (eaten == FoodType::ICE_TEA)
			iceTeaUntilSec_ = nowSec + 10;
		else if (eaten == FoodType::VIRUS)
			virusUntilSec_ = nowSec + 10;

		// 课程
		score.onEat(eaten);

		// 事件挑战进度
		if (eventMgr.state() == EventState::ACTIVE)
		{
			const FoodType ef = mapEventToFood(eventMgr.activeEvent());
			if (eaten == ef)
			{
				const GameEvent ae = eventMgr.activeEvent();
				const bool completed = eventMgr.onEatEventFood();
				if (completed) { score.onAbilityEventCompleted(); settle_.onChallengeCompleted(ae); }
			}
		}

		// 
		++eatCount;
		snake.grow();

		foods.erase(foods.begin() + i);

		// 卷死债务期：只扣债务，不补普通食物
		if (deathFoodDebt > 0)
		{
			--deathFoodDebt;
		}
		else
		{
			if (eventMgr.state() == EventState::ACTIVE)
				spawnEventFoodsOnly(1, mapEventToFood(eventMgr.activeEvent()));
			else
				spawnFoods(1);
		}

		// 更新结算信息
		settle_.onEatFood(eaten);

		break;
	}

	if (timer.seconds() >= DEBUG_TOTAL_TIME_SECONDS)
	{
		running = false;
		return;
	}

	if (prevEventState_ == EventState::ACTIVE && eventMgr.state() == EventState::IDLE)
	{
		const ChallengeResult cr = eventMgr.consumeLastChallengeResult();
		if (cr == ChallengeResult::Success)
		{
			anim_.startEventResultToast(true, timer.seconds());
		}
		else if (cr == ChallengeResult::Fail)
		{
			anim_.startEventResultToast(false, timer.seconds());
		}
	}
	prevEventState_ = eventMgr.state();
}
