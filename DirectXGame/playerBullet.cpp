#include "PlayerBullet.h"
#include "Enemy.h" // OnCollision で使用
#include <cassert>

using namespace KamataEngine;

void PlayerBullet::Initialize(const Vector3& position, const Vector3& velocity) {
	// 弾用のモデルは別途用意が必要です。ここでは仮に共通のモデルを使用
	// 実際には、ResourceLoaderなどで弾専用のモデルをロードする必要があります
	// model_ = ResourceLoader::GetInstance()->GetBulletModel();
	// 仮で、Initializeでモデルを取得する処理（もしあれば）
	// 現状の環境に合わせて model_ の取得/設定を行ってください

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	this->velocity_ = velocity;

	// 弾は小さいのでスケールを設定
	worldTransform_.scale_ = {kRadius * 2.0f, kRadius * 2.0f, kRadius * 2.0f};

	// 寿命タイマーをリセット
	lifeTimer_ = kLifeTime;

	// 行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
}

void PlayerBullet::Update() {
	// 寿命が尽きたらデスフラグを立てる
	if (--lifeTimer_ <= 0) {
		isDead_ = true;
		return;
	}

	// 移動
	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);

	// ワールド行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
}

void PlayerBullet::Draw(Camera* camera) {
	if (model_) {
		model_->Draw(worldTransform_, *camera);
	}
}

AABB PlayerBullet::GetAABB() const {
	AABB aabb;
	const Vector3& worldPos = worldTransform_.translation_;

	// 弾は球体に近い想定で、ここではAABBを計算
	aabb.min = {worldPos.x - kRadius, worldPos.y - kRadius, worldPos.z - kRadius};
	aabb.max = {worldPos.x + kRadius, worldPos.y + kRadius, worldPos.z + kRadius};

	return aabb;
}

void PlayerBullet::OnCollision(const Enemy* enemy) {
	(void)enemy;

	// 衝突したら消滅する
	isDead_ = true;

	// ここで、敵にダメージを与えるなどの処理を実装する（Enemy::OnHit(damage) など）
}