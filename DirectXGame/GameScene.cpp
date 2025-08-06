#include "GameScene.h"
#include "MyMath.h"

using namespace KamataEngine;
// 初期化/////////////////////////////////////////////////////////////
void GameScene::Initialize() {

	model_ = KamataEngine::Model::Create();

	modelBlock_ = Model::CreateFromOBJ("block");

	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	modelPlayer_ = Model::CreateFromOBJ("player");

	modelEnemy_ = Model::CreateFromOBJ("enemy");

	worldTransform_.Initialize();

	// 自キャラの生成
	player_ = new Player();
	// 自キャラの初期化
	//
	//
	//
	// player_->Initialize(modelPlayer_, &camera_,playerPosition);

	// 生成
	skydome_ = new Skydome();
	// 初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// 要素数
	// const uint32_t kNumBlockVirtical = 10;
	// const uint32_t kNumBlockHorizontal = 20;
	//// ブロック一個分の横幅
	// const float kBlockWidth = 2.0f;
	// const float kBlockHeigth = 2.0f;
	//  要素数を変更する
	// worldTransformBlocks_.resize(kNumBlockVirtical);
	// for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
	//	worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	// }

	//// キューブの生成
	// for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
	//	for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
	//		if ((i + j) % 2 == 0)
	//			continue;
	//		worldTransformBlocks_[i][j] = new WorldTransform();
	//		worldTransformBlocks_[i][j]->Initialize();
	//		worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
	//		worldTransformBlocks_[i][j]->translation_.y = kBlockHeigth * i;
	//	}
	// }
	debugCamera_ = new DebugCamera(1280, 720);

	mapChipFiled_ = new MapChipFiled;

	mapChipFiled_->LoadMapChipCsv("Resources/blocks.csv");

	GenerateBlocks();

	// 座標をマップ地プ番号で指定
	Vector3 playerPosition = mapChipFiled_->GetMapChipPositionByIndex(3, 18);
	player_->Initialize(modelPlayer_, &camera_, playerPosition);

	camera_.Initialize();

	cameraController_ = new CameraController();

	cameraController_->Initialize();

	cameraController_->SetTarget(player_);

	cameraController_->Reset();

	// カメラ移動範囲
	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);

	// マップチップデータのセット
	// 自キャラの生成と初期化
	player_->SetMapChipFiled(mapChipFiled_);

	// 敵
	enemy_ = new Enemy();
	Vector3 enemyPosition = mapChipFiled_->GetMapChipPositionByIndex(15, 18);

	enemy_->Initialize(modelEnemy_, &camera_, enemyPosition);
}

// 更新/////////////////////////////////////////////////////////////////////////////////////
void GameScene::Update() {
	// 自キャラの更新
	player_->Update();
	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			// アフィン変換行列の作成
			worldTransformBlock->matWorld_ = MakeAffineMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			/*	worldTransformBlock->matWorld_ = アフィン変換行列　*/

			// 定数バッファに転送する
			worldTransformBlock->TransferMatrix();
		}
	}

	cameraController_->Update();
	debugCamera_->Update();
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_0)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif // _DEBUG
	if (isDebugCameraActive_) {
		// デバックカメラの更新
		debugCamera_->Update();
		// デバックカメラのビュー行列
		camera_.matView = debugCamera_->GetCamera().matView;
		// デバックカメラのプロジェクション行列
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	} else {
		// ビュープロジェクション行列の更新と転送
		camera_.matView = cameraController_->GetViewProjection().matView;
		camera_.matProjection = cameraController_->GetViewProjection().matProjection;
		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();
	}

	skydome_->Update();

	enemy_->Update();
}
// 描画/////////////////////////////////////////////////////////////////////////////////
void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	// player_->Draw();
	//  ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	skydome_->Draw();
	player_->Draw();
	enemy_->Draw();

	Model::PostDraw();
}
// デストラクタ////////////////////////////////////////////////////////////////////////////////
GameScene::~GameScene() {
	delete model_;
	// 自キャラの解放
	delete player_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	delete debugCamera_;

	delete skydome_;

	delete modelSkydome_;

	delete mapChipFiled_;

	delete cameraController_;

	delete enemy_;
}

void GameScene::GenerateBlocks() {
	uint32_t numBlockVirtical = mapChipFiled_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipFiled_->GetNumBlockHorizontal();
	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(numBlockVirtical);

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		// 一列の要素数を設定（横方向のブロック数）
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (UINT32 j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipFiled_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipFiled_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}