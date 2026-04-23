# TD OpenGL Moderne - Version macOS (adaptée à votre projet)

Document adapté à votre dépôt actuel:
- `tp2.cpp`
- `common/GLShader.h` et `common/GLShader.cpp`
- `basic.vs` et `basic.fs`
- `CMakeLists.txt` + `launch.sh`

Objectif: répondre à tout le TD A.I -> B.VI avec une version directement exploitable sur macOS.

## 1. Contexte macOS

Sur macOS:
- GLEW n'est pas obligatoire pour ce TD (les symboles nécessaires sont déjà exposés).
- Le code Windows `wglSwapIntervalEXT(...)` ne s'applique pas.
- OpenGL est marqué "deprecated" par Apple, mais fonctionne encore pour ce type d'exercices.

Dans votre code, la partie Windows est déjà correctement protégée:

```cpp
#ifdef WIN32
wglSwapIntervalEXT(1);
#endif
```

## 2. Architecture minimale demandée par le TD

Le TD recommande trois fonctions principales:

```cpp
bool Initialise();
void Render(int width, int height);
void Terminate();
```

Et une callback de redimensionnement:
- `glfwSetWindowSizeCallback(...)`
- option Retina: `glfwSetFramebufferSizeCallback(...)`

## A. Fonctionnalités de base

## I. Création et compilation des shaders

### Version correcte pour votre cas

```cpp
GLShader g_BasicShader;

bool Initialise()
{
    if (!g_BasicShader.LoadVertexShader("basic.vs")) return false;
    if (!g_BasicShader.LoadFragmentShader("basic.fs")) return false;
    if (!g_BasicShader.Create()) return false;

#ifdef WIN32
    wglSwapIntervalEXT(1);
#endif
    return true;
}

void Terminate()
{
    g_BasicShader.Destroy();
}
```

### Compatibilité des versions GLSL

- OpenGL ES 2.0 -> `#version 100`
- OpenGL 2.0 -> `#version 110`
- OpenGL 2.1 -> `#version 120`

Pour ce TD, vous restez sur GLSL 120.

### Shaders (déjà présents dans votre dossier)

`basic.vs`
```glsl
#version 120

attribute vec2 a_position;
attribute vec3 a_color;
varying vec4 v_color;

void main(void)
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_color = vec4(a_color, 1.0);
}
```

`basic.fs`
```glsl
#version 120

varying vec4 v_color;

void main(void)
{
    gl_FragColor = v_color;
}
```

## II. Rendu à base de triangles

Une passe de rendu type:
1. `glViewport`
2. `glClearColor` + `glClear`
3. `glUseProgram`
4. préparer les attributs (ou VAO)
5. états de rendu optionnels
6. `glDrawArrays` ou `glDrawElements`

Exemple:

```cpp
void Render(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint basicProgram = g_BasicShader.GetProgram();
    glUseProgram(basicProgram);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}
```

## III. Spécifications des attributs

### Position seule

```cpp
static const float triangle[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.0f,  0.5f
};

GLuint program = g_BasicShader.GetProgram();
int loc_position = glGetAttribLocation(program, "a_position");

glEnableVertexAttribArray(loc_position);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, triangle);
```

### Pourquoi `glGetAttribLocation`?

Ne pas supposer que `a_position` est toujours sur la location 0.

## Exercice A.1 - Ajouter la couleur

```cpp
static const float triangle[] = {
    // x,    y,     r,   g,   b
    -0.5f, -0.5f,  1.f, 0.f, 0.f,
     0.5f, -0.5f,  0.f, 1.f, 0.f,
     0.0f,  0.5f,  0.f, 0.f, 1.f
};

int loc_position = glGetAttribLocation(program, "a_position");
int loc_color    = glGetAttribLocation(program, "a_color");

GLsizei stride = sizeof(float) * 5;

glEnableVertexAttribArray(loc_position);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, stride, triangle);

glEnableVertexAttribArray(loc_color);
glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, stride, triangle + 2);
```

Le triangle s'affiche en dégradé car la couleur est interpolée entre les sommets.

## Exercice A.2 - Utiliser une structure `Vertex`

```cpp
#include <cstddef>

struct Vertex {
    float px, py;
    float r, g, b;
};

static const Vertex triangle[] = {
    {-0.5f, -0.5f, 1.f, 0.f, 0.f},
    { 0.5f, -0.5f, 0.f, 1.f, 0.f},
    { 0.0f,  0.5f, 0.f, 0.f, 1.f}
};

glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (const void*)offsetof(Vertex, px));

glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (const void*)offsetof(Vertex, r));
```

## B. Fonctionnalités avancées

## IV. Vertex Buffer Objects (VBO)

But: ne plus renvoyer les sommets CPU -> GPU à chaque frame.

### Initialisation

```cpp
GLuint g_VBO = 0;

glGenBuffers(1, &g_VBO);
glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER, 0);
```

### Rendu

```cpp
glBindBuffer(GL_ARRAY_BUFFER, g_VBO);

glEnableVertexAttribArray(loc_position);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (const void*)offsetof(Vertex, px));

glEnableVertexAttribArray(loc_color);
glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (const void*)offsetof(Vertex, r));

glDrawArrays(GL_TRIANGLES, 0, 3);
```

### Terminaison

```cpp
glDeleteBuffers(1, &g_VBO);
```

## Exercice B.1 - VBO avec offsets relatifs

Avec VBO lié, le dernier argument de `glVertexAttribPointer` est un offset relatif dans le buffer.

## V. Index Buffer Object (IBO/EBO)

### Exercice B.2 - Dessin indexé

```cpp
static const unsigned short indices[] = {0, 1, 2};
GLuint g_EBO = 0;

glGenBuffers(1, &g_EBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

// Draw
// Le dernier paramètre vaut 0 car l'EBO est bind.
glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
```

Terminaison:

```cpp
glDeleteBuffers(1, &g_EBO);
```

## VI. Vertex Array Objects (VAO)

### Exercice B.3 - Retirer la config d'attributs de la boucle de rendu

### Initialisation VAO

```cpp
GLuint g_VAO = 0;

glGenVertexArrays(1, &g_VAO);
glBindVertexArray(g_VAO);

glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);

glEnableVertexAttribArray(loc_position);
glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (const void*)offsetof(Vertex, px));

glEnableVertexAttribArray(loc_color);
glVertexAttribPointer(loc_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (const void*)offsetof(Vertex, r));

// sortir du VAO avant de reset les bindings globaux
glBindVertexArray(0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
```

### Boucle Render simplifiée

```cpp
glUseProgram(g_BasicShader.GetProgram());
glBindVertexArray(g_VAO);
glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
glBindVertexArray(0);
```

### Terminaison

```cpp
glDeleteVertexArrays(1, &g_VAO);
```

## Callback resize (important pour votre cas macOS)

```cpp
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
```

Dans `main`:

```cpp
glfwSetWindowSizeCallback(window, OnWindowSize);
glfwSetFramebufferSizeCallback(window, OnFramebufferSize);
```

Le callback framebuffer est le plus fiable pour Retina/high-DPI.

## Build et exécution sur votre machine

Votre script actuel est correct:

```sh
./launch.sh
```

Contenu:
```sh
cmake -B build
cmake --build build
./build/tp2
```

## Remarques spécifiques à votre projet actuel

1. Vous avez bien `GLShader` disponible via `common/GLShader.h`.
2. `CMakeLists.txt` compile déjà `common/GLShader.cpp`.
3. Les fichiers `basic.vs` et `basic.fs` existent déjà dans `tp2/`.
4. Sous macOS, le fallback dans `GLShader.cpp` vers `<OpenGL/gl3.h>` évite la dépendance GLEW obligatoire.

## Résumé

Ce document couvre toutes les sections demandées:
- A.I Création/compilation shaders
- A.II Rendu triangle
- A.III Attributs
- B.IV VBO
- B.V IBO/EBO
- B.VI VAO
- Exos A.1, A.2, B.1, B.2, B.3

Et il est entièrement adapté à votre environnement macOS actuel.
