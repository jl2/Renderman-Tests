#include "ri.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>
#include <time.h>

#define PI (3.141592654)

size_t randUInt(size_t min, size_t max) {
    return ((rand()%(max-min)) + min);
}

typedef struct camera_s {
    RtPoint location;
    RtPoint look_at;
    double roll;
} camera_t;

typedef struct scene_info_s {
    camera_t cam;
    char *fprefix;
} scene_info_t;

void AimZ(RtPoint direction);
void PlaceCamera(camera_t *cam);

void AimZ(RtPoint direction)
{
    double xzlen, yzlen, yrot, xrot;

    if (direction[0]==0 && direction[1]==0 && direction[2]==0)
        return;

    /*
     * The initial rotation about the y axis is given by the projection of
     * the direction vector onto the x,z plane: the x and z components
     * of the direction.
     */
    xzlen = sqrt(direction[0]*direction[0]+direction[2]*direction[2]);
    if (xzlen == 0)
        yrot = (direction[1] < 0) ? 180.0 : 0.0;
    else
        yrot = 180.0*acos(direction[2]/xzlen)/PI;

    /*
     * The second rotation, about the x axis, is given by the projection on
     * the y,z plane of the y-rotated direction vector: the original y
     * component, and the rotated x,z vector from above.
     */
    yzlen = sqrt(direction[1]*direction[1]+xzlen*xzlen);
    xrot = 180*acos(xzlen/yzlen)/PI; /* yzlen should never be 0 */

    if (direction[1] > 0)
        RiRotate(xrot, 1.0, 0.0, 0.0);
    else
        RiRotate(-xrot, 1.0, 0.0, 0.0);

    /* The last rotation declared gets performed first */
    if (direction[0] > 0)
        RiRotate(-yrot, 0.0, 1.0, 0.0);
    else
        RiRotate(yrot, 0.0, 1.0, 0.0);
}

void PlaceCamera(camera_t *cam)
{
    RtPoint direction;
    /* RiIdentity(); */
    RiRotate(-cam->roll, 0.0, 0.0, 1.0);
    direction[0] = cam->look_at[0]-cam->location[0];
    direction[1] = cam->look_at[1]-cam->location[1];
    direction[2] = cam->look_at[2]-cam->location[2];
    double len = direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2];
    len = sqrt(len);
    direction[0] *= 1.0/len;
    direction[1] *= 1.0/len;
    direction[2] *= 1.0/len;
    AimZ(direction);
    RiTranslate(-cam->location[0], -cam->location[1], -cam->location[2]);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Not enough arguments given!\n");
        return 1;
    } else if (argc > 2) {
        printf("Too many arguments given!\n");
        return 1;
    }
    char *fprefix = argv[1];
    srand(time(NULL));
    const size_t NUM_FRAMES = 100;
    RtInt md = 4;
    scene_info_t scene;
    double rad = 8.0;
    double t = 0.0;
    const double tmin = 0.0;
    const double tmax = 2.0;
    double dt = (tmax-tmin)/NUM_FRAMES;
    size_t fnum;

    RiBegin(RI_NULL);
    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(1);

    scene.cam.location[0] = 0;
    scene.cam.location[1] = rad;
    scene.cam.location[2] = 0;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;

    scene.fprefix = fprefix;

    for (fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        /* scene.cam.location[0] = rad*sin(t); */
        /* scene.cam.location[1] = (double)fnum+(NUM_FRAMES/4.0); */
        /* scene.cam.location[2] = rad*cos(t); */
        /* scene.cam.look_at[1] = rad; */
        t += dt;
        printf("Rendering frame %lu\n", (unsigned long)fnum);
        RtInt on = 1;
        char buffer[256];
        RtString on_string = "on";
        RtInt samples = 2;
        RtPoint light1Pos = {80,80,80};
        RtPoint light2Pos = {0,120,0};
        RtPoint light3Pos = {0,40,0};

        RiImager("background", RI_NULL);

        RiFrameBegin(fnum);

        
        sprintf(buffer, "images/%s%05lu.jpg", scene.fprefix, (unsigned long)fnum);
        RiDisplay(buffer,(char*)"jpeg",(char*)"rgb",RI_NULL);
  
        RiFormat(1200,1200,1.0);

        RiProjection((char*)"perspective",RI_NULL);

        PlaceCamera(&scene.cam);
        
        RiAttribute("visibility", "int trace", &on, RI_NULL);
        RiAttribute( "visibility",
                     "int camera", (RtPointer)&on,
                     "int transmission", (RtPointer)&on,
                     "int diffuse", (RtPointer)&on,
                     "int specular", (RtPointer)&on,
                     "int photon", (RtPointer)&on,
                     RI_NULL );
        RiAttribute( "light", (RtToken)"shadows", (RtPointer)&on_string, (RtToken)"samples", (RtPointer)&samples, RI_NULL );

        RiAttribute((RtToken)"light", "string shadow", (RtPointer)&on_string, RI_NULL);
        RiLightSource("distantlight", "point from", (RtPointer)light1Pos, RI_NULL);
        RiLightSource("distantlight", "point from", (RtPointer)light2Pos, RI_NULL);
        RiLightSource("pointlight", "point from", (RtPointer)light3Pos, RI_NULL);
        RiWorldBegin();

        
        RiAttributeBegin();
        /* RtColor col = {((double)fnum)/NUM_FRAMES,1.0-((double)fnum)/NUM_FRAMES,0.0}; */
        RtColor col = {0.0,1.0,0.0};
        RiSurface((char*)"matte", RI_NULL);
        RiColor(col);
        /* RtColor opa = {0.75,0.75,0.75}; */
        /* RiOpacity(opa); */

        RiTransformBegin();
        RtFloat bsz = t+0.1;
        RtFloat mats[16*4] = {bsz, 0.0, 0.0, 0.0,
                              0.0, bsz, 0.0, 0.0,
                              0.0, 0.0, bsz, 0.0,
                              0.0, 0.0, 0.0, 1.0,

                              bsz, 0.0, 0.0, 0.0,
                              0.0, bsz, 0.0, 0.0,
                              0.0, 0.0, bsz, 0.0,
                              2.0, 0.0, 0.0, 1.0,

                              bsz, 0.0, 0.0, 0.0,
                              0.0, bsz, 0.0, 0.0,
                              0.0, 0.0, bsz, 0.0,
                              2.0, 0.0, 2.0, 1.0,

                              bsz, 0.0, 0.0, 0.0,
                              0.0, bsz, 0.0, 0.0,
                              0.0, 0.0, bsz, 0.0,
                              0.0, 0.0, 2.0, 1.0,
        };
        RtInt ops[] = {1001, 0*16,
                       1001, 1*16,
                       1001, 2*16,
                       1001, 3*16,
                       0, 4, 0,1,2,3};
        RiTranslate(-1.0, 0.0, -1.0);
        RiBlobby(4,
                 /* Ints */
                 14, ops,
                 /* Floats */
                 4 * 16, mats,
                 /* Strings */
                 0, (RtString*)RI_NULL, RI_NULL);

        RiTransformEnd();
        RiAttributeEnd();

        RiWorldEnd();
        RiFrameEnd();

    }
    RiEnd();

    return 0;
}

