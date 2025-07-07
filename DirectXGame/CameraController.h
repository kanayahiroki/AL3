#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

using namespace MathUtility;

class Player;
class CameraController {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	void Reset();

	// Getter
	const KamataEngine::Camera& GetViewProjection() const { return camera_; }

	// 矩形
	struct Rect {
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};

	// カメラ移動範囲
	Rect movableArea_ = {0, 100, 0, 100};

	void SetMovableArea(Rect area) { movableArea_ = area; }

	// カメラの目標座標
	KamataEngine::Vector3 targetPosition_;

	// 座標補間割合
	static inline const float kInterpolationRate = 1.0f;

	// 速度掛け算
	static inline const float kVelocityBias = 4.0f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect targetMargin = {-9.0f, 9.0f, -5.0f, 5.0f};

private:
	KamataEngine::Camera camera_;
	Player* target_ = nullptr;
	// 追従対象とカメラの座標の差（オフセット）
	Vector3 targetOffset_ = {0, 0, -15.0f};
};