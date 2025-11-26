#pragma once
#include "KamataEngine.h"
class HitEffect 
{
public:
	void Initialize();
	void Update();
	void Draw();

private:
	//モデル(借りてくる用)
	static KamataEngine::Model* model_;
	// カメラ(借りてくる用)
	static KamataEngine::Camera* camera_;
};