#include "ShieldEnemy.h"
#include "KamataEngine.h"
#include "MapChipFiled.h"
#include "MyMath.h"
#include "Player.h"
#include "UpData.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

// 敵の進行方向の壁との衝突をチェック
bool ShieldEnemy::CheckMapCollision(const Vector3& nextPosition) {
	if (!mapChipField_) {
		return false; // マップが設定されていなければチェックしない
	}

	// 次のフレームの移動後の位置をベースにする
	Vector3 checkPos = nextPosition;

	// 進行方向に基づいてチェックする角のオフセットを決定
	// (ここでは、敵の幅 kWidth/2 を考慮して外側の角を見る)
	float offsetX = (direction_ == Direction::kRight) ? kHalfWidth : -kHalfWidth;

	// 衝突判定を行うための2点 (進行方向側の上下の角)
	Vector3 cornerTop = checkPos + Vector3(offsetX, kHeight / 2.0f, 0.0f);
	Vector3 cornerBottom = checkPos + Vector3(offsetX, -kHeight / 2.0f, 0.0f);

	// 1. 上側の角のマップインデックスを取得し、ブロックかチェック
	MapChipField::IndexSet indexSetTop = mapChipField_->GetMapChipIndexSetByPosition(cornerTop);
	MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSetTop.xIndex, indexSetTop.yIndex);

	// 2. 下側の角のマップインデックスを取得し、ブロックかチェック
	MapChipField::IndexSet indexSetBottom = mapChipField_->GetMapChipIndexSetByPosition(cornerBottom);
	MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSetBottom.xIndex, indexSetBottom.yIndex);

	// 上下どちらかの角がブロックに触れていれば衝突と判定
	if (typeTop == MapChipType::kBlock || typeBottom == MapChipType::kBlock) {
		return true;
	}

	return false;
}

void ShieldEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model);

	// 02_09 7枚目
	model_ = model;
	// 02_09 7枚目
	camera_ = camera;
	// 02_09 7枚目
	//worldTransform_.Initialize();

	// 代わりに WorldTransform のメンバを手動で初期化
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};


	worldTransform_.translation_ = position;
	// 02_09 7枚目 角度調整
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	// 02_09 16枚目
	velocity_ = {-kWalkSpeed, 0, 0};
	// 02_09 20枚目
	walkTimer = 0.0f;
}

// 02_09 スライド5枚目
void ShieldEnemy::UpDate() {

	switch (behavior_) {
	case Behavior::kRoot:
		BehaviorRootUpdate(); // 現在のUpDateロジックをここに移動
		break;
	case Behavior::kDeth:
		BehaviorDethUpdate();
		break;
	}

	// 02_09 20枚目
	walkTimer += 1.0f / 60.0f;

	// 02_09 23枚目 回転アニメーション
	// worldTransform_.rotation_.x = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);

	float param = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);

	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;

	worldTransform_.rotation_.x = degree * (std::numbers::pi_v<float> / 180.0f);

	// 02_09 スライド8枚目 ワールド行列更新
	upData->WorldTransformUpData(worldTransform_);
}

// 02_09 スライド5枚目
void ShieldEnemy::Draw() {
	// 02_09 スライド9枚目  モデル描画
	model_->Draw(worldTransform_, *camera_);
}

// 02_10 スライド14枚目
AABB ShieldEnemy::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

// 02_10 21枚目
void ShieldEnemy::OnCollision(const Player* player) {
	(void)player;

	if (behavior_ == Behavior::kDeth) {
		// すでにデス状態なら何もしない
		return;
	}

	// プレイヤーが攻撃中なら敵が死ぬ
	if (player->IsAttack()) {
		// 1. プレイヤーと自分の位置関係を計算
		// (プレイヤーのX座標 - 自分のX座標)
		float diffX = const_cast<Player*>(player)->GetWorldPosition().x - worldTransform_.translation_.x;

		bool isHitFromBack = false;

		// 2. 自分の向きとプレイヤーの位置で「背後か」を判定
		if (direction_ == Direction::kRight) {
			// 自分が「右」を向いている時、プレイヤーが「左(マイナス)」にいれば背後
			if (diffX < 0)
				isHitFromBack = true;
		} else {
			// 自分が「左」を向いている時、プレイヤーが「右(プラス)」にいれば背後
			if (diffX > 0)
				isHitFromBack = true;
		}

		// 3. 背後からの攻撃なら死亡、正面なら耐える
		if (isHitFromBack) {
			behavior_ = Behavior::kDeth;
			isCollisionDisabled_ = true;
			BehaviorDethInitialize();
		}
	}
}

// 02_10 スライド14枚目
Vector3 ShieldEnemy::GetWorldPosition() {

	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void ShieldEnemy::BehaviorRootInitialize() {
	// 通常行動初期化処理
	// ここでは特に初期化する内容はないが、将来的に追加する可能性がある
}
void ShieldEnemy::BehaviorDethInitialize() {
	// 攻撃行動初期化処理
	// デスタイマーをリセット
	deathTimer_ = 0.0f;

	// 現在のY軸の回転角度を保存する
	initialRotationY_ = worldTransform_.rotation_.y;

	// X軸の歩行アニメーションをリセットする
	worldTransform_.rotation_.x = 0.0f;
}

void ShieldEnemy::BehaviorDethUpdate() {
	// 攻撃行動更新処理

	// デスタイマーを進行
	deathTimer_ += 1.0f / 60.0f;

	// 演出の進行度 (0.0 から 1.0 に向かう)
	float t = deathTimer_ / kDeathMotionTime;

	// ⭐︎ if 文で t を 1.0 に制限
	if (t > 1.0f) {
		t = 1.0f;
	}

	// Y軸回転 (継続的な回転)
	// Y軸は初期角度 + 演出による回転量を設定
	float addedYAngle = (kDeathRotationSpeedY * t * 360.0f) * (std::numbers::pi_v<float> / 180.0f);

	// ⭐︎ 初期角度 + 演出による回転量
	worldTransform_.rotation_.y = initialRotationY_ + addedYAngle;

	float xAngle = (kDeathRotationMaxX * t) * (std::numbers::pi_v<float> / 180.0f);

	worldTransform_.rotation_.x = xAngle;

	// 演出が一定時間経過したらデスフラグを立てる
	if (deathTimer_ >= kDeathMotionTime) {
		// 敵のワールド座標を最終的に更新したい場合はここで処理する (例: 穴に落ちるなど)

		// デスフラグを立てる (GameScene側で削除される)
		isEnemyDead_ = true;
	}
}

void ShieldEnemy::BehaviorRootUpdate() {
	// 通常行動更新処理

	// 次のフレームの移動後の位置を計算
	Vector3 nextPosition = worldTransform_.translation_ + velocity_;

	// 次のフレームの移動後の位置を計算
	bool hitWall = CheckMapCollision(nextPosition);

	if (hitWall) {
		// 壁に当たった場合の反転処理

		// 1. 速度を反転
		velocity_.x *= -1.0f;

		// 2. 向き (direction_) を反転
		if (direction_ == Direction::kLeft) {
			direction_ = Direction::kRight;
		} else {
			direction_ = Direction::kLeft;
		}

		// 3. モデルの回転角を新しい向きに合わせて調整
		// kRight (右) の場合は Y軸 90度 (π/2)
		// kLeft (左) の場合は Y軸 270度 (3π/2)
		worldTransform_.rotation_.y = (direction_ == Direction::kRight) ? std::numbers::pi_v<float> / 2.0f : std::numbers::pi_v<float> * 3.0f / 2.0f;
	}

	worldTransform_.translation_ += velocity_;
}