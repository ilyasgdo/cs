#version 330 core

// Variables d'entrée (doivent correspondre parfaitement aux "out" du Vertex Shader)
in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoords;

// Sortie finale (la couleur du pixel à l'écran)
out vec4 FragColor;

struct Light {
    vec3 direction;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

uniform Light u_light;
uniform vec3 u_CameraPos;
uniform sampler2D u_diffuseMap; // Pour lire la texture

void main() {
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(u_light.direction);
    vec3 V = normalize(u_CameraPos - v_Position);
    
    // Récupération de la couleur de la texture aux coordonnées UV
    vec3 texColor = texture(u_diffuseMap, v_TexCoords).rgb;
    
    // --- Illumination avec la couleur de la texture ---
    // Ambiant
    vec3 ambient = u_light.ambientColor * texColor;
    
    // Diffus (Lambert)
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * u_light.diffuseColor * texColor;
    
    // Spéculaire (Blinn-Phong)
    vec3 spec = vec3(0.0);
    if (NdotL > 0.0) {
        vec3 H = normalize(L + V);
        spec = pow(max(dot(N, H), 0.0), 32.0) * u_light.specularColor;
    }
    
    FragColor = vec4(ambient + diffuse + spec, 1.0);
}