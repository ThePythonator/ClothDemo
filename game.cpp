#include "game.hpp"

using namespace blit;

const float GRAVITY = 200.0f;

const uint16_t WINDOW_WIDTH = 320;
const uint16_t WINDOW_HEIGHT = 240;

const uint8_t CLOTH_WIDTH = 10;
const uint8_t CLOTH_HEIGHT = 10;

const float SPEED = 80.0f;

class Node {
public:
    float x, y;

    Node() {
        x = y = 0.0f;
        last_x = last_y = 0.0f;
    }

    Node(float x, float y) {
        this->x = x;
        this->y = y;

        last_x = x;
        last_y = y;
    }

    void update(float dt) {
        float x_vel = x - last_x;
        float y_vel = y - last_y;

        x_vel *= 0.99f;
        y_vel *= 0.99f;


        if (x < 0) {
            x = 0;
            x_vel = -x_vel;
        }
        else if (x > WINDOW_WIDTH) {
            x = WINDOW_WIDTH;
            x_vel = -x_vel;
        }

        if (y < 0) {
            y = 0;
            y_vel = -y_vel;
        }
        else if (y > WINDOW_HEIGHT) {
            y = WINDOW_HEIGHT;
            y_vel = -y_vel;
        }

        float next_x = x + x_vel;
        float next_y = y + y_vel + 0.5f * GRAVITY * dt * dt;

        last_x = x;
        last_y = y;

        x = next_x;
        y = next_y;
    }

    void render() {
        screen.circle(Point(x, y), 1);
    }

private:
    float last_x, last_y;
};

class Line {
public:
    Line() {
        n1 = n2 = nullptr;
        normal_length = 5.0f;
        max_length = 10.0f;
        strength = 1.0f;
    }

    Line(Node* n1, Node* n2, float normal_length, float max_length, float strength = 1.0f) {
        this->n1 = n1;
        this->n2 = n2;

        this->normal_length = normal_length;
        this->max_length = max_length;
        this->strength = strength;
    }

    void constrain_nodes() {
        float dist_x = n1->x - n2->x;
        float dist_y = n1->y - n2->y;
        float dist = std::sqrtf(dist_x * dist_x + dist_y * dist_y);
        //printf("%f, %f\n", dist, max_length);

        if (dist > max_length) {
            broken = true;
        }

        if (!broken) {
            float percentage = (normal_length - dist) / dist;

            float shift_x = dist_x * percentage * 0.5f * strength;
            float shift_y = dist_y * percentage * 0.5f * strength;

            n1->x += shift_x;
            n1->y += shift_y;

            n2->x -= shift_x;
            n2->y -= shift_y;
        }
    }

    void render() {
        if (!broken) {
            screen.line(Point(n1->x, n1->y), Point(n2->x, n2->y));
        }
    }

private:
    Node* n1, * n2;

    float normal_length;
    float max_length;

    // Strength is between 0 and 1
    // 1 instantly pulls back to normal length, and 0.1 takes a long time to pull back to normal length (0 never does)
    float strength;

    bool broken = false;
};

const uint16_t s = (CLOTH_WIDTH - 1) * CLOTH_HEIGHT + (CLOTH_HEIGHT - 1) * CLOTH_WIDTH;// +(CLOTH_WIDTH - 1) * (CLOTH_HEIGHT - 1);

Node nodes[CLOTH_WIDTH * CLOTH_HEIGHT];
Line lines[s];

float dt = 0.0f;
float last = 0.0f;

float left_x = 0.0f;
float right_x = 0.0f;
float left_y = 0.0f;
float right_y = 0.0f;


///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() {
    set_screen_mode(ScreenMode::hires);

    float base_x = 20;
    float base_y = 20;

    float step = 10;

    float normal = std::sqrtf(base_x * base_x + base_y * base_y);

    for (uint16_t i = 0; i < CLOTH_WIDTH; i++) {
        for (uint16_t j = 0; j < CLOTH_HEIGHT; j++) {
            nodes[i * CLOTH_WIDTH + j] = Node(base_x + i * step, base_y + j * step);
        }
    }

    for (uint16_t i = 0; i < CLOTH_WIDTH; i++) {
        for (uint16_t j = 0; j < CLOTH_HEIGHT - 1; j++) {
            lines[i * (CLOTH_WIDTH - 1) + j] = Line(&nodes[i * CLOTH_WIDTH + j], &nodes[i * CLOTH_WIDTH + j + 1], step, step * 3.0f, 0.5f);
        }
    }
    for (uint16_t i = 0; i < CLOTH_WIDTH - 1; i++) {
        for (uint16_t j = 0; j < CLOTH_HEIGHT; j++) {
            lines[(CLOTH_WIDTH - 1) * CLOTH_HEIGHT + i * (CLOTH_WIDTH) + j] = Line(&nodes[i * CLOTH_WIDTH + j], &nodes[(i + 1) * CLOTH_WIDTH + j], step, step * 3.0f, 0.5f);
        }
    }
    /*for (uint16_t i = 0; i < CLOTH_WIDTH - 1; i++) {
        for (uint16_t j = 0; j < CLOTH_HEIGHT - 1; j++) {
            lines[(CLOTH_WIDTH - 1) * CLOTH_HEIGHT + (CLOTH_HEIGHT - 1) * CLOTH_WIDTH + i * (CLOTH_WIDTH - 1) + j] = Line(&nodes[i * CLOTH_WIDTH + j], &nodes[(i + 1) * CLOTH_WIDTH + j + 1], normal, normal * 3.0f, 0.0f);
        }
    }*/


    left_x = nodes[0].x;
    left_y = nodes[0].y;

    right_x = nodes[CLOTH_WIDTH * (CLOTH_HEIGHT - 1)].x;
    right_y = nodes[CLOTH_WIDTH * (CLOTH_HEIGHT - 1)].y;
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) {

    // clear the screen -- screen is a reference to the frame buffer and can be used to draw all things with the 32blit
    screen.clear();

    // draw some text at the top of the screen
    screen.alpha = 255;
    screen.mask = nullptr;

    screen.pen = Pen(255, 255, 255);
    for (uint16_t i = 0; i < s; i++) {
        lines[i].render();
    }

    screen.pen = Pen(255, 0, 255);
    for (uint16_t i = 0; i < CLOTH_WIDTH * CLOTH_HEIGHT; i++) {
        nodes[i].render();
    }

    screen.pen = Pen(0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) {
    // Get dt
    dt = (time - last) / 1000.0f;
    last = time;

    if (buttons.state & Button::DPAD_DOWN) {
        left_y += SPEED * dt;
    }
    if (buttons.state & Button::DPAD_UP) {
        left_y -= SPEED * dt;
    }
    if (buttons.state & Button::DPAD_RIGHT) {
        left_x += SPEED * dt;
    }
    if (buttons.state & Button::DPAD_LEFT) {
        left_x -= SPEED * dt;
    }

    if (buttons.state & Button::B) {
        right_y += SPEED * dt;
    }
    if (buttons.state & Button::X) {
        right_y -= SPEED * dt;
    }
    if (buttons.state & Button::A) {
        right_x += SPEED * dt;
    }
    if (buttons.state & Button::Y) {
        right_x -= SPEED * dt;
    }

    nodes[0].x = left_x;
    nodes[0].y = left_y;

    nodes[CLOTH_WIDTH * (CLOTH_HEIGHT - 1)].x = right_x;
    nodes[CLOTH_WIDTH * (CLOTH_HEIGHT - 1)].y = right_y;



    for (uint8_t j = 0; j < 2; j++) {
        // Loop to get a better approximation
        for (uint16_t i = 0; i < s; i++) {
            lines[i].constrain_nodes();
        }
    }

    for (uint16_t i = 0; i < CLOTH_WIDTH * CLOTH_HEIGHT; i++) {
        nodes[i].update(dt);
    }


    /*left_x = nodes[0].x;
    left_y = nodes[0].y;

    right_x = nodes[CLOTH_WIDTH * (CLOTH_HEIGHT - 1)].x;
    right_y = nodes[CLOTH_WIDTH * (CLOTH_HEIGHT - 1)].y;*/
}