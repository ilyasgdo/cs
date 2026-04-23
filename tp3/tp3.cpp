#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB // <-- INDISPENSABLE SUR MAC POUR LES VAO

#include <GLFW/glfw3.h>
#include "common/GLShader.h" 
#include <cstddef>
#include <cmath>
#include <iostream>
#include "dragoondata.h"

// --- Données brutes du Cube (Exercice 1) ---
const GLfloat cube_vertices[] = {
    -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0,  1.0,  1.0,  -1.0,  1.0,  1.0, // front
    -1.0, -1.0, -1.0,   1.0, -1.0, -1.0,   1.0,  1.0, -1.0,  -1.0,  1.0, -1.0  // back
};
const GLushort cube_elements[] = {
    0,1,2, 2,3,0, // front
    1,5,6, 6,2,1, // right
    7,6,5, 5,4,7, // back
    4,0,3, 3,7,4, // left
    4,5,1, 1,0,4, // bottom
    3,2,6, 6,7,3  // top
};

// Tableau final pour le cube généré avec normales (36 sommets de 6 floats : X,Y,Z, NX,NY,NZ)
GLfloat g_CubeData[36 * 6]; 

GLShader g_BasicShader;

// Identifiants pour le Dragon
GLuint g_DragonVAO = 0; 
GLuint g_DragonVBO = 0;
GLuint g_DragonEBO = 0;

// Identifiants pour le Cube
GLuint g_CubeVAO = 0;
GLuint g_CubeVBO = 0;

static int g_WindowWidth = 800;
static int g_WindowHeight = 600;

void OnWindowSize(GLFWwindow*, int w, int h) {
    g_WindowWidth = w;
    g_WindowHeight = h;
}

void OnFramebufferSize(GLFWwindow*, int fbw, int fbh) {
    glViewport(0, 0, fbw, fbh);
}

// ==========================================================================
// --- Fonctions Mathématiques Basiques (Matrices 4x4) ---
// ==========================================================================
void MatrixIdentity(float* m) {
    for(int i=0; i<16; ++i) m[i] = (i%5 == 0) ? 1.0f : 0.0f;
}

// Multiplication de matrices 4x4 (out = a * b)
void MatrixMultiply(float* out, const float* a, const float* b) {
    float temp[16];
    for(int c=0; c<4; ++c) {
        for(int r=0; r<4; ++r) {
            temp[c*4+r] = a[0*4+r]*b[c*4+0] + a[1*4+r]*b[c*4+1] + a[2*4+r]*b[c*4+2] + a[3*4+r]*b[c*4+3];
        }
    }
    for(int i=0; i<16; ++i) out[i] = temp[i];
}

void MatrixTranslation(float* m, float tx, float ty, float tz) {
    MatrixIdentity(m);
    m[12] = tx; m[13] = ty; m[14] = tz;
}

void MatrixRotationX(float* m, float angle) {
    MatrixIdentity(m);
    float c = cos(angle); float s = sin(angle);
    m[5] = c; m[9] = -s; m[6] = s; m[10] = c;
}

void MatrixRotationY(float* m, float angle) {
    MatrixIdentity(m);
    float c = cos(angle); float s = sin(angle);
    m[0] = c; m[8] = s; m[2] = -s; m[10] = c;
}

void MatrixRotationZ(float* m, float angle) {
    MatrixIdentity(m);
    float c = cos(angle); float s = sin(angle);
    m[0] = c; m[4] = -s; m[1] = s; m[5] = c;
}

void MatrixPerspective(float* m, float fov_deg, float aspect, float near_z, float far_z) {
    MatrixIdentity(m);
    float f = 1.0f / tan((fov_deg * 3.1415926535f / 180.0f) / 2.0f);
    m[0] = f / aspect; m[5] = f;
    m[10] = (far_z + near_z) / (near_z - far_z);
    m[11] = -1.0f;
    m[14] = (2.0f * far_z * near_z) / (near_z - far_z);
    m[15] = 0.0f;
}

// ==========================================================================
// --- Exercice 2.1 : Calcul des normales C++ ---
// ==========================================================================
void GenerateCubeWithNormals() {
    for(int i=0; i<12; ++i) { // Pour chaque triangle (12 triangles)
        int idx0 = cube_elements[i*3+0] * 3;
        int idx1 = cube_elements[i*3+1] * 3;
        int idx2 = cube_elements[i*3+2] * 3;

        // Récupération des 3 points du triangle
        float p0[3] = {cube_vertices[idx0], cube_vertices[idx0+1], cube_vertices[idx0+2]};
        float p1[3] = {cube_vertices[idx1], cube_vertices[idx1+1], cube_vertices[idx1+2]};
        float p2[3] = {cube_vertices[idx2], cube_vertices[idx2+1], cube_vertices[idx2+2]};

        // Calcul des vecteurs U et V
        float u[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
        float v[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};

        // Produit vectoriel pour la normale (N = U x V)
        float n[3] = {
            u[1]*v[2] - u[2]*v[1],
            u[2]*v[0] - u[0]*v[2],
            u[0]*v[1] - u[1]*v[0]
        };
        // Normalisation
        float len = sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
        if(len > 0.0f) { n[0]/=len; n[1]/=len; n[2]/=len; }

        // Ajout des 3 sommets (Positions + Normales) dans le tableau final
        for(int vtx=0; vtx<3; ++vtx) {
            float* p = (vtx==0) ? p0 : (vtx==1) ? p1 : p2;
            int base = (i*3 + vtx) * 6;
            g_CubeData[base+0] = p[0]; g_CubeData[base+1] = p[1]; g_CubeData[base+2] = p[2];
            g_CubeData[base+3] = n[0]; g_CubeData[base+4] = n[1]; g_CubeData[base+5] = n[2];
        }
    }
}

// ==========================================================================

bool Initialise() {
    if (!g_BasicShader.LoadVertexShader("basic.vs")) return false;
    if (!g_BasicShader.LoadFragmentShader("basic.fs")) return false;
    if (!g_BasicShader.Create()) return false;

    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK);     
    glFrontFace(GL_CCW);     

    GLuint basicProgram = g_BasicShader.GetProgram();
    GLint loc_position = glGetAttribLocation(basicProgram, "a_position");
    GLint loc_normal   = glGetAttribLocation(basicProgram, "a_normal");

    // --- SETUP DU DRAGON ---
    glGenVertexArrays(1, &g_DragonVAO);
    glBindVertexArray(g_DragonVAO);
    glGenBuffers(1, &g_DragonVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_DragonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DragonVertices), DragonVertices, GL_STATIC_DRAW);
    glGenBuffers(1, &g_DragonEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_DragonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(DragonIndices), DragonIndices, GL_STATIC_DRAW);

    GLsizei strideD = 8 * sizeof(float);
    if (loc_position >= 0) {
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, strideD, (const void*)0);
    }
    if (loc_normal >= 0) {
        glEnableVertexAttribArray(loc_normal);
        glVertexAttribPointer(loc_normal, 3, GL_FLOAT, GL_FALSE, strideD, (const void*)(3 * sizeof(float)));
    }
    glBindVertexArray(0);

    // --- SETUP DU CUBE ---
    GenerateCubeWithNormals(); // On calcule les normales en C++

    glGenVertexArrays(1, &g_CubeVAO);
    glBindVertexArray(g_CubeVAO);
    glGenBuffers(1, &g_CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_CubeData), g_CubeData, GL_STATIC_DRAW);

    GLsizei strideC = 6 * sizeof(float); // X,Y,Z, NX,NY,NZ (pas d'UVs pour le cube)
    if (loc_position >= 0) {
        glEnableVertexAttribArray(loc_position);
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, GL_FALSE, strideC, (const void*)0);
    }
    if (loc_normal >= 0) {
        glEnableVertexAttribArray(loc_normal);
        glVertexAttribPointer(loc_normal, 3, GL_FLOAT, GL_FALSE, strideC, (const void*)(3 * sizeof(float)));
    }
    glBindVertexArray(0);

    return true;
}

void Terminate() {
    g_BasicShader.Destroy();
    glDeleteBuffers(1, &g_DragonVBO); glDeleteBuffers(1, &g_DragonEBO); glDeleteVertexArrays(1, &g_DragonVAO);
    glDeleteBuffers(1, &g_CubeVBO); glDeleteVertexArrays(1, &g_CubeVAO);
}

void Render(float time) {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // Fond un peu plus sombre pour bien voir la lumière
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint basicProgram = g_BasicShader.GetProgram();
    glUseProgram(basicProgram);

    // --- Calcul des Matrices Communes ---
    float view[16], projection[16];
    
    // Caméra reculée de 15 unités (donc positionnée à Z=15 dans le monde)
    MatrixTranslation(view, 0.0f, 0.0f, -15.0f); 
    
    float aspect = (float)g_WindowWidth / (float)(g_WindowHeight > 0 ? g_WindowHeight : 1);
    MatrixPerspective(projection, 45.0f, aspect, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(basicProgram, "u_View"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(basicProgram, "u_Projection"), 1, GL_FALSE, projection);

    // --- Configuration de la Lumière et de la Caméra ---
    // La caméra est à (0, 0, 15) dans l'espace Monde
    glUniform3f(glGetUniformLocation(basicProgram, "u_CameraPos"), 0.0f, 0.0f, 15.0f);

    // Lumière blanche venant d'en haut à droite et en avant [cite: 3773, 3792]
    glUniform3f(glGetUniformLocation(basicProgram, "u_light.direction"), 1.0f, 1.0f, 1.0f); 
    glUniform3f(glGetUniformLocation(basicProgram, "u_light.ambientColor"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(basicProgram, "u_light.diffuseColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(basicProgram, "u_light.specularColor"), 1.0f, 1.0f, 1.0f);

    GLint loc_Model = glGetUniformLocation(basicProgram, "u_Model");

    // ==========================================
    // 1. AFFICHAGE DU DRAGON (A droite, matériau type "Plastique Vert")
    // ==========================================
    float modelD[16], transD[16], rotD[16];
    MatrixTranslation(transD, 4.0f, -4.0f, 0.0f); 
    MatrixRotationY(rotD, time * 0.5f);
    MatrixMultiply(modelD, transD, rotD);

    glUniformMatrix4fv(loc_Model, 1, GL_FALSE, modelD);

    // Envoi des propriétés du matériau [cite: 3896]
    glUniform3f(glGetUniformLocation(basicProgram, "u_material.ambientColor"), 0.05f, 0.2f, 0.05f);
    glUniform3f(glGetUniformLocation(basicProgram, "u_material.diffuseColor"), 0.1f, 0.8f, 0.1f);
    glUniform3f(glGetUniformLocation(basicProgram, "u_material.specularColor"), 1.0f, 1.0f, 1.0f); // Reflet blanc
    glUniform1f(glGetUniformLocation(basicProgram, "u_material.shininess"), 64.0f); // Brillance moyenne/haute

    glBindVertexArray(g_DragonVAO);
    glDrawElements(GL_TRIANGLES, 45000, GL_UNSIGNED_SHORT, (const void*)0);
    glBindVertexArray(0);

    // ==========================================
    // 2. AFFICHAGE DU CUBE (A gauche, matériau type "Métal Rouge")
    // ==========================================
    float modelC[16], transC[16], rX[16], rY[16], rZ[16], temp[16], rotTotal[16];
    MatrixTranslation(transC, -4.0f, 0.0f, 0.0f); 
    
    MatrixRotationY(rY, time);         
    MatrixRotationX(rX, time * 0.5f);  
    MatrixRotationZ(rZ, time * 0.25f); 
    MatrixMultiply(temp, rX, rY);
    MatrixMultiply(rotTotal, rZ, temp);
    MatrixMultiply(modelC, transC, rotTotal);

    glUniformMatrix4fv(loc_Model, 1, GL_FALSE, modelC);

    // Envoi des propriétés du matériau [cite: 3896]
    glUniform3f(glGetUniformLocation(basicProgram, "u_material.ambientColor"), 0.2f, 0.05f, 0.05f);
    glUniform3f(glGetUniformLocation(basicProgram, "u_material.diffuseColor"), 0.8f, 0.1f, 0.1f);
    glUniform3f(glGetUniformLocation(basicProgram, "u_material.specularColor"), 1.0f, 0.8f, 0.8f); // Reflet légèrement rouge (métallique)
    glUniform1f(glGetUniformLocation(basicProgram, "u_material.shininess"), 128.0f); // Très brillant

    glBindVertexArray(g_CubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36); 
    glBindVertexArray(0);
}

int main(void) {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(g_WindowWidth, g_WindowHeight, "TP OpenGL - Dragon & Cube", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 
    glfwSetWindowSizeCallback(window, OnWindowSize);
    glfwSetFramebufferSizeCallback(window, OnFramebufferSize);

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    OnFramebufferSize(window, fbw, fbh);

    if (!Initialise()) {
        std::cerr << "Erreur d'initialisation" << std::endl;
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        float time = glfwGetTime();
        Render(time); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Terminate();
    glfwTerminate();
    return 0;
}