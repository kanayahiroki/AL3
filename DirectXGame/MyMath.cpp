#include "MyMath.h"
using namespace KamataEngine;
using namespace MathUtility;
#include <cmath>
#include <numbers>

KamataEngine::Matrix4x4 MakeAffineMatrix(KamataEngine::Vector3& scale, KamataEngine::Vector3& rotaion, KamataEngine::Vector3& translation) {
	// スケーリング行列の作成
	Matrix4x4 matScale = MakeScaleMatrix(scale);

	// 回転行列の作成
	Matrix4x4 matRotX = MakeRotateXMatrix(rotaion.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotaion.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotaion.z);
	Matrix4x4 matRot = matRotZ * matRotX * matRotY;

	// 平行移動行列の作成
	Matrix4x4 matTrans = MakeTranslateMatrix(translation);

	// スケーリング、回転、平行移動の合成
	Matrix4x4 matWorld = matScale * matRot * matTrans;

	return matWorld;
}

float EaseInOut(float x1, float x2, float t) {

	float easedT = -(std::cosf(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f;

	return Lerp(x1, x2, easedT);
}

bool IsCollision(const AABB& aabb1, const AABB& aabb2) {

	return (aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x) && // x軸
	       (aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y) && // y軸
	       (aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);   // z軸
}