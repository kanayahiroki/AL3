#pragma once
#include "KamataEngine.h"
#include "MyMath.h"

// 02_10 14枚目 Enemy.h と同様の構造
class Enemy;

class PlayerBullet {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(Camera* camera);

	/// <summary>
	/// デスフラグを取得
	/// </summary>
	bool IsDead() const { return isDead_; }

	/// <summary>
	/// 衝突応答
	/// </summary>
	void OnCollision(const Enemy* enemy);

	// AABBを取得
	AABB GetAABB() const;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	void SetModel(Model* model) { model_ = model; }

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;


	// 速度
	Vector3 velocity_ = {};

	// デスフラグ
	bool isDead_ = false;

	// 弾の当たり判定サイズ
	static inline const float kRadius = 0.2f;

	// 弾の寿命(フレーム単位)
	static inline const int32_t kLifeTime = 60 * 2; // 2秒

	// 寿命タイマー
	int32_t lifeTimer_ = kLifeTime;
};

