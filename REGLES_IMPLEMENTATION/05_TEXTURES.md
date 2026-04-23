# Textures

## Création d'une texture

```cpp
GLuint texID;
glGenTextures(1, &texID);
glBindTexture(GL_TEXTURE_2D, texID);
```

## Paramètres de filtrage

```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
```

Filtres disponibles :
- `GL_NEAREST` : plus proche voisin (pixelisé)
- `GL_LINEAR` : bilinéaire (lissé)
- `GL_LINEAR_MIPMAP_LINEAR` : trilinéaire (nécessite mipmaps)

## Transfert de données

```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
```

Paramètres dans l'ordre :
1. Target : `GL_TEXTURE_2D`
2. Niveau mipmap : `0` (le plus haut)
3. Format interne : `GL_RGBA8`, `GL_SRGB8`, `GL_SRGB8_ALPHA8`
4. Largeur, hauteur
5. Bordure : toujours `0`
6. Format source : `GL_RGBA`, `GL_RGB`
7. Type des composants : `GL_UNSIGNED_BYTE`
8. Pointeur vers les données (ou `NULL` pour allocation seule)

## Mipmaps

```cpp
glGenerateMipmap(GL_TEXTURE_2D);
```

Appeler après `glTexImage2D`. Nécessaire si le filtre de minification utilise des mipmaps.

## Chargement avec stb_image

Dans un seul fichier .cpp :
```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
```

Chargement :
```cpp
int w, h;
uint8_t* data = stbi_load(filename, &w, &h, nullptr, STBI_rgb_alpha);
if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}
```

## Utilisation dans le rendu

### Côté C++ — activer et binder

```cpp
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, textureA);

glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, textureB);
```

Pour indiquer au shader quel canal utiliser :

```cpp
glUniform1i(glGetUniformLocation(program, "u_sampler"), 0);
```

### Côté GLSL — sampler

Vertex Shader :
```glsl
in vec2 a_texcoords;
out vec2 v_texcoords;
// dans main :
v_texcoords = a_texcoords;
```

Fragment Shader :
```glsl
in vec2 v_texcoords;
uniform sampler2D u_sampler;
// dans main :
vec4 color = texture(u_sampler, v_texcoords);
```

Note : en GLSL 330 on utilise `texture()` au lieu de `texture2D()`.

## Multi-texturing

Charger deux textures sur deux canaux différents et les combiner dans le fragment shader :

```glsl
uniform sampler2D u_tex0;
uniform sampler2D u_tex1;

vec4 c0 = texture(u_tex0, v_texcoords);
vec4 c1 = texture(u_tex1, v_texcoords);
vec4 final = c0 * c1;
```

## Destruction

```cpp
glDeleteTextures(1, &texID);
```
