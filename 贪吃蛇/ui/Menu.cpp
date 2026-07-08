#include "Menu.h"

#include "Renderer.h"
#include <graphics.h>
#include <windows.h>
#include <string>
#include "../system/RecordManager.h"
#include "SettlementManager.h"
#include "../system/Bgm.h"

static bool g_skinUnlocked[4] = { true, false, false, false };
struct RectI
{
	int x = 0, y = 0, w = 0, h = 0;
};

//  前置声明：让下面的代码能提前看到这个函数
static const wchar_t* SkinBgmPathLocal(SnakeSkin s);
static const wchar_t* MenuBgPathLocal();
static const wchar_t* SkinPreviewPathLocal(SnakeSkin s);
static const wchar_t* RulesBgPathLocal();
static const wchar_t* RecordsBgPathLocal();

static bool InRect(const RectI& r, int mx, int my)
{
	return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h;
}

static bool MouseClickEdge(int& mx, int& my)
{
	POINT pt{};
	if (!GetCursorPos(&pt))
		return false;

	HWND hwnd = GetHWnd();
	if (!hwnd || !ScreenToClient(hwnd, &pt))
		return false;

	mx = pt.x;
	my = pt.y;

	static bool prevDown = false;
	const bool downNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	const bool click = (downNow && !prevDown);
	prevDown = downNow;
	return click;
}

static void DrawButton(const RectI& r, const std::wstring& text)
{
	Renderer::drawPanel(r.x, r.y, r.w, r.h);
	Renderer::drawText(r.x + 28, r.y + 10, text, 22);
}

static const wchar_t* SkinName(SnakeSkin s)
{
	switch (s)
	{
	case SnakeSkin::Red:    return L"红色";
	case SnakeSkin::Green:  return L"绿色";
	case SnakeSkin::Blue:   return L"蓝色";
	case SnakeSkin::Purple: return L"紫色";
	default:                return L"红色";
	}
}

static SnakeSkin NextSkin(SnakeSkin s)
{
	int v = (int)s + 1;
	if (v >= (int)SnakeSkin::COUNT) v = 0;
	return (SnakeSkin)v;
}

static void DrawRulesPage(int w, int h, const RectI& backBtn)
{
	cleardevice();
	Renderer::drawPanel(0, 0, w, h);
	Renderer::drawImageStretch(RulesBgPathLocal(), 0, 0, w, h);

	Renderer::drawText(40, 24, L"游戏规则", 30);

	int y = 74;
	auto line = [&](const std::wstring& s, COLORREF c, bool bold, int size)
	{
		Renderer::drawTextStyled(40, y, s, size, c, bold, L"微软雅黑"); // 字体可改
		y += 30;
	};

	// 标题行：黄色+加粗
	line(L"1) 目标：达到绩点3.5", RGB(255, 215, 0), true, 26);

	// 普通行：白色不加粗
	line(L"   - 成就感 = (GPA×20 + 能力值) × (10 - 卷死次数) / 10。",BLUE, false, 22);
	line(L"   - GPA：课程按学分加权平均；吃到对应课程食物，该课成绩+0.5，最高5.0（初始2.0）。", BLUE, false, 22);
	line(L"   - 蛇成长：吃到任意食物，蛇长度+1（课程/道具/事件食物都算）。", BLUE, false, 22);

	// 重点提示：红色+加粗
	line(L"   - 死亡：撞墙/撞自己都会+1死亡；撞自己额外+1【卷死次数】，学得越多越卷(bushi)", RGB(255, 80, 80), true, 22);

	line(L"   - 撞墙：蛇身变墙；撞自己：蛇身变食物，蛇长度重置为3。", BLUE, false, 22);
	line(L"2) 事件（选择确定才参与）：10秒内吃到5个事件食物 -> 成就感+3。", RGB(120, 255, 120), true, 22);
	line(L"   - 选修课事件：确定后永久解锁【选修课】并加入平时掉落与绩点计算；跳过则本局不存在。", BLUE, false, 22);
	line(L"3) 道具：冰红茶/病毒持续10秒。", RGB(120, 200, 255), true, 22);
	line(L"   - 默认空格加速1.5倍；冰红茶期间空格加速3倍；病毒期间速度0.5倍且空格无效。", BLUE, false, 22);

	DrawButton(backBtn, L"返回");
	FlushBatchDraw();
}

static void DrawRecordsPage(int w, int h, const RectI& backBtn,
	const std::vector<GameRecord>& rs, int mx, int my, bool click,
	int& openIndex, int& deleteIndex)  // 移除 pendingDeleteIndex
{
	cleardevice();
	Renderer::drawPanel(0, 0, w, h);
	Renderer::drawImageStretch(RecordsBgPathLocal(), 0, 0, w, h);
	Renderer::drawTextStyled(40, 24, L"游戏记录", 30, RGB(255, 215, 0), true, L"微软雅黑");

	const int startY = 90;
	const int rowH = 92;
	const int rowPanelH = 80;

	openIndex = -1;
	deleteIndex = -1;

	for (int i = 0; i < (int)rs.size() && i < 8; ++i)
	{
		const auto& r = rs[i];

		RectI row{ 40, startY + i * rowH, w - 80, rowPanelH };
		Renderer::drawPanel(row.x, row.y, row.w, row.h);

		const int paddingX = 12;
		const int paddingY = 10;

		const int btnW = 92;
		const int btnH = 32;
		const int btnGap = 12;

		RectI delBtn{
			row.x + row.w - btnW - paddingX,
			row.y + (row.h - btnH) / 2,
			btnW,
			btnH
		};

		RectI detailBtn{
			delBtn.x - btnGap - btnW,
			delBtn.y,
			btnW,
			btnH
		};

		RectI infoArea{
			row.x + paddingX,
			row.y + paddingY,
			(detailBtn.x - btnGap) - (row.x + paddingX),
			row.h - paddingY * 2
		};

		std::wstring info;
		info += L"GPA: " + std::to_wstring(r.gpa);
		info += L"\n";
		info += L"幸福感: " + std::to_wstring((int)r.happiness);

		Renderer::drawTextInRect(
			infoArea.x, infoArea.y, infoArea.w, infoArea.h,
			info,
			16, WHITE, false,
			DT_WORDBREAK | DT_EDITCONTROL | DT_LEFT | DT_TOP,
			L"微软雅黑");

		// “详细”按钮
		Renderer::drawPanel(detailBtn.x, detailBtn.y, detailBtn.w, detailBtn.h);
		Renderer::drawTextInRect(
			detailBtn.x, detailBtn.y, detailBtn.w, detailBtn.h,
			L"详细",
			18, WHITE, true,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER,
			L"微软雅黑");

		// 删除
		Renderer::drawPanel(delBtn.x, delBtn.y, delBtn.w, delBtn.h);
		Renderer::drawTextInRect(
			delBtn.x, delBtn.y, delBtn.w, delBtn.h,
			L"删除",
			18, WHITE, true,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER,
			L"微软雅黑");

		if (click)
		{
			if (InRect(detailBtn, mx, my))
			{
				openIndex = i;
			}
			else if (InRect(delBtn, mx, my))
			{
				deleteIndex = i;   // 直接删除，不等待二次确认
			}
		}
	}

	DrawButton(backBtn, L"返回");
	FlushBatchDraw();
}

static void ShowRecordDetailBlocking(int w, int h, const GameRecord& r)
{
	SettlementManager s;
	s.reset();
	s.loadFromRecord(r);

	//  返回按钮（右下角）
	const RectI backBtn{ w - 200, h - 90, 160, 52 };

	while (true)
	{
		int mx = -1, my = -1;
		const bool click = MouseClickEdge(mx, my);

		cleardevice();
		s.draw(w, h);

		// 覆盖一个“返回”按钮
		DrawButton(backBtn, L"返回");

		FlushBatchDraw();

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			return;

		if (click && InRect(backBtn, mx, my))
			return;

		Sleep(10);
	}
}

static void DrawSkinSelectPage(int w, int h, const RectI& backBtn,
	SnakeSkin& skin, Bgm& bgm, int mx, int my, bool click)
{
	cleardevice();
	setfillcolor(BLACK);
	solidrectangle(0, 0, w, h);

	const int imgW = 500;
	const int imgH = 281;
	const int gapX = 40;
	const int gapY = 22;

	const int gridW = imgW * 2 + gapX;
	const int gridH = imgH * 2 + gapY;
	const int topY = 44;
	const int startX = (w - gridW) / 2;
	const int startY = topY;

	// 定义四个皮肤按钮的区域
	RectI skinRects[4] = {
		{ startX,                 startY,                 imgW, imgH }, // Red
		{ startX + imgW + gapX, startY,                 imgW, imgH }, // Green
		{ startX,                startY + imgH + gapY,   imgW, imgH }, // Blue
		{ startX + imgW + gapX, startY + imgH + gapY,   imgW, imgH }  // Purple
	};

	// 绘制所有皮肤卡片
	for (int i = 0; i < 4; ++i)
	{
		SnakeSkin s = (SnakeSkin)i;
		const RectI& r = skinRects[i];
		bool unlocked = g_skinUnlocked[i];

		// 边框底板
		Renderer::drawPanel(r.x - 3, r.y - 3, r.w + 6, r.h + 6);
		Renderer::drawImageStretch(SkinPreviewPathLocal(s), r.x, r.y, r.w, r.h);

		// 如果是锁定状态，绘制锁图标（左上角）
		if (!unlocked)
		{
			// 半透明黑色背景
			setfillcolor(RGB(0, 0, 0, 180));
			solidrectangle(r.x + 5, r.y + 5, r.x + 35, r.y + 35);
			settextcolor(WHITE);
			settextstyle(20, 0, L"Segoe UI Symbol");
			outtextxy(r.x + 12, r.y + 8, L"🔒");
		}

		// 当前选中高亮
		if (s == skin)
		{
			setlinecolor(RGB(255, 215, 0));
			rectangle(r.x - 4, r.y - 4, r.x + r.w + 4, r.y + r.h + 4);
		}
	}

	// 返回按钮（居中）
	int backY = startY + gridH + 14;
	if (backY + backBtn.h > h - 10) backY = h - 10 - backBtn.h;
	const RectI centeredBackBtn{ (w - backBtn.w) / 2, backY, backBtn.w, backBtn.h };
	DrawButton(centeredBackBtn, L"返回");

	FlushBatchDraw();

	// 鼠标点击处理
	if (!click) return;

	// 先检测返回按钮
	if (InRect(centeredBackBtn, mx, my))
	{
		// 返回菜单（由上层循环处理，这里不直接关闭）
		return;
	}

	// 检测点击哪个皮肤按钮
	for (int i = 0; i < 4; ++i)
	{
		if (InRect(skinRects[i], mx, my))
		{
			SnakeSkin clickedSkin = (SnakeSkin)i;
			if (g_skinUnlocked[i])
			{
				// 已解锁 → 正常切换皮肤
				if (clickedSkin != skin)
				{
					skin = clickedSkin;
					bgm.playLoop(SkinBgmPathLocal(skin));
				}
			}
			else
			{
				int ret = MessageBoxW(GetHWnd(),
					L"如果高程能给个优的话，这些皮肤自然就解锁了...",
					L"提示", MB_OK);
				if (ret == IDOK)
				{
					// 彩蛋：解锁 Green, Blue, Purple
					g_skinUnlocked[1] = true;
					g_skinUnlocked[2] = true;
					g_skinUnlocked[3] = true;
					MessageBoxW(
						GetHWnd(),

						L"……系统似乎检测到了某种神秘力量。\n隐藏奖励已解锁！\n\n",
						L"",
						MB_OK 
					);
				}
			}
			break; // 只处理第一个点击的按钮
		}
	}
}

static const wchar_t* SkinBgmPathLocal(SnakeSkin s)
{
	switch (s)
	{
	case SnakeSkin::Red:    return L"./assets/bgms/red.wav";
	case SnakeSkin::Green:  return L"./assets/bgms/green.wav";
	case SnakeSkin::Blue:   return L"./assets/bgms/blue.wav";
	case SnakeSkin::Purple: return L"./assets/bgms/purple.wav";
	default:                return L"./assets/bgms/red.wav";
	}
}

static const wchar_t* MenuBgPathLocal()
{
	return L"./assets/back/menu_bg.png";
}

static const wchar_t* SkinPreviewPathLocal(SnakeSkin s)
{
	switch (s)
	{
	case SnakeSkin::Red:    return L"./assets/back/skin_red.png";
	case SnakeSkin::Green:  return L"./assets/back/skin_green.png";
	case SnakeSkin::Blue:   return L"./assets/back/skin_blue.png";
	case SnakeSkin::Purple: return L"./assets/back/skin_purple.png";
	default:                return L"./assets/back/skin_red.png";
	}
}

static const wchar_t* RulesBgPathLocal()
{
	return L"./assets/back/rules_bg.png";
}

static const wchar_t* RecordsBgPathLocal()
{
	return L"./assets/back/records_bg.png";
}

Menu::Result Menu::run(int screenW, int screenH, Bgm& bgm) const
{
	BeginBatchDraw();

	//  主菜单按钮恢复：统一尺寸、垂直排列、居中
	const int btnW = 320;
	const int btnH = 58;
	const int gapY = 18;

	const int firstY = 260;
	RectI startBtn{ (screenW - btnW) / 2, firstY + (btnH + gapY) * 0, btnW, btnH };
	RectI skinBtn{ (screenW - btnW) / 2, firstY + (btnH + gapY) * 1, btnW, btnH };
	RectI rulesBtn{ (screenW - btnW) / 2, firstY + (btnH + gapY) * 2, btnW, btnH };
	RectI recordsBtn{ (screenW - btnW) / 2, firstY + (btnH + gapY) * 3, btnW, btnH };

	// 返回按钮在子页面里会居中绘制（DrawSkinSelectPage里有自己的居中back）
	RectI backBtn{ 40, screenH - 90, 160, 52 };

	bool showRules = false;
	bool showRecords = false;
	SnakeSkin skin = SnakeSkin::Red;
	bool showSkinSelect = false;

	

	while (true)
	{
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			EndBatchDraw();
			return Result{ false, true, skin };
		}

		int mx = -1, my = -1;
		const bool click = MouseClickEdge(mx, my);

		if (!showRules && !showRecords && !showSkinSelect)
		{
			cleardevice();
			Renderer::drawPanel(0, 0, screenW, screenH);
			Renderer::drawImageStretch(MenuBgPathLocal(), 0, 0, screenW, screenH);


			DrawButton(startBtn, L"开始游戏");

			std::wstring skinText = L"蛇皮肤：";
			skinText += SkinName(skin);
			DrawButton(skinBtn, skinText);

			DrawButton(rulesBtn, L"游戏规则");
			DrawButton(recordsBtn, L"游戏记录");

			FlushBatchDraw();

			if (click)
			{
				if (InRect(startBtn, mx, my))
				{
					EndBatchDraw();
					return Result{ true, false, skin };
				}
				if (InRect(skinBtn, mx, my))
				{
					showSkinSelect = true;
				}
				if (InRect(rulesBtn, mx, my))
				{
					showRules = true;
				}
				if (InRect(recordsBtn, mx, my))
				{
					showRecords = true;
				}
			}
		}
		else if (showSkinSelect)
		{
			const int imgW = 500, imgH = 281, gapX = 40, gapY2 = 22;
			const int gridW = imgW * 2 + gapX;
			const int gridH = imgH * 2 + gapY2;
			const int topY = 44;
			const int startX = (screenW - gridW) / 2;
			const int startY = topY;

			int backY = startY + gridH + 14;
			if (backY + backBtn.h > screenH - 10) backY = screenH - 10 - backBtn.h;

			const RectI centeredBackBtn{ (screenW - backBtn.w) / 2, backY, backBtn.w, backBtn.h };

			DrawSkinSelectPage(screenW, screenH, backBtn, skin, bgm, mx, my, click);

			if (click && InRect(centeredBackBtn, mx, my))
				showSkinSelect = false;
		}
		else if (showRules)
		{
			DrawRulesPage(screenW, screenH, backBtn);

			if (click && InRect(backBtn, mx, my))
				showRules = false;
		}
		else if (showRecords)
		{
			RecordManager rm;
			auto rs = rm.loadAll();
			std::reverse(rs.begin(), rs.end());

			int openIndex = -1;
			int deleteIndex = -1;
			// 注意：函数参数少了 pendingDeleteIndex
			DrawRecordsPage(screenW, screenH, backBtn, rs, mx, my, click, openIndex, deleteIndex);

			if (click && InRect(backBtn, mx, my))
			{
				showRecords = false;
				// 不再需要重置 pendingDeleteIndex
			}
			else if (deleteIndex >= 0 && deleteIndex < (int)rs.size())
			{
				rm.removeAtFromNewestIndex((size_t)deleteIndex);
				// 删除后留在记录页，下一帧重新 loadAll 自动刷新
			}
			else if (openIndex >= 0 && openIndex < (int)rs.size())
			{
				ShowRecordDetailBlocking(screenW, screenH, rs[openIndex]);
			}
		}

		Sleep(10);
	}
}