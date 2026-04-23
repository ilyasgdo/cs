# Correction Gamma et Illumination Ambiante

## Correction Gamma

### Le problème

- Les moniteurs appliquent une décompression gamma (loi de puissance ~2.2)
- Les images (PNG, JPEG) sont stockées en sRGB (non-linéaire, gamma ~2.2)
- Les opérations mathématiques (éclairage) nécessitent des couleurs **linéaires**

### Règle : linéariser en entrée, compresser en sortie

**En entrée** (lecture) :
- Couleur provenant d'une image/texture → non-linéaire → linéariser avec `pow(color, 2.2)`
- Couleur provenant d'une équation → déjà linéaire → ne rien faire

**En sortie** (écriture) :
- Couleur linéaire → compresser avec `pow(color, 1.0/2.2)` avant d'écrire dans le framebuffer

### Méthode manuelle dans le shader

```glsl
// Linéariser en lecture
vec3 texColor = pow(texture(u_sampler, v_TexCoords).rgb, vec3(2.2));

// Compresser en sortie
FragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
```

### Méthode automatique OpenGL (recommandée)

**Pour les textures** — utiliser un format interne sRGB :
```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
```
Ou avec alpha :
```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
```
Note : la composante Alpha reste toujours linéaire.

**Pour le framebuffer** — activer la conversion automatique :
```cpp
glEnable(GL_FRAMEBUFFER_SRGB);
```

Avec ces deux options, le GPU gère la conversion automatiquement et il n'y a plus besoin de `pow()` dans le shader.

## Illumination Hémisphérique Ambiante

### Principe

Au lieu d'une couleur ambiante uniforme, on mélange deux couleurs :
- **SkyColor** (hémisphère "up", le ciel)
- **GroundColor** (hémisphère "down", le sol)

### Calcul

```glsl
vec3 skyDir = normalize(u_SkyDirection);
float NdotSky = dot(N, skyDir);
float hemiFactor = NdotSky * 0.5 + 0.5;
vec3 AmbientColor = u_Ia * mix(u_GroundColor, u_SkyColor, hemiFactor);
```

Explication :
- `NdotSky` est entre `[-1, +1]`
- On reparamètre en `[0, 1]` avec `x * 0.5 + 0.5`
- `mix(a, b, t)` interpole linéairement : `a * (1-t) + b * t`

### Résultat final

```glsl
vec3 FinalColor = AmbientColor + DiffuseColor + SpecularColor;
```

### Uniforms à passer depuis C++

```cpp
glUniform3f(glGetUniformLocation(prog, "u_SkyDirection"), 0, 1, 0);
glUniform3f(glGetUniformLocation(prog, "u_SkyColor"), 0.6f, 0.8f, 1.0f);
glUniform3f(glGetUniformLocation(prog, "u_GroundColor"), 0.2f, 0.2f, 0.1f);
```
