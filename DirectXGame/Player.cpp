#define NOMINMAX
#include "Player.h"
#include "MapChipFiled.h"
#include <algorithm>
#include <numbers>

using namespace KamataEngine;

using namespace MathUtility;

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const Vector3& position) {
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
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::Update() {
	// １移動入力//
	InputMove();

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo_;

	// 移動量に速度の値をコピー
	collisionMapInfo_.move = velocity_;

	// ②移動量を加味して衝突判定する//
	// ②マップ衝突チェック
	CheckMapCollision(collisionMapInfo_);

	//// 移動
	// worldTransform_.translation_ += velocity_;

	// ③判断結果を反映して移動させる
	CheckMapMove(collisionMapInfo_);

	// ④天井に接着している場合の処理
	CheckMapCeiling(collisionMapInfo_);

	// ⑤壁に接着している場合の処理
	CheckMapWall(collisionMapInfo_);
	// ⑥接地状態の切り替え
	CheckMapLanding(collisionMapInfo_);

	//// 着地フラグ
	// bool landing = false;
	//// 地面との当たり判定
	//// 下降中?
	// if (velocity_.y < 0) {
	//	// Y座標が地面以下になったら着地
	//	if (worldTransform_.translation_.y <= 1.0f) {
	//		landing = true;
	//	}
	// }
	//// 接地判定
	// if (onGround_) {
	//	// ジャンプ開始
	//	if (velocity_.y > 0.0f) {
	//		// 空中状態に移行
	//		onGround_ = false;
	//	}
	// } else {
	//	// 着地
	//	if (landing) {
	//		// めり込み
	//		worldTransform_.translation_.y = 1.0f;
	//		// 摩擦で横方向速度が減哀する
	//		velocity_.x *= (1.0f - kAttenuation);
	//		// 下方向速度をリセット
	//		velocity_.y = 0.0f;
	//		// 接地状態に移行
	//		onGround_ = true;
	//	}
	// }

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// ⑦旋回制御
	AnimateTurn();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Player::Draw() { model_->Draw(worldTransform_, *camera_); }

Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	// ジャンプ開始（仮処理）
	velocity_ += Vector3(0, 1, 0);
}

void Player::InputMove() {
	if (onGround_) {

		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) ///////////////////////////
			{
				acceleration.x += kAcceleration;
				if (velocity_.x < 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					// 旋回開始時の角度を記録する
					turnFIrstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}

			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) ////////////////////
			{
				acceleration.x -= kAcceleration;
				//// 右移動中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向に入力中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					// 旋回開始時の角度を記録する
					turnFIrstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			}
			// velocity_.x -= kAcceleration;
			//  加速・減速
			velocity_ += acceleration;
			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			// 非入力時は移動減哀をかける
			velocity_.x *= (1.0f - kAttenuation);
		}

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ += Vector3(0, kJumpAcceleration, 0);
		}
		// 空中
	} else {
		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

// マップ衝突チェック
void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionUp(info);    // 上
	CheckMapCollisionDown(info);  // 下
	CheckMapCollisionRight(info); // 右
	CheckMapCollisionLeft(info);  // 左
}

// マップ衝突チェック　上//////////////////////////////////////////
void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.move.y <= 0) {
		return;
	}
	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 上方向判定
	MapChipType mapChipType;

	MapChipType mapChipTypeNext;

	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipFiled::IndexSet indexSet;
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左上点の判定
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
		// 現在座標が壁の外か判定
		MapChipFiled::IndexSet indexSetNow;
		indexSetNow = mapChipFiled_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipFiled::Rect rect = mapChipFiled_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank)); // y移動量を求める
			// 天井に当たったことを記録する
			info.ceiling = true;
		}
	}
}

/// マップ衝突チェック　下/////////////////////////////////////////////////////////////
void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	// 下降あり？
	if (info.move.y >= 0) {
		return;
	}
	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 上方向判定
	MapChipType mapChipType;

	MapChipType mapChipTypeNext;

	// 真下の当たり判定を行う
	bool hit = false;
	// 左下点の判定
	MapChipFiled::IndexSet indexSet;
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
		// 現在座標が壁の外か判定
		MapChipFiled::IndexSet indexSetNow;
		indexSetNow = mapChipFiled_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipFiled::Rect rect = mapChipFiled_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank)); // y移動量を求める
			// 床に当たったことを記録する
			info.landing = true;
		}
	}
}

// マップ衝突チェック　右//////////////////////////////////////////
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	// 右移動あり？
	if (info.move.x <= 0) {
		return;
	}
	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 上方向判定
	MapChipType mapChipType;

	MapChipType mapChipTypeNext;

	// 真右の当たり判定を行う
	bool hit = false;
	// 右上点の判定
	MapChipFiled::IndexSet indexSet;
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(+kWidth / 2.0f, 0, 0));
		// めり込み先ブロックの範囲矩形
		MapChipFiled::Rect rect = mapChipFiled_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank)); // x移動量を求める
		// 壁に当たったことを記録する
		info.hitWal = true;
	}
}

// マップ衝突チェック　左//////////////////////////////////////////
void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	// 左移動あり？
	if (info.move.x >= 0) {
		return;
	}
	// 移動後の４つの角の座標
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 上方向判定
	MapChipType mapChipType;

	MapChipType mapChipTypeNext;

	// 真左の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipFiled::IndexSet indexSet;
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左下点の判定
	indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(-kWidth / 2.0f, 0, 0));
		// めり込み先ブロックの範囲矩形
		MapChipFiled::Rect rect = mapChipFiled_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.move.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank)); // x移動量を求める
		// 壁に当たったことを記録する
		info.hitWal = true;
	}
}

// ③判断結果を反映して移動させる
void Player::CheckMapMove(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_ += info.move;
}

// ④天井に接着している場合の処理
void Player::CheckMapCeiling(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.ceiling) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

// ⑤壁に接着している場合の処理
void Player::CheckMapWall(const CollisionMapInfo& info) {
	// 壁接触による減速
	if (info.hitWal) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// ⑥接地状態の切り替え
void Player::CheckMapLanding(const CollisionMapInfo& info) {
	// 自キャラが接地状態？
	if (onGround_) {
		// 接地状態の処理
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 落下判定

			// 移動後の４つの角の座標
			std::array<Vector3, kNumCorner> positionsNew;
			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}

			// 上方向判定
			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;
			// 左下点の判定
			MapChipFiled::IndexSet indexSet;
			indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 右下点の判定
			indexSet = mapChipFiled_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kGroundSearchHeight, 0));
			mapChipType = mapChipFiled_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 落下なら空中状態に切り替え
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}

	} else {
		// 空中状態の処理
		if (info.landing) {
			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

// ⑦旋回制御
void Player::AnimateTurn() {
	if (turnTimer_ > 0.0f) {
		// 旋回タイマーを1/60秒だけカウントダウンする
		turnTimer_ = 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		// 状態に応じた角度を所得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFIrstRotationY_, turnTimer_ / kTimeTurn); // 旋回タイマーを使って角度補間
	}
}

KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0},
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0},
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0},
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}