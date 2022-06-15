#define GL_SILENCE_DEPRACATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES
#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(statement) std::cout << statement << "\n"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const float BG_RED = 0.29f;
const float BG_GREEN = 0.48f;
const float BG_BLUE = 0.28f;
const float BG_OPACITY = 1.0f;
const int VIEWPORT_X = 0;
const int VIEWPORT_Y = 0;
const int VIEWPORT_WIDTH = WINDOW_WIDTH;
const int VIEWPORT_HEIGHT = WINDOW_HEIGHT;
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
const char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";
const char SPRITE1[] = "banana.png";
const char SPRITE2[] = "banana.png";
const char SPRITE3[] = "monkey.png";
float previous_ticks = 0.0f;
float trans_y;
float rotate_x;
int counter = 0;
int limit = 100;
bool jump = true;

SDL_Window* display_window;
bool game_is_running = true;
ShaderProgram program;
glm::mat4 view_matrix;
glm::mat4 projection_matrix;
glm::mat4 model_banana1;
glm::mat4 model_banana2;
glm::mat4 model_monkey;

GLuint player_texture_id1;
GLuint player_texture_id2;
GLuint player_texture_id3;

GLuint load_texture(const char* filepath) {
    int image_width;
    int image_height;
    int number_of_components;
    unsigned char* image = stbi_load(filepath, &image_width, &image_height, &number_of_components, STBI_rgb_alpha);
    if(image == NULL){
        LOG("Unable to load image. Make sure the path is correct.");
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

void initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Simple 2D Scene", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    view_matrix = glm::mat4(1.0f);
    model_banana1 = glm::mat4(1.0f);
    model_banana2 = glm::mat4(1.0f);
    model_monkey = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    trans_y = 0.0f;
    rotate_x = 0.0f;
    program.SetViewMatrix(view_matrix);
    program.SetProjectionMatrix(projection_matrix);
    player_texture_id1 = load_texture(SPRITE1);
    player_texture_id2 = load_texture(SPRITE2);
    player_texture_id3 = load_texture(SPRITE3);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            game_is_running = false;
        }
    }
}

void update() {
    counter++;
    model_banana1 = glm::mat4(1.0f);
    model_banana2 = glm::mat4(1.0f);
    model_monkey = glm::mat4(1.0f);
    model_banana1 = glm::translate(model_banana1, glm::vec3(2.0f, 2.0f, 0.0f));
    model_banana2 = glm::translate(model_banana2, glm::vec3(-2.0f, 2.0f, 0.0f));
    model_monkey = glm::translate(model_monkey, glm::vec3(0.0f, -4.0f, 0.0f));


    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    rotate_x += 90.0f * delta_time;

    model_banana1 = glm::rotate(model_banana1, glm::radians(rotate_x), glm::vec3(0.0f, 0.0f, 1.0f));
    model_banana2 = glm::rotate(model_banana2, -glm::radians(rotate_x), glm::vec3(0.0f, 0.0f, 1.0f));
    if (counter > limit) {
        jump = !jump;
        counter = 0;
    }
    if (jump) {
        trans_y += 2.0f * delta_time;
    }
    else {
        trans_y -= 2.0f * delta_time;
    }
    model_monkey = glm::translate(model_monkey, glm::vec3(0.0f, trans_y, 0.0f));
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id) {
    program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    float vertices[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
    };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(program.texCoordAttribute);

    draw_object(model_banana1, player_texture_id1);
    draw_object(model_banana2, player_texture_id2);
    draw_object(model_monkey, player_texture_id3);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
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