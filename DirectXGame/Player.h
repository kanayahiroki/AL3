#pragma once
#include "KamataEngine.h"
class Player {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine ::Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();

	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 0.5f;

	// 左右
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 旋回開始時の角度
	float turnFIrstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.9f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.5f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.5f;
	// ジャンプ初速（上方向
	static inline const float kJumpAcceleration = 2.0f;

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Vector3 velocity_ = {};

	LRDirection lrDirection_ = LRDirection::kRight;
};