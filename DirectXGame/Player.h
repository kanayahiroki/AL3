#pragma once
#include "KamataEngine.h"

class MapChipFiled;
    // ２移動量を加味して衝突判定する//
    // マップとの当たり判定情報
    struct CollisionMapInfo 
{
	bool ceiling = false;       // 天井衝突フラグ
	bool landing = false;       // 着地フラグ
	bool hitWal = false;        // 壁接触フラグ
	KamataEngine::Vector3 move; // 移動量
};

class Player {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine ::Vector3& position);
	// 更新
	void Update();
	// 描画
	void Draw();

	void SetMapChipFiled(MapChipFiled* mapChipFiled) { mapChipFiled_ = mapChipFiled; }
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
	static inline const float kGravityAcceleration = 0.1f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 1.0f;
	// ジャンプ初速（上方向
	static inline const float kJumpAcceleration = 1.0f;

	// 毎フレーム追従
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// 速度加算
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	void InputMove();

	// マップチップとの判定情報
	struct CollisionMapInfo {
		bool ceiline = false;       // 天井に接触しているか
		bool landing = false;       // 着地しているか
		bool hitWall = false;       // 壁に接触しているか
		KamataEngine::Vector3 move; // 移動量
	};

	void CheckMapCollision(CollisionMapInfo& info);

	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);

	// ③判定結果を反映させて移動させる
	void CheckMapMove(const CollisionMapInfo& info);
	// ④天井に接触している場合の処理
	void CheckMapCeiling(const CollisionMapInfo& info);
	// ⑦旋回制御
	// void AnimateTurn();

	// 角
	enum Corner {
		kLeftTop,
		kLeftBottom,
		kRightTop,
		kRightBottom,

		kNumCorner, // 角の数
	};

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	static inline const float kBlank = 0.1f;

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::Vector3 velocity_ = {};

	LRDirection lrDirection_ = LRDirection::kRight;

	// マップチップによるフィールド
	MapChipFiled* mapChipFiled_ = nullptr;
};
