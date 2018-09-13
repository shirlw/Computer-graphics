#include "OpenGP/Image/Image.h"
#include "bmpwrite.h"
#include <iostream>

using namespace OpenGP;

using Colour = Vec3; // RGB Value
Colour red() { return Colour(1.0f, 0.0f, 0.0f); }
Colour green() { return Colour(0.0f, 1.0f, 0.0f); }
Colour blue() { return Colour(0.0f, 0.0f, 1.0f); }
Colour gray() { return Colour(0.5f, 0.5f, 0.5f); }
Colour white() { return Colour(1.0f, 1.0f, 1.0f); }
Colour black() { return Colour(0.0f, 0.0f, 0.0f); }

class Sphere{
public:
    Vec3 spherePos;
    float sphereRadius;
    Colour colour;
};

class Plane{
public:
    Vec3 planePos;
    Vec3 planeNor;
    Colour colour;
};

class Light{
public:
    Vec3 lightPos;
    float lightIntensity;
    float lightIntensityAmb;
};

float checkSphereIntersection(Vec3 origin, Vec3 ray, Vec3 spherePos, float sphereRadius){
    //equation of intersection with sphere
    Vec3 EsubC = origin - spherePos;
    float disc = std::powf(ray.dot(EsubC),2) - EsubC.dot(EsubC) + sphereRadius*sphereRadius;
    if(disc >= 0) {
        //position where ray hits sphere
        float t = -ray.dot(EsubC) - std::sqrtf(disc);
        return t;
    }else{
        return 0.0f;
    }
}

Colour findSphereColour(Vec3 E,float t,Vec3 ray,Sphere sphere,Light light, Plane plane){
    Vec3 pos = E + t*ray;
    Vec3 normal = (pos - sphere.spherePos)/sphere.sphereRadius;
    normal=normal.normalized();
    Vec3 lightDir = light.lightPos - pos;
    lightDir = lightDir.normalized();
    Vec3 viewDir = E - pos;
    viewDir = viewDir.normalized();
    Vec3 h = (viewDir+lightDir);
    h= h.normalized();
    float p = 10.0f;

    Colour sColour;
    sColour = (light.lightIntensityAmb*sphere.colour
            +light.lightIntensity*std::fmaxf(0.0f, normal.dot(lightDir))*sphere.colour
            +light.lightIntensity*std::powf(std::fmaxf(0.0f, normal.dot(h)),p)*gray());

    return sColour;
}

Colour findPlaneColour(Vec3 E,float t,Vec3 ray,Plane plane,Light light, Sphere sphere){
    Vec3 pos = E + t*ray;
    Vec3 lightDir = light.lightPos - pos;
    lightDir = lightDir.normalized();
    Vec3 viewDir = E - pos;
    viewDir = viewDir.normalized();
    Vec3 h = (viewDir+lightDir);
    h= h.normalized();
    Vec3 normal = plane.planeNor;
    normal=normal.normalized();
    float p = 10.0f;
    Colour pColour;

    //check if the reflection ray hits plane
    Vec3 d = -viewDir;
    Vec3 reflectionRay = d - 2*(d.dot(normal))*normal;
    float km = 0.3f;
    float rt = checkSphereIntersection(pos, reflectionRay, sphere.spherePos, sphere.sphereRadius);
    float sphereIS = checkSphereIntersection(pos, lightDir, sphere.spherePos, sphere.sphereRadius);
    if (rt!=0.0f){ //sphere intersects with reflection ray
        pColour = km*findSphereColour(E,t,reflectionRay,sphere,light,plane);
        //add ambient light
        pColour +=(1-km)*light.lightIntensityAmb*plane.colour;
        //check if there is intersection between light rays and sphere
        if (sphereIS == 0.0f){ //no sphere intersection, add diffuse and specular lighting
            pColour = (1-km)*(pColour
                    +light.lightIntensity*std::fmaxf(0.0f, plane.planeNor.dot(lightDir))*plane.colour
                    +light.lightIntensity*std::powf(std::fmaxf(0.0f, plane.planeNor.dot(h)),p)*gray());
        }
    }else if (t>0){//everything below the plane
        pColour =light.lightIntensityAmb*plane.colour;
        //check if there is intersection between light rays and sphere
        if (sphereIS == 0.0f){ //no sphere intersection, add diffuse and specular lighting
            pColour = pColour
                    +light.lightIntensity*std::fmaxf(0.0f, plane.planeNor.dot(lightDir))*plane.colour
                    +light.lightIntensity*std::powf(std::fmaxf(0.0f, plane.planeNor.dot(h)),p)*gray();
        }
    }else{//color everything above the plane the bg color
        pColour =white();
    }
    return pColour;
}


int main(int, char**){

    int wResolution = 640;
    int hResolution = 480;
    float aspectRatio = float(wResolution) / float(hResolution);
    // #rows = hResolution, #cols = wResolution
    Image<Colour> image(hResolution, wResolution);

    /// TODO: define camera position and sphere position here
    //these 3 represent the vectors of camera
    Vec3 W = Vec3(0.0f, 0.0f, -1.0f);
    Vec3 V = Vec3(0.0f, 1.0f, 0.0f);
    Vec3 U = Vec3(1.0f, 0.0f, 0.0f);
    //distance from grid to camera is negative W
    float d = 1.0f; //focal length
    Vec3 E = -d*W;

    //want grid to be defined in terms of camera
    float left = -1.0f*aspectRatio;
    float right = 1.0f*aspectRatio;
    float bottom = -1.0f;
    float top = 1.0f;

    //for Vec3(x,y,z), +x is right, +y is up, -z is forwards of you
    //define sphere center position and radius
    Sphere sphere;
    sphere.spherePos = Vec3(0.0f, 0.0f, -8.0f);
    sphere.sphereRadius= 2.0f;
    sphere.colour = blue();

    //light position
    Light light;
    light.lightPos = Vec3(4.0f, 4.0f, -2.0f);
    light.lightIntensity = 1.0f;
    light.lightIntensityAmb = 0.3f;

    //define plane normal and position
    Plane plane;
    plane.planePos = Vec3(0.0f, -2.0f, -4.0f);
    plane.planeNor = Vec3(0.0f, 1.0f, 0.0f);
    plane.planeNor = plane.planeNor.normalized();
    plane.colour = green();

    for (int row = 0; row < image.rows(); ++row) {
        for (int col = 0; col < image.cols(); ++col) {

            //pixels in image
            Vec3 pixel = left*U + (col*(right-left)/image.cols())*U; //horz location, double the number of pixels
            pixel += bottom*V + (row*(top-bottom)/image.rows())*V; //vert

            Colour boxColour= black();
            int numSamples = 4;

            for(int i = 1; i<=numSamples;i++){ //create 4 rays/pixel for each image(row,col)

                //direction of ray that passes through that pixel
                //in an orthographic view, rays start at pixel location on image plane and all same direction -w
                Vec3 ray = pixel - E;
                ray = ray.normalized();


                ///ray plane intersection and shading
                float denom = ray.dot(plane.planeNor);

                //check if sphere intersection
                float t = checkSphereIntersection(E, ray, sphere.spherePos, sphere.sphereRadius);
                if (t!=0.0f){ //sphere intersects with ray
                     boxColour +=  findSphereColour(E, t, ray, sphere, light, plane);
                }else if (denom != 0){
                    float t = (plane.planePos - E).dot(plane.planeNor)/denom;
                    boxColour += findPlaneColour(E, t, ray, plane, light, sphere);
                }else{//to make the pixels at plane edge the same bg color
                    boxColour += white();
                }
                //move the pixel either horizontally or vertically
                int numSamplesSqrt = (int) sqrt(numSamples);
                if (i%numSamplesSqrt==0){ //if you're at the end of the row, go down and back to the left
                    pixel += ((top-bottom)/image.rows())*V*2;
                    pixel -= (numSamplesSqrt-1)*(((right-left)/image.cols())*U*2);
                }else{
                    pixel += ((right-left)/image.cols())*U*2;
                }


            }
            image(row, col) = boxColour/numSamples;
       }
    }

    bmpwrite("../../out.bmp", image);
    imshow(image);

    return EXIT_SUCCESS;
}
