#include "Player.h"
#include "GameScene.h"
#include <KamataEngine.h>
#include "Windows.h"
#include "assert.h"
using namespace KamataEngine;

Player::~Player() {}

void Player::Initialize(
	KamataEngine::Model* model, uint32_t textureHandle, KamataEngine::Camera* camera) 
{
	assert(model);
	worldTransform_.Initialize();
	model_ = KamataEngine::Model::Create();
	textureHandle_ = textureHandle;
	camera_ = camera;
}

void Player::Update() { worldTransform_.TransferMatrix(); }

void Player::Draw() 
{ 
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());
	
	// 3Dモデル描画 
	model_->Draw(worldTransform_, *camera_, textureHandle_);
	// 3Dモデル描画後処理
	Model::PostDraw();
}

