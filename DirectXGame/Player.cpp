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

void Player::InputMove() {
	// 1. 左右の移動入力
	if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
		Vector3 acceleration = {};

		// 地上なら自由に動ける
		bool canControl = onGround_;

		// ★空中での特別許可
		if (!onGround_) {
			// A. 壁に接触している時の離脱入力を許可
			if (Input::GetInstance()->PushKey(DIK_RIGHT) && velocity_.x <= 0.01f)
				canControl = true;
			if (Input::GetInstance()->PushKey(DIK_LEFT) && velocity_.x >= -0.01f)
				canControl = true;

			// B. 【重要】壁ジャンプ直後など、既に一定以上の速度がある場合は
			// 下手に上書きせず、その勢いを維持させる（入力を受け付けないことで速度を保つ）
			if (std::abs(velocity_.x) > kLimitRunSpeed * 0.5f) {
				canControl = false;
			}
		}

		if (canControl) {
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (velocity_.x < 0.0f)
					velocity_.x *= (1.0f - kAttenuation);
				acceleration.x += kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f)
					velocity_.x *= (1.0f - kAttenuation);
				acceleration.x -= kAcceleration / 60.0f;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			velocity_ = Add(velocity_, acceleration);
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		}
	} else {
		// ★非入力時の減衰を「地上のみ」にする
		// これにより、壁ジャンプした後の慣性が空中で死ななくなります
		if (onGround_) {
			velocity_.x *= (1.0f - kAcceleration);
		}
	}

	if (std::abs(velocity_.x) <= 0.0001f)
		velocity_.x = 0.0f;

	// 2. ジャンプ・落下処理
	if (onGround_) {
		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			velocity_.y = kJumpAcceleration / 60.0f;
			jumpCount_ = 1;
			onGround_ = false;
		}
	} else {
		velocity_ = Add(Vector3(0, -kGravityAcceleration / 60.0f, 0), velocity_);
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);

		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			if (jumpCount_ < kMaxJumpCount) {
				velocity_.y = kJumpAcceleration / 60.0f;
				jumpCount_++;
			}
		}
	}
}

// 02_07 スライド13枚目
void Player::CheckMapCollision(CollisionMapInfo& info) {

	CheckMapCollisionUp(info);
	CheckMapCollisionDown(info);
	CheckMapCollisionRight(info);
	CheckMapCollisionLeft(info);
}

// マップ衝突判定上方向
void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.move.y <= 0) {
		return;
	}
	// 移動後の4つの角の計算
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, +kHeight / 2.0f, 0));
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, +kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
			info.ceiling = true;
		}
	}
}

// マップ衝突判定下方向
void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	// 下降あり？
	if (info.move.y >= 0) {
		return;
	}

	// 移動後の4つの角の計算
	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// 隣接セルがともにブロックであればヒット
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.move + Vector3(0, -kHeight / 2.0f, 0));
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + Vector3(0, -kHeight / 2.0f, 0));
		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
			// 地面に当たったことを記録する
			info.landing = true;
		}
	}
}

void Player::UpdateOnGround(const CollisionMapInfo& info) {

	// 自キャラが接地状態？
	if (onGround_) {

		// ジャンプ開始(スライド18）
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {

			// 移動後の4つの角の座標(スライド19）
			std::array<Vector3, kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
			}
			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;
			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0.0f, -kGroundSearchHeight, 0.0f));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {

				hit = true;
			}

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0.0f, -kGroundSearchHeight, 0.0f));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {

				hit = true;
			}

			// 落下開始
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}

	} else {
		// 着地フラグ
		//  接地フラグ(スライド16）
		if (info.landing) {
			// 接地状態に切り替える（落下を止める）
			onGround_ = true;
			// 接地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);

			jumpCount_ = 0; // 着地したのでリセット！

			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

// 02_08スライド27枚目 壁接地中の処理
void Player::UpdateOnWall(const CollisionMapInfo& info) {

	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// マップ衝突判定右方向
void Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	// 右移動がない、かつ finalCheck(move.x=0) でもない場合は抜ける
	if (info.move.x < 0.0f) {
		return;
	}

	// 移動後の4つの角の計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 右の当たり判定を行う
	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 右上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		hit = true;
	}
	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込み先ブロックの範囲矩形を取得
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// 自キャラの右端をブロックの左端に揃える（めり込みを排除）
		float distanceToWall = rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank);
		info.move.x = std::min(0.0f, distanceToWall);
		velocity_.x = 0.0f;

		info.hitWall = true;
	}
}

// マップチップ衝突判定左方向
void Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	// 左移動がない、かつ finalCheck(move.x=0) でもない場合は抜ける
	if (info.move.x > 0.0f) {
		return;
	}

	// 移動後の4つの角の計算
	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.move, static_cast<Corner>(i));
	}

	// 左の当たり判定を行う
	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 左上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		hit = true;
	}
	// 左下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込み先ブロックの範囲矩形を取得
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		// 自キャラの左端をブロックの右端に揃える（めり込みを排除）
		float distanceToWall = rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank);
		info.move.x = std::min(0.0f, distanceToWall);
		velocity_.x = 0.0f;

		info.hitWall = true;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {

	Vector3 offsetTable[] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, //  kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  //  kLeftTop
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::UpDate() {

	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞いを切り替える
		behavior_ = behaviorRequest_;
		// 各振る舞いごとの初期化を実行
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}
		// 振る舞いリクエストをクリア
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	// 通常行動
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
	// 攻撃行動
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}

}
//// アフィン変換行列の生成
// worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

//// 定数バッファに転送する
// worldTransform_.TransferMatrix();

void Player::Draw() {
	if (model_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

// 02_10 10枚目
Vector3 Player::GetWorldPosition() {

	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

// 02_10 14枚目
AABB Player::GetAABB() {

	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

AABB Player::GetAttackAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	// 攻撃判定をプレイヤーの現在の向きに応じて調整
	if (lrDirection_ == LRDirection::kRight) {
		// 右向きの場合: プレイヤーの右側に攻撃範囲を広げる
		aabb.min = {worldPos.x + kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
		aabb.max = {worldPos.x + kWidth / 2.0f + 1.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}; // 幅を1.0f広げる
	} else {
		// 左向きの場合: プレイヤーの左側に攻撃範囲を広げる
		aabb.min = {worldPos.x - kWidth / 2.0f - 1.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f}; // 幅を1.0f広げる
		aabb.max = {worldPos.x - kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};
	}

	return aabb;
	;
}

// 02_10 21枚目
void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;


	// 02_12 12枚目 書き換え
	isDead_ = true;

	if (IsAttack()) 
	{
		// 敵を倒した処理
		isDead_ = false; // 仮で死なないようにする
		return;
	}
}

void Player::BehaviorRootUpdate() {
	// 移動入力
	InputMove();

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_;

	// 1. 通常のマップ衝突チェック（これで現在のフレームの移動を制限）
	CheckMapCollision(collisionMapInfo);

	// --- 壁ジャンプ用の「遊び」判定 ---
	CollisionMapInfo wallCheck = {};
	// 左右に少しだけ(0.1f)判定を広げて、壁の近くにいるか調べる
	float sideBuffer = 0.1f;
	// 右を向いていたら右を、左を向いていたら左の壁をチェック
	wallCheck.move.x = (lrDirection_ == LRDirection::kRight) ? sideBuffer : -sideBuffer;
	CheckMapCollision(wallCheck);

	// 【重要】wallCheck.hitWall を使って判定する
	if (!onGround_ && wallCheck.hitWall) {
		// 壁際にいるときにジャンプボタンを押したら
		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			// 2. 速度の書き換え（重複を削除して整理）
			if (lrDirection_ == LRDirection::kRight) {
				// 右を向いて壁に当たっているなら、左へ跳ぶ
				velocity_.x = -kWallJumpXVelocity;
				lrDirection_ = LRDirection::kLeft;
				worldTransform_.translation_.x -= kBlank * 2.0f; // 強制脱出
			} else {
				// 左を向いて壁に当たっているなら、右へ跳ぶ
				velocity_.x = kWallJumpXVelocity;
				lrDirection_ = LRDirection::kRight;
				worldTransform_.translation_.x += kBlank * 2.0f; // 強制脱出
			}
			velocity_.y = kWallJumpYVelocity;

			jumpCount_ = 1;

			// 壁ジャンプした瞬間の移動量を反映
			collisionMapInfo.move = velocity_;

			// 旋回アニメーションの開始設定
			turnFirstRotationY_ = worldTransform_.rotation_.y;
			turnTimer_ = kTimeTurn;
		}
	}

	// 3. 座標の更新（実際の移動）
	worldTransform_.translation_ += collisionMapInfo.move;

	// 4. 貫通防止の最終チェック（移動後の座標が壁の中なら押し戻す）
	CollisionMapInfo finalCheck = {};
	finalCheck.move = {0, 0, 0};
	CheckMapCollision(finalCheck);
	worldTransform_.translation_ += finalCheck.move;

	// 天井接触による落下開始
	if (collisionMapInfo.ceiling) {
		velocity_.y = 0;
	}

	// 壁接触中の摩擦処理
	UpdateOnWall(collisionMapInfo);

	// 壁ずり落ち（wallCheckの結果も参考にするとより安定します）
	if (!onGround_ && wallCheck.hitWall && velocity_.y < 0.0f) {
		if (velocity_.y < -kWallSlideSpeed) {
			velocity_.y = -kWallSlideSpeed;
		}
	}

	// 接地判定
	UpdateOnGround(collisionMapInfo);

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		worldTransform_.rotation_.y = EaseInOut(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	upData->WorldTransformUpData(worldTransform_);

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		behaviorRequest_ = Behavior::kAttack;
	}
}

void Player::BehaviorAttackUpdate() {
	
	//攻撃中の前進速度の設定
	
	// 攻撃中の移動速度を直接設定（向きに応じて）
	float attackSpeed = 0.2f; // 攻撃中の推進力

	// 攻撃中は、他の入力（重力など）を無視して強制的に速度を設定
	velocity_ = {0.0f, 0.0f, 0.0f};

	if (lrDirection_ == LRDirection::kRight) {
		velocity_.x = attackSpeed;
	} else { // kLeft
		velocity_.x = -attackSpeed;
	}

	
	// 2. 衝突判定と移動のコアロジックを再利用

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_; // 現在設定した速度（移動量）を使用

	// マップ衝突チェック（ここで壁に当たれば move が補正される）
	CheckMapCollision(collisionMapInfo);

	// 移動
	worldTransform_.translation_ += collisionMapInfo.move;

	// 壁接触中の処理（速度を減衰させるなど）
	UpdateOnWall(collisionMapInfo);

	
	// 攻撃中の見た目の変化 (回転)
	float t = static_cast<float>(attackParametoer_) / 30.0f;
	worldTransform_.rotation_.z = Lerp(0.0f, std::numbers::pi_v<float> / 4.0f, std::sin(t * std::numbers::pi_v<float>));

	
	// ワールド行列更新とカウンター
	upData->WorldTransformUpData(worldTransform_);

	attackParametoer_++;

	// 既定の時間が経過したら通常モードへ
	if (attackParametoer_ > 30){


			worldTransform_.rotation_.z = 0.0f;
		// 2. !!! 攻撃によって設定されたX軸方向の速度をリセットする !!!
		velocity_.x = 0.0f;
			behaviorRequest_ = Behavior::kRoot;
		}

}

void Player::BehaviorRootInitialize() 
{
	worldTransform_.rotation_.z = 0.0f; // 攻撃終了後の回転をリセット
}

void Player::BehaviorAttackInitialize() 
{ 
	attackParametoer_ = 0; 
	
	
}
