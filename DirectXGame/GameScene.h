#pragma once
#include "CameraController.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include "Skydome.h"
#include <vector>
#include "mapChipFiled.h"

class GameScene {
public:
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Model* modelBlock_ = nullptr;
	// 3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;
	// モデルプレイヤー
	KamataEngine::Model* modelPlayer_ = nullptr;

	// モデル敵
	KamataEngine::Model* modelEnemy_ = nullptr;

	// 自キャラ
	Player* player_ = nullptr;

	// キューブ
	Skydome* skydome_ = nullptr;

	// 敵
	Enemy* enemy_ = nullptr;

	// 表示ブロック
	void GenerateBlocks();

	MapChipFiled* mapChipFiled_;

	CameraController* cameraController_;

	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// デストラクタ
	~GameScene();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Camera camera_;

	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	bool isDebugCameraActive_ = false;
};