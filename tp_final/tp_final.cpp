#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB

#include <GLFW/glfw3.h>
#include "common/GLShader.h"
#include <cstddef>
#include <cmath>
#include "dragoondata.h"

const GLfloat cube_vertices[] = {
    -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0,  1.0,  1.0,  -1.0,  1.0,  1.0,
    -1.0, -1.0, -1.0,   1.0, -1.0, -1.0,   1.0,  1.0, -1.0,  -1.0,  1.0, -1.0
};

const GLushort cube_elements[] = {
    0,1,2, 2,3,0,
    1,5,6, 6,2,1,
    7,6,5, 5,4,7,
    4,0,3, 3,7,4,
    4,5,1, 1,0,4,
    3,2,6, 6,7,3
};

GLfloat g_CubeData[36 * 8];

GLShader g_BasicShader;

GLuint g_DragonVAO = 0;
GLuint g_DragonVBO = 0;
GLuint g_DragonEBO = 0;

GLuint g_CubeVAO = 0;
GLuint g_CubeVBO = 0;

GLuint g_Texture = 0;

static int g_WindowWidth = 800;
static int g_WindowHeight = 600;

static float g_Phi = 0.0f;
static float g_Theta = 0.0f;
static float g_Radius = 15.0f;

static double g_LastMouseX = 0.0;
static double g_LastMouseY = 0.0;
static bool g_MousePressed = false;

const float PI = 3.14159265359f;

void MatrixIdentity(float* m)
{
    for(int i = 0; i < 16; i++)
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

void MatrixMultiply(float* out, const float* a, const float* b)
{
    float temp[16];
    for(int c = 0; c < 4; c++) {
        for(int r = 0; r < 4; r++) {
            temp[c * 4 + r] = a[0*4 + r] * b[c*4 + 0]
                            + a[1*4 + r] * b[c*4 + 1]
                            + a[2*4 + r] * b[c*4 + 2]
                            + a[3*4 + r] * b[c*4 + 3];
        }
    }
    for(int i = 0; i < 16; i++)
        out[i] = temp[i];
}

void MatrixTranslation(float* m, float tx, float ty, float tz)
{
    MatrixIdentity(m);
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
}

void MatrixRotationY(float* m, float angle)
{
    MatrixIdentity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[0] = c;  m[8] = s;
    m[2] = -s; m[10] = c;
}

void MatrixRotationX(float* m, float angle)
{
    MatrixIdentity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[5] = c;  m[9] = -s;
    m[6] = s;  m[10] = c;
}

void MatrixRotationZ(float* m, float angle)
{
    MatrixIdentity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[0] = c; m[4] = -s;
    m[1] = s; m[5] = c;
}

void MatrixPerspective(float* m, float fov, float aspect, float near, float far)
{
    MatrixIdentity(m);
    float f = 1.0f / tanf((fov * PI / 180.0f) / 2.0f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}

void LookAt(float* m, float ex, float ey, float ez,
                       float tx, float ty, float tz,
                       float ux, float uy, float uz)
{
    float fx = -(tx - ex);
    float fy = -(ty - ey);
    float fz = -(tz - ez);
    float fl = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= fl;
    fy /= fl;
    fz /= fl;

    float rx = uy * fz - uz * fy;
    float ry = uz * fx - ux * fz;
    float rz = ux * fy - uy * fx;
    float rl = sqrtf(rx*rx + ry*ry + rz*rz);
    rx /= rl;
    ry /= rl;
    rz /= rl;

    float uux = fy * rz - fz * ry;
    float uuy = fz * rx - fx * rz;
    float uuz = fx * ry - fy * rx;

    MatrixIdentity(m);
    m[0]  = rx;   m[4]  = ry;   m[8]  = rz;
    m[1]  = uux;  m[5]  = uuy;  m[9]  = uuz;
    m[2]  = fx;   m[6]  = fy;   m[10] = fz;

    m[12] = -(rx * ex + ry * ey + rz * ez);
    m[13] = -(uux * ex + uuy * ey + uuz * ez);
    m[14] = -(fx * ex + fy * ey + fz * ez);
}


void OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_MousePressed = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &g_LastMouseX, &g_LastMouseY);
    }
}

void OnCursorPos(GLFWwindow* window, double xpos, double ypos)
{
    if (!g_MousePressed)
        return;

    float dx = (float)(xpos - g_LastMouseX);
    float dy = (float)(ypos - g_LastMouseY);

    g_Phi   += dx * 0.005f;
    g_Theta += dy * 0.005f;

    if (g_Phi > PI)
        g_Phi -= 2.0f * PI;
    if (g_Phi < -PI)
        g_Phi += 2.0f * PI;

    if (g_Theta > PI / 2.0f - 0.01f)
        g_Theta = PI / 2.0f - 0.01f;
    if (g_Theta < -PI / 2.0f + 0.01f)
        g_Theta = -PI / 2.0f + 0.01f;

    g_LastMouseX = xpos;
    g_LastMouseY = ypos;
}

void OnScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    g_Radius -= (float)yoffset;
    if (g_Radius < 1.0f)  g_Radius = 1.0f;
    if (g_Radius > 50.0f) g_Radius = 50.0f;
}

void OnFramebufferSize(GLFWwindow* window, int w, int h)
{
    g_WindowWidth = w;
    g_WindowHeight = h;
    glViewport(0, 0, w, h);
}


bool Initialise()
{
    if (!g_BasicShader.LoadVertexShader("basic.vs"))
        return false;
    if (!g_BasicShader.LoadFragmentShader("basic.fs"))
        return false;
    if (!g_BasicShader.Create())
        return false;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glGenTextures(1, &g_Texture);
    glBindTexture(GL_TEXTURE_2D, g_Texture);

    GLubyte texData[] = {255,255,255, 0,0,0, 0,0,0, 255,255,255};
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint prog = g_BasicShader.GetProgram();
    GLint lp = glGetAttribLocation(prog, "a_position");
    GLint ln = glGetAttribLocation(prog, "a_normal");
    GLint lt = glGetAttribLocation(prog, "a_texcoords");

    // --- Dragon ---

    glGenVertexArrays(1, &g_DragonVAO);
    glBindVertexArray(g_DragonVAO);

    glGenBuffers(1, &g_DragonVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_DragonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DragonVertices), DragonVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_DragonEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_DragonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DragonIndices), DragonIndices, GL_STATIC_DRAW);

    if(lp >= 0){
        glEnableVertexAttribArray(lp);
        glVertexAttribPointer(lp, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    }
    if(ln >= 0){
        glEnableVertexAttribArray(ln);
        glVertexAttribPointer(ln, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    if(lt >= 0){
        glEnableVertexAttribArray(lt);
        glVertexAttribPointer(lt, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    }

    // --- Cube (generation des normales par produit vectoriel) ---

    for(int i = 0; i < 12; i++)
    {
        int i0 = cube_elements[i * 3 + 0] * 3;
        int i1 = cube_elements[i * 3 + 1] * 3;
        int i2 = cube_elements[i * 3 + 2] * 3;

        float p0[3] = { cube_vertices[i0], cube_vertices[i0+1], cube_vertices[i0+2] };
        float p1[3] = { cube_vertices[i1], cube_vertices[i1+1], cube_vertices[i1+2] };
        float p2[3] = { cube_vertices[i2], cube_vertices[i2+1], cube_vertices[i2+2] };

        float u[3] = { p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2] };
        float v[3] = { p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2] };

        float n[3];
        n[0] = u[1]*v[2] - u[2]*v[1];
        n[1] = u[2]*v[0] - u[0]*v[2];
        n[2] = u[0]*v[1] - u[1]*v[0];

        float len = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
        if(len > 0){
            n[0] /= len;
            n[1] /= len;
            n[2] /= len;
        }

        for(int j = 0; j < 3; j++)
        {
            float *p;
            if(j == 0) p = p0;
            else if(j == 1) p = p1;
            else p = p2;

            int base = (i * 3 + j) * 8;
            g_CubeData[base + 0] = p[0];
            g_CubeData[base + 1] = p[1];
            g_CubeData[base + 2] = p[2];
            g_CubeData[base + 3] = n[0];
            g_CubeData[base + 4] = n[1];
            g_CubeData[base + 5] = n[2];
            g_CubeData[base + 6] = (j == 1 || (i % 2 == 0 && j == 2)) ? 1.0f : 0.0f;
            g_CubeData[base + 7] = (j == 2) ? 1.0f : 0.0f;
        }
    }

    glGenVertexArrays(1, &g_CubeVAO);
    glBindVertexArray(g_CubeVAO);

    glGenBuffers(1, &g_CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_CubeData), g_CubeData, GL_STATIC_DRAW);

    if(lp >= 0){
        glEnableVertexAttribArray(lp);
        glVertexAttribPointer(lp, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    }
    if(ln >= 0){
        glEnableVertexAttribArray(ln);
        glVertexAttribPointer(ln, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    if(lt >= 0){
        glEnableVertexAttribArray(lt);
        glVertexAttribPointer(lt, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(0);

    return true;
}


void Render(float time)
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint prog = g_BasicShader.GetProgram();
    glUseProgram(prog);

    float camX = g_Radius * cosf(g_Theta) * cosf(g_Phi);
    float camY = g_Radius * sinf(g_Theta);
    float camZ = g_Radius * cosf(g_Theta) * sinf(g_Phi);

    float view[16];
    float proj[16];
    LookAt(view, camX, camY, camZ, 0, 0, 0, 0, 1, 0);
    MatrixPerspective(proj, 45.0f, (float)g_WindowWidth / (float)g_WindowHeight, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Projection"), 1, GL_FALSE, proj);

    glUniform3f(glGetUniformLocation(prog, "u_CameraPos"), camX, camY, camZ);

    glUniform3f(glGetUniformLocation(prog, "u_light.direction"), 1.0f, -1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(prog, "u_light.diffuseColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(prog, "u_light.specularColor"), 1.0f, 1.0f, 1.0f);

    glUniform3f(glGetUniformLocation(prog, "u_SkyDirection"), 0.0f, 1.0f, 0.0f);
    glUniform3f(glGetUniformLocation(prog, "u_SkyColor"), 0.6f, 0.8f, 1.0f);
    glUniform3f(glGetUniformLocation(prog, "u_GroundColor"), 0.2f, 0.2f, 0.1f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_Texture);
    glUniform1i(glGetUniformLocation(prog, "u_diffuseMap"), 0);

    // --- dragon ---

    float worldDragon[16], tDragon[16], rDragon[16];
    MatrixTranslation(tDragon, 4.0f, -4.0f, 0.0f);
    MatrixRotationY(rDragon, time * 0.5f);
    MatrixMultiply(worldDragon, tDragon, rDragon);

    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, worldDragon);
    glUniform3f(glGetUniformLocation(prog, "u_material.ambientColor"), 0.15f, 0.15f, 0.15f);
    glUniform3f(glGetUniformLocation(prog, "u_material.diffuseColor"), 0.1f, 0.8f, 0.1f);
    glUniform3f(glGetUniformLocation(prog, "u_material.specularColor"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(prog, "u_material.shininess"), 64.0f);

    glBindVertexArray(g_DragonVAO);
    glDrawElements(GL_TRIANGLES, 45000, GL_UNSIGNED_SHORT, 0);

    // --- cube ---

    float worldCube[16], tCube[16], rotX[16], rotY[16], rotZ[16];
    float tmp[16], tmp2[16];
    MatrixTranslation(tCube, -4.0f, 0.0f, 0.0f);
    MatrixRotationY(rotY, time);
    MatrixRotationX(rotX, time * 0.5f);
    MatrixRotationZ(rotZ, time * 0.25f);
    MatrixMultiply(tmp, rotX, rotY);
    MatrixMultiply(tmp2, rotZ, tmp);
    MatrixMultiply(worldCube, tCube, tmp2);

    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, worldCube);
    glUniform3f(glGetUniformLocation(prog, "u_material.ambientColor"), 0.15f, 0.15f, 0.15f);
    glUniform3f(glGetUniformLocation(prog, "u_material.diffuseColor"), 0.8f, 0.1f, 0.1f);
    glUniform3f(glGetUniformLocation(prog, "u_material.specularColor"), 1.0f, 0.8f, 0.8f);
    glUniform1f(glGetUniformLocation(prog, "u_material.shininess"), 128.0f);

    glBindVertexArray(g_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}


int main(void)
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "TP Final", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, OnMouseButton);
    glfwSetCursorPosCallback(window, OnCursorPos);
    glfwSetScrollCallback(window, OnScroll);
    glfwSetFramebufferSizeCallback(window, OnFramebufferSize);

    if (!Initialise())
        return -1;

    while (!glfwWindowShouldClose(window))
    {
        Render((float)glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &g_DragonVBO);
    glDeleteBuffers(1, &g_DragonEBO);
    glDeleteVertexArrays(1, &g_DragonVAO);
    glDeleteBuffers(1, &g_CubeVBO);
    glDeleteVertexArrays(1, &g_CubeVAO);
    glDeleteTextures(1, &g_Texture);

    glfwTerminate();
    return 0;
}
