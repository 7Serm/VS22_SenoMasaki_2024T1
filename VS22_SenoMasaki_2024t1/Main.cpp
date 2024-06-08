# include <Siv3D.hpp>
# include<HamFramework.hpp>

/*
	古き良き書き方での実装
	・安全性や利便性などは一切考えていない
	//距離を変数名に使うときにはdistと表記する事が好ましい
	//値の変わらない変数にはconst 大文字＿で表す事で見やすく区別しやすい
*/

//==============================
// 前方宣言
//==============================
class Ball;
class Bricks;
class Paddle;

//==============================
// 定数
//==============================
namespace constans {

	namespace Ball {
		/// @brief ボールの速さ
		constexpr double SPEED = 480.0;
	}
	namespace Brick {
		/// @brief ブロックのサイズ
		constexpr Size SIZE{ 40, 20 };

		/// @brief ブロックの数　縦
		constexpr int Y_COUNT = 5;

		/// @brief ブロックの数　横
		constexpr int X_COUNT = 20;

		/// @brief 合計ブロック数
		constexpr int MAX = Y_COUNT * X_COUNT;
	}

	namespace Paddle {
		constexpr Size SIZE{ 60,10 };
	}

	namespace reflect {
		constexpr Vec2 VERTICAL{ 1,-1 };
		constexpr Vec2 HORIZONTAL{ -1,1 };
	}
}
//==============================
// クラス
//==============================



class Ball {
private:
	Vec2 velocity;

	Circle ball;


public:
	Ball() : velocity({ 0, -constans::Ball::SPEED }), ball({ 400,400,Size() }) {}

	int Size() {
		int size = Random(5, 30);
		return size;
	}
	void Update() {
		ball.moveBy(velocity * Scene::DeltaTime());
	}
	void Draw() const {
		ball.draw();
	}

	Vec2 GetVelocity() {
		return velocity;
	}

	Circle GetCircle() {
		return ball;
	}

	void SetVelocity(Vec2 newVelocity) {
		using namespace constans::Ball;
		velocity = newVelocity.setLength(SPEED);
	}

	void Reflect(const Vec2 refVec) {
		velocity *= refVec;
	}
};

class Bricks {
public:
	Rect bricks[constans::Brick::MAX];

	Bricks() {
		using namespace constans::Brick;
		// ブロックを初期化
		for (int y = 0; y < Y_COUNT; ++y) {
			for (int x = 0; x < X_COUNT; ++x) {
				int index = y * X_COUNT + x;
				bricks[index] = Rect{
					x * SIZE.x,
					60 + y * SIZE.y,
					SIZE
				};
			}
		}
	}

	void Intersecta(Ball* const target);

	void Draw() const {
		//ブロック描画
		using namespace constans::Brick;
		for (int i = 0; i < MAX; ++i) {
			bricks[i].stretched(-1).draw(HSV{ bricks[i].y - 40 });
		}
	}
};

class Paddle
{
private:
	Rect paddle;

public:
	Paddle() :paddle(Rect(Arg::center(Cursor::Pos().x, 500), constans::Paddle::SIZE)) {}

	void Intersects(Ball* target);

	int Bounce() {
		int size = Random(1, 40);
		return size;
	}

	void Update() {
		paddle.x = Cursor::Pos().x - (constans::Paddle::SIZE.x / 2);
	}

	void Draw() const {
		paddle.rounded(3).draw();
	}
};

class Wall
{
public:
	static void Intersects(Ball* const target) {
		using namespace constans;
		if (!target) {
			return;
		}

		auto  velocity = target->GetVelocity();
		auto ball = target->GetCircle();

		if ((ball.y < 0) && (velocity.y < 0)) {
			target->Reflect(reflect::VERTICAL);
		}

		if (((ball.x < 0) && (velocity.x < 0)) || ((Scene::Width() < ball.x) && (0 < velocity.x))) {
			target->Reflect(reflect::HORIZONTAL);
		}
	}
};

void Bricks::Intersecta(Ball* const target) {
	using namespace constans;
	using namespace constans::Brick;

	if (!target) {
		return;
	}

	auto ball = target->GetCircle();

	for (int i = 0; i < MAX; ++i) {
		Rect& refBrick = bricks[i];
		if (refBrick.intersects(ball)) {
			if (refBrick.bottom().intersects(ball) || refBrick.top().intersects(ball)) {
				target->Reflect(reflect::VERTICAL);
			}
			else
			{
				target->Reflect(reflect::HORIZONTAL);
			}

			refBrick.y -= 600;

			break;
		}
	}
}


void Paddle::Intersects(Ball* const target)
{
	if (!target) {
		return;
	}

	auto velocity = target->GetVelocity();
	auto ball = target->GetCircle();

	if ((0 < velocity.y) && paddle.intersects(ball)) {
		target->SetVelocity(Vec2{ (ball.x - paddle.center().x) * Bounce(),-velocity.y });
	}
}

static bool Button(const Rect& rect, const Font& font, const String& text, bool enabled)
{

	if (enabled)
	{
		rect.draw(ColorF{ 0.3, 0.7, 1.0 });
		font(text).drawAt(40, (rect.x + rect.w / 2), (rect.y + rect.h / 2));
	}
	else
	{
		rect.draw(ColorF{ 0.5 });
		font(text).drawAt(40, (rect.x + rect.w / 2), (rect.y + rect.h / 2), ColorF{ 0.7 });
	}

	return (enabled && rect.leftClicked());
}

static bool OutScreen(Ball* target)
{
	if (!target) {
		return false;
	}

	auto ball = target->GetCircle();
	return (Scene::Height() < ball.y);
}


class Title :public SceneManager<String>::Scene
{
public:
	const Font font{ FontMethod::MSDF,48,Typeface::Bold };

	Title(const InitData& init) :IScene{ init } {

	}

	void update()override
	{
		if (Button(Rect{ 250, 400, 300, 80 }, font, U"ゲームスタート", true))
		{
			changeScene(U"GameScene");
		}
	}

	void draw()const override
	{

	}
};

class GameScene :public SceneManager<String>::Scene {
public:
	Ball ball;
	Bricks bricks;
	Paddle paddle;
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	GameScene(const InitData& init) :IScene{ init }
	{
		ball.Size();
	}

	void update() override
	{

		paddle.Update();
		ball.Update();


		if (OutScreen(&ball) == true) {
			if (Button(Rect{ 250, 400, 300, 80 }, font, U"リプレイ", true))
			{
				changeScene(U"GameScene");
			}
		}


		bricks.Intersecta(&ball);
		Wall::Intersects(&ball);
		paddle.Intersects(&ball);


	};

	void draw()const override
	{
		bricks.Draw();
		ball.Draw();
		paddle.Draw();
	}
};



void Main()
{
	SceneManager<String>manager;
	manager.add<Title>(U"Title");
	manager.add<GameScene>(U"GameScene");
	while (System::Update())
	{
		if (not manager.update())
		{
			break;
		}
	}
};

