#pragma once
#include "KamataEngine.h"
#include "MapChipFiled.h"
#include "MyMath.h"
#include "UpData.h"

using namespace KamataEngine;

// 02_10 20枚目
class Player;

/// <summary>
/// 敵
/// </summary>
class Enemy {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model_"></param>
	/// <param name="camera_"></param>
	/// <param name="position"></param>
	/// 
	
	// 振る舞い
	enum class Behavior { 
		kRoot, kDeth, kUnknown };


	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void UpDate();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	// 02_10 スライド14枚目
	AABB GetAABB();

	/// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// 02_12 11枚目 デスフラグ
	bool IsEnemyDead() const { return isEnemyDead_; }
	// 02_10 スライド14枚目 ワールド座標を取得
	Vector3 GetWorldPosition();
	// 02_10 スライド20枚目 衝突応答
	void OnCollision(const Player* player);

	// 通常行動更新
	void BehaviorRootUpdate();

	// 攻撃行動更新
	void BehaviorDethUpdate();

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動初期化
	void BehaviorDethInitialize();

	// 衝突判定用関数
	bool CheckMapCollision(const Vector3& nextPosition);


	// 衝突検出フラグのgetter
	bool IsCollisionDetected() const { return isCollisionDetected_; }

	// 衝突検出フラグのsetter
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	// 衝突判定を無効にするフラグのsetter
	void SetCollisionDisabled(bool isDisabled) { isCollisionDisabled_ = isDisabled; }

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
	enum class Direction {
		kRight,
		kLeft,
	};
	Direction direction_ = Direction::kLeft;

	// 敵の幅の半分 (衝突判定で使う)
	static inline const float kHalfWidth = 0.4f;
	

	

	// モデル
	Model* model_ = nullptr;

	UpData* upData = nullptr;

	// テクスチャハンドル
	//  uint32_t textureHandle_ = 0u;

	Camera* camera_ = nullptr;

	// 02_09 15枚目
	static inline const float kWalkSpeed = 0.02f;
	// 02_09 15枚目
	Vector3 velocity_ = {};

	// 02_09 19枚目
	// 最初の角度
	static inline const float kWalkMotionAngleStart = -10.0f;

	// 02_09 19枚目
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = 40.0f;

	// 02_09 19枚目
	static inline const float kWalkMotionTime = 1.0f;
	// 02_09 20枚目
	float walkTimer = 0.0f;

	// 02_10 14枚目 当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	bool isEnemyDead_ = false;

	// デス演出用のタイマー
	float deathTimer_ = 0.0f;

	// デス演出時間 (例: 1.0秒)
	static inline const float kDeathMotionTime = 1.0f;

	// Y軸回転速度 (例: 1周/0.1秒 = 10回転/秒)
	static inline const float kDeathRotationSpeedY = 10.0f;

	// X軸の最大回転角 (例: 180度, 演出でひっくり返すため)
	static inline const float kDeathRotationMaxX = 180.0f;

	// デス演出開始時のY軸回転角度を保持
	float initialRotationY_ = 0.0f;

	// 衝突検出フラグ
	bool isCollisionDetected_ = false;

	// 衝突判定を無効にするためのフラグ
	bool isCollisionDisabled_ = false;

};