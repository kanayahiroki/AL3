#pragma once
#include "KamataEngine.h"
#include "MyMath.h"
// 前方宣言
class MapChipFiled;
class Enemy;

// ２移動量を加味して衝突判定する//
// マップとの当たり判定情報
struct CollisionMapInfo {
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

	static inline const float kAcceleration = 0.02f; // 加速度
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
	// 旋回時間<秒>
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

	// マップチップSetter
	void SetMapChipField(MapChipFiled* mapChipFiled) { mapChipFiled_ = mapChipFiled; }

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	// 衝突応答
	void OnCollision(const Enemy* enemy);

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

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// ①移動入力
	void InputMove();

	// ②マップ衝突判定
	void CheckMapCollision(CollisionMapInfo& info);
	// マップ衝突判定上
	void CheckMapCollisionUp(CollisionMapInfo& info);

	// マップ衝突判定下
	void CheckMapCollisionDown(CollisionMapInfo& info);

	// マップ衝突判定右
	void CheckMapCollisionRight(CollisionMapInfo& info);

	// マップ衝突判定左
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	// ③判断結果を反映して移動させる
	void CheckMapMove(const CollisionMapInfo& info);

	// ④天井に接着している場合の処理
	void CheckMapCeiling(const CollisionMapInfo& info);

	// ⑤壁に接着している場合の処理
	void CheckMapWall(const CollisionMapInfo& info);

	// ⑥接地状態の切り替え
	void CheckMapLanding(const CollisionMapInfo& info);

	// ⑦旋回制御
	void AnimateTurn();

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     //
		kNumCorner    // 要素数
	};

	// 指定した角の座標計算
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// 隙間
	static inline const float kBlank = 0.1f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.5f;

	// 微小な数値
	static inline const float kGroundSearchHeight = 0.1f;

	// 壁の速度減衰率
	static inline const float kAttenuationWall = 0.5f;
};