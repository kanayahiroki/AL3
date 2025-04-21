#pragma once
#include <KamataEngine.h>
class Player 
{
public:
	~Player();
	// 初期化
	void Initialize(KamataEngine::Model*model,uint32_t textureHandle,KamataEngine::Camera*camera);

	// 更新
	void Update();

	// 描画
	void Draw();

	KamataEngine::Camera* camera_ = nullptr;

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;
};