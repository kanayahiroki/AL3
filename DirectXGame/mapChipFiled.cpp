#include "MapChipFiled.h"
#include <fstream>
#include <map>
#include <sstream>

using namespace KamataEngine;

std::map<std::string, MapChipType> MapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
};
// マップチップデータをリセット

void MapChipFiled::ResetMapChipDate() {
	mapChipDate_.date.clear();
	mapChipDate_.date.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDateLine : mapChipDate_.date) {
		mapChipDateLine.resize(kNumBlockHorizontal);
	}
}

void MapChipFiled::LoadMapChipCsv(const std::string& filePath) {
	ResetMapChipDate();
	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	//	マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();
	// CSVからマップチップデータを読み込み
	for (uint32_t i = 0; i < kNumBlockVirtical; i++) {
		std::string line;
		getline(mapChipCsv, line);

		// １行分の文字列をストリームに変換して解折しやすくする
		std::istringstream line_stream(line);
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string world;
			getline(line_stream, world, ',');
			if (MapChipTable.contains(world)) {
				mapChipDate_.date[i][j] = MapChipTable[world];
			}
		}
	}
}

MapChipType MapChipFiled::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}

	return mapChipDate_.date[yIndex][xIndex];
	MapChipType();
}

KamataEngine::Vector3 MapChipFiled::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {

	return KamataEngine::Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0);
}

// 座標からマップチップ番号を計算
MapChipFiled::IndexSet MapChipFiled::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2) / kBlockWidth);
	indexSet.yIndex = kNumBlockVirtical - 1 - static_cast<uint32_t>((position.y + kBlockHeight / 2) / kBlockHeight);
	return indexSet;
}

// ブロックの範囲取得関数
MapChipFiled::Rect MapChipFiled::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	// 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}