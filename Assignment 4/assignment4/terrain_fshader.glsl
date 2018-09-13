R"(
#version 330 core
uniform sampler2D noiseTex;

uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D water;

// The camera position
uniform vec3 viewPos;

in vec2 uv;
// Fragment position in world space coordinates
in vec3 fragPos;

out vec4 color;

void main() {

    // Directional light source
    vec3 lightDir = normalize(vec3(1,1,1));

    // Texture size in pixels
    ivec2 size = textureSize(noiseTex, 0);

    /// TODO: Calculate surface normal N
    /// HINT: Use textureOffset(,,) to read height at uv + pixelwise offset
    /// HINT: Account for texture x,y dimensions in world space coordinates (default f_width=f_height=5)
    vec3 A = vec3( uv.x + 1.0/size.x, uv.y, textureOffset(noiseTex, uv, ivec2(1,0)) );
    vec3 B = vec3( uv.x - 1.0/size.x, uv.y, textureOffset(noiseTex, uv, ivec2(-1,0)) );
    vec3 C = vec3( uv.x, uv.y + 1.0/size.y, textureOffset(noiseTex, uv, ivec2(0,1)) );
    vec3 D = vec3( uv.x, uv.y - 1.0/size.y, textureOffset(noiseTex, uv, ivec2(0,-1)) );
    vec3 N = normalize( cross(normalize(A-B), normalize(C-D)) );

    /// TODO: Texture according to height and slope
    /// HINT: Read noiseTex for height at uv
    float height = texture(noiseTex, uv).r;
    float slope = 1.0f - N.z;
    //water is at below altitude
    if (height <= 0.05f){
        color = texture(water,uv).rgba;
        N= vec3(0,0,1);//all the normals for water should be pointing up
    //sand is at low altitude (beside water) and low slope
    }else if (height > 0.05f && height < 0.1f && slope < 0.8){
        color = texture(sand,uv).rgba;
    //grass is at medium altitude and low/medium slope
    } else if (height >= 0.1f && height < 0.3f && slope < 0.9){
        color = texture(grass,uv).rgba;
   /* //rock is at medium altitude and high slope or high altitude and medium slope
    } else if (height >= 0.4f && height < 0.7f){
        color = vec4(0,1,0,1);*/
    //snow is at high altitude and small slope
    } else if (height >= 0.55f && slope < 0.8){
        color = texture(snow,uv).rgba;
    //else fill in with rock
    }else{
        color = texture(rock,uv).rgba;
    }

    /*
    if (slope < 0.8){ //small slope
    if (slope < 0.9f && slope >= 0.8f){//medium slope
    if(slope >= 0.9f){//steep slope*/

    /// TODO: Calculate ambient, diffuse, and specular lighting
    /// HINT: max(,) dot(,) reflect(,) normalize()
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
