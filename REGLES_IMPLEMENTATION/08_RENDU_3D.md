# Rendu 3D

## Depth Test (Z-Buffer)

En 3D il faut activer le test de profondeur pour un rendu correct :

```cpp
glEnable(GL_DEPTH_TEST);
```

Et **très important**, effacer le depth buffer à chaque frame :

```cpp
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

Sans cela, après la première frame, rien ne s'affiche car tous les fragments échouent au test.

Par défaut le test est `GL_LESS` (strictement inférieur). On peut changer :

```cpp
glDepthFunc(GL_LEQUAL);
```

Pour désactiver (par exemple en 2D) :

```cpp
glDisable(GL_DEPTH_TEST);
```

## Face Culling

En 3D, les faces arrière sont invisibles. On les supprime pour économiser :

```cpp
glEnable(GL_CULL_FACE);
```

Par défaut, les faces `GL_BACK` sont supprimées (ordre counterclockwise = front).

## Structure d'un programme de rendu 3D

### Initialisation

```cpp
bool Initialise() {
    // 1. Charger et créer les shaders
    // 2. Activer depth test et culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // 3. Créer les textures
    // 4. Créer les VAO/VBO/IBO pour chaque objet
    return true;
}
```

### Rendu (chaque frame)

```cpp
void Render(float time) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);

    // Matrices vue et projection (une seule fois par frame)
    // Uniforms globaux (lumière, caméra)

    // Pour chaque objet :
    //   1. Calculer sa World Matrix
    //   2. Passer la World Matrix en uniform
    //   3. Passer les uniforms du matériau
    //   4. Binder le VAO
    //   5. glDrawElements ou glDrawArrays
}
```

### Boucle principale

```cpp
while (!glfwWindowShouldClose(window)) {
    Render(glfwGetTime());
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

## Configuration GLFW pour macOS

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
```

## Données du Dragon

Le dragon utilise `dragondata.h` :

- `DragonVertices[133140]` : tableau de floats, 8 floats par sommet `{X,Y,Z, NX,NY,NZ, U,V}`
- `DragonIndices[45000]` : tableau d'indices en `unsigned short`

Les normales sont déjà "smooth" (pas besoin de recalculer).
Les UV sont fournies pour le texturing.

Rendu :
```cpp
glBindVertexArray(g_DragonVAO);
glDrawElements(GL_TRIANGLES, 45000, GL_UNSIGNED_SHORT, 0);
```
