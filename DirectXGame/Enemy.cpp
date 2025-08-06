#include "Enemy.h"
#include "MapChipFiled.h"
#include "MyMath.h"
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