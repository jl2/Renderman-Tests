#include <ri.h>

#include <cmath>
#include <cstdlib>
#include <iostream>

void doFrame(int fNum, char *fName);

int main(int argc, char *argv[])
{
    if (argc<2) {
        std::cerr << "No filename given.\n";
        return 1;
    }
    int i;
    RiBegin(RI_NULL);

    for (i=1;i<=360; ++i) {
        doFrame(i, argv[1]);
    }
  
    RiEnd();
}

void doFrame(int fNum, char *fName) {

    RtPoint points[4] = {-0.5,0,-0.5,
                         -0.5,0,0.5,
                         0.5,0,0.5,
                         0.5,0,-0.5};
  
    RiFrameBegin(fNum); {

        static RtColor Color = {.2, .4, .6} ;
  
        RtFloat radius=1.0,
            zmin = -1.0,
            zmax = 1.0,
            thetamax=360;
        char buffer[256];
  
        std::sprintf(buffer, "images/%s%03d.tif", fName, fNum);
        //   std::cout << buffer << "\n";
  
        RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
        RiFormat(800, 600, 1.3);
        RiLightSource((char*)"distantlight",RI_NULL);
        RiProjection((char*)"perspective",RI_NULL);
  
        RiTranslate(0.0,0.0,8.5);
        RiRotate(-40.0, 1.0,0.0,0.0);
        RiRotate(-40.0, 0.0,1.0,0.0);
  
        RtColor bgcolor = {0.9,0.9,0.9};
        RiImager((char*)"background", (char*)"color bgcolor", &bgcolor, RI_NULL);  

        RiWorldBegin(); {
  
            RiColor(Color);

            RtFloat  roughness = 0.03;
            int trace = 1;
            //RtFloat km = .3;
            //RtFloat maxKm = 1.0;
            //  RtFloat opac[] = {0.4,0.4,0.4};
            RtFloat color[] = {0.9,0.9,0.9};
            const char *texName = "texture2.tx";
  
            RiColor(color);
            RiSurface((char*)"paintedplastic", (char*)"texturename", &texName, RI_NULL);
            RiRotate(fNum, 0,1,0);

            RiAttributeBegin(); {
                RiTranslate(-5.0,2.5,0.0);
                RiSphere(2.0,-2.0,2.0,360.0,RI_NULL);
            } RiAttributeEnd();

            RiAttributeBegin(); {
                RiTranslate(0.0,2.5,0.0);
                RiCylinder(2.0,-2.0,2.0,360.0,RI_NULL);
            }RiAttributeEnd();

            RiAttributeBegin(); {
                RiTranslate(5.0,2.5,0.0);
                RiCone(4.0,2.0,360.0,RI_NULL);
            } RiAttributeEnd();

            RiAttributeBegin(); {
                RiTranslate(-5.0,-2.5,0.0);
                RiParaboloid(4.0,0.0,4.0,360.0,RI_NULL);
            } RiAttributeEnd();

            RtPoint p1 = {-1,-1,-4};
            RtPoint p2 = {4,2,4};
            RiAttributeBegin(); {
                RiTranslate(0.0,-2.5,0.0);
                RiHyperboloid(p1, p2, 360.0,RI_NULL);
            } RiAttributeEnd();

            RiAttributeBegin(); {
                RiTranslate(5.0,-2.5,0.0);
                RiTorus(2.0,0.5,0,360,360,RI_NULL);
            } RiAttributeEnd();

  
        } RiWorldEnd();
    } RiFrameEnd();
}
