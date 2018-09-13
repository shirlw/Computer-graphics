R"(
#version 330 core

uniform sampler2D water;

// The camera position
uniform vec3 viewPos;
in vec3 fragPos;

in vec2 uv;
out vec4 color;

void main() {
   color =  texture(water,uv).rgba;
// Directional light source
vec3 lightDir = normalize(vec3(1,1,1));
vec3 N= vec3(0,0,1);
vec4 lightColor = vec4(1,1,1,1);
float ambientStrength = 0.1;
float specularStrength = 0.5;

vec4 ambient = ambientStrength * lightColor;
float diff = max(dot(N, lightDir), 0.0);
vec4 diffuse = diff * lightColor;
vec3 viewDir = normalize(viewPos - fragPos);
vec3 reflectDir = reflect(-lightDir, N);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec4 specular = specularStrength * spec * lightColor;
color = (ambient+diffuse+specular) * color;
}
)"
