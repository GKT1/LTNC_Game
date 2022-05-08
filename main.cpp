#include <bits/stdc++.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
using namespace std;

const int WINDOW_WIDTH = 720;
const int WINDOW_HEIGHT = 720;
const int TABLE_WIDTH = 300;
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
auto startProgramTime =  std::chrono::high_resolution_clock::now();
float dt = 0.0f;


enum class CollisionType
{
	None,
	Top,
	Middle,
	Bottom,
	Left,
	Right,
	Table,
	Ground,
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
class Paddle
{
public:
    Vec2 position;
	Vec2 velocity;
	Vec2 delta;
	SDL_Rect rect{};
	deque<pair<int,pair<int,int> > > dq;
	Paddle(Vec2 position, Vec2 velocity): position(position), velocity(velocity)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = PADDLE_WIDTH;
		rect.h = PADDLE_HEIGHT;
	}
	void Update(int x,int y,int currentTime)
	{
        dq.push_back({currentTime,{x - position.x,y - position.y}});
        delta.x += dq.back().second.first;
	    delta.y += dq.back().second.second;
	    while(!dq.empty() && currentTime-dq.front().first>100)
        {
            delta.x -= dq.front().second.first;
            delta.y -= dq.front().second.second;
            dq.pop_front();
        }
        //cout<<delta.x<<" "<<delta.y<<'\n';
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
	int height = 6000;
	int CheckCollideWithTable = -1;
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
		if(CheckCollideWithTable == 1)height = max(4000.0f, height - dt * 1.5f);
		else height = min(6000.0f, height + dt * 2);
		//cout<<height<<'\n';
		//cout<<CheckCollideWithTable<<'\n';
	}
	void Draw(SDL_Renderer* renderer,auto texBall)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
        rect.w = BALL_WIDTH * height / 6000;
		rect.h = BALL_HEIGHT * height / 6000;
		SDL_RenderCopy(renderer, texBall, NULL, &rect);
	}
	void DrawEffect(SDL_Renderer* renderer,auto texBallEffect)
	{
	    SDL_Rect rect{position.x - 20, position.y - 20, 80, 80};
	    SDL_RenderCopy(renderer, texBallEffect, NULL, &rect );
	}
	void reset()
	{
	    height = 6000;
	    CheckCollideWithTable = -1;
	    position.x = WINDOW_WIDTH / 2.0f;
        position.y = WINDOW_HEIGHT / 2.0f;
	}
	Contact CheckTableAndGroundCollision(Table const& table)
	{
	    Contact contact{};
	    if(height<=4500)
        {
            float ballX = position.x + BALL_WIDTH / 2;
            float ballY = position.y + BALL_HEIGHT / 2;
            float tableLeft = table.position.x;
            float tableRight = table.position.x + TABLE_WIDTH;
            float tableTop = table.position.y;
            float tableBottom = table.position.y + TABLE_HEIGHT;
            if (ballX >= tableRight||
                ballX <= tableLeft||
                ballY >= tableBottom||
                ballY <= tableTop)
                {
                    if(height<=3000)
                        contact.type = CollisionType::Ground;
                }
            else contact.type = CollisionType::Table;
        }
        return contact;
	}
	Contact CheckPaddleCollision( Paddle const& paddle)
    {
        float ballLeft = position.x;
        float ballRight = position.x + BALL_WIDTH;
        float ballTop = position.y;
        float ballBottom = position.y + BALL_HEIGHT;
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
    Contact CheckWallCollision()
    {
        float ballLeft = position.x;
        float ballRight = position.x + BALL_WIDTH;
        float ballTop = position.y;
        float ballBottom = position.y + BALL_HEIGHT;
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
    void changeDirectionX(Paddle paddle)
    {
        //cout<<paddle.delta.x<<" "<<paddle.delta.y<<'\n';

        velocity.x = BALL_SPEED * min(max(paddle.delta.x,-100.0f),100.0f) / 300 ;
        //cout<< BALL_SPEED << " "<< velocity.x<<'\n';
		//if(paddle.delta.x > 20)velocity.x = BALL_SPEED * 0.4;
		//else if(paddle.delta.x < -20 )velocity.x = BALL_SPEED * -0.4;
		//else if(paddle.delta.x > 1 )velocity.x = BALL_SPEED * 0.3;
		//else if(paddle.delta.x < 1 )velocity.x = BALL_SPEED * -0.3;
		//else if(paddle.delta.x > 0 )velocity.x = BALL_SPEED * 0.2;
		//else if(paddle.delta.x < 0 )velocity.x = BALL_SPEED * -0.2;
		//else velocity.x = 0;
    }
	void CollideWithPaddle(Contact const& contact,Paddle paddle)
	{
	    turn ^= 1;
	    CheckCollideWithTable = 1;
	    height = 6000;
	    velocity.y = -velocity.y;
	    changeDirectionX(paddle);
	}
	void CollideWithWall(Contact const& contact)
	{
	    CheckCollideWithTable = -1;
		reset();
		if(velocity.y < 0)turn = 1;
		else turn = 0;
		velocity.x = 0;
	}
	void CollideWithTable()
	{
	    CheckCollideWithTable =  -1;
	}
};
class PlayerScore
{
public:
    SDL_Renderer* renderer;
	TTF_Font* font;
	SDL_Surface* surface{};
	SDL_Texture* texture{};
	SDL_Rect rect{};
	PlayerScore(Vec2 position, SDL_Renderer* renderer, TTF_Font* font)
		: renderer(renderer), font(font)
	{
		//surface = TTF_RenderText_Solid(font, "0", {255, 165, 0, 255});
		surface = TTF_RenderText_Solid(font, "0", {0, 0, 0, 255});
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

		surface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), {0, 0, 0, 255});
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
};
void playGame(auto renderer)
{

    // Initialize sound effects
    Mix_Chunk* paddleHitSound = Mix_LoadWAV("S27.wav");
    //
    SDL_Texture* texTable = IMG_LoadTexture(renderer,"table.png");
    SDL_Texture* backGround = IMG_LoadTexture(renderer,"background.png");
    SDL_Texture* texPaddle1 = IMG_LoadTexture(renderer,"paddle1.png");
    SDL_Texture* texPaddle2 = IMG_LoadTexture(renderer,"paddle2.png");
    SDL_Texture* texBall = IMG_LoadTexture(renderer,"ball.png");
    SDL_Texture* texBallEffect = IMG_LoadTexture(renderer,"effect.png");
    //
    TTF_Font* scoreFont = TTF_OpenFont("DejaVuSansMono.ttf", 40);

	// Game logic
	{
	    //
	    auto TableTime =  startProgramTime;
        // Create the ball
        SDL_ShowCursor(SDL_DISABLE);
        Table table(Vec2(240,60));
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
            int currentTime = chrono::duration<float, std::chrono::milliseconds::period>(startTime-startProgramTime).count();
            paddleTwo.Update(mouse_x,max(WINDOW_HEIGHT/2,mouse_y),currentTime);
            if (ball.position.y-paddleOne.position.y<100)
            {
                if (paddleOne.position.x<ball.position.x)
                    paddleOne.Update(paddleOne.position.x + 10, 50.0f,currentTime);
                else
                    paddleOne.Update(paddleOne.position.x - 10, 50.0f,currentTime);
            }
            else if(rand() % 20 == 0)
            {
                if (rand() % 2)
                    paddleOne.position.y = min(paddleOne.position.y + rand() % 5, 70.0f);
                else
                    paddleOne.position.y = max(paddleOne.position.y - rand() % 5, 30.0f);
                if (rand() % 2)
                    paddleOne.position.x = min(paddleOne.position.x + rand() % 10, 700.0f);
                else
                    paddleOne.position.x = max(paddleOne.position.x - rand() % 10, 100.0f);
            }
            //paddleOne.Update(rand()%720,50.0f,currentTime);

            // Update the ball position
            ball.Update(dt);
            // Check collisions
            Contact contact={};
            //with table and ground
            contact = ball.CheckTableAndGroundCollision(table);
            if (contact.type == CollisionType::Table && ball.CheckCollideWithTable == 1)
            {
                Mix_PlayChannel(-1, paddleHitSound, 0);
                TableTime = startTime;
                ball.CollideWithTable();
                if(turn==1)ball.changeDirectionX(paddleTwo);
                else       ball.changeDirectionX(paddleOne);
            }
            if (contact.type == CollisionType::Ground)
            {
                ball.CollideWithWall(contact);
            }
            // with paddle
            //1
            contact = ball.CheckPaddleCollision(paddleOne);
            if (contact.type != CollisionType::None && turn == 1 && ball.CheckCollideWithTable == -1)
            {
                Mix_PlayChannel(-1, paddleHitSound, 0);
                ball.CollideWithPaddle(contact,paddleOne);
            }
            //2
            contact = ball.CheckPaddleCollision(paddleTwo);
            if (contact.type != CollisionType::None && turn == 0 && ball.CheckCollideWithTable == -1)
            {
                Mix_PlayChannel(-1, paddleHitSound, 0);
                ball.CollideWithPaddle(contact,paddleTwo);
            }
            //with wall
            contact = ball.CheckWallCollision();
            if (contact.type != CollisionType::None)
            {
                if (turn == 0)
                {
                    if (ball.CheckCollideWithTable == 1)
                    {
                        ++playerTwoScore;
                        playerTwoScoreText.SetScore(playerTwoScore);
                    }
                    else
                    {
                        ++playerOneScore;
                        playerOneScoreText.SetScore(playerOneScore);
                    }
                }
                else if (turn == 1)
                {
                    if (ball.CheckCollideWithTable == -1)
                    {
                        ++playerTwoScore;
                        playerTwoScoreText.SetScore(playerTwoScore);
                    }
                    else
                    {
                        ++playerOneScore;
                        playerOneScoreText.SetScore(playerOneScore);
                    }
                }
                ball.CollideWithWall(contact);
            }
            //cout<<paddleTwo.delta.x<<" "<<paddleTwo.delta.y<<'\n';
            // Draw background
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
            // Draw the table
            table.Draw(renderer,texTable);
            // Draw the ball
            ball.Draw(renderer,texBall);
            //
            if(chrono::duration<float, std::chrono::milliseconds::period>(TableTime - startTime).count()>-50)ball.DrawEffect(renderer,texBallEffect);
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
		}
	}

    SDL_DestroyTexture(texTable);
    SDL_DestroyTexture(backGround);
    SDL_DestroyTexture(texPaddle1);
    SDL_DestroyTexture(texPaddle2);
    SDL_DestroyTexture(texBall);
    TTF_CloseFont(scoreFont);
}
class Button
{
    public:
    Vec2 position;
	SDL_Rect rect{};
	Button(Vec2 position): position(position)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = 150;
		rect.h = 100;
	}
	int checkMouseOverButton()
	{
	    float buttonLeft = rect.x;
        float buttonRight = rect.x + rect.w;
        float buttonTop = rect.y;
        float buttonBottom = rect.y + rect.h;
        if(mouse_x>buttonRight||
           mouse_x<buttonLeft||
           mouse_y>buttonBottom||
           mouse_y<buttonTop
           )
            return 0;
        return 1;
	}
	void Draw(SDL_Renderer* renderer,auto texButton)
	{
	    if(checkMouseOverButton())
	    {
	        rect.w += 30;rect.h += 20;
	        SDL_RenderCopy(renderer, texButton, NULL, &rect);
	        rect.w -= 30;rect.h -= 20;
	    }
        else SDL_RenderCopy(renderer, texButton, NULL, &rect);
	}
};
class Title
{
    public:
    Vec2 position;
	SDL_Rect rect{};
	Title(Vec2 position): position(position)
	{
		rect.x = static_cast<int>(position.x);
		rect.y = static_cast<int>(position.y);
		rect.w = 200;
		rect.h = 200;
	}
	void Draw(SDL_Renderer* renderer,auto texTitle)
	{
		SDL_RenderCopy(renderer, texTitle, NULL, &rect);
	}
};
int main()
{
    srand(time(0));
	// Initialize SDL components
	//SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Pong", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    //TTF
    TTF_Init();
    //Mixer
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    //Image
    IMG_Init(IMG_INIT_PNG);

    //
    SDL_Texture* texPlay = IMG_LoadTexture(renderer,"play.png");
    SDL_Texture* texQuit = IMG_LoadTexture(renderer,"quit.png");
    SDL_Texture* texMenu = IMG_LoadTexture(renderer,"menu.png");
    SDL_Texture* texTitle = IMG_LoadTexture(renderer,"title.png");
    Button play(Vec2(430,860)),quit(Vec2(430,960));
    Title title(Vec2(280,360));
    //animation 1
    SDL_RenderCopy(renderer, texTitle, NULL, NULL);
    SDL_RenderPresent(renderer);
    sleep(2);
    //animation 2
    while(play.rect.y>500)
    {
        play.rect.y--;
        quit.rect.y--;
        title.rect.y--;
        // Draw background
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        // Draw button
        play.Draw(renderer,texPlay);
        quit.Draw(renderer,texQuit);
        title.Draw(renderer,texTitle);
        // Present the backbuffer
        SDL_RenderPresent(renderer);
    }
    //
    {
        int checkMouseButtonDown;
        while(running)
        {
            SDL_Event event;
            checkMouseButtonDown = 0;
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
                else if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    checkMouseButtonDown = 1;
                }
            }
            //
            if (play.checkMouseOverButton() && checkMouseButtonDown)
            {
                playGame(renderer);
            }
            if (quit.checkMouseOverButton() && checkMouseButtonDown)
            {
                running = false;
            }
            // Draw background
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texMenu, NULL, NULL);
            // Draw button
            play.Draw(renderer,texPlay);
            quit.Draw(renderer,texQuit);
            title.Draw(renderer,texTitle);
            // Present the backbuffer
            SDL_RenderPresent(renderer);
            //playGame(renderer);
        }
    }

    //quit SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	IMG_Quit();

	return 0;
}
