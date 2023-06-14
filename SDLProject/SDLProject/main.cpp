/**
* Author: Zhilue Li
* Assignment: Simple 2D Scene
* Date due: 2023-06-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH  = 960,
          WINDOW_HEIGHT = 720;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char PLAYER_SPRITE[] = "bear.png";
const char PLAYER_SPRITE2[] = "fox.png";

GLuint player_texture_id;
GLuint player_texture_id2;

const int TRIANGLE_RED     = 1.0,
          TRIANGLE_BLUE    = 0.4,
          TRIANGLE_GREEN   = 0.4,
          TRIANGLE_OPACITY = 1.0;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND     = 90.0f;

const int MAX_FRAME = 40;

SDL_Window* g_display_window;

bool g_game_is_running = true;
bool g_is_growing      = true;
int g_frame_counter = 0;

ShaderProgram g_program;
glm::mat4 g_view_matrix,
          g_model_matrix,
          g_model_matrix2,
          g_projection_matrix,
          g_trans_matrix;

float g_triangle_x      = 0.0f;
float g_triangle_rotate = 0.0f;
float g_previous_ticks  = 0.0f;
float g_triangle2_x      = 0.0f;
float g_triangle2_rotate = 0.0f;
float g_previous2_ticks  = 0.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0; // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0; // this value MUST be zero

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Delta Time!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    g_trans_matrix = g_model_matrix;
    
    g_program.SetProjectionMatrix(g_projection_matrix);
    g_program.SetViewMatrix(g_view_matrix);
    // Notice we haven't set our model matrix yet!
    
    g_program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);
    
    glUseProgram(g_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    player_texture_id = load_texture(PLAYER_SPRITE);
    player_texture_id2 = load_texture(PLAYER_SPRITE2);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

void update()
{
    
    
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    g_frame_counter++;
        
    glm::vec3 scale_vector;
    glm::vec3 scale_vector2;
        
    if (g_frame_counter >= MAX_FRAME)
    {
        g_is_growing = !g_is_growing;
        g_frame_counter = 0;
    }
    scale_vector = glm::vec3(g_is_growing ? (1.0f + delta_time * 8) : (1.0f - delta_time * 8),
                                 g_is_growing ? (1.0f + delta_time * 8) : (1.0f - delta_time * 8),
                                 1.0f);
    
    scale_vector2 = glm::vec3(g_is_growing ? (1.0f + delta_time * 10) : (1.0f - delta_time * 10),
                              g_is_growing ? (1.0f + delta_time * 10) : (1.0f - delta_time * 10),
                              1.0f);
    
    g_triangle_x += 1.0f * delta_time;
    g_triangle_rotate += DEGREES_PER_SECOND * delta_time; // 90-degrees per second
    g_triangle2_x += 0.5f * delta_time;
    g_triangle2_rotate += DEGREES_PER_SECOND/2 * delta_time; // 90-degrees per second
    g_model_matrix = glm::mat4(1.0f);
    g_model_matrix2 = glm::mat4(1.0f);

    /* Translate -> Rotate */
    
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(g_triangle_x, 0.0f, 0.0f));
    g_model_matrix = glm::rotate(g_model_matrix, glm::radians(g_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    g_model_matrix = glm::scale(g_model_matrix, scale_vector);
    
    g_model_matrix2 = glm::translate(g_model_matrix2, glm::vec3(g_triangle2_x, 0.0f, 0.0f));
    g_model_matrix2 = glm::scale(g_model_matrix2, scale_vector2);
    g_model_matrix2 = glm::rotate(g_model_matrix2, glm::radians(g_triangle2_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
    
    
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_program.SetModelMatrix(g_model_matrix);
    
    float vertices[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
        };
    
    glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.positionAttribute);
    
    // Textures
    float texture_coordinates[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
        };
    
    glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.texCoordAttribute);
    
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, player_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_program.positionAttribute);
    glDisableVertexAttribArray(g_program.texCoordAttribute);
    
    
    
    
    g_program.SetModelMatrix(g_model_matrix2);
    
    float vertices2[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
        };
    
    glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
    glEnableVertexAttribArray(g_program.positionAttribute);

    glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.texCoordAttribute);
    
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, player_texture_id2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(g_program.positionAttribute);
    glDisableVertexAttribArray(g_program.texCoordAttribute);

    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}

