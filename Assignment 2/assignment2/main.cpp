/*
 * CSC 305 201801 UVIC
 * The purpose of this source file is to demonstrate the Mesh class which you may use in assignment 2
 * Its only functionality is to render vertices/normals/textures and load textures from png files
 * A demonstration of an ImGui menu window is also included in this file
*/
#include "Mesh/Mesh.h"
#include "OpenGP/GL/glfw_helpers.h"

#include <OpenGP/types.h>
#include <OpenGP/MLogger.h>
#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/ImguiRenderer.h>

using namespace OpenGP;

const long double Pi = 3.141592653589793238L;

struct TriangleMesh{
    std::vector<Vec3> vertList;
    std::vector<unsigned int> tInd;
    std::vector<Vec3> normList;
    std::vector<Vec2> tCoordList;
};

bool WriteObj(std::string filename, TriangleMesh mesh){
    //write v_list, vt_list, vn_list, and f_list in order
    std::ofstream myfile;
    myfile.open (filename);
    for(Vec3 v:mesh.vertList){
        myfile << "v " << v(0) << " "<< v(1) << " "<< v(2) << "\n";
    }
    int i =0;
    myfile << "f ";
    for(unsigned int t:mesh.tInd){
        if (i%3==0 && i!=0){
            myfile << "\nf ";
        }
        myfile << t << " ";
        i=i++;
    }
    myfile << "\n ";
    for(Vec3 v:mesh.normList){
        myfile << "vn " << v(0) << " "<< v(1) << " "<< v(2) << "\n";
    }
    for(Vec2 v:mesh.tCoordList){
        myfile << "vt " << v(0) << " "<< v(1) << "\n";
    }
    myfile.close();
    return 0;
}

bool ReadObj(std::string filename, std::vector<Vec3> & v_list,
             std::vector<unsigned int> & f_list,
             std::vector<Vec3> & vn_list,
             std::vector<Vec2> & vt_list){
    //read contents to corresponding lists
    std::string line;
    float a,b,c;
    std::string ch;
    std::ifstream myfile (filename);
    if (myfile.is_open()) {
        while (std::getline(myfile, line)){
            std::istringstream iss(line);
            iss >> ch;
            if (ch.compare("vt")==0){
                iss >> a >> b;
                vt_list.push_back(Vec2(a,b));
            }else{
                iss >> a >> b>> c;
                if (ch.compare("v")==0){
                    v_list.push_back(Vec3(a,b,c));
                }else if (ch.compare("f")==0){
                    f_list.push_back(a);
                    f_list.push_back(b);
                    f_list.push_back(c);
                }else if (ch.compare("vn")==0){
                    vn_list.push_back(Vec3(a,b,c));
                }
            }
            //printf("%f\t%f\t%f\n", a, b, c);
        }
        myfile.close();
    }else {
        std::cout << "Unable to open file";
    }
    return 0;
}

void GenerateTorusMesh(OpenGP::Vec3 c, float rMaj, float rMin, int n_lat, int n_long){
    TriangleMesh torusMesh;
    Vec3 tubeCenter;
    //Generate points ie intersections of longitudes and latitudes ie vertices
    for (int p = 0;p < n_long;p++) {
        for (int q = 0;q < n_lat;q++) {
            tubeCenter = Vec3(c(0)+rMaj*cos(2*Pi*p/n_long), c(1) ,c(2)+rMaj*sin(2*Pi*p/n_long));
            torusMesh.vertList.push_back(Vec3(tubeCenter(0)+ rMin*cos(2*Pi*q/n_lat)*cos(2*Pi*p/n_long),
                                               tubeCenter(1)+ rMin*sin(2*Pi*q/n_lat),
                                               tubeCenter(2)+rMin*cos(2*Pi*q/n_lat)*sin(2*Pi*p/n_long)));
        }
    }

    // Generate triangles
    int x=0;
    for (int p = 0;p < n_long;p++) {
        for (int q = (p*n_lat);q < n_lat+(p*n_lat);q++) {
            //if it's the last one, needs to wrap around wil beginning longitude
            if(q+n_lat>=(n_lat*n_long)){
                torusMesh.tInd.push_back(q);
                torusMesh.tInd.push_back((q+1)%n_lat==0 ? q-n_lat+1:q+1);
                torusMesh.tInd.push_back(x);

                torusMesh.tInd.push_back((q+1)%n_lat==0 ? q-n_lat+1:q+1);
                torusMesh.tInd.push_back(x);
                torusMesh.tInd.push_back((x+1)%n_lat==0 ? 0 : x+1);
                x++;
            }else{
                torusMesh.tInd.push_back(q);
                torusMesh.tInd.push_back((q+1)%n_lat==0 ? q-n_lat+1:q+1);
                torusMesh.tInd.push_back(q+n_lat);

                torusMesh.tInd.push_back((q+1)%n_lat==0 ? q-n_lat+1:q+1);
                torusMesh.tInd.push_back(q+n_lat);
                torusMesh.tInd.push_back((q+1)%n_lat==0 ? q+1:q+n_lat+1);
            }
        }
    }

    //Texture coordinates
    int lat = 0;
    int lon = 0;
    for(Vec3 v: torusMesh.vertList){
        if(lat == n_lat){
            lat = 0;
            lon++;
        }
        float u = (float) lon / (float) n_long;
        float v = (float) lat/(float) n_lat;
        torusMesh.tCoordList.push_back(Vec2(u,v));
        lat++;
    }
    WriteObj("torus.obj", torusMesh);
}

void GenerateCylinderMesh(OpenGP::Vec3 c, float r, float h, int n_div){
    TriangleMesh cylinderMesh;
    //Determine the north pole
    //cylinderMesh.vertList.push_back(Vec3(c(0),c(1)+(h/2),c(2)));

    //Generate vertices at top cap
    for (int p = 0;p < n_div;p++) {
        cylinderMesh.vertList.push_back(Vec3(r*sin(2*Pi*p/n_div),c(1)+h/2,r*cos(2*Pi*p/n_div)));
    }
    //Generate vertices at bottom cap
    for (int p = 0;p < n_div;p++) {
        cylinderMesh.vertList.push_back(Vec3(r*sin(2*Pi*p/n_div),c(1)-h/2,r*cos(2*Pi*p/n_div)));
    }

    //Determine the south pole
    //cylinderMesh.vertList.push_back(Vec3(c(0),c(1)-(h/2),c(2)));


    // Generate triangles at top and bottom caps and through tube
    //int endVert = (n_div*2)+1;
    for (int i = 1; i <= n_div;i++){
        // Generate triangles at top cap
        /*cylinderMesh.tInd.push_back(0);
        cylinderMesh.tInd.push_back(i);
        cylinderMesh.tInd.push_back((i+1)==n_div+1 ? 1 : (i+1));*/

        // Generate triangles through tube
        cylinderMesh.tInd.push_back(i-1);
        cylinderMesh.tInd.push_back(i%n_div==0 ? 0: i); //wrap around
        cylinderMesh.tInd.push_back(i+n_div-1);

        cylinderMesh.tInd.push_back(i%n_div==0 ? 0: i);
        cylinderMesh.tInd.push_back(i+n_div-1);
        cylinderMesh.tInd.push_back(i%n_div==0 ? i :i+n_div);

        // Generate triangles at bottom cap
        /*cylinderMesh.tInd.push_back(endVert);
        cylinderMesh.tInd.push_back(endVert-i);
        cylinderMesh.tInd.push_back((endVert-i-1)==endVert-n_div-1 ? endVert-1 : (endVert-i-1));*/
    }

    //middle of top cap of cylinder texture image
    //cylinderMesh.tCoordList.push_back(Vec2(0.75,0.75));
    int i = 0;
    for(Vec3 vec:cylinderMesh.vertList){
       //cylinderMesh.tCoordList.push_back(Vec2(0.75f+(v(0)-c(0))/(2*r), 0.75f + (v(1)-c(1))/(2*r)));
       if (i<n_div){//top vertexes
           float u = (float) i/(float) n_div;
           float v = 0.5f;
           cylinderMesh.tCoordList.push_back(Vec2(u,v));
       }else{//bottom vertexes
           float u = (float) (i-n_div)/(float) n_div;
           float v = 0.0f;
           cylinderMesh.tCoordList.push_back(Vec2(u,v));
       }
       i++;
    }
    //cylinderMesh.tCoordList.push_back(Vec2(0.25,0.75));
    WriteObj("cylinder.obj", cylinderMesh);
}

void GenerateSphereMesh(OpenGP::Vec3 c, float r, int n_lat, int n_long){
    //Note: inputting latitude 4 means there are 3 horizontal lines around sphere that can intersect
    TriangleMesh sphereMesh;
    //float phi, theta, u, v;

    //Determine the north pole, eg take along the y-axis
    sphereMesh.vertList.push_back(Vec3(c(0),c(1)+r,c(2)));
    //Generate points ie intersections of longitudes and latitudes ie vertices
    for (int p = 1;p < n_lat;p++) {
        for (int q = 0;q < n_long;q++) {
            sphereMesh.vertList.push_back(Vec3(c(0)+r*sin(Pi*p/n_lat)*cos(2*Pi*q/n_long),
                                               c(1)+r*cos(Pi*p/n_lat),
                                               c(2)+r*sin(Pi*p/n_lat)*sin(2*Pi*q/n_long)));
        }
    }
    //Determine the south pole
    sphereMesh.vertList.push_back(Vec3(c(0),c(1)-r,c(2)));

    // Generate triangle fan at north pole
    for (int i = 1; i <= n_long;i++){
        sphereMesh.tInd.push_back(0);
        sphereMesh.tInd.push_back(i);
        sphereMesh.tInd.push_back((i+1)==n_long+1 ? 1 : (i+1)); //wrap around
    }

    // Generate triangle fan at south pole
    int endVert = ((n_lat-1)*n_long)+1;
    for (int i = 1; i <= n_long;i++){
        sphereMesh.tInd.push_back(endVert);
        sphereMesh.tInd.push_back(endVert-i);
        sphereMesh.tInd.push_back((endVert-i-1)==endVert-n_long-1 ? endVert-1 : (endVert-i-1));
    }

    //Generate triangle like strips around sphere
    for (int p = 0;p < n_lat-2;p++) {
        for (int q = 1+(p*n_long);q <= n_long+(p*n_long);q++) {
            //top triangles
            sphereMesh.tInd.push_back(q);
            sphereMesh.tInd.push_back(q%n_long==0 ? q-n_long+1: q+1); //wrap around
            sphereMesh.tInd.push_back(q+n_long);

            //bottoms triangles
            sphereMesh.tInd.push_back(q%n_long==0 ? q-n_long+1: q+1);
            sphereMesh.tInd.push_back(q+n_long);
            sphereMesh.tInd.push_back(q%n_long==0 ? q+1: q+n_long+1);
        }
    }

    Vec3 d;
    for(Vec3 v:sphereMesh.vertList){
            d = Vec3((c(0)-v(0)), (c(1)-v(1)), (c(2)-v(2)));
            d.normalized();
           sphereMesh.tCoordList.push_back(Vec2(0.5 + atan2(d(0),d(2))/(2*Pi),0.5 - asin(d(1))/Pi));
    }
    WriteObj("sphere.obj", sphereMesh);
}

void GenerateCubeMesh(OpenGP::Vec3 c,float h){//, int xSize, int ySize,int zSize){
    TriangleMesh cubeMesh;
    float r = h/2;

    //starting vertex, right top front corner i think
    Vec3 vertex = Vec3(c(0)+r, c(1)+r, c(2)+r);
    cubeMesh.vertList.push_back(vertex);

    //first find the front 4 vertices of the cube
    for (int i = 1;i < 4;i++) {
        if (i==1){//move left
            vertex = Vec3(vertex(0)-h, vertex(1), vertex(2));
        }else if(i==2){//move down
            vertex = Vec3(vertex(0), vertex(1)-h, vertex(2));
        }else{//move right
            vertex = Vec3(vertex(0)+h, vertex(1), vertex(2));
        }
        cubeMesh.vertList.push_back(vertex);
    }
    //move to the back four vertices
    vertex = Vec3(vertex(0), vertex(1), vertex(2)-h);
    cubeMesh.vertList.push_back(vertex);
    for (int j = 1;j < 4;j++) {
        if (j==1){//up
            vertex = Vec3(vertex(0), vertex(1)+h, vertex(2));
        }else if(j==2){//left
            vertex = Vec3(vertex(0)-h, vertex(1), vertex(2));
        }else{//down
            vertex = Vec3(vertex(0), vertex(1)-h, vertex(2));
        }
        cubeMesh.vertList.push_back(vertex);
    }

    //Texture Coordinates
    cubeMesh.tCoordList.push_back(Vec2(1,1));
    cubeMesh.tCoordList.push_back(Vec2(0,1));
    cubeMesh.tCoordList.push_back(Vec2(0,0));
    cubeMesh.tCoordList.push_back(Vec2(1,0));

    cubeMesh.tCoordList.push_back(Vec2(0,0));
    cubeMesh.tCoordList.push_back(Vec2(0,1));
    cubeMesh.tCoordList.push_back(Vec2(1,1));
    cubeMesh.tCoordList.push_back(Vec2(1,0));


    //Generate triangles (12 of them)
    cubeMesh.tInd.push_back(0);//first face triangle
    cubeMesh.tInd.push_back(1);
    cubeMesh.tInd.push_back(2);

    cubeMesh.tInd.push_back(0);//second face triangle
    cubeMesh.tInd.push_back(2);
    cubeMesh.tInd.push_back(3);

    cubeMesh.tInd.push_back(5);
    cubeMesh.tInd.push_back(7);
    cubeMesh.tInd.push_back(6);

    cubeMesh.tInd.push_back(5);
    cubeMesh.tInd.push_back(4);
    cubeMesh.tInd.push_back(7);

    cubeMesh.tInd.push_back(0);
    cubeMesh.tInd.push_back(5);
    cubeMesh.tInd.push_back(1);

    cubeMesh.tInd.push_back(1);
    cubeMesh.tInd.push_back(5);
    cubeMesh.tInd.push_back(6);

    cubeMesh.tInd.push_back(1);
    cubeMesh.tInd.push_back(6);
    cubeMesh.tInd.push_back(2);

    cubeMesh.tInd.push_back(2);
    cubeMesh.tInd.push_back(6);
    cubeMesh.tInd.push_back(7);

    cubeMesh.tInd.push_back(2);
    cubeMesh.tInd.push_back(7);
    cubeMesh.tInd.push_back(3);

    cubeMesh.tInd.push_back(3);
    cubeMesh.tInd.push_back(7);
    cubeMesh.tInd.push_back(4);

    cubeMesh.tInd.push_back(0);
    cubeMesh.tInd.push_back(3);
    cubeMesh.tInd.push_back(4);

    cubeMesh.tInd.push_back(0);
    cubeMesh.tInd.push_back(4);
    cubeMesh.tInd.push_back(5);

    WriteObj("cube.obj", cubeMesh);
}

int main() {
    Application app;
    ImguiRenderer imrenderer;
    Mesh renderMesh;
    renderMesh.init();

    std::vector<Vec3> objVertList;
    std::vector<unsigned int> objFaceList;
    std::vector<Vec3> objNormList;
    std::vector<Vec2> objTexList;

    //cube
    Vec3 cubeCenter = Vec3(-0.5f,-0.5f,0.5f);
    float height = 1.0f;
    GenerateCubeMesh(cubeCenter, height);
//    ReadObj("cube.obj", objVertList, objFaceList, objNormList, objTexList);

    //sphere
    Vec3 sphereCenter = Vec3(0.0f, 0.0f, 0.0f);
    float r = 1.0f;
    int n_lat = 20;
    int n_long = 20;
    GenerateSphereMesh(sphereCenter, r, n_lat, n_long);
//    ReadObj("sphere.obj", objVertList, objFaceList, objNormList, objTexList);

    //cylinder
    Vec3 cylinderCenter = Vec3(0.0f, 0.0f, 0.0f);
    float cr = 1.0f;
    float h = 2.0f;
    int n_div = 20;
    GenerateCylinderMesh(cylinderCenter, cr, h, n_div);
//    ReadObj("cylinder.obj", objVertList, objFaceList, objNormList, objTexList);

    //torus
    Vec3 torusCenter = Vec3(-0.5f, -0.5f, 0.5f);
    float rMaj = 0.5f; //distance from center of tube to center of torus
    float rMin = 0.25f; //radius of tube
    int tn_lat = 10;
    int tn_long = 20;
    GenerateTorusMesh(torusCenter, rMaj, rMin, tn_lat, tn_long);
    ReadObj("torus.obj", objVertList, objFaceList, objNormList, objTexList);



    //load the object (cube/sphere/cylinder)
    renderMesh.loadVertices(objVertList, objFaceList);
//    renderMesh.loadTextures("1.png");
//    renderMesh.loadTextures("earth.png");
//    renderMesh.loadTextures("soup.png");
    renderMesh.loadTextures("arrow.png");
    renderMesh.loadTexCoords(objTexList);


    //printing out contents of vertices and faces after reading from obj file
    /*for(Vec3 v:objVertList){
       std::cout << "v " << v(0) << " "<< v(1) << " "<< v(2) << "\n";
    }
    int i=0;
    for(unsigned int t:objFaceList){
        if (i%3==0){
            std::cout << "\nf ";
        }
        std::cout << t << " ";
        i=i++;
    }
    std::cout << "\n";
    for(Vec3 v:objNormList){
       std::cout << "vn " << v(0) << " "<< v(1) << " "<< v(2) << "\n";
    }
    for(Vec2 v:objTexList){
       std::cout << "vt " << v(0) << " "<< v(1) << " "<< "\n";
    }*/


    //example square object
    /*Mesh renderMesh;

    /// Example rendering a mesh
    /// Call to compile shaders
    renderMesh.init();

    /// Load Vertices and Indices (minimum required for Mesh::draw to work)
    std::vector<Vec3> vertList;
    vertList.push_back(Vec3(0,0,0));
    vertList.push_back(Vec3(1,0,0));
    vertList.push_back(Vec3(1,1,0));
    vertList.push_back(Vec3(0,1,0));
    std::vector<unsigned int> indexList;
    indexList.push_back(0); // Face 1
    indexList.push_back(1);
    indexList.push_back(2);
    indexList.push_back(2); // Face 2
    indexList.push_back(3);
    indexList.push_back(0);
    renderMesh.loadVertices(vertList, indexList);

    /// Load normals
    std::vector<Vec3> normList;
    normList.push_back(Vec3(0,0,1));
    normList.push_back(Vec3(0,0,1));
    normList.push_back(Vec3(0,0,1));
    normList.push_back(Vec3(0,0,1));
    renderMesh.loadNormals(normList);

    /// Load textures (assumes texcoords)
    renderMesh.loadTextures("earth.png");

    /// Load texture coordinates (assumes textures)
    std::vector<Vec2> tCoordList;
    tCoordList.push_back(Vec2(0,0));
    tCoordList.push_back(Vec2(1,0));
    tCoordList.push_back(Vec2(1,1));
    tCoordList.push_back(Vec2(0,1));
    renderMesh.loadTexCoords(tCoordList);*/

    /// Create main window, set callback function
    auto &window1 = app.create_window([&](Window &window){
        int width, height;
        std::tie(width, height) = window.get_size();

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClearColor(0.0f, 0.0f, 0.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// Wireframe rendering, might be helpful when debugging your mesh generation
//         glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

        float ratio = width / (float) height;
        Mat4x4 modelTransform = Mat4x4::Identity();
        Mat4x4 model = modelTransform.matrix();
        Mat4x4 projection = OpenGP::perspective(70.0f, ratio, 0.1f, 10.0f);

        //camera movement
        float time = .5f * (float)glfwGetTime();
        Vec3 cam_pos(2*cos(time), 2.0, 2*sin(time));
        Vec3 cam_look(0.0f, 0.0f, 0.0f);
        Vec3 cam_up(0.0f, 1.0f, 0.0f);
        Mat4x4 view = OpenGP::lookAt(cam_pos, cam_look, cam_up);

        renderMesh.draw(model, view, projection);
    });
    window1.set_title("Assignment 2");

    /// Create window for IMGUI, set callback function
    auto &window2 = app.create_window([&](Window &window){
        int width, height;
        std::tie(width, height) = window.get_size();

        imrenderer.begin_frame(width, height);

        ImGui::BeginMainMenuBar();
        ImGui::MenuItem("File");
        ImGui::MenuItem("Edit");
        ImGui::MenuItem("View");
        ImGui::MenuItem("Help");
        ImGui::EndMainMenuBar();

        ImGui::Begin("Test Window 1");
        ImGui::Text("This is a test imgui window");
        ImGui::End();

        glClearColor(0.15f, 0.15f, 0.15f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        imrenderer.end_frame();
    });
    window2.set_title("imgui Test");

    return app.run();
}
