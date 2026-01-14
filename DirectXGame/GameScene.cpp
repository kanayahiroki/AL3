#include "GameScene.h"
#include "Enemy.h"
#include "Player.h"
#include "ShieldEnemy.h"

using namespace KamataEngine;

GameScene::~GameScene() {
	delete sprite_;

	delete debugCamera_;

	delete model_;

	delete blockModel_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}

	worldTransformBlocks_.clear();

	delete skydome_;

	delete modelSkydome_;

	delete mapChipField_;

	// 02_10 6枚目 敵クラス削除
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	// 02_11_17枚目
	delete deathParticles_;
	delete deathParticle_model_;

	delete modelClear_;
}

void GameScene::Initialize() {
	// ここにインゲームの初期化処理を書く
	// textureHandle_ = TextureManager::Load("player.png");

	////スプライトインスタンスの生成
	sprite_ = Sprite::Create(textureHandle_, {100, 50});

	model_ = Model::Create();

	worldTransform_.Initialize();

	// カメラの初期化
	camera_.Initialize();

	blockModel_ = Model::CreateFromOBJ("block");

	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	camera_.farZ = 1000.0f;

	// 02_03天球
	// skydome生成
	skydome_ = new Skydome();
	// 初期化
	modelSkydome_ = Model::CreateFromOBJ("skyDome", true);

	skydome_->Initialize(modelSkydome_, &camera_);

	mapChipField_ = new MapChipField;

	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	GenerateBlocks();

	// 自キャラの生成
	player_ = new Player();

	modelPlayer_ = Model::CreateFromOBJ("uma", true);

	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);

	player_->SetMapChipField(mapChipField_);

	// クリアモデルの生成
	modelClear_ = Model::CreateFromOBJ("clear", true); // OBJファイル名
	worldTransformClear_.Initialize();


	// 表示したい位置を設定（例：画面中央付近、カメラの少し前）
	worldTransformClear_.translation_ = {0.0f, 100.0f, 0.0f}; // 座標は調整してください
	worldTransformClear_.scale_ = {4.0f, 4.0f, 4.0f};

	modelGameOver_ = Model::CreateFromOBJ("gameOver", true); // OBJファイル名
	worldTransformGameOver_.Initialize();

	worldTransformGameOver_.translation_ = {0.0f, 100.0f, 0.0f};
	worldTransformGameOver_.scale_ = {4.0f, 4.0f, 4.0f}; // clearのscaleと合わせる

	// 自キャラの初期化
	player_->Initialize(modelPlayer_, &camera_, playerPosition);

	CController_ = new CameraController(); // 生成

	CController_->Initialize(&camera_); // 初期化

	CController_->SetTarget(player_); // 追従対象セット

	CController_->Reset(); // リセット

	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	CController_->SetMovableArea(cameraArea);

	// マップチップフィールドの生成と初期化
	// 自キャラの生成と初期化
	//  02_07 スライド5枚目

	// 02_09 10枚目 敵クラス
	// enemy_ = new Enemy();
	// 02_09 10枚目 敵モデル
	enemy_model_ = Model::CreateFromOBJ("neko");
	// 02_09 10枚目 敵位置決めて敵クラス初期化
	// Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(14, 18);
	// enemy_->Initialize(enemy_model_, &camera_, enemyPosition);

	// 02_10 5枚目（for文の中身全部）
	for (int32_t i = 0; i < 3; ++i) {
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(14 + i * 3, 18);

		newEnemy->Initialize(enemy_model_, &camera_, enemyPosition);

		// 敵は壁に当たると反転する
		newEnemy->SetMapChipField(mapChipField_);
		enemies_.push_back(newEnemy);
	}

	// shieldEnemy_model_ = Model::CreateFromOBJ("Shield");
	// for (int32_t i = 0; i < 3; ++i) {
	//	ShieldEnemy* newShieldEnemy = new ShieldEnemy();
	//
	//	Vector3 ShieldEnemyPosition = mapChipField_->GetMapChipPositionByIndex(15 + i * 3, 20);
	//
	//	newShieldEnemy->Initialize(shieldEnemy_model_, &camera_, ShieldEnemyPosition);
	//
	//	// 敵は壁に当たると反転する
	//	newShieldEnemy->SetMapChipField(mapChipField_);
	//	ShieldEnemies_.push_back(newShieldEnemy);
	// }

	// 02_11_16枚目 モデル読み込み
	deathParticle_model_ = Model::CreateFromOBJ("deathParticle");

	//// 02_11_16枚目 仮の生成処理 後で消す
	// deathParticles_ = new DeathParticles;
	// deathParticles_->Initialize(deathParticle_model_, &camera_, playerPosition);

	// 02_12 4枚目 ゲームプレイフェーズから開始
	phase_ = Phase::kFadeIn;

	// 02_13 27枚目
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);
}

void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:
		// 02_12 13枚目 if文から中身まで全部実装
		// Initialize関数のいきなりパーティクル発生処理は消す
		if (player_->IsDead()) {
			// 死亡演出
			phase_ = Phase::kDeath;

			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticle_model_, &camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:

		break;
	}
}

void GameScene::GenerateBlocks() {
	// 要素数
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する

	worldTransformBlocks_.resize(numBlockVirtical);

	// キューブの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::Update() {

#pragma endregion

	ChangePhase();

	switch (phase_) {

	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 0.01f);
			phase_ = Phase::kPlay;
		}

		skydome_->Update();
		CController_->Updata();
		//		worldTransformSkydome_.UpdateMatrix();
		//		cameraController->Update();

		// 自キャラの更新
		player_->UpDate();

		for (Enemy* enemy : enemies_) {
			enemy->UpDate();
		}

		// UpdateCamera();
#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_M)) {
			// フラグをトグル
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// UpdateBlocks();
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				upData->WorldTransformUpData(*worldTransformBlock);
			}
		}
		break;

	case Phase::kPlay:
		// ゲームプレイフェーズの処理

		//   skydome生成
		skydome_->Update();

		CController_->Updata();

		//  自キャラの更新
		player_->UpDate();

		for (Enemy* enemy : enemies_) {
			enemy->UpDate();
		}

		// --- 3. 敵の削除とクリア判定 (ここがポイント) ---
		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->IsEnemyDead()) {
				delete enemy;
				return true;
			}
			return false;
		});

		// 敵とシールド敵が両方いなくなったらクリア！
		if (enemies_.empty() && ShieldEnemies_.empty()) {
			isClear_ = true;
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeIn, 2.0f);

			worldTransformClear_.translation_ = player_->GetWorldPosition();
			worldTransformClear_.translation_.y +=7.0f;
			upData->WorldTransformUpData(worldTransformClear_);
		}

#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_M)) {
			// フラグをトグル
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送AL3_02_02*/
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送AL3_02_02*/

			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				// アフィン変換行列の生成
				upData->WorldTransformUpData(*worldTransformBlock);
			}
		}

		CheckAllCollisions();
		break;

	case Phase::kDeath:
		// デス演出フェーズ

		if (deathParticles_ && deathParticles_->IsFinished()) {
			isGameOver_ = true;
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeIn, 2.0f);

			worldTransformGameOver_.translation_ = player_->GetWorldPosition();
			worldTransformGameOver_.translation_.y += 7.0f;
			upData->WorldTransformUpData(worldTransformGameOver_);
		}

		if (isGameOver_) {
			gameOverTimer_ += 1.0f / 60.0f;
			float angle = (gameOverTimer_ / 2.0f) * 2.0f * std::numbers::pi_v<float>;

			worldTransformGameOver_.translation_.y += std::sin(angle) * 0.01f;

			upData->WorldTransformUpData(worldTransformGameOver_);
		}

		//    skydome生成
		skydome_->Update();
		CController_->Updata();

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->UpDate();
		}

		// 02_11 18枚目 デスパーティクルあれば更新
		if (deathParticles_) {
			deathParticles_->Update();
		}

		break;

	case Phase::kFadeOut:


		fade_->Update();

		// 【追加】ゲームオーバー演出
		if (isGameOver_) {
			gameOverTimer_ += 1.0f / 60.0f;
			float angle = gameOverTimer_ / 2.0f * 2.0f * std::numbers::pi_v<float>;
			worldTransformGameOver_.translation_.y += std::sin(angle) * 0.01f;
			upData->WorldTransformUpData(worldTransformGameOver_);
		}

		if (isClear_) {
			clearTimer_ += 1.0f / 60.0f;
			float angle = clearTimer_ / 2.0f * 2.0f * std::numbers::pi_v<float>;
			worldTransformClear_.translation_.y += std::sin(angle) * 0.01f;
			upData->WorldTransformUpData(worldTransformClear_);
		}
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		skydome_->Update();
		CController_->Updata();

		for (Enemy* enemy : enemies_) {
			enemy->UpDate();
		}

		break;
	}
}

void GameScene::Draw() {

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	if (!isClear_ && !isGameOver_) {
		if (!player_->IsDead()) {
			player_->Draw();
		}

		// ブロック描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				blockModel_->Draw(*worldTransformBlock, camera_);
			}
		}

		// 敵描画
		for (Enemy* enemy : enemies_) {
			enemy->Draw();
		}
	}

	// 天球描画
	skydome_->Draw();

	if (isClear_) {
		modelClear_->Draw(worldTransformClear_, camera_);
	}

	// 02_11 18枚目 デスパーティクルあれば描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}
	// 【追加】ゲームオーバーロゴ
	if (isGameOver_) {
		modelGameOver_->Draw(worldTransformGameOver_, camera_);
	}
	// クリアしていたら「CLEAR」モデルを描画
	if (isClear_) {
		modelClear_->Draw(worldTransformClear_, camera_);
	}

	Model::PostDraw();

	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	// スプライト描画後処理
	Sprite::PostDraw();

	// 02_13 28枚目
	fade_->Draw();
}

// 02_10 16枚目
void GameScene::CheckAllCollisions() {

	// 判定対象1と2の座標
	AABB aabb1, aabb2;

#pragma region 自キャラと敵キャラの当たり判定
	{
		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾全ての当たり判定
		for (Enemy* enemy : enemies_) {

			// ⭐︎ 衝突無効フラグのチェックを追加
			if (enemy->IsCollisionDisabled()) {
				continue; // コリジョン無効の敵はスキップ
			}

			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_->OnCollision(enemy);
				// 敵弾の衝突時コールバックを呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion
}