#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH  = 1280,
          WINDOW_HEIGHT = 960;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int TRIANGLE_RED = 1.0,
          TRIANGLE_BLUE = 0.4,
          TRIANGLE_GREEN = 0.4,
          TRIANGLE_OPACITY = 1.0;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex.glsl",
           F_SHADER_PATH[] = "shaders/fragment.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const float MINIMUM_COLLISION_DISTANCE = 1.0f;

SDL_Window* display_window;
bool game_is_running = true;
bool is_growing = true;

ShaderProgram program;
glm::mat4 view_matrix, paddle_1, projection_matrix, trans_matrix, paddle_2, ball;

float previous_ticks = 0.0f;


glm::vec3 paddle_1_position = glm::vec3(-2.0f, 0.0f, 0.0f);
glm::vec3 paddle_1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 paddle_2_position = glm::vec3(2.0f, 0.0f, 0.0f);
glm::vec3 paddle_2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.5f, 1.0f, 0.0f);

glm::vec3 player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_rotation    = glm::vec3(0.0f, 0.0f, 0.0f);

float player_speed = 2.0f;  // move 1 unit per second
float ball_speed = 0.5f;


#define LOG(argument) std::cout << argument << '\n'


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Pong Clone",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    paddle_1 = glm::mat4(1.0f);
    paddle_2 = glm::mat4(1.0f);
    //paddle_2 = glm::translate(paddle_2, glm::vec3(1.0f, 1.0f, 0.0f));
    //paddle_2_position += paddle_2_movement;
    
    ball = glm::mat4(1.0f);
    
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);
    
    glUseProgram(program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    paddle_1_movement = glm::vec3(0.0f);
    paddle_2_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        paddle_1_movement.x = -1.0f;
                        break;
                        
                    case SDLK_RIGHT:
                        // Move the player right
                        paddle_1_movement.x = 1.0f;
                        break;
                    
                    case SDLK_UP:
                        paddle_1_movement.y = 1.0f;
                        break;
                    
                    case SDLK_DOWN:
                        paddle_1_movement.y = -1.0f;
                        break;
                        
                    case SDLK_w:
                        paddle_2_movement.y = 1.0f;
                        break;
                        
                    case SDLK_s:
                        paddle_2_movement.y = -1.0f;
                        break;
                        
                    case SDLK_a:
                        paddle_2_movement.x = -1.0f;
                        break;
                        
                    case SDLK_d:
                        paddle_2_movement.x = 1.0f;
                        break;
                        
                        
                    case SDLK_q:
                        // Quit the game with a keystroke
                        game_is_running = false;
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        paddle_1_movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        paddle_1_movement.x = 1.0f;
    }
    
    if (key_state[SDL_SCANCODE_UP])
    {
        paddle_1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        paddle_1_movement.y = -1.0f;
    }
    
    if (key_state[SDL_SCANCODE_W])
    {
        paddle_2_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        paddle_2_movement.y = -1.0f;
    }
    if (key_state[SDL_SCANCODE_A])
    {
        paddle_2_movement.x = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_D])
    {
        paddle_2_movement.x = 1.0f;
    }
    
    // This makes sure that the player can't move faster diagonally
    if (glm::length(paddle_1_movement) > 1.0f)
    {
        paddle_1_movement = glm::normalize(paddle_1_movement);
    }
    
    if (glm::length(paddle_2_movement) > 1.0f)
    {
        paddle_2_movement = glm::normalize(paddle_2_movement);
    }
}



bool check_collision(glm::vec3 &position_a, glm::vec3 &position_b)
{
    float x_distance = fabs(position_a[0] - position_b[0]) - ((0.2 + 1) / 2.0f);
    float y_distance = fabs(position_a[1] - position_b[1]) - ((0.2 + 1) / 2.0f);

    if (x_distance < 0 && y_distance < 0) {
        // Collision!
        return true;
    }

    // No collision
    return false;
}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;
    
    glm::vec3 future_ball_position = ball_position + ball_movement * ball_speed * delta_time;

    // Check if ball hits the top or bottom of the screen in the next frame
    if (future_ball_position.y >= 3.75f - 0.1f || future_ball_position.y <= -3.75f + 0.1f) {  // take into account ball size
        // Reflect the ball's vertical movement
        ball_movement.y *= -1.0f;
        }
    
    if (future_ball_position.x >= 5.0f - 0.1f)
        {
            std::cout << "Player 1 Wins!" << std::endl;
            game_is_running = false;
        }
        else if (future_ball_position.x <= -5.0f + 0.1f)
        {
            std::cout << "Player 2 Wins!" << std::endl;
            game_is_running = false;
        }
    
    // Check if ball hits paddles in the next frame
    if (check_collision(future_ball_position, paddle_1_position) || check_collision(future_ball_position, paddle_2_position)) {
        ball_movement.x *= -1.0f; // Reflect the ball's movement
        std::cout << std::time(nullptr) << ": BAll Collision.\n";
    }

    // Add direction * units per second * elapsed time
    paddle_1_position += paddle_1_movement * player_speed * delta_time;
    paddle_2_position += paddle_2_movement * player_speed * delta_time;
    ball_position += ball_movement * ball_speed * delta_time;
    
    // Keep player paddle in bounds
    if (paddle_1_position.y > 3.75f - 0.5f) paddle_1_position.y = 3.75f - 0.5f;
    else if (paddle_1_position.y < -3.75f + 0.5f) paddle_1_position.y = -3.75f + 0.5f;
    
    // Keep other paddle in bounds
    if (paddle_2_position.y > 3.75f - 0.5f) paddle_2_position.y = 3.75f - 0.5f;
    else if (paddle_2_position.y < -3.75f + 0.5f) paddle_2_position.y = -3.75f +0.5f;
    
    
    paddle_1 = glm::mat4(1.0f);
    paddle_2 = glm::mat4(1.0f);
    paddle_1 = glm::translate(paddle_1, paddle_1_position);
    paddle_2 = glm::translate(paddle_2, paddle_2_position);
    ball = glm::mat4(1.0f);
    ball = glm::translate(ball, ball_position);
        
}

void drawPaddle(glm::mat4& modelMatrix) {
    // Set up the model matrix
    program.SetModelMatrix(modelMatrix);
    // Define vertices for a rectangle
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    // Draw the triangles

    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Disable the attribute arrays
    glDisableVertexAttribArray(program.positionAttribute);
}

void drawBall(glm::mat4& modelMatrix){
    // Set up the model matrix
    program.SetModelMatrix(modelMatrix);
    // Define vertices for a rectangle
    float vertices[] = {
        -0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f,  // triangle 1
        -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, 0.1f   // triangle 2
    };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Disable the attribute arrays
    glDisableVertexAttribArray(program.positionAttribute);
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    drawPaddle(paddle_1);
    drawPaddle(paddle_2);
    
    drawBall(ball);
    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();
    
    while (game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
