
#include <stdio.h>
#include <chrono>
#include <thread>

#include "imgui/imgui.h"
#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "box2d/box2d.h"

// GLFW main window pointer
GLFWwindow* g_mainWindow = nullptr;


struct Camera {
    Camera()
    {
        m_width = 1280;
        m_height = 800;
        ResetView();
    }

    void ResetView()
    {
        m_center.Set(0.0f, 20.0f);
        m_zoom = 1.0f;
    }
    b2Vec2 ConvertScreenToWorld(const b2Vec2& ps)
    {
        float w = float(m_width);
        float h = float(m_height);
        float u = ps.x / w;
        float v = (h - ps.y) / h;

        float ratio = w / h;
        b2Vec2 extents(ratio * 25.0f, 25.0f);
        extents *= m_zoom;

        b2Vec2 lower = m_center - extents;
        b2Vec2 upper = m_center + extents;

        b2Vec2 pw;
        pw.x = (1.0f - u) * lower.x + u * upper.x;
        pw.y = (1.0f - v) * lower.y + v * upper.y;
        return pw;
    }
    b2Vec2 ConvertWorldToScreen(const b2Vec2& pw)
    {
        float w = float(m_width);
        float h = float(m_height);
        float ratio = w / h;
        b2Vec2 extents(ratio * 25.0f, 25.0f);
        extents *= m_zoom;

        b2Vec2 lower = m_center - extents;
        b2Vec2 upper = m_center + extents;

        float u = (pw.x - lower.x) / (upper.x - lower.x);
        float v = (pw.y - lower.y) / (upper.y - lower.y);

        b2Vec2 ps;
        ps.x = u * w;
        ps.y = (1.0f - v) * h;
        return ps;
    }
    void BuildProjectionMatrix(float* m, float zBias);

    b2Vec2 m_center;
    float m_zoom;
    int32 m_width;
    int32 m_height;
};

void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW error occured. Code: %d. Description: %s\n", error, description);
}

int main()
{

    Camera camera;
    camera.m_height = 800;
    camera.m_width = 800;

    if (glfwInit() == 0) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    g_mainWindow = glfwCreateWindow(camera.m_width, camera.m_height, "My game", NULL, NULL);

    if (g_mainWindow == NULL) {
        fprintf(stderr, "Failed to open GLFW g_mainWindow.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(g_mainWindow);

    // Load OpenGL functions using glad
    int version = gladLoadGL(glfwGetProcAddress);

    // Control the frame rate. One draw per monitor refresh.

    // clear previous frame (avoid creates shadows)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    std::chrono::duration<double> frameTime(0.0);
    std::chrono::duration<double> sleepAdjust(0.0);

    //main application loop
    while (!glfwWindowShouldClose(g_mainWindow)) {
        // use std::chrono to control frame rate. Objective here is to maintain
        // a steady 60 frames per second (no more, hopefully no less)
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

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
    return 0;
}
