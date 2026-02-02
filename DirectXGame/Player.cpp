#define NOMINMAX
#include "Player.h"
#include "MapChipFiled.h"
#include "MyMath.h"
#include "UpData.h"
#include <algorithm>
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	if (position.x == 0.0f && position.y == 0.0f && position.z == 0.0f) {
		worldTransform_.translation_ = {0.0f, 0.0f, 5.0f};
	}
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

// --- 修正版 InputMove: 空中での左右移動入力を削除 ---
void Player::InputMove() {
	if (onGround_) {
		// 地上のみ：左右移動と向き更新
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				acceleration.x += kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				acceleration.x -= kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ = Add(velocity_, acceleration);
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			velocity_.x *= (1.0f - kAcceleration);
		}
	}

	if (std::abs(velocity_.x) <= 0.0001f)
		velocity_.x = 0.0f;

	// ジャンプ処理
	if (onGround_) {
		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			velocity_.y = kJumpAcceleration / 60.0f;
			jumpCount_ = 1;
			onGround_ = false;
		}
	} else {
		// 空中：重力のみ（左右入力は受け付けない）
		velocity_.y -= kGravityAcceleration / 60.0f;
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);

		if (Input::GetInstance()->TriggerKey(DIK_UP) && jumpCount_ < kMaxJumpCount) {
			velocity_.y = kJumpAcceleration / 60.0f;
			jumpCount_++;
		}
	}
}

// --- 修正版 BehaviorRootUpdate: 壁ジャン時に向きを即時反転 ---
void Player::BehaviorRootUpdate() {
	InputMove();

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	CheckMapCollision(collisionMapInfo);

	// 壁ジャンプ用の遊び判定
	CollisionMapInfo wallCheck = {};
	float sideBuffer = 0.1f;
	wallCheck.move.x = (lrDirection_ == LRDirection::kRight) ? sideBuffer : -sideBuffer;
	CheckMapCollision(wallCheck);

	if (!onGround_ && wallCheck.hitWall) {
		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			// 壁を蹴った瞬間に向きを反転！
			if (lrDirection_ == LRDirection::kRight) {
				velocity_.x = -kWallJumpXVelocity;
				lrDirection_ = LRDirection::kLeft;
			} else {
				velocity_.x = kWallJumpXVelocity;
				lrDirection_ = LRDirection::kRight;
			}
			velocity_.y = kWallJumpYVelocity;
			jumpCount_ = kMaxJumpCount;

			// 見た目の回転を開始
			turnFirstRotationY_ = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;

			collisionMapInfo.move = velocity_;
		}
	}

	worldTransform_.translation_ += collisionMapInfo.move;

	// 貫通防止
	CollisionMapInfo finalCheck = {};
	CheckMapCollision(finalCheck);
	worldTransform_.translation_ += finalCheck.move;

	if (collisionMapInfo.ceiling)
		velocity_.y = 0;
	UpdateOnWall(collisionMapInfo);
	UpdateOnGround(collisionMapInfo);

	// 旋回制御（Ease補間）
	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	upData->WorldTransformUpData(worldTransform_);

	// --- 落下死の判定 ---
	// マップの底が Y=0 だとしたら、少し余裕を持って -5.0f くらいに設定
	const float kFallDeathY = -5.0f;

	if (worldTransform_.translation_.y < kFallDeathY) {
		isDead_ = true;
	}

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		behaviorRequest_ = Behavior::kAttack;
	}
}

// --- 以下、既存のマップ判定等のロジック（変更なし） ---

void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	if (info.move.y <= 0)
		return;
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
	bool hit = false;
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		info.ceiling = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.move.y >= 0)
		return;
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
	bool hit = false;
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		info.landing = true;
	}
}

void Player::UpdateOnGround(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f)
			onGround_ = false;
		else {
			std::array<Vector3, kNumCorner> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}
			bool hit = false;
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0.0f, -kGroundSearchHeight, 0.0f));
			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
				hit = true;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0.0f, -kGroundSearchHeight, 0.0f));
			if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
				hit = true;
			if (!hit)
				onGround_ = false;
		}
	} else if (info.landing) {
		onGround_ = true;
		velocity_.x *= (1.0f - kAttenuationLanding);
		jumpCount_ = 0;
		velocity_.y = 0.0f;
	}
}

void Player::UpdateOnWall(const CollisionMapInfo& info) {
	if (info.hitWall)
		velocity_.x *= (1.0f - kAttenuationWall);
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	if (info.move.x < 0.0f)
		return;
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
	bool hit = false;
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::min(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		velocity_.x = 0.0f;
		info.hitWall = true;
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	if (info.move.x > 0.0f)
		return;
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}
	bool hit = false;
	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock)
		hit = true;

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::max(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
		velocity_.x = 0.0f;
		info.hitWall = true;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::UpDate() {
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;
		switch (behavior_) {
		case Behavior::kRoot:
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}
		behaviorRequest_ = Behavior::kUnknown;
	}
	switch (behavior_) {
	case Behavior::kRoot:
		BehaviorRootUpdate();
		break;
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}
}

void Player::Draw() {
	if (model_)
		model_->Draw(worldTransform_, *camera_);
}

Vector3 Player::GetWorldPosition() { return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]}; }

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	return {
	    {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
        {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
    };
}

AABB Player::GetAttackAABB() {
	Vector3 worldPos = GetWorldPosition();
	if (lrDirection_ == LRDirection::kRight) {
		return {
		    {worldPos.x + kWidth / 2.0f,        worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
            {worldPos.x + kWidth / 2.0f + 1.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
        };
	}
	return {
	    {worldPos.x - kWidth / 2.0f - 1.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
        {worldPos.x - kWidth / 2.0f,        worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
    };
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	isDead_ = true;
	if (IsAttack())
		isDead_ = false;
}

void Player::BehaviorAttackUpdate() {
	float attackSpeed = 0.2f;
	velocity_ = {0.0f, 0.0f, 0.0f};
	if (lrDirection_ == LRDirection::kRight)
		velocity_.x = attackSpeed;
	else
		velocity_.x = -attackSpeed;

	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;
	CheckMapCollision(collisionMapInfo);
	worldTransform_.translation_ += collisionMapInfo.move;
	UpdateOnWall(collisionMapInfo);

	float t = static_cast<float>(attackParametoer_) / 30.0f;
	worldTransform_.rotation_.z = Lerp(0.0f, std::numbers::pi_v<float> / 4.0f, std::sin(t * std::numbers::pi_v<float>));
	upData->WorldTransformUpData(worldTransform_);

	attackParametoer_++;
	if (attackParametoer_ > 30) {
		worldTransform_.rotation_.z = 0.0f;
		velocity_.x = 0.0f;
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::BehaviorRootInitialize() { worldTransform_.rotation_.z = 0.0f; }
void Player::BehaviorAttackInitialize() { attackParametoer_ = 0; }