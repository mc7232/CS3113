#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 2

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include <SDL_mixer.h>

struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* target;
    Entity* win;
    Entity* lose;
    Mix_Music* bgm;
};

const int WINDOW_WIDTH  = 960,
          WINDOW_HEIGHT = 720;

const float BG_RED     = 2.55f,
            BG_BLUE    = 2.55f,
            BG_GREEN   = 2.55f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const int FONTBANK_SIZE = 16;
const char TARGET[] = "assets/hand.png";
const char OBS[] = "assets/mizore.png";
const char PLAYER1[] = "assets/bbird.png";
const char TEXT[] = "assets/font.png";

GLuint text_texture_id;

//Bird
float bird[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
//Mizore
float mizo[] = {-1.0f, -2.0f, 1.0f, -2.0f, 1.0f, 2.0f, -1.0f, -2.0f, -1.0f, 2.0f, 1.0f, 2.0f};
//Hand
float hand[] = {-0.45f, -1.5f, 0.45f, -1.5f, 0.45f, 1.5f, -0.45f, -1.5f, -0.45f, 1.5f, 0.45f, 1.5f};

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

GameState state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;
glm::vec3 temp;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}


void DrawText(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;
    
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    for (int i = 0; i < text.size(); i++) {
        int spritesheet_index = (int) text[i];
        float offset = (screen_size + spacing) * i;
        
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Lander",
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
    
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    GLuint platform_texture_id = load_texture(TARGET);
    GLuint obstacle_texture_id = load_texture(OBS);
    
    state.platforms = new Entity[2];
    
    state.platforms[0].texture_id = platform_texture_id;
    state.platforms[0].set_position(glm::vec3(2.25f, -3.8f, 0.0f));
    state.platforms[0].set_width(1.0f);
    state.platforms[0].set_height(3.0f);
    state.platforms[0].update(0.0f, NULL, 0);
    
    state.platforms[1].texture_id = obstacle_texture_id;
    state.platforms[1].set_position(glm::vec3(4.0f, -1.9f, 0.0f));
    state.platforms[1].set_width(2.0f);
    state.platforms[1].set_height(4.0f);
    state.platforms[1].update(0.0f, NULL, 0);
    
    state.player = new Entity();
    state.player->set_position(glm::vec3(-4.0f, 4.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, -1.5f, 0.0f));
    state.player->texture_id = load_texture(PLAYER1);
    
    state.player->set_height(1.0f);
    state.player->set_width(1.0f);
    
    state.target = &state.platforms[0];
    state.win = new Entity();
    state.lose = new Entity();
    state.win->deactivate();
    state.lose->deactivate();
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.bgm = Mix_LoadMUS("assets/bgm.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
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
    
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A])
    {
        temp = state.player->get_acceleration();
        temp.x -= 0.5f;
        state.player->set_acceleration(temp);
    }
    else if (key_state[SDL_SCANCODE_D])
    {
        temp = state.player->get_acceleration();
        temp.x += 0.5f;
        state.player->set_acceleration(temp);
    }
    
    if (key_state[SDL_SCANCODE_W]){
        temp = state.player->get_velocity();
        temp.y += 0.05f;
        state.player->set_velocity(temp);
    }
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    
    delta_time += accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }
    if(state.player->collided_left || state.player->collided_right || state.player->get_position().y < -4.5f || state.player->get_position().x < -5.5f || state.player->get_position().x > 5.5f){
        state.lose->activate();
        state.player->deactivate();
    }else if(state.player->collided_bottom && state.player->get_position().y > -1.0f){
        state.lose->activate();
        state.player->deactivate();
    }else if(state.player->collided_bottom && state.player->get_position().y < -1.0f){
        state.win->activate();
        state.player->deactivate();
    //else if(state.player->check_collision(state.target)){
    //    state.win->activate();
    //    state.player->deactivate();
    }
    while (delta_time >= FIXED_TIMESTEP) {
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }
    
    accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    state.player->render(&program, bird);
    state.platforms[0].render(&program, hand);
    state.platforms[1].render(&program, mizo);
    text_texture_id = load_texture(TEXT);
    if(state.lose->get_active()){
        DrawText(&program, text_texture_id, "LOSE", 0.8f, 0.5f, glm::vec3(-2.0f, 2.0f, 0.0f));
    }else if(state.win->get_active()){
        DrawText(&program, text_texture_id, "WIN", 0.8f, 0.5f, glm::vec3(-1.5f, 2.0f, 0.0f));
    }
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete [] state.platforms;
    delete state.player;
    Mix_FreeMusic(state.bgm);
}

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
