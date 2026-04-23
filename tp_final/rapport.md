# Rapport TP Final — Préparation au Projet + TP 05 Gamma & Ambient

---

## Exercice 2.1 — Fonction LookAt (View Matrix)

La View Matrix transforme les coordonnées du repère monde vers le repère de la caméra.
Elle est l'inverse de la World Matrix de la caméra.

Pour la construire on calcule trois vecteurs orthonormaux à partir de la position de l'œil, d'une cible et d'un vecteur up de référence, puis on les assemble dans une matrice avec les produits scalaires formant la translation inverse.

```cpp
void LookAt(float* m, float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz) {
    float fx = -(tx - ex), fy = -(ty - ey), fz = -(tz - ez);
    float fl = sqrt(fx*fx + fy*fy + fz*fz);
    fx /= fl; fy /= fl; fz /= fl;

    float rx = uy*fz - uz*fy, ry = uz*fx - ux*fz, rz = ux*fy - uy*fx;
    float rl = sqrt(rx*rx + ry*ry + rz*rz);
    rx /= rl; ry /= rl; rz /= rl;

    float uux = fy*rz - fz*ry, uuy = fz*rx - fx*rz, uuz = fx*ry - fy*rx;

    MatrixIdentity(m);
    m[0] = rx;  m[4] = ry;  m[8]  = rz;  m[12] = -(rx*ex + ry*ey + rz*ez);
    m[1] = uux; m[5] = uuy; m[9]  = uuz; m[13] = -(uux*ex + uuy*ey + uuz*ez);
    m[2] = fx;  m[6] = fy;  m[10] = fz;  m[14] = -(fx*ex + fy*ey + fz*ez);
    m[3] = 0;   m[7] = 0;   m[11] = 0;   m[15] = 1;
}
```

**Étapes :**
1. `forward` = `normalize(-(target - eye))` — pointe hors de l'écran (repère main droite)
2. `right` = `normalize(cross(up, forward))` — axe horizontal
3. `up corrigé` = `cross(forward, right)` — axe vertical recalculé
4. La 4ème colonne contient `-dot(axe, eye)` pour chaque axe — c'est la projection de la position dans le repère caméra, inversée

**Dans le Vertex Shader :** la View Matrix est appliquée après la Model Matrix mais avant la Projection, uniquement sur la position (pas sur les normales qui restent en espace monde).

```glsl
gl_Position = u_Projection * u_View * u_Model * vec4(a_position, 1.0);
```

Les normales sont transformées par `mat3(u_Model)` seulement car la View Matrix n'a pas de sens pour les normales (elles doivent rester en world space pour l'illumination).

---

## Caméra orbitale (arcball)

La position de la caméra est sur une sphère de rayon R centrée sur la cible. Les coordonnées sphériques (R, phi, theta) sont converties en cartésiennes :

```cpp
float camX = g_Radius * cos(g_Theta) * cos(g_Phi);
float camY = g_Radius * sin(g_Theta);
float camZ = g_Radius * cos(g_Theta) * sin(g_Phi);
```

**Contrôles :**
- Clic gauche + déplacement horizontal → modifie `phi` (azimut)
- Clic gauche + déplacement vertical → modifie `theta` (élévation)
- Molette → modifie `R` (distance)

**Limites :** phi ∈ (-π, +π), theta ∈ (-π/2, +π/2) pour éviter le retournement.

```cpp
void OnMouseButton(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_MousePressed = (action == GLFW_PRESS);
        glfwGetCursorPos(win, &g_LastMouseX, &g_LastMouseY);
    }
}

void OnCursorPos(GLFWwindow* win, double xpos, double ypos) {
    if (!g_MousePressed) return;
    float dx = (float)(xpos - g_LastMouseX);
    float dy = (float)(ypos - g_LastMouseY);
    g_Phi += dx * 0.005f;
    g_Theta += dy * 0.005f;
    if (g_Phi > 3.14159f) g_Phi -= 2.0f * 3.14159f;
    if (g_Phi < -3.14159f) g_Phi += 2.0f * 3.14159f;
    if (g_Theta > 3.14159f / 2.0f - 0.01f) g_Theta = 3.14159f / 2.0f - 0.01f;
    if (g_Theta < -3.14159f / 2.0f + 0.01f) g_Theta = -3.14159f / 2.0f + 0.01f;
    g_LastMouseX = xpos;
    g_LastMouseY = ypos;
}

void OnScroll(GLFWwindow* win, double xoff, double yoff) {
    g_Radius -= (float)yoff;
    if (g_Radius < 1.0f) g_Radius = 1.0f;
    if (g_Radius > 50.0f) g_Radius = 50.0f;
}
```

La caméra utilise ensuite `LookAt(view, camX, camY, camZ, 0, 0, 0, 0, 1, 0)` en ciblant l'origine.

La position caméra est aussi passée au shader pour le calcul du spéculaire :
```cpp
glUniform3f(glGetUniformLocation(prog, "u_CameraPos"), camX, camY, camZ);
```

---

## Exercice 3.1 — Correction Gamma manuelle (dans le shader)

Les textures et couleurs provenant d'images sont stockées en sRGB (non-linéaire, gamma ≈ 2.2). Les opérations d'éclairage nécessitent des couleurs linéaires.

**En entrée** : linéariser avec `pow(color, vec3(2.2))`
**En sortie** : compresser avec `pow(color, vec3(1.0/2.2))`

```glsl
vec3 texColor = pow(texture(u_sampler, v_TexCoords).rgb, vec3(2.2));
// ... calculs d'illumination ...
FragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
```

---

## Exercice 3.2 — Correction Gamma automatique (GL_SRGB8 + GL_FRAMEBUFFER_SRGB)

OpenGL peut gérer automatiquement les conversions gamma. C'est la méthode utilisée dans notre implémentation finale.

**Pour les textures** — utiliser `GL_SRGB8` comme format interne :
```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
```
Le GPU linéarise automatiquement les texels lors de l'échantillonnage.

**Pour le framebuffer** — activer la conversion automatique linéaire → sRGB en sortie :
```cpp
glEnable(GL_FRAMEBUFFER_SRGB);
```
Le GPU compresse gamma automatiquement lors de l'écriture dans le color buffer.

Avec ces deux options, aucun `pow()` n'est nécessaire dans le shader.

---

## Illumination hémisphérique ambiante

Au lieu d'une couleur ambiante uniforme, on mélange deux couleurs (ciel et sol) en fonction de l'orientation de la normale.

Le produit scalaire entre la normale N et la direction du ciel donne une valeur dans [-1, +1]. On reparamètre en [0, 1] avec `x * 0.5 + 0.5` puis on utilise `mix()` pour interpoler.

```glsl
float HemisphereFactor = dot(N, normalize(u_SkyDirection)) * 0.5 + 0.5;
vec3 ambient = u_material.ambientColor * mix(u_GroundColor, u_SkyColor, HemisphereFactor) * texColor;
```

**Uniforms passés depuis le C++ :**
```cpp
glUniform3f(glGetUniformLocation(prog, "u_SkyDirection"), 0, 1, 0);
glUniform3f(glGetUniformLocation(prog, "u_SkyColor"), 0.6f, 0.8f, 1.0f);
glUniform3f(glGetUniformLocation(prog, "u_GroundColor"), 0.2f, 0.2f, 0.1f);
```

---

## Modèle d'illumination complet — Blinn-Phong

Le résultat final combine les trois composantes :

```glsl
vec3 FinalColor = Ambient + Diffuse + Specular;
```

### Diffuse (Lambert)
```glsl
vec3 diffuse(vec3 N, vec3 L) {
    float NdotL = max(dot(N, L), 0.0);
    return NdotL * u_light.diffuseColor * u_material.diffuseColor;
}
```

### Spéculaire (Blinn-Phong)
Utilise le half-vector H = normalize(L + V) au lieu du vecteur réfléchi R.
```glsl
vec3 specular(vec3 N, vec3 L, vec3 V) {
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    return pow(NdotH, u_material.shininess) * u_light.specularColor * u_material.specularColor;
}
```

Le spéculaire n'est calculé que si la face est éclairée (`NdotL > 0`).

---

## Vertex Shader final

```glsl
#version 330 core

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texcoords;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    vec4 worldPos = u_Model * vec4(a_position, 1.0);
    v_Position = worldPos.xyz;
    v_Normal = mat3(u_Model) * a_normal;
    v_TexCoords = a_texcoords;
    gl_Position = u_Projection * u_View * worldPos;
}
```

## Fragment Shader final

```glsl
#version 330 core

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoords;

out vec4 FragColor;

struct Light {
    vec3 direction;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct Material {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};

uniform Light u_light;
uniform Material u_material;
uniform vec3 u_CameraPos;
uniform sampler2D u_diffuseMap;

uniform vec3 u_SkyDirection;
uniform vec3 u_SkyColor;
uniform vec3 u_GroundColor;

vec3 diffuse(vec3 N, vec3 L) {
    float NdotL = max(dot(N, L), 0.0);
    return NdotL * u_light.diffuseColor * u_material.diffuseColor;
}

vec3 specular(vec3 N, vec3 L, vec3 V) {
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    return pow(NdotH, u_material.shininess) * u_light.specularColor * u_material.specularColor;
}

void main() {
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(-u_light.direction);
    vec3 V = normalize(u_CameraPos - v_Position);

    vec3 texColor = texture(u_diffuseMap, v_TexCoords).rgb;

    float HemisphereFactor = dot(N, normalize(u_SkyDirection)) * 0.5 + 0.5;
    vec3 ambient = u_material.ambientColor * mix(u_GroundColor, u_SkyColor, HemisphereFactor) * texColor;

    vec3 diff = diffuse(N, L) * texColor;

    vec3 spec = vec3(0.0);
    if (dot(N, L) > 0.0) {
        spec = specular(N, L, V);
    }

    FragColor = vec4(ambient + diff + spec, 1.0);
}
```

---

## Compilation et exécution

```bash
cd tp_final
mkdir -p build && cd build
cmake ..
make
cd .. && ./build/tp_final
```
