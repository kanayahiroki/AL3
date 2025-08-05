#pragma once
#include "KamataEngine.h"

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct MapChipData // グローバル
{
	std::vector<std::vector<MapChipType>> date;
};

class MapChipFiled

{
public:
	// １ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	// 1マップチップデータ
	MapChipData mapChipDate_;

	// インデックスセット
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	// 「指定座標がマップチップの何番の位置にあるのか」を計算する関数
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	// 範囲矩形
	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	// ブロックの範囲取得関数
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	// 1読み込みデータをリセットする関数
	void ResetMapChipDate();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }
};