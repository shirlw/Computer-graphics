#version 330 core
// When you edit these shaders, Clear CMake Configuration so they are copied to build folders

out vec4 FragColor;

uniform int hasNormals;
uniform int hasTextures;

uniform sampler2D tex;
in vec2 uv;

void main() {
    if(hasTextures== 1){
        FragColor = texture(tex,uv);
    }else{
        FragColor = vec4(1);
    }
}
