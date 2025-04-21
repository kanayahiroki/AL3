#pragma once
#include "KamataEngine.h"
#include "Player.h"

// ゲームシーン
class GameScene {

public:
	~GameScene();
	// 初期化
	void initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// 自キャラ
	Player* player_ = nullptr;

private:
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// カメラ
	KamataEngine::Camera camera_;
};
