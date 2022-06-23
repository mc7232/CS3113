#define GL_SILENCE_DEPRACATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <iostream>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

enum Coordinate {
    x_coordinate,
    y_coordinate
};

#define LOG(statement) std:: cout << statement << "\n"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

const float BG_RED = 0.0f;
const float BG_GREEN = 0.0f;
const float BG_BLUE = 0.0f;
const float BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0;
const int VIEWPORT_Y = 0;
const int VIEWPORT_WIDTH = WINDOW_WIDTH;
const int VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
const char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char PLAYER_ONE[] = "paddle.png";
const char PLAYER_TWO[] = "paddle.png";
const char BALL[] = "ball.png";

const float MINIMUM_COLLISION_DISTANCE = 0.5f;

SDL_Window* display_window;
bool game_is_running = true;
ShaderProgram program;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;
glm::mat4 player_one;
glm::mat4 player_two;
glm::mat4 ball;
float previous_ticks = 0.0f;

GLuint player_one_texture_id;
GLuint player_two_texture_id;
GLuint ball_texture_id;

SDL_Joystick* player_one_controller;

glm::vec3 player_one_position = glm::vec3(-4.5f, 0.0f, 0.0f);
glm::vec3 player_one_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_two_position = glm::vec3(4.5f, 0.0f, 0.0f);
glm::vec3 player_two_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f);
glm::vec3 ball_movement = glm::vec3(1.0f, 0.0f, 0.0f);

float get_screen_to_ortho(float coordinate, Coordinate axis) {
    switch (axis) {
    case x_coordinate:
        return ((coordinate / WINDOW_WIDTH * 10.0) - (10.0 / 2.0));
    case y_coordinate:
        return((((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * 7.5f) - (7.5f / 2.0));

    default:
        return 0.0f;
    }
}

GLuint load_texture(const char* filepath) {
    int image_width;
    int image_height;
    int number_of_components;
    unsigned char* image = stbi_load(filepath, &image_width, &image_height, &number_of_components, STBI_rgb_alpha);
    if (image == NULL) {
        LOG("Could not load image file. Please check filepath.");
        assert(false);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}

bool check_collision(glm::vec3& position_a, glm::vec3& position_b) {
    return sqrt(pow(position_b[0] - position_a[0], 2) + pow(position_b[1] - position_a[1], 2)) < MINIMUM_COLLISION_DISTANCE;
}

void initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    player_one_controller = SDL_JoystickOpen(0);
    display_window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    view_matrix = glm::mat4(1.0f);
    player_one = glm::mat4(1.0f);
    player_two = glm::mat4(1.0f);
    ball = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetViewMatrix(view_matrix);
    program.SetProjectionMatrix(projection_matrix);

    player_one_texture_id = load_texture(PLAYER_ONE);
    player_two_texture_id = load_texture(PLAYER_TWO);
    ball_texture_id = load_texture(BALL);

    glUseProgram(program.programID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

}

void process_input() {
    player_one_movement = glm::vec3(0.0f);
    player_two_movement = glm::vec3(0.0f);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (player_two_position.y < 3.0f) {
                    player_two_movement.y = 1.0f;
                }
                break;
            case SDLK_DOWN:
                if (player_two_position.y > -3.0f) {
                    player_two_movement.y = -1.0f;
                }
                break;
            case SDLK_w:
                if (player_one_position.y < 3.0f) {
                    player_one_movement.y = 1.0f;
                }
                break;
            case SDLK_s:
                if (player_one_position.y > -3.0f) {
                    player_one_movement.y = -1.0f;
                }
                break;
            case SDLK_q:
                game_is_running = false;
                break;
            default:
                break;
            }
        default:
            break;
        }
    }

    const Uint8* key_states = SDL_GetKeyboardState(NULL);
    if (key_states[SDL_SCANCODE_UP]) {
        if (player_two_position.y < 3.0f) {
            player_two_movement.y = 1.0f;
        }
    }
    else if (key_states[SDL_SCANCODE_DOWN]) {
        if (player_two_position.y > -3.0f) {
            player_two_movement.y = -1.0f;
        }
    }
    if (key_states[SDL_SCANCODE_W]) {
        if (player_one_position.y < 3.0f) {
            player_one_movement.y = 1.0f;
        }
    }
    else if (key_states[SDL_SCANCODE_S]) {
        if (player_one_position.y > -3.0f) {
            player_one_movement.y = -1.0f;
        }
    }
}

void update() {
    player_one = glm::mat4(1.0f);
    player_two = glm::mat4(1.0f);
    ball = glm::mat4(1.0f);
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    player_one_position += player_one_movement * delta_time * 1.5f;
    player_two_position += player_two_movement * delta_time * 1.5f;
    ball_position += ball_movement * delta_time * 2.5f;
    player_one = glm::translate(player_one, player_one_position);
    player_two = glm::translate(player_two, player_two_position);
    ball = glm::translate(ball, ball_position);
    if (ball_position.y > 3.5f || ball_position.y < -3.5f) {
        ball_movement.y = -ball_movement.y;
    }
    if (ball_position.x > 5.0f || ball_position.x < -5.0f) {
        game_is_running = false;
    }
    if (check_collision(player_one_position, ball_position)) {
        ball_movement.x = -ball_movement.x;
        if (ball_movement.y < 1.0f && ball_movement.y > -1.0f) {
            ball_movement.y = 1.0f;
        }
    }
    else if (check_collision(player_two_position, ball_position)) {
        ball_movement.x = -ball_movement.x;
        if (ball_movement.y < 1.0f && ball_movement.y > -1.0f) {
            ball_movement.y = -1.0f;
        }
    }
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id) {
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    float vertices[] = {
       -0.1f, -0.5f, 0.1f, -0.5f, 0.1f, 0.5f,
       -0.1f, -0.5f, -0.1f, 0.5f, 0.1f, 0.5f
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    draw_object(player_one, player_one_texture_id);
    draw_object(player_two, player_two_texture_id);

    float vertices2[] = {
        -0.2f, -0.2f, 0.2f, -0.2f, 0.2f, 0.2f,
        -0.2f, -0.2f, 0.2f, 0.2f, -0.2f, 0.2f
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
    glEnableVertexAttribArray(program.positionAttribute);
   

    draw_object(ball, ball_texture_id);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_JoystickClose(player_one_controller);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialize();
    while (game_is_running) {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}
