#version 330 core

// Entrée venant du Vertex Shader
in vec3 v_Normal;

// Ta propre variable de sortie pour la couleur
out vec4 FragColor; 

const vec3 LightDirection = vec3(1.0, -1.0, -1.0);

void main() {
    vec3 N = normalize(v_Normal); 
    vec3 L = normalize(-LightDirection); 
    
    float LambertDiffuse = max(dot(N, L), 0.0);
    
    FragColor = vec4(vec3(LambertDiffuse), 1.0);
}