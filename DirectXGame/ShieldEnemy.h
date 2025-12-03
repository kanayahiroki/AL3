#pragma once
#include "KamataEngine.h"
#include "MapChipFiled.h"
#include "MyMath.h"
#include "UpData.h"

#include <algorithm>
#include <cassert>
#include <numbers>

class MapChipField;
class Player; // Playerとの衝突判定に必要

using namespace KamataEngine;

/// <summary>
/// 敵
/// </summary>
class ShieldEnemy {
public:
	// 振る舞い
	enum class Behavior { kRoot, kDeth, kUnknown };

	// 敵の移動方向
	enum class Direction {
		kRight,
		kLeft,
	};

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void UpDate();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// AABBを取得
	AABB GetAABB();

	/// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// デスフラグ
	bool IsEnemyDead() const { return isEnemyDead_; }

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// 衝突応答
	void OnCollision(const Player* player);

	// 通常行動更新
	void BehaviorRootUpdate();

	// 死亡行動更新
	void BehaviorDethUpdate();

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 死亡行動初期化
	void BehaviorDethInitialize();

	// 衝突判定用関数
	bool CheckMapCollision(const Vector3& nextPosition);

	// 衝突判定を無効にするフラグのsetter
	void SetCollisionDisabled(bool isDisabled) { isCollisionDisabled_ = isDisabled; }

	// 衝突検出フラグのgetter
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	UpData* upData = nullptr;
private:
	// ワールド変換データ
	WorldTransform worldTransform_;

	// マップチップによるフィールドへのポインタ
	MapChipField* mapChipField_ = nullptr;

	// 振る舞い
	Behavior behavior_ = Behavior::kRoot;

	// 次の振る舞いリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 敵の移動方向 (左右)
	Direction direction_ = Direction::kLeft;

	// 敵の幅の半分 (衝突判定で使う)
	static inline const float kHalfWidth = 0.4f;

	// モデル
	Model* model_ = nullptr;


	Camera* camera_ = nullptr;

	// 速度
	Vector3 velocity_ = {};

	// 速度定数
	static inline const float kWalkSpeed = 0.02f;

	// アニメーション定数
	static inline const float kWalkMotionAngleStart = -10.0f;
	static inline const float kWalkMotionAngleEnd = 40.0f;
	static inline const float kWalkMotionTime = 1.0f;
	float walkTimer = 0.0f;

	// 当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	bool isEnemyDead_ = false;

	// デス演出用のタイマー
	float deathTimer_ = 0.0f;

	// デス演出時間 (例: 1.0秒)
	static inline const float kDeathMotionTime = 1.0f;

	// Y軸回転速度 (例: 10回転/秒)
	static inline const float kDeathRotationSpeedY = 10.0f;

	// X軸の最大回転角 (例: 180度, 演出でひっくり返すため)
	static inline const float kDeathRotationMaxX = 180.0f;

	// デス演出開始時のY軸回転角度を保持
	float initialRotationY_ = 0.0f;

	// 衝突判定を無効にするためのフラグ
	bool isCollisionDisabled_ = false;
};