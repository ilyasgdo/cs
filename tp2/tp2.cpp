#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include "common/GLShader.h"
#include <cstddef>

GLShader g_BasicShader;
GLuint g_VBO = 0;

static int g_WindowWidth = 640;
static int g_WindowHeight = 480;

void OnWindowSize(GLFWwindow*, int w, int h)
{
    g_WindowWidth = w;
    g_WindowHeight = h;
}

void OnFramebufferSize(GLFWwindow*, int fbw, int fbh)
{
    glViewport(0, 0, fbw, fbh);
}


struct Vertex {
    float px, py;
    float r, g, b;
};

static const Vertex triangle[] = {
    {-0.5f, -0.5f, 1.f, 1.f, 0.f},
    { 0.5f, -0.5f, 1.f, 1.f, 0.f},
    { 0.0f,  0.5f, 1.f, 1.f, 1.f}
};

bool Initialise()
{
    if (!g_BasicShader.LoadVertexShader("basic.vs")) return false;
    if (!g_BasicShader.LoadFragmentShader("basic.fs")) return false;
    if (!g_BasicShader.Create()) return false;

#ifdef WIN32
    wglSwapIntervalEXT(1);
#endif

    glGenBuffers(1, &g_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

    GLuint basicProgram = g_BasicShader.GetProgram();
    glUseProgram(basicProgram);

    GLint loc_position = glGetAttribLocation(basicProgram, "a_position");
    GLint loc_color = glGetAttribLocation(basicProgram, "a_color");

    if (loc_position >= 0)
    {
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (const void*)offsetof(Vertex, px));
    }

    if (loc_color >= 0)
    {
        glEnableVertexAttribArray(loc_color);
        glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (const void*)offsetof(Vertex, r));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void Terminate()
{
    g_BasicShader.Destroy();
    glDeleteBuffers(1, &g_VBO);
}

void Render()
{
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint basicProgram = g_BasicShader.GetProgram();
    glUseProgram(basicProgram);

    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);

    GLint loc_position = glGetAttribLocation(basicProgram, "a_position");
    GLint loc_color = glGetAttribLocation(basicProgram, "a_color");

    if (loc_position >= 0)
    {
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (const void*)offsetof(Vertex, px));
    }

    if (loc_color >= 0)
    {
        glEnableVertexAttribArray(loc_color);
        glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (const void*)offsetof(Vertex, r));
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetWindowSizeCallback(window, OnWindowSize);
    glfwSetFramebufferSizeCallback(window, OnFramebufferSize);

    int fbw = 0;
    int fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    OnFramebufferSize(window, fbw, fbh);

    if (!Initialise())
    {
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window))
    {
        Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Terminate();
    glfwTerminate();
    return 0;
}
