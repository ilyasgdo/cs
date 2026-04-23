# Caméra et LookAt

## Principe

La caméra est un objet de la scène avec sa propre World Matrix.
La View Matrix est l'inverse de la World Matrix de la caméra.

```
ViewMatrix = WorldMatrix_cam⁻¹ = RotationMatrix_camᵀ × TranslationMatrix_cam⁻¹
```

## Fonction LookAt

Paramètres : `position` (œil), `target` (cible), `up` (référence verticale, généralement `{0,1,0}`).

### Étapes de construction

1. **forward** = `normalize(-(target - position))` (pointe hors de l'écran, repère main droite)
2. **right** = `normalize(cross(up, forward))`
3. **up corrigé** = `cross(forward, right)`
4. **Produits scalaires** (projection de la position dans le repère caméra) :
   - `dot(-position, right)`
   - `dot(-position, up_corrigé)`
   - `dot(-position, forward)`
5. Assembler la matrice : les 3 premières colonnes = transposée des 3 vecteurs, 4ème colonne = produits scalaires

### Implémentation

```cpp
void LookAt(float* m, float ex, float ey, float ez,
            float tx, float ty, float tz,
            float ux, float uy, float uz) {
    float fx = -(tx-ex), fy = -(ty-ey), fz = -(tz-ez);
    float fl = sqrt(fx*fx+fy*fy+fz*fz);
    fx/=fl; fy/=fl; fz/=fl;

    float rx = uy*fz - uz*fy, ry = uz*fx - ux*fz, rz = ux*fy - uy*fx;
    float rl = sqrt(rx*rx+ry*ry+rz*rz);
    rx/=rl; ry/=rl; rz/=rl;

    float uux = fy*rz - fz*ry, uuy = fz*rx - fx*rz, uuz = fx*ry - fy*rx;

    MatrixIdentity(m);
    m[0]=rx;  m[4]=ry;  m[8]=rz;   m[12]=-(rx*ex+ry*ey+rz*ez);
    m[1]=uux; m[5]=uuy; m[9]=uuz;  m[13]=-(uux*ex+uuy*ey+uuz*ez);
    m[2]=fx;  m[6]=fy;  m[10]=fz;  m[14]=-(fx*ex+fy*ey+fz*ez);
    m[3]=0;   m[7]=0;   m[11]=0;   m[15]=1;
}
```

## Caméra orbitale (arcball)

La position de la caméra est sur une sphère de rayon R. La souris contrôle les angles.

### Inputs
- Axe horizontal souris → azimut (phi)
- Axe vertical souris → élévation (theta)
- Molette → distance R

### Coordonnées sphériques → cartésiennes

```cpp
float camX = R * cos(theta) * cos(phi);
float camY = R * sin(theta);
float camZ = R * cos(theta) * sin(phi);
```

### Limites d'angles

- Azimut (phi) : `(-PI, +PI)`
- Élévation (theta) : `(-PI/2, +PI/2)`

### Utilisation avec LookAt

```cpp
LookAt(viewMatrix, camX, camY, camZ, targetX, targetY, targetZ, 0, 1, 0);
```

## Chaîne complète de transformation

```
gl_Position = Projection × View × Model × vec4(a_position, 1.0)
```

Les trois matrices correspondent à :
- **Model** : local → monde
- **View** : monde → caméra
- **Projection** : caméra → NDC (clipping)
