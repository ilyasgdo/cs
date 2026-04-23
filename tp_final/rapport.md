# Rapport TP — Préparation à la soutenance

---

## Partie 1 — Matrices monde

### Exercice 1.1 — Multiplication de deux matrices

L'exercice demande d'implémenter une fonction multipliant deux matrices homogènes 4×4 stockées en colonnes (column-major). Chaque élément du résultat est la somme des produits ligne×colonne. On utilise un tableau temporaire pour permettre le cas `out == a` ou `out == b`.

```cpp
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
```

Le stockage column-major signifie que l'indice `[colonne*4 + ligne]` donne l'élément à la ligne `ligne` et colonne `colonne`. La première colonne occupe les indices 0-3, la deuxième 4-7, etc.

---

### Exercice 1.2 — World Matrix unique

L'exercice demande de remplacer les matrices de transformation séparées par une unique World Matrix. L'ordre de concaténation est (de droite à gauche) :

**WorldMatrix = Translation × Rotation × Scale**

Dans notre cas, pour le dragon qui tourne autour de Y et est translaté :

```cpp
float worldDragon[16], tDragon[16], rDragon[16];
MatrixTranslation(tDragon, 4.0f, -4.0f, 0.0f);
MatrixRotationY(rDragon, time * 0.5f);
MatrixMultiply(worldDragon, tDragon, rDragon);
```

`worldDragon` est la World Matrix finale envoyée au shader en un seul appel :

```cpp
glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, worldDragon);
```

Pour le cube avec trois rotations combinées :

```cpp
float worldCube[16], tCube[16], rotX[16], rotY[16], rotZ[16];
float tmp[16], tmp2[16];
MatrixTranslation(tCube, -4.0f, 0.0f, 0.0f);
MatrixRotationY(rotY, time);
MatrixRotationX(rotX, time * 0.5f);
MatrixRotationZ(rotZ, time * 0.25f);
MatrixMultiply(tmp, rotX, rotY);
MatrixMultiply(tmp2, rotZ, tmp);
MatrixMultiply(worldCube, tCube, tmp2);
```

Cela réduit à une seule matrice uniform envoyée au GPU au lieu de plusieurs matrices séparées.

Les fonctions de base utilisées :

```cpp
void MatrixIdentity(float* m)
{
    for(int i = 0; i < 16; i++)
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
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
```

La View Matrix est passée au Vertex Shader et appliquée à la position uniquement (après la World Matrix, avant la Projection). Les normales restent en espace monde pour les calculs d'illumination.

```glsl
void main()
{
    vec4 worldPos = u_Model * vec4(a_position, 1.0);
    v_Position  = worldPos.xyz;

    v_Normal    = mat3(u_Model) * a_normal;
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

## TP 05 — Partie 3 : Correction Gamma

### Contexte — Le problème de la colorimétrie

Les couleurs affichées sur un moniteur sont dites "perceptuelles". Le moniteur convertit le courant en intensité lumineuse à travers une loi de puissance (pow). Le facteur de puissance est appelé **gamma**. La plupart des moniteurs ont un gamma compris entre 1.8 et 2.4. La norme sRGB (Microsoft & HP) fixe le gamma approximativement à **2.2**.

Cela signifie deux choses :
- Les moniteurs n'ont pas une réponse linéaire : l'intensité croît en suivant une courbe de puissance
- Cette courbe distribue plus de valeurs aux extrémités (noirs et blancs), ce qui correspond mieux à la perception humaine

Le problème pour le programmeur : les opérations mathématiques (addition, multiplication dans les shaders) sont linéaires, mais les couleurs des pixels sont non-linéaires (compressées gamma).

### Ce qu'il faut retenir

- Le moniteur applique toujours une **décompression gamma** (mise à la puissance gamma)
- Une couleur visible sur un moniteur est donc toujours **non-linéaire**
- Les images (PNG, JPEG) stockent les couleurs avec une **compression gamma** (non-linéaire, sRGB 2.2)
- Une couleur compressée gamma traitée par un moniteur redevient **linéaire**

### Comment savoir si une couleur est linéaire ?

- Si la couleur a été **dessinée** (image) ou **capturée** (color picker) → elle est **non-linéaire**
- Si la couleur a été **générée par une équation** (normal maps, occlusion maps) → elle est **linéaire**

Les couleurs passées en uniform (matériaux, lumières) qui ont été choisies visuellement sont **non-linéaires** et doivent être linéarisées.

---

### Exercice 3.1 — Correction gamma manuelle dans le Fragment Shader

L'exercice demande d'appliquer les corrections gamma en lecture et en écriture dans le Fragment Shader.

**En lecture** — une couleur non-linéaire (texture sRGB) doit être linéarisée en appliquant la puissance 2.2. La couleur source non-linéaire = `couleur^(1/2.2)`, donc pour la linéariser on applique l'inverse `couleur^2.2` :

```glsl
vec3 texColor = pow(texture(u_diffuseMap, v_TexCoords).rgb, vec3(2.2));
```

De même pour les couleurs de matériaux passées en uniform et choisies visuellement :

```glsl
vec3 matDiffuse = pow(u_material.diffuseColor, vec3(2.2));
```

**En écriture** — comme le moniteur applique automatiquement une décompression gamma, si la couleur en sortie du shader est linéaire (résultat de nos calculs d'éclairage), il faut appliquer une compression gamma de `1.0/2.2` :

```glsl
vec3 finalColor = ambient + diff + spec;
FragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
```

---

### Exercice 3.2 — Correction gamma automatique avec les fonctions OpenGL

Les GPUs sont capables d'effectuer ces conversions automatiquement pour les color buffers et les textures. C'est la méthode utilisée dans notre implémentation finale.

**En lecture** — on force la conversion automatique d'une texture lors de l'échantillonnage en spécifiant un format interne sRGB. Le GPU linéarise automatiquement les texels :

```cpp
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
```

Avec alpha, on utilise `GL_SRGB8_ALPHA8`. La composante Alpha reste toujours linéaire (séparée de SRGB8).

**En sortie** — on force les écritures dans le color buffer à convertir automatiquement de linéaire vers gamma :

```cpp
glEnable(GL_FRAMEBUFFER_SRGB);
```

Avec ces deux fonctions OpenGL, aucun `pow()` dans le shader n'est nécessaire pour les textures ni pour le framebuffer.

**Note importante** : les couleurs passées en uniform (matériaux, lumières) ne sont **pas** corrigées automatiquement par le GPU. Si elles ont été choisies visuellement, il faudrait les linéariser manuellement dans le shader ou les passer déjà linéarisées depuis le C++.

Notre implémentation dans `Initialise()` :

```cpp
glEnable(GL_FRAMEBUFFER_SRGB);

glGenTextures(1, &g_Texture);
glBindTexture(GL_TEXTURE_2D, g_Texture);
GLubyte texData[] = { 255,255,255, 0,0,0, 0,0,0, 255,255,255 };
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
```

---

## TP 05 — Partie 2 : Illumination ambiante hémisphérique

### Contexte

Dans le TP illumination précédent on a implémenté le modèle de Phong avec une composante ambiante uniforme. Ici on remplace cette ambiance uniforme par une illumination hémisphérique qui combine deux couleurs :
- **SkyColor** pour l'hémisphère "up" (le ciel)
- **GroundColor** pour l'hémisphère "down" (le sol)

### Principe

La technique utilise un vecteur de référence SkyDirection (typiquement `(0, 1, 0)`) et le produit scalaire entre la normale du fragment et ce vecteur.

Le produit scalaire `dot(N, SkyDirection)` est dans `[-1, +1]` quand les vecteurs sont normalisés. Pour faciliter le mixage des couleurs, on reparamètre le résultat de sorte que le domaine soit `[0, 1]` :

**HemisphereFactor = NdotSky × 0.5 + 0.5**

On utilise ensuite la fonction `mix()` du GLSL qui interpole linéairement deux couleurs :

**AmbientColor = Ia × mix(GroundColor, SkyColor, HemisphereFactor)**

### Implémentation dans le Fragment Shader

```glsl
float HemisphereFactor = dot(N, normalize(u_SkyDirection)) * 0.5 + 0.5;
vec3 ambient = u_material.ambientColor * mix(u_GroundColor, u_SkyColor, HemisphereFactor) * texColor;
```

La couleur finale combine les trois composantes du modèle de Phong :

```glsl
vec3 FinalColor = Ambient + Diffuse + Specular;
FragColor = vec4(FinalColor, 1.0);
```

### Uniforms passés depuis le C++

```cpp
glUniform3f(glGetUniformLocation(prog, "u_SkyDirection"), 0, 1, 0);
glUniform3f(glGetUniformLocation(prog, "u_SkyColor"), 0.6f, 0.8f, 1.0f);
glUniform3f(glGetUniformLocation(prog, "u_GroundColor"), 0.2f, 0.2f, 0.1f);
```

Le résultat est une ambiance qui varie naturellement : les faces orientées vers le haut reçoivent la couleur du ciel, celles vers le bas la couleur du sol, et celles horizontales reçoivent un mélange des deux.

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
