# Exemple de Shaders Complet

## Vertex Shader (basic.vs)

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

## Fragment Shader (basic.fs) — Blinn-Phong + Gamma + Ambient Hémisphérique

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

void main() {
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(-u_light.direction);
    vec3 V = normalize(u_CameraPos - v_Position);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 texColor = texture(u_diffuseMap, v_TexCoords).rgb;

    float hemiFactor = dot(N, normalize(u_SkyDirection)) * 0.5 + 0.5;
    vec3 ambient = u_material.ambientColor * mix(u_GroundColor, u_SkyColor, hemiFactor);

    vec3 diffuse = NdotL * u_light.diffuseColor * u_material.diffuseColor * texColor;

    float spec = 0.0;
    if (NdotL > 0.0)
        spec = pow(NdotH, u_material.shininess);
    vec3 specular = spec * u_light.specularColor * u_material.specularColor;

    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(finalColor, 1.0);
}
```

## Ce que fait chaque élément

| Élément | Source (cours/TP) |
|---------|-------------------|
| `mat3(u_Model) * a_normal` | TP à rendre — Normal Matrix simplifiée |
| `normalize(-u_light.direction)` | TP illumination — direction VERS la lumière |
| `normalize(L + V)` | TP illumination — half-vector Blinn-Phong |
| `max(dot(N, L), 0.0)` | TP illumination — Lambert |
| `pow(NdotH, shininess)` | TP illumination — spéculaire Blinn-Phong |
| `if (NdotL > 0.0)` | TP illumination — spéculaire seulement si face éclairée |
| `texture(u_diffuseMap, ...)` | TP Textures — sampling |
| `dot(N, skyDir) * 0.5 + 0.5` | TP gamma/ambient — reparamétrisation [-1,1] → [0,1] |
| `mix(ground, sky, factor)` | TP gamma/ambient — interpolation hémisphérique |
| `GL_SRGB8` + `GL_FRAMEBUFFER_SRGB` | TP gamma — correction automatique côté CPU |
