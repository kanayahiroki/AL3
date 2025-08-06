#include "Enemy.h"
#include "MapChipFiled.h"

#include <algorithm>
#include <numbers>

using namespace KamataEngine;
using namespace MathUtility;

// 初期化
void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, KamataEngine::Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 因数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	worldTransform_.translation_ = position;

	// 初期回転
	worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};

	// アニメーション処理用の初期化
	walkTimer_ = 0.0f;
}

// 更新
void Enemy::Update() {

	// 移動
	worldTransform_.translation_.x += velocity_.x;

	// タイマーを加算
	walkTimer_ += 5.0f / 60.0f;

	// 回転アニメーション
	worldTransform_.rotation_.x = std::sin(walkTimer_);

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	worldTransform_.TransferMatrix();
}
// 描画
void Enemy::Draw() { model_->Draw(worldTransform_, *camera_); }

KamataEngine::Vector3 Enemy::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player) { (void)player; }