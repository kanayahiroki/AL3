#pragma once
#include "KamataEngine.h"
#include "MyMath.h"

class Player;
class Enemy {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, KamataEngine::Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.04f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = 5.0f;
	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 5.0f;
	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 5.0f;

	// 経過時間
	float walkTimer_ = 0.0f;

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	// 衝突応答
	void OnCollision(const Player* player);

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
};