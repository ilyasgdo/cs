# Illumination

## Modèle de Phong

Le modèle de Phong décompose la lumière en trois composantes :

```
FinalColor = Ambient + Diffuse + Specular
```

## Composante Diffuse (Lambert)

Basée sur la loi du cosinus de Lambert :

```glsl
vec3 N = normalize(v_Normal);
vec3 L = normalize(-u_light.direction);
float NdotL = max(dot(N, L), 0.0);
vec3 Diffuse = NdotL * u_light.diffuseColor * u_material.diffuseColor;
```

- `N` : normale du fragment (en espace monde)
- `L` : direction VERS la lumière (donc `-direction`)
- `NdotL` : clamped à 0 pour éviter les valeurs négatives

## Composante Spéculaire (Phong)

Utilise le vecteur réfléchi :

```glsl
vec3 R = reflect(u_light.direction, N);
vec3 V = normalize(u_CameraPos - v_Position);
float RdotV = max(dot(R, V), 0.0);
float spec = pow(RdotV, u_material.shininess);
vec3 Specular = spec * u_light.specularColor * u_material.specularColor;
```

Le spéculaire n'apparaît que sur les faces éclairées (vérifier `NdotL > 0`).

## Composante Spéculaire (Blinn-Phong)

Remplace le vecteur réfléchi R par le half-vector H :

```glsl
vec3 H = normalize(L + V);
float NdotH = max(dot(N, H), 0.0);
float spec = pow(NdotH, u_material.shininess);
vec3 Specular = spec * u_light.specularColor * u_material.specularColor;
```

Note : `Shininess_Blinn ≈ ¼ × Shininess_Phong`

## Structures GLSL (recommandées par le cours)

```glsl
struct Light {
    vec3 direction;
    vec3 diffuseColor;
    vec3 specularColor;
};
uniform Light u_light;

struct Material {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};
uniform Material u_material;
```

Accès depuis C++ :
```cpp
glGetUniformLocation(program, "u_light.direction");
glGetUniformLocation(program, "u_material.shininess");
```

## Combinaison texture + illumination

Remplacer Kd (couleur diffuse du matériau) par la texture :

```glsl
vec3 texColor = texture(u_diffuseMap, v_TexCoords).rgb;
vec3 Diffuse = NdotL * u_light.diffuseColor * texColor;
```

## Espace de calcul

Tous les calculs doivent se faire dans le **même espace** (généralement l'espace Monde) :

- Position du fragment : `vec4 worldPos = u_Model * vec4(a_position, 1.0);`
- Normale : `vec3 worldNormal = mat3(u_Model) * a_normal;`
- Direction lumière : déjà en espace monde (passée en uniform)
- Position caméra : déjà en espace monde (passée en uniform)

## Normales

### Hard edges (normales aux faces)

Chaque sommet d'un triangle a la même normale = normale du triangle.
Calculée par produit vectoriel :

```cpp
float u[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
float v[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};
float n[3] = {u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]};
```

Puis normaliser.

### Soft edges (normales aux sommets)

Moyenne des normales des faces adjacentes. Souvent fournies dans les données (comme le dragon).

## Bonus : Atténuation

Pour les lumières ponctuelles :

```
attenuation = 1.0 / (kc + kl*d + kq*d²)
```

Avec `d` = distance entre la source et le point éclairé.

## Bonus : Lumière omnidirectionnelle

Remplacer la direction fixe par un vecteur calculé :

```glsl
vec3 L = normalize(u_lightPosition - v_Position);
```
