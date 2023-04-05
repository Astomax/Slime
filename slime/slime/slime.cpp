#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <random>
#include <vector>
#include <math.h>
#include <array>
#include <thread>


const float PI = 3.141592f;
const int window_size = 600;
const int real_window_size = 600;

const int bot_count = 500000;

const float speed = 3;

const float range = 20;
const float spread = 0.4f * PI;

const float random_steer = 0.01f * PI;
const float steering_strength = 0.4 * PI;

int bots_grid[window_size][window_size] = { { 0 } };



int directions[9][2] = {
    { 0, 0 },
    { 0, 1 },
    { 1, 0 },
    { 0, -1 },
    { -1, 0 },
    { 1, 1 },
    { -1, 1 },
    { -1, -1 },
    { 1, -1 }
};

const float fade_speed = 0.1f;
int color_treshold =4;
float texture[window_size][window_size] = { { 0 } };



float RandomFloat(float Max) {
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / Max));
}

float min_blur = 0;

float blur_directions[8][3] = {
    { 1, 0, 0.125f },
    { 0, -1, 0.125f },
    { -1, 0, 0.125f },
    { 0, 1, 0.125f },
    { 1, 1, 0.125f },
    { -1, 1, 0.125f },
    { -1, -1, 0.125f },
    { 1, -1, 0.125f }
};


void Blur() {



    float invert = 1.0f - min_blur;

    for (int i = 1; i < window_size - 1; i++) {
        for (int i2 = 1; i2 < window_size - 1; i2++) {

            float blur = min_blur;

            for (int i3 = 0; i3 < 9; i3++) {

                float map_value = texture[i + int(blur_directions[i3][0])][i2 + int(blur_directions[i3][1])];

                blur += blur_directions[i3][2] * map_value * invert;
            }

            texture[i][i2] = blur;
        }
    }
}





float color_count = 400;
float rainbow_colors[500][3] = { { 0, 0, 0 } };

void FillRainbowColors() {

    int filling[6][2] = {
        { 1, 0 },
        { 0, 1 },
        { 2, 0 },
        { 1, 1 },
        { 0, 0 },
        { 2, 1}
    };

    float start_colors[6][3] = {
        { 1, 0, 0 },
        { 1, 1, 0 },
        { 0, 1, 0 },
        { 0, 1, 1 },
        { 0, 0, 1 },
        { 1, 0, 1 }
    };

    for (int i = 0; i < 6; i++) {

        float r = start_colors[i][0];
        float g = start_colors[i][1];
        float b = start_colors[i][2];

        bool removing = false;
        if (filling[i][1] == 1) {
            removing = true;
        }

        for (int i2 = 0; i2 < color_count / 6; i2++) {
            int index = (i * color_count / 6) + i2;

            switch (filling[i][0]) {
            case 0:
                if (removing) {
                    r -= 1.0f / (color_count / 6);
                    break;
                }
                r += 1.0f / (color_count / 6);
                break;

            case 1:
                if (removing) {
                    g -= 1.0f / (color_count / 6);
                    break;
                }
                g += 1.0f / (color_count / 6);
                break;

            case 2:
                if (removing) {
                    b -= 1.0f / (color_count / 6);
                    break;
                }
                b += 1.0f / (color_count / 6);
                break;
            }

            rainbow_colors[index][0] = r;
            rainbow_colors[index][1] = g;
            rainbow_colors[index][2] = b;

        }
    }
}



struct Bot {

    float x;
    float y;

    float angle;

    void AdjustAngle() {
        float new_angle = angle - spread + RandomFloat(random_steer);


        int highest = 1;
        int index = 1;

        for (int i = 0; i < 3; i++) {

            int new_x = x + cos(new_angle) * range;
            int new_y = y + sin(new_angle) * range;


            int value = 0;


            for (int i2 = 0; i2 < 9; i2++) {
                int new_x2 = new_x + directions[i2][0];
                int new_y2 = new_y + directions[i2][1];



                if (new_x2 < 0) {
                    new_x2 += window_size;
                }
                else if (new_x2 > window_size - 1) {
                    new_x2 -= window_size;
                }

                if (new_y2 < 0) {
                    new_y2 += window_size;
                }
                else if (new_y2 > window_size - 1) {
                    new_y2 -= window_size;
                }


                value += bots_grid[new_x2][new_y2];
            }



            if (value >= highest) {
                highest = value;
                index = i;
            }

            new_angle += spread;
        }

        index -= 1;
        angle = angle + steering_strength * index;


    }

    


    void Move() {
        x += cos(angle) * speed;
        y += sin(angle) * speed;


        if (y > window_size - 1) {
            y -= window_size;
        }
        else if (y < 0) {
            y += window_size;
        }

        if (x > window_size - 1) {
            x -= window_size;
        }
        else if (x < 0) {
            x += window_size;
        }
    }
};




int RandomMinPlus() {
    if (rand() % 2 == 0) {
        return -1;
    }
    return 1;
}


Bot* bots = new Bot[bot_count];

void MoveBots(int from, int to) {
    for (int i = from; i < to; i++) {

        bots[i].AdjustAngle();
        bots[i].Move();
    }
}



int main(void)
{
    std::srand(time(NULL));

    GLFWwindow* window;

    FillRainbowColors();
    

    for (int i = 0; i < bot_count; i++) {
        float x = rand() % window_size;
        float y = rand() % window_size;

        float random_angle = RandomFloat(2 * PI);

        Bot new_bot;

        new_bot.x = x;
        new_bot.y = y;
        new_bot.angle = random_angle;

        bots[i] = new_bot;

    }


    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(real_window_size, real_window_size, "enge dingen", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0.0, window_size, window_size, 0.0);





   
    float angle = 0.001f;
   
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */


        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);


      


        glBegin(GL_POINTS);



        for (int i = 0; i < bot_count; i++) {
            int x = bots[i].x;
            int y = bots[i].y;

            bots_grid[x][y]++;
        }

        double xpos, ypos;
        
        glfwGetCursorPos(window, &xpos, &ypos);

        if (xpos >= 0 && xpos < window_size && ypos >= 0 && ypos < window_size) {
            for (int r = 1; r < 700; r++) {
                break;

                for (int i2 = 0; i2 < 20; i2++) {

                    float x = (float(r) * cos(angle)) + (float)xpos;
                    float y = (float(r) * sin(angle)) + (float)ypos;



                    int final_x = round(x);
                    int final_y = round(y);

                    angle += (2 * PI) / 49;

                    if (final_x >= 0 && final_x < window_size && final_y >= 0 && final_y < window_size) {
                        bots_grid[final_x][final_y] = 10000000;
                    }

                }
            }



        }
        

        int thread_count = 20;
        int a = bot_count / thread_count;

        std::thread t0(MoveBots, a * 0, a * 1);
        std::thread t1(MoveBots, a * 1, a * 2);
        std::thread t2(MoveBots, a * 2, a * 3);
        std::thread t3(MoveBots, a * 3, a * 4);
        std::thread t4(MoveBots, a * 4, a * 5);
        std::thread t5(MoveBots, a * 5, a * 6);
        std::thread t6(MoveBots, a * 6, a * 7);
        std::thread t7(MoveBots, a * 7, a * 8);
        std::thread t8(MoveBots, a * 8, a * 9);
        std::thread t9(MoveBots, a * 9, a * 10);
        std::thread t10(MoveBots, a * 10, a * 11);
        std::thread t11(MoveBots, a * 11, a * 12);
        std::thread t12(MoveBots, a * 12, a * 13);
        std::thread t13(MoveBots, a * 13, a * 14);
        std::thread t14(MoveBots, a * 14, a * 15);
        std::thread t15(MoveBots, a * 15, a * 16);
        std::thread t16(MoveBots, a * 16, a * 17);
        std::thread t17(MoveBots, a * 17, a * 18);
        std::thread t18(MoveBots, a * 18, a * 19);
        std::thread t19(MoveBots, a * 19, a * 20);

        t0.join();
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
        t7.join();
        t8.join();
        t9.join();
        t10.join();
        t11.join();
        t12.join();
        t13.join();
        t14.join();
        t15.join();
        t16.join();
        t17.join();
        t18.join();
        t19.join();



       
       
        Blur();
        
        int highest = 0;

        for (int i = 0; i < window_size; i++) {
            for (int i2 = 0; i2 < window_size; i2++) {
                int value = bots_grid[i][i2];
                if (highest < value) {
                    highest = value;
                }
            }
        }
        
        float highest2 = 0;

        for (int i = 0; i < window_size; i++) {
            for (int i2 = 0; i2 < window_size; i2++) {
                
                float value = bots_grid[i][i2];

                float color = (1.0f / color_treshold) * value;

                float value2 = texture[i][i2] + color - fade_speed;

                if (highest2 < value2) {
                    highest2 = value2;
                }
            }
        }


        for (int i = 0; i < window_size; i++) {
            for (int i2 = 0; i2 < window_size; i2++) {
                float value = bots_grid[i][i2];
                
                float color = (1.0f / color_treshold) * value;

                texture[i][i2] += color - fade_speed;


                float value2 = texture[i][i2];

                float value3 = (1.0f / highest2) * value2;
                
                if (value3 < 0) {
                    value3 = 0;
                }

                int index = floor(value3 * 500.0f);

                float r = rainbow_colors[index][0];
                float g = rainbow_colors[index][1];
                float b = rainbow_colors[index][2];

                //glColor3f(color, color, color);
                //glColor3f(r, g, b);
                glColor3f(color, g, b);
                glVertex2f(i, i2);

                bots_grid[i][i2] = 0;
            }
        }






        glEnd();



        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
