#include <OpenGP/GL/Application.h>

using namespace OpenGP;

// Used snippets from CSC305 2018 lab
const int width=720, height=720;
#define POINTSIZE 10.0f

const char* line_vshader =
#include "line_vshader.glsl"
;
const char* line_fshader =
#include "line_fshader.glsl"
;

void init();
std::unique_ptr<Shader> lineShader;
std::unique_ptr<GPUMesh> line;
std::unique_ptr<GPUMesh> bezierCurve;
std::vector<Vec2> controlPoints;
std::vector<Vec2> curvePoints;

// Calculate the bezier points with de Castlejau
void drawBezier(std::vector<Vec2> points, float t) {
    if (points.size() ==1){
        curvePoints.push_back(points[0]);
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

int main(int, char**){

    Application app;
    init();

    // Mouse position and selected point
    Vec2 position = Vec2(0,0);
    Vec2 *selection = nullptr;

    // Display callback
    Window& window = app.create_window([&](Window&){
        glViewport(0,0,width,height);
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(POINTSIZE);

        lineShader->bind();

        // Draw line red
        lineShader->set_uniform("selection", -1);
        line->set_attributes(*lineShader);
        line->set_mode(GL_LINE_STRIP);
        line->draw();

        //Draw bezier curve red
        bezierCurve->set_attributes(*lineShader);
        bezierCurve->set_mode(GL_LINE_STRIP);
        bezierCurve->draw();

        // Draw points red and selected point blue
        if(selection!=nullptr) lineShader->set_uniform("selection", int(selection-&controlPoints[0]));
        line->set_mode(GL_POINTS);
        line->draw();

        lineShader->unbind();
    });
    window.set_title("Mouse");
    window.set_size(width, height);

    // Mouse movement callback
    window.add_listener<MouseMoveEvent>([&](const MouseMoveEvent &m){
        // Mouse position in clip coordinates
        Vec2 p = 2.0f*(Vec2(m.position.x()/width,-m.position.y()/height) - Vec2(0.5f,-0.5f));
        if( selection && (p-position).norm() > 0.0f) {
            //Make selected control points move with cursor
            selection->x() = position.x();
            selection->y() = position.y();
            line->set_vbo<Vec2>("vposition", controlPoints);
            //erase the previous bezier curve and redraw
            curvePoints.clear();
            for( float i = 0.0f ; i <= 1.0f ; i += 0.01f ){
                drawBezier(controlPoints, i);
            }
            bezierCurve->set_vbo<Vec2>("vposition", curvePoints);
        }
        position = p;
    });

    // Mouse click callback
    window.add_listener<MouseButtonEvent>([&](const MouseButtonEvent &e){
        // Mouse selection case
        if( e.button == GLFW_MOUSE_BUTTON_LEFT && !e.released) {
            selection = nullptr;
            for(auto&& v : controlPoints) {
                if ( (v-position).norm() < POINTSIZE/std::min(width,height) ) {
                    //get the address of the control point that was clicked
                    selection = &v;
                    break;
                }
            }
        }
        // Mouse release case
        if( e.button == GLFW_MOUSE_BUTTON_LEFT && e.released) {
            if(selection) {
                //change the clicked control point to the new position
                selection->x() = position.x();
                selection->y() = position.y();
                selection = nullptr;
                line->set_vbo<Vec2>("vposition", controlPoints);
                //erase the previous bezier curve and redraw
                curvePoints.clear();
                for( float i = 0.0f ; i <= 1.0f ; i += 0.01f ){
                    drawBezier(controlPoints, i);
                }
                bezierCurve->set_vbo<Vec2>("vposition", curvePoints);
            }
        }
    });

    return app.run();
}

void init(){
    glClearColor(1,1,1, /*solid*/1.0 );

    lineShader = std::unique_ptr<Shader>(new Shader());
    lineShader->verbose = true;
    lineShader->add_vshader_from_source(line_vshader);
    lineShader->add_fshader_from_source(line_fshader);
    lineShader->link();

    //5 control points in total
    controlPoints = std::vector<Vec2>();
    Vec2 p1 = Vec2(-0.7f,-0.2f);
     Vec2 p2 = Vec2(-0.3f, 0.2f);
     Vec2 p3 = Vec2( 0.3f, 0.5f);
     Vec2 p4 = Vec2( 0.7f, 0.0f);
     Vec2 p5 = Vec2( 0.8f, 0.2f);
    controlPoints.push_back(p1);
    controlPoints.push_back(p2);
    controlPoints.push_back(p3);
    controlPoints.push_back(p4);
    controlPoints.push_back(p5);


    //draw the bezier curve given the control points
    curvePoints = std::vector<Vec2>();
    for( float i = 0.0f ; i <= 1.0f ; i += 0.01f ){
        drawBezier(controlPoints, i);
    }

    //set up the line
    line = std::unique_ptr<GPUMesh>(new GPUMesh());
    line->set_vbo<Vec2>("vposition", controlPoints);
    std::vector<unsigned int> indices = {0,1,2,3,4};
    line->set_triangles(indices);

    //set up the bezier curve
    bezierCurve = std::unique_ptr<GPUMesh>(new GPUMesh());
    bezierCurve->set_vbo<Vec2>("vposition", curvePoints);
    std::vector<unsigned int> indicesCurve;
    for (unsigned int i = 0.0f ; i <= 100 ; i ++ ){
        indicesCurve.push_back(i);
    }
    bezierCurve->set_triangles(indicesCurve);
}
