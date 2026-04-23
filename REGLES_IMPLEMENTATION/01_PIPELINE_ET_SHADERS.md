# Pipeline OpenGL et Shaders

## Pipeline de rendu

Le trajet des données dans le GPU suit cet ordre :

```
Attributs de Vertex → Vertex Shader → Rasterizer → Fragment Shader → Tests (Depth, Stencil) → Framebuffer
```

- **Vertex Shader** : programmable, transforme les sommets
- **Rasterizer** : configurable, interpole les varyings entre les sommets
- **Fragment Shader** : programmable, calcule la couleur de chaque pixel

## Création d'un Shader Program

Toujours suivre ces étapes dans cet ordre :

```
1. glCreateShader(GL_VERTEX_SHADER)      → créer le shader object
2. glShaderSource(shader, ...)           → fournir le code source
3. glCompileShader(shader)               → compiler
4. glGetShaderiv(shader, GL_COMPILE_STATUS, ...) → vérifier
5. Répéter pour GL_FRAGMENT_SHADER
6. glCreateProgram()                     → créer le program
7. glAttachShader(program, vs)           → attacher le vertex shader
8. glAttachShader(program, fs)           → attacher le fragment shader
9. glLinkProgram(program)                → linker
10. glValidateProgram(program)           → valider
11. glUseProgram(program)                → activer
```

En pratique on utilise la classe `GLShader` fournie :

```cpp
g_BasicShader.LoadVertexShader("basic.vs");
g_BasicShader.LoadFragmentShader("basic.fs");
g_BasicShader.Create();
```

## Version GLSL

Toujours spécifier en première ligne du shader :

```glsl
#version 330 core
```

Cela correspond à OpenGL 3.3.

## Communication CPU → GPU

### Attributs (par vertex)

Données qui varient par sommet (position, normale, UV...).

Côté GLSL :
```glsl
in vec3 a_position;
in vec3 a_normal;
in vec2 a_texcoords;
```

Côté C++ :
```cpp
GLint loc = glGetAttribLocation(program, "a_position");
glEnableVertexAttribArray(loc);
glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, stride, offset);
```

### Uniforms (globaux)

Données constantes pour un draw call (matrices, couleurs, paramètres).

Côté GLSL :
```glsl
uniform mat4 u_Model;
uniform vec3 u_light.direction;
```

Côté C++ :
```cpp
GLint loc = glGetUniformLocation(program, "u_Model");
glUniformMatrix4fv(loc, 1, GL_FALSE, matrice);
glUniform3f(loc, x, y, z);
```

### Varyings (inter-shader)

Données passées du Vertex Shader au Fragment Shader, interpolées par le rasterizer.

Vertex Shader :
```glsl
out vec3 v_Normal;
```

Fragment Shader :
```glsl
in vec3 v_Normal;
```

## Convention de nommage (du cours)

| Préfixe | Signification |
|---------|---------------|
| `a_` | attribut (in du vertex shader) |
| `v_` | varying (out du VS, in du FS) |
| `u_` | uniform |
| `g_` | variable globale C++ |

## Handles OpenGL

- Type `GLuint` : valeur `0` = invalide
- Type `GLint` : valeur `-1` = invalide
- Toujours vérifier avec `glGetError()` en cas de doute
