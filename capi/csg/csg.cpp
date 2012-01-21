#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>

#include <ri.h>


void doFrame(int fNum, char *fName);

inline size_t idx(int x, int y, int width, int height);

static const double PI = 3.141592654;
double xv(double t) {
    return 16*cos(t);
}

double yv(double t) {
    return 16*sin(t);
}
int main(int argc, char *argv[])
{
    if (argc<2) {
        std::cerr << "No output filename given.\n";
        return 1;
    }
    int i;

    RiBegin(RI_NULL);

    for (i=1;i<=360; ++i) {
        doFrame(i, argv[1]);
    }
  
    RiEnd();
  
    exit(0);
}

void doFrame(int fNum, char *fName) {

    RiFrameBegin(fNum);
    static RtColor Color = {.2, .4, .6} ;

    char buffer[256];
    sprintf(buffer, "images/%s%03d.tif", fName, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);
    RiLightSource((char*)"distantlight",RI_NULL);
    RiProjection((char*)"perspective",RI_NULL);
  
    RiTranslate(0.0,0.0,50);
    // RiRotate(-40.0, 1.0,0.0,0.0);
    // RiRotate(-20.0, 0.0,1.0,0.0);
  
    RiWorldBegin();
  
    RiColor(Color);

    RiRotate(fNum, 0.0,1.0,0.0);

    RtFloat  roughness = 0.03;
    int trace = 1;
    //RtFloat km = .3;
    //RtFloat maxKm = 1.0;
    RtFloat opac[] = {0.5, 0.9, 0.3};

    // RiBasis(RiBezierBasis, 3, RiBezierBasis, 3);
    //km = abs(sin(2.0*PI*((double)fNum/100.0)));

    // RiSurface((char*)"funkyglass", "roughness", (RtPointer)&roughness, RI_NULL);
    // RiOpacity(opac);
    // RiAttribute((char*)"visibility", "int trace", &trace, RI_NULL);

    RiSurface((char*)"matte", RI_NULL);
    RiSolidBegin("difference");
    
    // RiTranslate(15,0,0);
    // RiSolidBegin("primitive");
    // RiSphere(10, -10, 10, 360, RI_NULL);
    // RiSolidEnd();

    double t=-PI;
    int steps = 8;
    double dt = (2*PI)/steps;
    RiSolidBegin("primitive");
    double rad=5.0;
    for (int ti=0; ti<steps; ++ti) {
        RiTransformBegin();
        RiTranslate(xv(t),yv(t),0);
        // RiSolidBegin("primitive");
        RiSphere(rad, -rad, rad, 360, RI_NULL);
        // RiSolidEnd();
        RiTransformEnd();
        t += dt;
    }

    for (int ti=0; ti<steps; ++ti) {
        RiTransformBegin();
        RiTranslate(0,xv(t),yv(t));
        // RiSolidBegin("primitive");
        RiSphere(rad, -rad, rad, 360, RI_NULL);
        // RiSolidEnd();
        RiTransformEnd();
        t += dt;
    }

    RiSolidEnd();

    RiSolidBegin("primitive");
    RiSphere(15, -15, 15, 360, RI_NULL);
    RiSolidEnd();
    
    // RiTranslate(-30.0,-30.0,0);
    // RiSolidBegin("primitive");
    // RiSphere(10, -10, 10, 360, RI_NULL);
    // RiSolidEnd();

    // RiTranslate(15.0,-15.0,0);
    // RiSolidBegin("primitive");
    // RiSphere(15, -15, 15, 360, RI_NULL);
    // RiSolidEnd();

    RiSolidEnd();
    RiWorldEnd();
    RiFrameEnd();
}
