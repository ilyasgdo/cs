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

vec3 gammaToLinear(vec3 color) {
    return vec3(pow(color.r, 2.2), pow(color.g, 2.2), pow(color.b, 2.2));
}

vec3 diffuse(vec3 N, vec3 L, vec3 ld, vec3 md) {
    float NdotL = max(dot(N, L), 0.0);
    return NdotL * ld * md;
}

vec3 specular(vec3 N, vec3 L, vec3 V, float shininess, vec3 ls, vec3 ms) {
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    return pow(NdotH, shininess) * ls * ms;
}

void main() {
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(u_light.direction);
    vec3 V = normalize(u_CameraPos - v_Position);
    
    vec3 texColor = texture(u_diffuseMap, v_TexCoords).rgb;
    
    vec3 linMatAmb = gammaToLinear(u_material.ambientColor);
    vec3 linMatDiff = gammaToLinear(u_material.diffuseColor);
    vec3 linMatSpec = gammaToLinear(u_material.specularColor);
    vec3 linLightDiff = gammaToLinear(u_light.diffuseColor);
    vec3 linLightSpec = gammaToLinear(u_light.specularColor);
    vec3 linSky = gammaToLinear(u_SkyColor);
    vec3 linGround = gammaToLinear(u_GroundColor);

    float HemisphereFactor = dot(N, normalize(u_SkyDirection)) * 0.5 + 0.5;
    vec3 ambient = linMatAmb * mix(linGround, linSky, HemisphereFactor) * texColor;
    
    vec3 diff = diffuse(N, L, linLightDiff, linMatDiff) * texColor;
    vec3 spec = vec3(0.0);
    if (dot(N, L) > 0.0) {
        spec = specular(N, L, V, u_material.shininess, linLightSpec, linMatSpec);
    }
    
    FragColor = vec4(ambient + diff + spec, 1.0);
}