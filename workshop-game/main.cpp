
#include <stdio.h>
#include <chrono>
#include <thread>

#include "imgui/imgui.h"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "draw_game.h"

#include "box2d/box2d.h"

// GLFW main window pointer
GLFWwindow* g_mainWindow = nullptr;

b2World* g_world;


void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW error occured. Code: %d. Description: %s\n", error, description);
}

int main()
{

    if (glfwInit() == 0) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    g_mainWindow = glfwCreateWindow(g_camera.m_width, g_camera.m_height, "My game", NULL, NULL);

    if (g_mainWindow == NULL) {
        fprintf(stderr, "Failed to open GLFW g_mainWindow.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(g_mainWindow);

    // Load OpenGL functions using glad
    int version = gladLoadGL(glfwGetProcAddress);

    // setup Box2D world and apply gravity
    b2Vec2 gravity;
    gravity.Set(0.0f, -10.0f);
    g_world = new b2World(gravity);

    // create debug draw. This will actually render all the game for us
    g_debugDraw.Create();
    g_world->SetDebugDraw(&g_debugDraw);


    // create ground and add it to the world
    b2BodyDef bodyDef;
    b2Body* groundBody = g_world->CreateBody(&bodyDef);

    //sample box body
    b2Body* b1;
    {
        b2EdgeShape shape;
        shape.SetTwoSided(b2Vec2(-40.0f, 0.0f), b2Vec2(40.0f, 0.0f));

        b2BodyDef bd;
        b1 = g_world->CreateBody(&bd);
        b1->CreateFixture(&shape, 0.0f);
    }

    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Control the frame rate. One draw per monitor refresh.
    std::chrono::duration<double> frameTime(0.0);
    std::chrono::duration<double> sleepAdjust(0.0);

    //main application loop
    while (!glfwWindowShouldClose(g_mainWindow)) {
        // use std::chrono to control frame rate. Objective here is to maintain
        // a steady 60 frames per second (no more, hopefully no less)
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

        glfwGetWindowSize(g_mainWindow, &g_camera.m_width, &g_camera.m_height);

        int bufferWidth, bufferHeight;
        glfwGetFramebufferSize(g_mainWindow, &bufferWidth, &bufferHeight);
        glViewport(0, 0, bufferWidth, bufferHeight);

        // clear previous frame (avoid creates shadows)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // run the simulation for one frame
        float timeStep = 60 > 0.0f ? 1.0f / 60 : float(0.0f);

        // enable objects to be draw
        uint32 flags = 0;
        flags += b2Draw::e_shapeBit;
        flags += b2Draw::e_jointBit;
        flags += b2Draw::e_aabbBit;
        flags += b2Draw::e_centerOfMassBit;

        g_debugDraw.SetFlags(flags);
        g_world->Step(timeStep, 8, 3); // TODO: explain these parameters
        g_world->DebugDraw();
        g_debugDraw.Flush();

        // put the rendered frame on the screen
        glfwSwapBuffers(g_mainWindow);

        glfwPollEvents();

        // Throttle to cap at 60 FPS. Which means if it's going to be past
        // 60FPS, sleeps a while instead of doing more frames.
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> target(1.0 / 60.0);
        std::chrono::duration<double> timeUsed = t2 - t1;
        std::chrono::duration<double> sleepTime = target - timeUsed + sleepAdjust;
        if (sleepTime > std::chrono::duration<double>(0)) {
            std::this_thread::sleep_for(sleepTime);
        }
        std::chrono::steady_clock::time_point t3 = std::chrono::steady_clock::now();
        frameTime = t3 - t1;

        // Compute the sleep adjustment using a low pass filter
        sleepAdjust = 0.9 * sleepAdjust + 0.1 * (target - frameTime);
    }


    glfwTerminate();

    g_debugDraw.Destroy();
    delete g_world;
    return 0;
}
