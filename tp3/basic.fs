#version 330 core

in vec3 v_Position;
in vec3 v_Normal;

out vec4 FragColor;

struct Light {
    vec3 direction; 
    vec3 ambientColor;
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

// --- EXERCICE 1 : Fonction Diffuse (Lambert) ---
vec3 diffuse(vec3 N, vec3 L, vec3 lightDiff, vec3 matDiff) {
    float NdotL = max(dot(N, L), 0.0);
    return NdotL * lightDiff * matDiff;
}

// --- EXERCICE 3 & 4 : Fonction Spéculaire (Blinn-Phong) ---
vec3 specular(vec3 N, vec3 L, vec3 V, float shininess, vec3 lightSpec, vec3 matSpec) {
    // Calcul du demi-vecteur H (Blinn-Phong)
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    
    // Puissance spéculaire
    float specFactor = pow(NdotH, shininess);
    return specFactor * lightSpec * matSpec;
}

void main() {
    // Renormalisation obligatoire
    vec3 N = normalize(v_Normal);
    vec3 L = normalize(u_light.direction);
    vec3 V = normalize(u_CameraPos - v_Position);
    
    // Composante Ambiante
    vec3 ambient = u_light.ambientColor * u_material.ambientColor;
    
    // Calcul Diffus (appel de fonction)
    vec3 diff = diffuse(N, L, u_light.diffuseColor, u_material.diffuseColor);
    
    // Calcul Spéculaire (appel de fonction conditionné)
    vec3 spec = vec3(0.0);
    if (max(dot(N, L), 0.0) > 0.0) { // Ne calcule le reflet que si la face est éclairée
        spec = specular(N, L, V, u_material.shininess, u_light.specularColor, u_material.specularColor);
    }
    
    // Combinaison finale
    vec3 result = ambient + diff + spec;
    FragColor = vec4(result, 1.0);
}