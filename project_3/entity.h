#ifndef ENTITY_H
#define ENTITY_H

enum EntityType{ PLATFORM, PLAYER, ITEM};

class Entity{
    private:
        int* animation_left = NULL;
        int* animation_right = NULL;
        int* animation_up = NULL;
        int* animation_down = NULL;

        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;

        int width = 1;
        int height = 1;

    public: 
        static const int SECONDS_PER_FRAME = 4;
        static const int LEFT = 0;
        static const int RIGHT = 1;
        static const int UP = 2;
        static const int DOWN = 3;

        float speed;
        glm::vec3 movement;

        GLuint texture_id;

        glm::mat4 model_matrix;

        int** walking = new int*[4]{
            animation_left, animation_right, animation_up, animation_down
        };

        int* animation_indices = NULL;
        int animation_frames = 0;
        int animation_index = 0;

        float animation_time = 0.0f;

        int animation_cols = 0;
        int animation_rows = 0;

        bool is_jumping = false;
        float jumping_power = 0;

        bool collided_top = false;
        bool collided_bottom = false;
        bool collided_left = false;
        bool collided_right = false;

        Entity();

        ~Entity();

        void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);

        void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);

        void render(ShaderProgram* program);

        void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);

        void const check_collision_x(Entity* colldable_entities, int collidable_entity_count);

        void const check_collision(Entity* other) const;

        void activate(){
            is_active = true;
        };

        void deactive(){
            is_active = false;
        };

        glm::vec3 const get_position() const{
            return position;
        };

        glm::vec3 const get_movement() const{
            return movement;
        };
        
        glm::vec3 const get_velocity() const{
            return velocity;
        };

        glm::vec3 const get_acceleration() const{
            return acceleration;
        };

        int const get_width() const{
            return width;
        };

        int const get_height() const{
            return height;
        };

        void const set_position(glm::vec3 new_position){
            position = new_position;
        };

        void const set_movement(glm::vec3 new_movement){
            movement = new_movement;
        };

        void const set_veloctity(glm::vec3 new_velocity){
            velocity = new_velocity;
        };

        void const set_acceleration(glm::vec3 new_acceleration){
            acceleration = new_acceleration;
        };

        void const set_width(float new_width){
            width = new_width;
        };

        void const set_height(float new_height){
            height = new_height;
        };
};
#endif
