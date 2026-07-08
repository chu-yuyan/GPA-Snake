#include "FoodSprite.h"

bool FoodSprite::inited_ = false;
IMAGE FoodSprite::imgs_[(int)FoodType::COUNT]{};
bool FoodSprite::loaded_[(int)FoodType::COUNT]{};

void FoodSprite::loadOne(FoodType t, const wchar_t* relativePath, int targetW, int targetH)
{
	const int idx = (int)t;
	loaded_[idx] = false;

	IMAGE raw;
	loadimage(&raw, relativePath);

	const int rw = raw.getwidth();
	const int rh = raw.getheight();
	if (rw <= 0 || rh <= 0) return;

	//  创建目标尺寸画布（后续可在这里做“保持比例缩放 + 居中 + 透明”）
	imgs_[idx].Resize(targetW, targetH);

	// 当前先保证尺寸一致，避免图片过大遮挡。
	SetWorkingImage(&imgs_[idx]);
	cleardevice();
	putimage(0, 0, &raw);
	SetWorkingImage(nullptr);

	loaded_[idx] = (imgs_[idx].getwidth() > 0 && imgs_[idx].getheight() > 0);
}

void FoodSprite::init(int cellSize)
{
	if (inited_) return;
	inited_ = true;

	for (int i = 0; i < (int)FoodType::COUNT; ++i)
		loaded_[i] = false;

	const int target = Food::Size * cellSize; // 2x2

	//  约定：路径相对“工作目录(ProjectDir)”
	loadOne(FoodType::CALCULUS,       L"./assets/foods/calculus.png", target, target);
	loadOne(FoodType::LINEAR_ALGEBRA, L"./assets/foods/linear.png", target, target);
	loadOne(FoodType::PHYSICS,        L"./assets/foods/physics.png", target, target);
	loadOne(FoodType::ENGLISH,        L"./assets/foods/english.png", target, target);
	loadOne(FoodType::PHYSICS_LAB,    L"./assets/foods/physics_lab.png", target, target);
	loadOne(FoodType::PE,             L"./assets/foods/pe.png", target, target);
	loadOne(FoodType::CPP,            L"./assets/foods/cpp.png", target, target);
	loadOne(FoodType::MILITARY,       L"./assets/foods/military.png", target, target);
	loadOne(FoodType::AI,             L"./assets/foods/ai.png", target, target);
	loadOne(FoodType::RESEARCH,       L"./assets/foods/research.png", target, target);
	loadOne(FoodType::POLITICS,       L"./assets/foods/politics.png", target, target);
	loadOne(FoodType::ELECTIVE,       L"./assets/foods/elective.png", target, target);

	loadOne(FoodType::ICE_TEA,        L"./assets/foods/ice_tea.png", target, target);
	loadOne(FoodType::VIRUS,          L"./assets/foods/virus.png", target, target);

	loadOne(FoodType::CET4,           L"./assets/foods/cet4.png", target, target);
	loadOne(FoodType::SITP,           L"./assets/foods/sitp.png", target, target);
	loadOne(FoodType::PROJECT,        L"./assets/foods/project.png", target, target);
	loadOne(FoodType::CONTEST,        L"./assets/foods/contest.png", target, target);
}

const IMAGE* FoodSprite::get(FoodType t)
{
	if (!inited_) return nullptr;
	const int idx = (int)t;
	return loaded_[idx] ? &imgs_[idx] : nullptr;
}