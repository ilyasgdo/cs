# Rapport TP — Préparation à la soutenance

---

## Partie 1 — Matrices monde

### Exercice 1.1 — Multiplication de deux matrices

L'exercice demande d'implémenter une fonction multipliant deux matrices homogènes 4×4 stockées en colonnes (column-major). Chaque élément du résultat est la somme des produits ligne×colonne. On utilise un tableau temporaire pour permettre le cas `out == a` ou `out == b`.

```cpp
void MatrixMultiply(float* out, const float* a, const float* b) {
    float temp[16];
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            temp[c*4+r] = a[0*4+r]*b[c*4+0] + a[1*4+r]*b[c*4+1]
                        + a[2*4+r]*b[c*4+2] + a[3*4+r]*b[c*4+3];
    for (int i = 0; i < 16; ++i) out[i] = temp[i];
}
```

Le stockage column-major signifie que l'indice `[colonne*4 + ligne]` donne l'élément à la ligne `ligne` et colonne `colonne`. La première colonne occupe les indices 0-3, la deuxième 4-7, etc.

---

### Exercice 1.2 — World Matrix unique

L'exercice demande de remplacer les matrices de transformation séparées par une unique World Matrix. L'ordre de concaténation est (de droite à gauche) :

**WorldMatrix = Translation × Rotation × Scale**

Dans notre cas, pour le dragon qui tourne autour de Y et est translaté :

```cpp
float mD[16], tD[16], rD[16];
MatrixTranslation(tD, 4, -4, 0);
MatrixRotationY(rD, time * 0.5f);
MatrixMultiply(mD, tD, rD);
```

`mD` est la World Matrix finale envoyée au shader en un seul appel :

```cpp
glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, mD);
```

Pour le cube avec trois rotations combinées :

```cpp
float mC[16], tC[16], rX[16], rY[16], rZ[16], tmp[16], rt[16];
MatrixTranslation(tC, -4, 0, 0);
MatrixRotationY(rY, time);
MatrixRotationX(rX, time * 0.5f);
MatrixRotationZ(rZ, time * 0.25f);
MatrixMultiply(tmp, rX, rY);
MatrixMultiply(rt, rZ, tmp);
MatrixMultiply(mC, tC, rt);
```

Cela réduit à une seule matrice uniform envoyée au GPU au lieu de plusieurs matrices séparées.

Les fonctions de base utilisées :

```cpp
void MatrixIdentity(float* m) {
    for (int i = 0; i < 16; ++i) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

void MatrixTranslation(float* m, float tx, float ty, float tz) {
    MatrixIdentity(m); m[12] = tx; m[13] = ty; m[14] = tz;
}

void MatrixRotationY(float* m, float angle) {
    MatrixIdentity(m);
    float c = cos(angle), s = sin(angle);
    m[0] = c; m[8] = s; m[2] = -s; m[10] = c;
}

void MatrixRotationX(float* m, float angle) {
    MatrixIdentity(m);
    float c = cos(angle), s = sin(angle);
    m[5] = c; m[9] = -s; m[6] = s; m[10] = c;
}

void MatrixRotationZ(float* m, float angle) {
    MatrixIdentity(m);
    float c = cos(angle), s = sin(angle);
    m[0] = c; m[4] = -s; m[1] = s; m[5] = c;
}
```

---

### Normal Matrix (Eq 1.2)

La normale doit être transformée par la transposée de l'inverse de la World Matrix :

**NormalMatrix = (WorldMatrix⁻¹)ᵀ**

La translation n'a pas de sens pour une normale (composante w = 0 en homogène). Le plus simple est de convertir la WorldMatrix en `mat3`, ce qui élimine la translation. Tant que la World Matrix ne contient pas de scale non-uniforme, `mat3(WorldMatrix)` est suffisant car pour une matrice orthogonale `M⁻¹ = Mᵀ` donc `(M⁻¹)ᵀ = M`.

Dans le Vertex Shader :

```glsl
v_Normal = mat3(u_Model) * a_normal;
```

---

## Partie 2 — Matrice vue

### Exercice 2.1 — Fonction LookAt

La View Matrix transforme du repère monde vers le repère de la caméra. C'est l'inverse de la World Matrix de la caméra. Comme la caméra n'a pas de scale, c'est une transformation orthogonale et on peut calculer l'inverse simplement :

**ViewMatrix = RotationMatrixᵀ_cam × TranslationMatrix⁻¹_cam**

La fonction LookAt prend trois paramètres : position de l'œil, position de la cible, vecteur up de référence.

Les 5 étapes de construction :

**Étape 1** — Vecteur `forward` : c'est `normalize(-(target - eye))`. Le signe moins vient du repère main droite où forward pointe hors de l'écran.

**Étape 2** — Vecteur `right` : produit vectoriel `cross(up, forward)` normalisé. Règle de la main droite : up sur l'index, forward sur le majeur, right est indiqué par le pouce.

**Étape 3** — Vecteur `up corrigé` : produit vectoriel `cross(forward, right)`. Forward sur le pouce, right sur l'index, up corrigé est indiqué par le majeur.

**Étape 4** — Trois produits scalaires : projection de `-position` dans le repère local de la caméra. Chaque produit scalaire entre la position et un des trois vecteurs (right, up, forward) donne la composante de translation inverse.

**Étape 5** — Assemblage : les 3 premières colonnes contiennent la transposée des 3 vecteurs (première colonne = toutes les composantes x, etc.), la 4ème colonne contient les produits scalaires.

```cpp
void LookAt(float* m, float ex, float ey, float ez,
            float tx, float ty, float tz,
            float ux, float uy, float uz) {
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

La View Matrix est passée au Vertex Shader et appliquée à la position uniquement (après la World Matrix, avant la Projection). Les normales restent en espace monde pour les calculs d'illumination.

```glsl
void main() {
    vec4 worldPos = u_Model * vec4(a_position, 1.0);
    v_Position = worldPos.xyz;
    v_Normal = mat3(u_Model) * a_normal;
    v_TexCoords = a_texcoords;
    gl_Position = u_Projection * u_View * worldPos;
}
```

La chaîne complète de transformation est donc : `gl_Position = Projection × View × Model × vertex`

---

## TP à rendre — Caméra orbitale

### 1. Caméra orbitale

La caméra est positionnée sur une sphère de rayon R centrée sur la cible. On raisonne en coordonnées sphériques (R, phi, theta) contrôlées par la souris.

**Formules de conversion sphérique → cartésien** (du cours) :

```
X = R × cos(Theta) × cos(Phi)
Y = R × sin(Theta)
Z = R × cos(Theta) × sin(Phi)
```

**Contrôles GLFW :**
- Axe horizontal de la souris → phi (azimut, rotation autour de Y)
- Axe vertical de la souris → theta (élévation)
- Molette → R (distance caméra-cible)

**Limites d'angles :** phi ∈ (-π, +π) et theta ∈ (-π/2, +π/2) pour éviter le retournement de la caméra.

Variables globales pour l'état de la caméra :

```cpp
static float g_Phi = 0.0f;
static float g_Theta = 0.0f;
static float g_Radius = 15.0f;
static double g_LastMouseX = 0.0, g_LastMouseY = 0.0;
static bool g_MousePressed = false;
```

Callback clic souris — on enregistre si le bouton gauche est pressé et la position initiale :

```cpp
void OnMouseButton(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_MousePressed = (action == GLFW_PRESS);
        glfwGetCursorPos(win, &g_LastMouseX, &g_LastMouseY);
    }
}
```

Callback mouvement souris — on accumule les deltas dans phi et theta avec clamping :

```cpp
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
```

Callback molette — on modifie le rayon R :

```cpp
void OnScroll(GLFWwindow* win, double xoff, double yoff) {
    g_Radius -= (float)yoff;
    if (g_Radius < 1.0f) g_Radius = 1.0f;
    if (g_Radius > 50.0f) g_Radius = 50.0f;
}
```

Enregistrement des callbacks dans le main :

```cpp
glfwSetMouseButtonCallback(win, OnMouseButton);
glfwSetCursorPosCallback(win, OnCursorPos);
glfwSetScrollCallback(win, OnScroll);
```

Utilisation dans le rendu — conversion sphérique → cartésien puis appel à LookAt avec target à l'origine :

```cpp
float camX = g_Radius * cos(g_Theta) * cos(g_Phi);
float camY = g_Radius * sin(g_Theta);
float camZ = g_Radius * cos(g_Theta) * sin(g_Phi);

float view[16];
LookAt(view, camX, camY, camZ, 0, 0, 0, 0, 1, 0);
```

La position de la caméra est aussi passée au shader pour le calcul de la composante spéculaire (direction de l'observateur V) :

```cpp
glUniform3f(glGetUniformLocation(prog, "u_CameraPos"), camX, camY, camZ);
```

---

## TP 05 — Correction Gamma

### Exercice 3.1 — Correction gamma manuelle dans le shader

Les couleurs stockées dans les images (PNG, JPEG) sont en sRGB, donc non-linéaires (gamma ≈ 2.2). Les opérations d'éclairage sont des opérations linéaires. Il faut donc linéariser les entrées et compresser la sortie.

**En entrée** — linéariser les textures en appliquant `pow(color, 2.2)` :

```glsl
vec3 texColor = pow(texture(u_diffuseMap, v_TexCoords).rgb, vec3(2.2));
```

**En sortie** — compresser en appliquant `pow(color, 1.0/2.2)` :

```glsl
FragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
```

Les couleurs passées en uniform (matériaux, lumières) qui ont été choisies visuellement (color picker, éditeur) sont aussi non-linéaires et doivent être linéarisées de la même façon.

---

### Exercice 3.2 — Correction gamma automatique OpenGL

Le GPU peut gérer les conversions automatiquement, ce qui évite les `pow()` dans le shader. C'est la méthode utilisée dans l'implémentation finale.

**En entrée** — format interne sRGB pour les textures. Le GPU linéarise automatiquement lors de l'échantillonnage :

```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
```

Note : avec `GL_SRGB8_ALPHA8`, la composante Alpha reste linéaire.

**En sortie** — activer la conversion automatique linéaire → sRGB dans le framebuffer :

```cpp
glEnable(GL_FRAMEBUFFER_SRGB);
```

Avec ces deux options, aucune correction manuelle dans le shader n'est nécessaire.

---

## TP 05 — Illumination ambiante hémisphérique

### 2. Ambient hémisphérique

Au lieu d'appliquer une couleur ambiante uniforme, on mélange deux couleurs : SkyColor (hémisphère haut, le ciel) et GroundColor (hémisphère bas, le sol).

On utilise le produit scalaire entre la normale N du fragment et la direction du ciel (SkyDirection, typiquement (0,1,0)). Ce produit scalaire est dans [-1, +1], on le reparamètre en [0, 1] :

**HemisphereFactor = NdotSky × 0.5 + 0.5**

Puis on interpole avec `mix()` :

```glsl
float HemisphereFactor = dot(N, normalize(u_SkyDirection)) * 0.5 + 0.5;
vec3 ambient = u_material.ambientColor * mix(u_GroundColor, u_SkyColor, HemisphereFactor) * texColor;
```

La couleur finale combine les trois composantes :

```glsl
vec3 FinalColor = Ambient + Diffuse + Specular;
FragColor = vec4(FinalColor, 1.0);
```

Uniforms passés depuis le C++ :

```cpp
glUniform3f(glGetUniformLocation(prog, "u_SkyDirection"), 0, 1, 0);
glUniform3f(glGetUniformLocation(prog, "u_SkyColor"), 0.6f, 0.8f, 1.0f);
glUniform3f(glGetUniformLocation(prog, "u_GroundColor"), 0.2f, 0.2f, 0.1f);
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

Contrôles :
- Clic gauche + glisser → rotation de la caméra
- Molette → zoom avant/arrière
