#include "GameScene.h"
#include <KamataEngine.h>
#include <Windows.h>

using namespace KamataEngine;
// Windows�A�v���ł̃G���g���[�|�C���g(main�֐�)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// �G���W���̏�����
	KamataEngine::Initialize(L"LE2D_09_�J�i��_�q���L_AL3");

	// �Q�[���V�[���̃C���X�^���X�쐬
	GameScene* gameScene = new GameScene();

	// �Q�[���V�[���̏�����
	gameScene->initialize();

	// ���C�����[�v
	while (true) {
		// �G���W���̍X�V
		if (KamataEngine::Update()) {
			break;
		}
		// DirexCommon�C���X�^���X�̎擾
		DirectXCommon* dxCommon = DirectXCommon::GetInstance();

		// �Q�[���V�[���̍X�V
		gameScene->Update();

		// �`��J�n
		dxCommon->PreDraw();

		// �Q�[���V�[���̕`��
		gameScene->Draw();

		// �`��I��
		dxCommon->PostDraw();
	}
	// �Q�[���V�[���̉��
	delete gameScene;

	// nullptr�̑��
	gameScene = nullptr;
	// �G���W���̏I������
	KamataEngine::Finalize();
	return 0;
}
