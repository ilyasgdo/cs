# Matrices de Transformation et Projection

## Conventions OpenGL

- Matrices **colonnes** (column-major)
- Stockage en mémoire : colonne après colonne
- Coordonnées homogènes 4×4
- Multiplication de droite à gauche : `Résultat = Projection × View × Model × vertex`

## Organisation en tableau float[16]

```
Index :  [0]  [4]  [8]  [12]
         [1]  [5]  [9]  [13]
         [2]  [6]  [10] [14]
         [3]  [7]  [11] [15]

Colonnes: c0    c1   c2   c3
```

## Matrice Identité

```cpp
void MatrixIdentity(float* m) {
    for (int i = 0; i < 16; ++i)
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}
```

## Matrice de Translation

La translation se place dans la 4ème colonne (indices 12, 13, 14) :

```cpp
void MatrixTranslation(float* m, float tx, float ty, float tz) {
    MatrixIdentity(m);
    m[12] = tx; m[13] = ty; m[14] = tz;
}
```

## Matrices de Rotation

### Autour de X
```cpp
void MatrixRotationX(float* m, float a) {
    MatrixIdentity(m);
    float c = cos(a), s = sin(a);
    m[5] = c; m[9] = -s; m[6] = s; m[10] = c;
}
```

### Autour de Y
```cpp
void MatrixRotationY(float* m, float a) {
    MatrixIdentity(m);
    float c = cos(a), s = sin(a);
    m[0] = c; m[8] = s; m[2] = -s; m[10] = c;
}
```

### Autour de Z
```cpp
void MatrixRotationZ(float* m, float a) {
    MatrixIdentity(m);
    float c = cos(a), s = sin(a);
    m[0] = c; m[4] = -s; m[1] = s; m[5] = c;
}
```

## Multiplication de matrices

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

## World Matrix (Model Matrix)

Combine Scale → Rotation → Translation (de droite à gauche) :

```cpp
float world[16], t[16], r[16];
MatrixTranslation(t, tx, ty, tz);
MatrixRotationY(r, angle);
MatrixMultiply(world, t, r);
```

Les 3 premières colonnes contiennent les axes orientés et mis à l'échelle. La 4ème colonne contient la translation.

## Projection Perspective

```cpp
void MatrixPerspective(float* m, float fov, float aspect, float near, float far) {
    MatrixIdentity(m);
    float f = 1.0f / tan((fov * 3.14159f / 180.0f) / 2.0f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}
```

Paramètres :
- `fov` : angle de vision vertical en degrés
- `aspect` : width / height
- `near`, `far` : plans de clipping

## Normal Matrix

Pour transformer correctement les normales :

```
NormalMatrix = transpose(inverse(WorldMatrix))
```

En GLSL, si la World Matrix n'a pas de scale non-uniforme (ce qui est notre cas en général), on peut simplifier :

```glsl
vec3 worldNormal = mat3(u_Model) * a_normal;
```

La translation n'a pas de sens pour une normale (w = 0 en homogène), d'où la conversion en `mat3`.

## Envoi au shader

```cpp
glUniformMatrix4fv(glGetUniformLocation(prog, "u_Model"), 1, GL_FALSE, matrice);
glUniformMatrix4fv(glGetUniformLocation(prog, "u_View"), 1, GL_FALSE, vue);
glUniformMatrix4fv(glGetUniformLocation(prog, "u_Projection"), 1, GL_FALSE, proj);
```

`GL_FALSE` = pas de transposition (la matrice est déjà en column-major).
