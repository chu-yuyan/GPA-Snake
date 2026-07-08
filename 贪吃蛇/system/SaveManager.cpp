#include "SaveManager.h"

#include <fstream>

void SaveManager::writeBest(int bestScore) const
{
	// 初级版：覆盖写；后续第三/四阶段再扩展为多记录与读取
	std::ofstream out("data/records.txt", std::ios::out | std::ios::trunc);
	if (!out) return;
	out << "bestScore=" << bestScore << "\n";
}