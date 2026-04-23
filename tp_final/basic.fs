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
