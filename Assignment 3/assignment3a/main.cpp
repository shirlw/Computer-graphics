#include <OpenGP/GL/Application.h>
#include <OpenGP/external/LodePNG/lodepng.cpp>

using namespace OpenGP;

// Used snippets from CSC305 2018 lab
const int width=720, height=720;
typedef Eigen::Transform<float,3,Eigen::Affine> Transform;

const char* fb_vshader =
#include "fb_vshader.glsl"
;
const char* fb_fshader =
#include "fb_fshader.glsl"
;
const char* quad_vshader =
#include "quad_vshader.glsl"
;
const char* quad_fshader =
#include "quad_fshader.glsl"
;

const float SpeedFactor = 1;
void init();
void quadInit(std::unique_ptr<GPUMesh> &quad);
void wingInit(std::unique_ptr<GPUMesh> &wing);
void loadTexture(std::unique_ptr<RGBA8Texture> &texture, const char* filename);
void drawScene(float timeCount);

std::unique_ptr<GPUMesh> quad;
std::unique_ptr<GPUMesh> wing;

std::unique_ptr<Shader> quadShader;
std::unique_ptr<Shader> fbShader;

std::unique_ptr<RGBA8Texture> wing_img;
std::unique_ptr<RGBA8Texture> cat;
std::unique_ptr<RGBA8Texture> stars;

std::unique_ptr<Framebuffer> fb;
std::unique_ptr<RGBA8Texture> c_buf;
std::vector<Vec2> wing_vposition; // holds the vertices of the wing curves

//Input the points of bezier curve into the wing with de Casteljau
void drawBezier(std::vector<Vec2> points, float t) {
    if (points.size() ==1){
        wing_vposition.push_back(points[0]);
    }else{
        int size = points.size();
        std::vector<Vec2> newPoints = std::vector<Vec2>();
        for (int i=0; i<size-1;i++){
            float x = (1-t)*points[i](0)+t*points[i+1](0);
            float y = (1-t)*points[i](1)+t*points[i+1](1);
            newPoints.push_back(Vec2(x,y));
        }
        drawBezier(newPoints, t);
    }
}

//get a single point on the bezier path
Vec2 drawBezierPath(Vec2 A, Vec2 B, Vec2 C, Vec2 D, float t) {
    Vec2 P;
    P(0) = pow((1 - t), 3) * A(0) + 3 * t * pow((1 -t), 2) * B(0) + 3 * (1-t) * pow(t, 2)* C(0) + pow (t, 3)* D(0);
    P(1) = pow((1 - t), 3) * A(1) + 3 * t * pow((1 -t), 2) * B(1) + 3 * (1-t) * pow(t, 2)* C(1) + pow (t, 3)* D(1);
    return P;
}

int main(int, char**){

    Application app;
    init();

    //initialize framebuffer, initialize color buffer texture and allocate memory, attach color texture to framebuffer
    fb = std::unique_ptr<Framebuffer>(new Framebuffer());
    c_buf = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
    c_buf->allocate(width, height);
    fb->attach_color_texture(*c_buf);


    Window& window = app.create_window([](Window&){
        glViewport(0,0,width,height);

        //First draw the scene onto framebuffer, bind and then unbind framebuffer
        fb->bind();
            glClear(GL_COLOR_BUFFER_BIT);
            drawScene(glfwGetTime());
        fb->unbind();

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        fbShader->bind();
        //Bind texture and set uniforms
        glActiveTexture(GL_TEXTURE0);
        c_buf->bind();
        fbShader->set_uniform("tex", 0);
        fbShader->set_uniform("tex_width", float(width));
        fbShader->set_uniform("tex_height", float(height));
        quad->set_attributes(*fbShader);
        quad->draw();
        //unbind the texture
        c_buf->unbind();
        fbShader->unbind();
    });
    window.set_title("FrameBuffer");
    window.set_size(width, height);

    return app.run();
}

void init(){
    glClearColor(1,1,1, /*solid*/1.0 );

    fbShader = std::unique_ptr<Shader>(new Shader());
    fbShader->verbose = true;
    fbShader->add_vshader_from_source(fb_vshader);
    fbShader->add_fshader_from_source(fb_fshader);
    fbShader->link();

    quadShader = std::unique_ptr<Shader>(new Shader());
    quadShader->verbose = true;
    quadShader->add_vshader_from_source(quad_vshader);
    quadShader->add_fshader_from_source(quad_fshader);
    quadShader->link();

    quadInit(quad);
    wingInit(wing);

    loadTexture(cat, "nyancat.png");
    loadTexture(stars, "background.png");
    loadTexture(wing_img, "wing.png");
}

void quadInit(std::unique_ptr<GPUMesh> &quad) {
    quad = std::unique_ptr<GPUMesh>(new GPUMesh());
    std::vector<Vec3> quad_vposition = {
        Vec3(-1, -1, 0),
        Vec3(-1,  1, 0),
        Vec3( 1, -1, 0),
        Vec3( 1,  1, 0)
    };
    quad->set_vbo<Vec3>("vposition", quad_vposition);
    std::vector<unsigned int> quad_triangle_indices = {
        0, 2, 1, 1, 2, 3
    };
    quad->set_triangles(quad_triangle_indices);
    std::vector<Vec2> quad_vtexcoord = {
        Vec2(0, 0),
        Vec2(0,  1),
        Vec2( 1, 0),
        Vec2( 1,  1)
    };
    quad->set_vtexcoord(quad_vtexcoord);
}

//given the control points, draw the resulting bezier curve
void drawCurve(std::vector<Vec2> wing_vcontrolpoints){
    for( float i = 0.0f ; i <= 1.0f ; i += 0.01f ){
        drawBezier(wing_vcontrolpoints, i);
    }
}

void wingInit(std::unique_ptr<GPUMesh> &wing) {
    wing = std::unique_ptr<GPUMesh>(new GPUMesh());
    //set up the control points for all 5 curves of the wing
    std::vector<Vec2> wing_vcontrolpoints1 = {
        Vec2(-0.5f,0.6f),
        Vec2(-0.06f, 0.77f),
        Vec2( -0.07f, -0.52f),
        Vec2( 0.45f, 0.46f),
        Vec2( 0.6f, -0.2f)
    };
     std::vector<Vec2> wing_vcontrolpoints2 = {
         Vec2(0.6f, -0.2f),
         Vec2(0.9f, -0.39f),
         Vec2( 0.09f, -0.68f),
         Vec2( 0.0f, -0.3f)
     };
     std::vector<Vec2> wing_vcontrolpoints3 = {
         Vec2(0.0f, -0.3f),
         Vec2(-0.16f, -0.38f),
         Vec2( -0.44f, -0.16f),
         Vec2( -0.19f, -0.02f)
     };
     std::vector<Vec2> wing_vcontrolpoints4 = {
         Vec2(-0.19f, -0.02f),
         Vec2(-0.38f, 0.06f),
         Vec2( -0.50f, 0.26f),
         Vec2( -0.31f, 0.3f)
     };
      std::vector<Vec2> wing_vcontrolpoints5 = {
          Vec2( -0.31f, 0.3f),
          Vec2(-0.49f, 0.33f),
          Vec2( -0.77f, 0.66f),
          Vec2( -0.4f,0.6f)
      };

    wing_vposition = std::vector<Vec2>();
    //insert hub for triangle fan
    wing_vposition.push_back(Vec2(0.0f,0.0f));
    //draw the bezier curve given the control points, then use the curve points as the wing vertices
    drawCurve(wing_vcontrolpoints1);
    drawCurve(wing_vcontrolpoints2);
    drawCurve(wing_vcontrolpoints3);
    drawCurve(wing_vcontrolpoints4);
    drawCurve(wing_vcontrolpoints5);
    wing->set_vbo<Vec2>("vposition", wing_vposition);

    //insert indices for all the vertices
    std::vector<unsigned int> wing_indices;
    for (unsigned int i = 0; i <= 501 ; i ++ ){
         wing_indices.push_back(i);
    }
    wing->set_triangles(wing_indices);

    //set texture coordinates
    std::vector<Vec2> wing_vtexcoord = {
        Vec2(0, 0),
        Vec2(0,  1),
        Vec2( 1, 0),
        Vec2( 1,  1)
    };
    wing->set_vtexcoord(wing_vtexcoord);
}

void loadTexture(std::unique_ptr<RGBA8Texture> &texture, const char *filename) {
    // Used snippet from https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;
    //decode
    unsigned error = lodepng::decode(image, width, height, filename);
    //if there's an error, display it
    if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

    // unfortunately they are upside down...lets fix that
    unsigned char* row = new unsigned char[4*width];
    for(int i = 0; i < int(height)/2; ++i) {
        memcpy(row, &image[4*i*width], 4*width*sizeof(unsigned char));
        memcpy(&image[4*i*width], &image[image.size() - 4*(i+1)*width], 4*width*sizeof(unsigned char));
        memcpy(&image[image.size() - 4*(i+1)*width], row, 4*width*sizeof(unsigned char));
    }
    delete row;

    texture = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
    texture->upload_raw(width, height, &image[0]);
}

void drawStars(){
    Transform TRS = Transform::Identity();
    quadShader->bind();
    quadShader->set_uniform("M", TRS.matrix());
    // Make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture to the active unit for drawing
    stars->bind();
    // Set the shader's texture uniform to the index of the texture unit we have
    quadShader->set_uniform("tex", 0);
    quad->set_attributes(*quadShader);
    quad->draw();
    stars->unbind();
}

void drawRightWing(Vec2 bezierPath, float r_rot){
    Transform TRS = Transform::Identity();
    //move along the animation path
    TRS *= Eigen::Translation3f(0.18-bezierPath(0), 0.25-bezierPath(1), 0);
    //rotate the wing
    TRS *= Eigen::AngleAxisf(-0.3+r_rot, Eigen::Vector3f::UnitZ());
    //translate the wing a bit before rotating so it rotates around the base
    TRS *= Eigen::Translation3f(0.09, 0.07, 0);
    //flip the image of the wing with the negative scale
    TRS *= Eigen::AlignedScaling3f(-0.175f, 0.2f, 1);
    quadShader->bind();
    quadShader->set_uniform("M", TRS.matrix());
    // Make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture to the active unit for drawing
    wing_img->bind();
    // Set the shader's texture uniform to the index of the texture unit we have
    quadShader->set_uniform("tex", 0);
    wing->set_attributes(*quadShader);
    wing->set_mode(GL_TRIANGLE_FAN);
    wing->draw();
    wing_img->unbind();
}

void drawCat(Vec2 bezierPath){
    //the cat goes along the animation path
    Transform TRS = Transform::Identity();
    TRS *= Eigen::Translation3f(0.1-bezierPath(0), 0.2-bezierPath(1), 0);
    TRS *= Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ());
    TRS *= Eigen::AlignedScaling3f(0.2f, 0.2f, 1);
    quadShader->bind();
    quadShader->set_uniform("M", TRS.matrix());
    // Make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture to the active unit for drawing
    cat->bind();
    // Set the shader's texture uniform to the index of the texture unit we have
    quadShader->set_uniform("tex", 0);
    quad->set_attributes(*quadShader);
    quad->draw();
    cat->unbind();
}

void drawLeftWing(Vec2 bezierPath, float l_rot){
    //left wing goes along the animation path slightly offset, rotates
    Transform TRS = Transform::Identity();
    TRS *= Eigen::Translation3f(0.07-bezierPath(0), 0.25-bezierPath(1), 0);
    TRS *= Eigen::AngleAxisf(-0.3+l_rot, Eigen::Vector3f::UnitZ());
    TRS *= Eigen::Translation3f(-0.09, 0.07, 0);
    TRS *= Eigen::AlignedScaling3f(0.2f, 0.2f, 1);
    quadShader->bind();
    quadShader->set_uniform("M", TRS.matrix());
    // Make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);
    // Bind the texture to the active unit for drawing
    wing_img->bind();
    // Set the shader's texture uniform to the index of the texture unit we have
    quadShader->set_uniform("tex", 0);
    wing->set_attributes(*quadShader);
    wing->set_mode(GL_TRIANGLE_FAN);
    wing->draw();
    wing_img->unbind();
}

void drawScene(float timeCount)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //find the whole number and decimal of the time
    float t = timeCount * SpeedFactor;
    float whole_num;
    float dec = modf (t , &whole_num);
    //get rotation frequency
    float l_rot = std::cos(7*t);
    float r_rot = std::sin(7*t);
    //set up the bezier curve of animation path given the control points
    Vec2 bezierPath = drawBezierPath(Vec2(1.1f,0.9f), Vec2(-0.5f, 0.9f),Vec2( 0.25f, -0.75f),Vec2( -1.1f, 0.06f),dec);

    //draw all the objects on scene from back to front
    drawStars();
    drawRightWing(bezierPath, r_rot);
    drawCat(bezierPath);
    drawLeftWing(bezierPath, l_rot);

    quadShader->unbind();
    glDisable(GL_BLEND);
}
