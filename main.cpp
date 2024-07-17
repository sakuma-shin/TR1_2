#include <Novice.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <string.h>
#include <assert.h>
#include "easing.h"
#include"Calculation.h"
#include"Hit.h"
#include"imgui.h"


const int kColmunWidth = 60;
const int kColmunHeight = 50;
const int kWindowWidth = 1280;
const int kWindowHeight = 720;
const int bulletMax = 16;
const int enemyNum = 4;

struct Player {
	Vector2 pos;
	Vector2 velocity;
	float width;
	float height;
	Corners corners;
	int isBulletShot;
	unsigned int color;
	int bulletCoolTime;
};

struct Bullet {
	//基本設定
	Vector2 pos[bulletMax];
	float radius;
	float speed;
	float posRad[bulletMax];
	float playerToBullet;
	unsigned int color;
	Vector2 memoryPos;
	float rad;
};

struct Enemy {
	Vector2 pos[enemyNum];
	float radius;
	unsigned int color;
	int isAlive[enemyNum];
};

void DrawCircle(Vector2 pos, float radiusX, float radiusY, unsigned int color) {
	Novice::DrawEllipse(
		int(pos.x), int(pos.y),
		int(radiusX), int(radiusY),
		0.0f, color, kFillModeSolid
	);
}

void DrawQuad(Corners a, int imageGH, int imageWidth, int imageHeight, unsigned int color) {

	Novice::DrawQuad(
		int(a.leftTop.x), int(a.leftTop.y),
		int(a.rightTop.x), int(a.rightTop.y),
		int(a.leftDown.x), int(a.leftDown.y),
		int(a.rightDown.x), int(a.rightDown.y),
		0, 0, imageWidth, imageHeight,
		imageGH, color
	);
}


void move(Vector2& pos, char keys[256], Vector2 velocity) {
	Vector2 speed;
	Vector2 moveSpeed;
	float theta = 0.0f;


	if (keys[DIK_UP] || keys[DIK_W]) {
		theta = (1.0f / 2.0f) * float(M_PI);
	}

	if (keys[DIK_DOWN] || keys[DIK_S]) {
		theta = (3.0f / 2.0f) * float(M_PI);
	}

	if (keys[DIK_RIGHT] || keys[DIK_D]) {
		theta = 0;
	}

	if (keys[DIK_LEFT] || keys[DIK_A]) {
		theta = 1.0f * float(M_PI);
	}

	if (keys[DIK_UP] && keys[DIK_RIGHT] || keys[DIK_W] && keys[DIK_D]) {
		theta = (1.0f / 4.0f) * float(M_PI);
	}

	if (keys[DIK_UP] && keys[DIK_LEFT] || keys[DIK_W] && keys[DIK_A]) {
		theta = (3.0f / 4.0f) * float(M_PI);
	}

	if (keys[DIK_DOWN] && keys[DIK_LEFT] || keys[DIK_S] && keys[DIK_A]) {
		theta = (5.0f / 4.0f) * float(M_PI);
	}

	if (keys[DIK_DOWN] && keys[DIK_RIGHT] || keys[DIK_S] && keys[DIK_D]) {
		theta = (7.0f / 4.0f) * float(M_PI);

	}

	speed.x = cosf(theta);
	speed.y = sinf(theta);

	moveSpeed.x = velocity.x * speed.x;
	moveSpeed.y = velocity.y * speed.y;

	if (keys[DIK_DOWN] || keys[DIK_UP] || keys[DIK_LEFT] || keys[DIK_RIGHT] || keys[DIK_S] || keys[DIK_W] || keys[DIK_A] || keys[DIK_D]) {
		pos.x = pos.x + moveSpeed.x;
		pos.y = pos.y - moveSpeed.y;
	}
}

void Rotate(Vector2& pos, float& theta, Vector2& initialPos) {
	Vector2 rotatedPos;

	theta += 0.05f;

	rotatedPos.x = (pos.x - initialPos.x) * cosf(theta) - (pos.y - initialPos.y) * sinf(theta);
	rotatedPos.y = (pos.x - initialPos.x) * sinf(theta) + (pos.y - initialPos.y) * cosf(theta);

	pos.x = rotatedPos.x + initialPos.x;
	pos.y = rotatedPos.y + initialPos.y;

}


//	矩形用の角の関数
Corners PosUpdate(Vector2 a, float width, float height) {
	Corners c = {};

	c.leftTop.x = (a.x - width / 2.0f);
	c.leftTop.y = (a.y - height / 2.0f);

	c.rightTop.x = ((a.x + width / 2.0f));
	c.rightTop.y = (a.y - height / 2.0f);

	c.leftDown.x = (a.x - width / 2.0f);
	c.leftDown.y = ((a.y + height / 2.0f));

	c.rightDown.x = ((a.x + width / 2.0f));
	c.rightDown.y = ((a.y + height / 2.0f));

	return c;
}

void VectorScreenPrintf(int x, int y, const Vector3& v, const char* label) {
	Novice::ScreenPrintf(x, y, "%.02f", v.x);
	Novice::ScreenPrintf(x + kColmunWidth, y, "%.02f", v.y);
	Novice::ScreenPrintf(x + kColmunWidth * 2, y, "%.02f", v.z);
	Novice::ScreenPrintf(x + kColmunWidth * 3, y, "%s", label);
}

void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* a) {
	Novice::ScreenPrintf(x, y, "%s", a);
	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			Novice::ScreenPrintf(
				x + (column + 1) * kColmunWidth, y + (row + 1) * kColmunHeight, "%0.02f", matrix.m[row][column]);
		}
	}
}

const char kWindowTitle[] = "LE2D_07_サクマ_シン_タイトル";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	float pi = float(M_PI);

	Player player;
	player.pos = { 640.0f,360.0f };
	player.width = 64.0f;
	player.height = 64.0f;
	player.velocity = { 8.0f,8.0f };
	player.isBulletShot = false;
	player.color = WHITE;
	player.bulletCoolTime = 150;

	Bullet bullet1;
	bullet1.radius = 16.0f;
	bullet1.speed = 8.0f;
	bullet1.playerToBullet = 0.0f;

	bullet1.color = RED;

	bullet1.rad = 0.0f;

	for (int i = 0;i < bulletMax;i++) {
		bullet1.pos[i] = player.pos;
		bullet1.posRad[i] = (float(i) / 8.0f) * pi;
	}

	Bullet bullet2;
	bullet2.radius = 16.0f;
	bullet2.speed = 8.0f;
	bullet2.playerToBullet = -64.0f;

	bullet2.color = RED;
	bullet2.rad = 0.0f;

	for (int i = 0;i < bulletMax;i++) {
		bullet2.pos[i] = player.pos;
		bullet2.posRad[i] = (float(i) / 8.0f) * pi;
	}

	Enemy enemy;
	enemy.pos[0] = { 64.0f,64.0f };
	enemy.pos[1] = { 64.0f,kWindowHeight - 64.0f };
	enemy.pos[2] = { kWindowWidth - 64.0f,64.0f };
	enemy.pos[3] = { kWindowWidth - 64.0f,kWindowHeight - 64.0f };
	enemy.radius = 32.0f;
	enemy.color = BLACK;

	for (int i = 0;i < enemyNum;i++) {
		enemy.isAlive[i] = true;
	}


	int whiteGH = Novice::LoadTexture("white1x1.png");

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		move(player.pos, keys, player.velocity);
		player.corners = PosUpdate(player.pos, player.width, player.height);

		if (!player.isBulletShot) {
			if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
				player.isBulletShot = true;
				bullet1.memoryPos = player.pos;
				bullet2.memoryPos = player.pos;
			}

			player.bulletCoolTime = 150;
			bullet1.playerToBullet = 0.0f;
			bullet2.playerToBullet = -64.0f;

			//弾を撃っていないときは弾の位置をプレイヤーと同じにする
			for (int i = 0;i < bulletMax;i++) {
				bullet1.pos[i] = player.pos;
				bullet2.pos[i] = player.pos;
			}

		} else {
			//プレイヤーと弾との距離を離す
			bullet1.playerToBullet += bullet1.speed;
			bullet2.playerToBullet += bullet2.speed;

			player.bulletCoolTime--;
			if (player.bulletCoolTime <= 0) {
				player.isBulletShot = false;
			}

			for (int i = 0;i < bulletMax;i++) {
				bullet1.posRad[i] += 0.1f;
				bullet2.posRad[i] += 0.1f;
				//プレイヤーとの距離から円状に位置を決める
				bullet1.pos[i].x = bullet1.memoryPos.x + bullet1.playerToBullet * cosf(bullet1.posRad[i]);
				bullet1.pos[i].y = bullet1.memoryPos.y + bullet1.playerToBullet * sinf(bullet1.posRad[i]);
				bullet2.pos[i].x = bullet2.memoryPos.x + bullet2.playerToBullet * cosf(bullet2.posRad[i]);
				bullet2.pos[i].y = bullet2.memoryPos.y + bullet2.playerToBullet * sinf(bullet2.posRad[i]);

				//敵との当たり判定。　当たったら敵を消滅させる
				for (int j = 0;j < enemyNum;j++) {
					if (HitCircle(bullet1.pos[i], bullet1.radius, enemy.pos[j], enemy.radius) ||
						HitCircle(bullet2.pos[i], bullet2.radius, enemy.pos[j], enemy.radius)) {
						enemy.isAlive[j] = false;
					}
				}
			}
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから

		if (player.isBulletShot) {
			for (int i = 0;i < bulletMax;i++) {
				DrawCircle(bullet1.pos[i], bullet1.radius, bullet1.radius, bullet1.color);
				if (bullet2.playerToBullet > 0) {
					DrawCircle(bullet2.pos[i], bullet2.radius, bullet2.radius, bullet2.color);
				}
			}
		}

		for (int i = 0;i < enemyNum;i++) {
			if (enemy.isAlive[i]) {
				DrawCircle(enemy.pos[i], enemy.radius, enemy.radius, enemy.color);
			}
		}

		DrawQuad(player.corners, whiteGH, 1, 1, player.color);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
