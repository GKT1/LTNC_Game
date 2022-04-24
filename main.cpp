#include <bits/stdc++.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
using namespace std;

const int WINDOW_WIDTH = 720;
const int WINDOW_HEIGHT = 720;
const int TABLE_WIDTH = 500;
const int TABLE_HEIGHT = 600;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 100;
const float BALL_SPEED = 0.4f;
const int BALL_WIDTH = 30;
const int BALL_HEIGHT = 30;

int playerOneScore = 0;
int playerTwoScore = 0;
int mouse_x;
int mouse_y;
int turn;
bool running = true;
float dt = 0.0f, oneSec = 0;


enum class CollisionType
{
	None,
	Top,
	Middle,
	Bottom,
	Left,
	Right
};
struct Contact
{
	CollisionType type;
};
class Vec2
{
public:
    float x, y;
	Vec2(): x(0.0f), y(0.0f){}
	Vec2(float x, float y): x(x), y(y){}
	Vec2 operator+(Vec2 const& rhs)
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}
	Vec2& operator+=(Vec2 const& rhs)
	{
		x += rhs.x;
		y += rhs.y;

		return *this;
	}
	Vec2 operator*(float rhs)
	{
		return Vec2(x * rhs, y * rhs);
	}
};
class Paddle
{
public:
    Vec2 position;
	Vec2 velocity;
	Vec2 delta;
	SDL_Rect rect{};
	Paddle(Vec2 position, Vec2 velocity): position(position), velocity(velocity)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = PADDLE_WIDTH;
		rect.h = PADDLE_HEIGHT;
	}
	void Update(int x,int y)
	{
	    if(oneSec > 0.25)
        {
            oneSec = 0;
            delta.x = 0;
            delta.y = 0;
        }
        delta.x += x - position.x;
	    delta.y += y - position.y;
		position.x = x;
		position.y = y;
	}
	void Draw(SDL_Renderer* renderer,auto texPaddle)
	{
	    rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		//SDL_RenderFillRect(renderer, &rect);
		SDL_RenderCopy(renderer, texPaddle, NULL, &rect);
	}
};
class Ball
{
public:
    Vec2 position;
	Vec2 velocity;
	SDL_Rect rect{};
	Ball(Vec2 position, Vec2 velocity): position(position), velocity(velocity)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = BALL_WIDTH;
		rect.h = BALL_HEIGHT;
	}
	void Update(float dt)
	{
		position += velocity * dt;
	}
	void Draw(SDL_Renderer* renderer,auto texBall)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);

		SDL_RenderCopy(renderer, texBall, NULL, &rect);
	}
	void CollideWithPaddle(Contact const& contact,Paddle paddle)
	{
	    turn ^= 1;
	    velocity.y=-velocity.y;
	    if(paddle.delta.x == 0 && paddle.delta.y == 0)
        {
            velocity.x = 0;
            return;
        }
		if(paddle.delta.x > 2)velocity.x = BALL_SPEED * 0.4;
		else if(paddle.delta.x < 2 )velocity.x = BALL_SPEED * -0.4;
		else if(paddle.delta.x > 1 )velocity.x = BALL_SPEED * 0.3;
		else if(paddle.delta.x < 1 )velocity.x = BALL_SPEED * -0.3;
		//else if(paddle.delta.x > 0 )velocity.x = BALL_SPEED * 0.2;
		//else if(paddle.delta.x < 0 )velocity.x = BALL_SPEED * -0.2;
		else velocity.x = 0;
	}
	void CollideWithWall(Contact const& contact)
	{
		position.x = WINDOW_WIDTH / 2.0f;
        position.y = WINDOW_HEIGHT / 2.0f;
		if(velocity.y<0)turn = 1;
		else turn = 0;
		velocity.x = 0;
	}
};
class Table
{
    public:
    Vec2 position;
	SDL_Rect rect{};
	Table(Vec2 position): position(position)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = TABLE_WIDTH;
		rect.h = TABLE_HEIGHT;
	}
	void Draw(SDL_Renderer* renderer,auto texTable)
	{
		SDL_RenderCopy(renderer, texTable, NULL, &rect);
	}
};
class PlayerScore
{
public:
	PlayerScore(Vec2 position, SDL_Renderer* renderer, TTF_Font* font)
		: renderer(renderer), font(font)
	{
		surface = TTF_RenderText_Solid(font, "0", {255, 165, 0, 255});
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = width;
		rect.h = height;
	}

	~PlayerScore()
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}

	void SetScore(int score)
	{
		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);

		surface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), {255, 165, 0, 255});
		texture = SDL_CreateTextureFromSurface(renderer, surface);

		int width, height;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
		rect.w = width;
		rect.h = height;
	}

	void Draw()
	{
		SDL_RenderCopy(renderer, texture, nullptr, &rect);
	}

	SDL_Renderer* renderer;
	TTF_Font* font;
	SDL_Surface* surface{};
	SDL_Texture* texture{};
	SDL_Rect rect{};
};
Contact CheckPaddleCollision(Ball const& ball, Paddle const& paddle)
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;
	float paddleLeft = paddle.position.x;
	float paddleRight = paddle.position.x + PADDLE_WIDTH;
	float paddleTop = paddle.position.y;
	float paddleBottom = paddle.position.y + PADDLE_HEIGHT;
	Contact contact{};
	if (ballLeft >= paddleRight||
        ballRight <= paddleLeft||
        ballTop >= paddleBottom||
        ballBottom <= paddleTop)
		return contact;

    contact.type = CollisionType::Middle;
	return contact;
}
Contact CheckWallCollision(Ball const& ball)
{
	float ballLeft = ball.position.x;
	float ballRight = ball.position.x + BALL_WIDTH;
	float ballTop = ball.position.y;
	float ballBottom = ball.position.y + BALL_HEIGHT;
	Contact contact{};
	if (ballLeft < 0.0f)
	{
		contact.type = CollisionType::Left;
	}
	else if (ballRight > WINDOW_WIDTH)
	{
		contact.type = CollisionType::Right;
	}
	else if (ballTop < 0.0f)
	{
		contact.type = CollisionType::Top;
	}
	else if (ballBottom > WINDOW_HEIGHT)
	{
		contact.type = CollisionType::Bottom;
	}
	return contact;
}
int main()
{
    srand(time(0));
	// Initialize SDL components
    SDL_Init(SDL_INIT_EVERYTHING);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    TTF_Init();
    TTF_Font* scoreFont = TTF_OpenFont("DejaVuSansMono.ttf", 40);


    SDL_Window* window = SDL_CreateWindow("Pong", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    // Initialize sound effects
    Mix_Chunk* paddleHitSound = Mix_LoadWAV("S27.wav");


    IMG_Init(IMG_INIT_PNG);
    SDL_Texture* texTable = IMG_LoadTexture(renderer,"table.png");
    SDL_Texture* backGround = IMG_LoadTexture(renderer,"background.png");
    SDL_Texture* texPaddle1 = IMG_LoadTexture(renderer,"paddle1_v2.png");
    SDL_Texture* texPaddle2 = IMG_LoadTexture(renderer,"paddle2_v2.png");
    SDL_Texture* texBall = IMG_LoadTexture(renderer,"ball.png");


	// Game logic
	{
        // Create the ball
        SDL_ShowCursor(SDL_DISABLE);
        Table table(Vec2(110,60));
        Ball ball(
            Vec2(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f),
            Vec2(0.0f, BALL_SPEED));
        // Create the paddles
        Paddle paddleOne(
            Vec2(WINDOW_WIDTH / 2.0f, 50.0f),
            Vec2(0.0f, 0.0f));
        Paddle paddleTwo(
            Vec2(WINDOW_HEIGHT - 50.0f, WINDOW_WIDTH / 2.0f),
            Vec2(0.0f, 0.0f));
        PlayerScore playerOneScoreText(Vec2(WINDOW_WIDTH / 6, WINDOW_HEIGHT / 4), renderer, scoreFont);
        PlayerScore playerTwoScoreText(Vec2(WINDOW_WIDTH / 6, WINDOW_HEIGHT / 4 * 3), renderer, scoreFont);
		while (running)
		{
			auto startTime = chrono::high_resolution_clock::now();
            // Input
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
                }
                else if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        running = false;
                    }
                }
                else if (event.type == SDL_MOUSEMOTION)
                {
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                }
            }

            // Update the paddle positions
            paddleTwo.Update(mouse_x,max(WINDOW_HEIGHT/2,mouse_y));
            paddleOne.position.x=rand()%720;

            // Update the ball position
            ball.Update(dt);
            // Check collisions
            if (Contact contact = CheckPaddleCollision(ball, paddleOne);
                contact.type != CollisionType::None&&
                turn == 1)
            {
                Mix_PlayChannel(-1, paddleHitSound, 0);
                ball.CollideWithPaddle(contact,paddleOne);
            }
            else if (contact = CheckPaddleCollision(ball, paddleTwo);
                contact.type != CollisionType::None&&
                turn == 0)
            {
                Mix_PlayChannel(-1, paddleHitSound, 0);
                ball.CollideWithPaddle(contact,paddleTwo);
            }
             else if (contact = CheckWallCollision(ball);
                contact.type != CollisionType::None)
            {
                ball.CollideWithWall(contact);
                if (contact.type == CollisionType::Top)
                {
                    ++playerTwoScore;

                    playerTwoScoreText.SetScore(playerTwoScore);
                }
                else if (contact.type == CollisionType::Bottom)
                {
                    ++playerOneScore;

                    playerOneScoreText.SetScore(playerOneScore);
                }
            }
            //cout<<paddleTwo.delta.x<<" "<<paddleTwo.delta.y<<'\n';
            // Clear the window to black
            SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            // Draw background
            SDL_RenderCopy(renderer, backGround, NULL, NULL);
            // Draw the table
            table.Draw(renderer,texTable);
            // Draw the ball
            ball.Draw(renderer,texBall);
            // Draw the paddles
            paddleOne.Draw(renderer,texPaddle2);
            paddleTwo.Draw(renderer,texPaddle1);
            // Draw the score
            playerOneScoreText.Draw();
            playerTwoScoreText.Draw();
            // Present the backbuffer
            SDL_RenderPresent(renderer);

			// Calculate frame time
			auto stopTime = std::chrono::high_resolution_clock::now();
			dt = chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
			// 1s reset delta.x and delta.y
			oneSec += dt;
		}
	}
    //quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(texTable);
    TTF_CloseFont(scoreFont);
    SDL_Quit();
	IMG_Quit();
	return 0;
}

