#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB

#include <GLFW/glfw3.h>
#include "common/GLShader.h" 
#include <cstddef>
#include <cmath>
#include <iostream>
#include "dragoondata.h"

// --- Données du Cube ---
const GLfloat cube_vertices[] = {
    -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0,  1.0,  1.0,  -1.0,  1.0,  1.0, 
    -1.0, -1.0, -1.0,   1.0, -1.0, -1.0,   1.0,  1.0, -1.0,  -1.0,  1.0, -1.0  
};
const GLushort cube_elements[] = {
    0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7, 
    4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3  
};

GLfloat g_CubeData[36 * 8]; // X,Y,Z, NX,NY,NZ, U,V
GLShader g_BasicShader;
GLuint g_DragonVAO = 0, g_DragonVBO = 0, g_DragonEBO = 0;
GLuint g_CubeVAO = 0, g_CubeVBO = 0;
GLuint g_Texture = 0;
static int g_WindowWidth = 800, g_WindowHeight = 600;

// --- Fonctions Mathématiques ---
void MatrixIdentity(float* m) { for(int i=0; i<16; ++i) m[i] = (i%5 == 0) ? 1.0f : 0.0f; }
void MatrixMultiply(float* out, const float* a, const float* b) {
    float temp[16];
    for(int c=0; c<4; ++c) {
        for(int r=0; r<4; ++r) {
            temp[c*4+r] = a[0*4+r]*b[c*4+0] + a[1*4+r]*b[c*4+1] + a[2*4+r]*b[c*4+2] + a[3*4+r]*b[c*4+3];
        }
    }
    for(int i=0; i<16; ++i) out[i] = temp[i];
}
void MatrixTranslation(float* m, float tx, float ty, float tz) { MatrixIdentity(m); m[12] = tx; m[13] = ty; m[14] = tz; }
void MatrixRotationX(float* m, float angle) { MatrixIdentity(m); float c = cos(angle), s = sin(angle); m[5]=c; m[9]=-s; m[6]=s; m[10]=c; }
void MatrixRotationY(float* m, float angle) { MatrixIdentity(m); float c = cos(angle), s = sin(angle); m[0]=c; m[8]=s; m[2]=-s; m[10]=c; }
void MatrixRotationZ(float* m, float angle) { MatrixIdentity(m); float c = cos(angle), s = sin(angle); m[0]=c; m[4]=-s; m[1]=s; m[5]=c; }
void MatrixPerspective(float* m, float fov, float asp, float n, float f_z) {
    MatrixIdentity(m); float f = 1.0f / tan((fov * 3.14159f / 180.0f) / 2.0f);
    m[0]=f/asp; m[5]=f; m[10]=(f_z+n)/(n-f_z); m[11]=-1.0f; m[14]=(2.0f*f_z*n)/(n-f_z); m[15]=0.0f;
}

// --- Initialisation ---
bool Initialise() {
    if (!g_BasicShader.LoadVertexShader("basic.vs") || !g_BasicShader.LoadFragmentShader("basic.fs") || !g_BasicShader.Create()) return false;

    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); //
    glEnable(GL_FRAMEBUFFER_SRGB); // Correction Gamma automatique [cite: 2279]

    // Texture sRGB [cite: 2277]
    glGenTextures(1, &g_Texture); glBindTexture(GL_TEXTURE_2D, g_Texture);
    GLubyte texData[] = { 255,255,255, 0,0,0, 0,0,0, 255,255,255 };
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint prog = g_BasicShader.GetProgram();
    GLint lp = glGetAttribLocation(prog, "a_position"), ln = glGetAttribLocation(prog, "a_normal"), lt = glGetAttribLocation(prog, "a_texcoords");

    // VAO Dragon
    glGenVertexArrays(1, &g_DragonVAO); glBindVertexArray(g_DragonVAO);
    glGenBuffers(1, &g_DragonVBO); glBindBuffer(GL_ARRAY_BUFFER, g_DragonVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(DragonVertices), DragonVertices, GL_STATIC_DRAW);
    glGenBuffers(1, &g_DragonEBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_DragonEBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DragonIndices), DragonIndices, GL_STATIC_DRAW);
    if(lp>=0){ glEnableVertexAttribArray(lp); glVertexAttribPointer(lp, 3, GL_FLOAT, GL_FALSE, 8*4, 0); }
    if(ln>=0){ glEnableVertexAttribArray(ln); glVertexAttribPointer(ln, 3, GL_FLOAT, GL_FALSE, 8*4, (void*)(3*4)); }
    if(lt>=0){ glEnableVertexAttribArray(lt); glVertexAttribPointer(lt, 2, GL_FLOAT, GL_FALSE, 8*4, (void*)(6*4)); }

    // VAO Cube
    for(int i=0; i<12; ++i){ // Génération Normales + UV
        int i0=cube_elements[i*3]*3, i1=cube_elements[i*3+1]*3, i2=cube_elements[i*3+2]*3;
        float p0[3]={cube_vertices[i0],cube_vertices[i0+1],cube_vertices[i0+2]}, p1[3]={cube_vertices[i1],cube_vertices[i1+1],cube_vertices[i1+2]}, p2[3]={cube_vertices[i2],cube_vertices[i2+1],cube_vertices[i2+2]};
        float u[3]={p1[0]-p0[0],p1[1]-p0[1],p1[2]-p0[2]}, v[3]={p2[0]-p0[0],p2[1]-p0[1],p2[2]-p0[2]};
        float n[3]={u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]};
        float l=sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]); if(l>0){n[0]/=l;n[1]/=l;n[2]/=l;}
        for(int j=0; j<3; ++j){
            float* p = (j==0)?p0:(j==1)?p1:p2; int b=(i*3+j)*8;
            g_CubeData[b]=p[0]; g_CubeData[b+1]=p[1]; g_CubeData[b+2]=p[2];
            g_CubeData[b+3]=n[0]; g_CubeData[b+4]=n[1]; g_CubeData[b+5]=n[2];
            g_CubeData[b+6]=(j==1||(i%2==0&&j==2))?1.f:0.f; g_CubeData[b+7]=(j==2)?1.f:0.f;
        }
    }
    glGenVertexArrays(1, &g_CubeVAO); glBindVertexArray(g_CubeVAO);
    glGenBuffers(1, &g_CubeVBO); glBindBuffer(GL_ARRAY_BUFFER, g_CubeVBO); glBufferData(GL_ARRAY_BUFFER, sizeof(g_CubeData), g_CubeData, GL_STATIC_DRAW);
    if(lp>=0){ glEnableVertexAttribArray(lp); glVertexAttribPointer(lp, 3, GL_FLOAT, GL_FALSE, 8*4, 0); }
    if(ln>=0){ glEnableVertexAttribArray(ln); glVertexAttribPointer(ln, 3, GL_FLOAT, GL_FALSE, 8*4, (void*)(3*4)); }
    if(lt>=0){ glEnableVertexAttribArray(lt); glVertexAttribPointer(lt, 2, GL_FLOAT, GL_FALSE, 8*4, (void*)(6*4)); }
    glBindVertexArray(0);
    return true;
}

void Render(float time) {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLuint prog = g_BasicShader.GetProgram(); glUseProgram(prog);
    
    float view[16], proj[16]; MatrixTranslation(view, 0, 0, -15);
    MatrixPerspective(proj, 45, (float)g_WindowWidth/g_WindowHeight, 0.1f, 100);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Projection"), 1, GL_FALSE, proj);
    
    // Lumière et Ambiance 
    glUniform3f(glGetUniformLocation(prog, "u_CameraPos"), 0, 0, 15);
    glUniform3f(glGetUniformLocation(prog, "u_light.direction"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(prog, "u_light.diffuseColor"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(prog, "u_light.specularColor"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(prog, "u_SkyDirection"), 0, 1, 0);
    glUniform3f(glGetUniformLocation(prog, "u_SkyColor"), 0.6f, 0.8f, 1.0f);
    glUniform3f(glGetUniformLocation(prog, "u_GroundColor"), 0.2f, 0.2f, 0.1f);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, g_Texture);
    glUniform1i(glGetUniformLocation(prog, "u_diffuseMap"), 0);

    // Rendu Dragon
    float mD[16], tD[16], rD[16]; MatrixTranslation(tD, 4, -4, 0); MatrixRotationY(rD, time*0.5f); MatrixMultiply(mD, tD, rD);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, mD);
    glUniform3f(glGetUniformLocation(prog, "u_material.ambientColor"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(prog, "u_material.diffuseColor"), 0.1f, 0.8f, 0.1f);
    glUniform3f(glGetUniformLocation(prog, "u_material.specularColor"), 1, 1, 1);
    glUniform1f(glGetUniformLocation(prog, "u_material.shininess"), 64);
    glBindVertexArray(g_DragonVAO); glDrawElements(GL_TRIANGLES, 45000, GL_UNSIGNED_SHORT, 0);

    // Rendu Cube
    float mC[16], tC[16], rX[16], rY[16], rZ[16], tmp[16], rt[16]; MatrixTranslation(tC, -4, 0, 0);
    MatrixRotationY(rY, time); MatrixRotationX(rX, time*0.5f); MatrixRotationZ(rZ, time*0.25f);
    MatrixMultiply(tmp, rX, rY); MatrixMultiply(rt, rZ, tmp); MatrixMultiply(mC, tC, rt);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, mC);
    glUniform3f(glGetUniformLocation(prog, "u_material.ambientColor"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(prog, "u_material.diffuseColor"), 0.8f, 0.1f, 0.1f);
    glUniform3f(glGetUniformLocation(prog, "u_material.specularColor"), 1, 0.8f, 0.8f);
    glUniform1f(glGetUniformLocation(prog, "u_material.shininess"), 128);
    glBindVertexArray(g_CubeVAO); glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(800, 600, "TP Final", NULL, NULL);
    if (!win) return -1;
    glfwMakeContextCurrent(win);
    if (!Initialise()) return -1;
    while (!glfwWindowShouldClose(win)) { Render(glfwGetTime()); glfwSwapBuffers(win); glfwPollEvents(); }
    glfwTerminate(); return 0;
}