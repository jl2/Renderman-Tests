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

RtFloat randFloat(RtFloat min, RtFloat max) {
    RtFloat zToOne = ((RtFloat)rand()/((RtFloat)RAND_MAX));
    return zToOne * (max-min) + min;
}
void randColor(RtColor *rc) {
    (*rc)[0] = randFloat(0.0,0.5);
    (*rc)[1] = randFloat(0.0,0.5);
    (*rc)[2] = randFloat(0.0,0.5);
}
/* void randomPointInUnitSphere(double *rx, double *ry, double *rz) { */
/*     do { */
/*         *rx = randFloat(-1.0,1.0); */
/*         *ry = randFloat(-1.0,1.0); */
/*         *rz = randFloat(-1.0,1.0); */
/*     } while (*rx**rx+*ry**ry+*rz**rz > 1.0); */
/* } */
/* void randomPointOnUnitSphere(double *rx, double *ry, double *rz) { */
/*     *rx = randFloat(-1.0,1.0); */
/*     *ry = randFloat(-1.0,1.0); */
/*     *rz = randFloat(-1.0,1.0); */
/*     double len = sqrt(*rx**rx+*ry**ry+*rz**rz); */
/*     *rx/=len; */
/*     *ry/=len; */
/*     *rz/=len; */
/* } */

void showDoubleArray(size_t len, RtFloat *mat) {
    printf("{");
    for (size_t i=0;i<len; ++i) {
        printf("%f%s", mat[i], (i<(len-1))?", ": "}\n");
    }
}

void showIntArray(size_t len, int *mat) {
    printf("{");
    for (size_t i=0;i<len; ++i) {
        printf("%d%s", mat[i], (i<(len-1))?", ": "}\n");
    }
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

RtFloat xf(RtFloat u, RtFloat v) {
    return 10.0*cos(u)*sin(v);
}

RtFloat yf(RtFloat u, RtFloat v) {
    return 10.0*cos(v);
}

RtFloat zf(RtFloat u, RtFloat v) {
    return 10.0*sin(u)*sin(v);
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
    const size_t NUM_FRAMES = 120;
    RtInt md = 4;
    scene_info_t scene;
    double rad = 16.0;
    double t = 0.0;
    const double tmin = 0.0;
    const double tmax = 2.0;
    double dt = (tmax-tmin)/NUM_FRAMES;
    size_t fnum;

    RiBegin(RI_NULL);
    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(1);

    scene.cam.location[0] = rad;
    scene.cam.location[1] = rad;
    scene.cam.location[2] = rad;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;

    scene.fprefix = fprefix;
    const size_t NUM_U = 30;
    const size_t NUM_V = 60;
    const size_t NUM_SPHERES = NUM_U * NUM_V;

    RtFloat *mats = malloc(sizeof(RtFloat)*16*NUM_SPHERES);
    size_t curOff = 0;
    /* const RtFloat crad = 18.5; */
    /* mats[curOff+0] = crad; */
    /* mats[curOff+1] = 0.0; */
    /* mats[curOff+2] = 0.0; */
    /* mats[curOff+3] = 0.0; */

    /* mats[curOff+4] = 0.0; */
    /* mats[curOff+5] = crad; */
    /* mats[curOff+6] = 0.0; */
    /* mats[curOff+7] = 0.0; */

    /* mats[curOff+8] = 0.0; */
    /* mats[curOff+9] = 0.0; */
    /* mats[curOff+10] = crad; */
    /* mats[curOff+11] = 0.0; */

    /* mats[curOff+12] = 0.0; */
    /* mats[curOff+13] = 0.0; */
    /* mats[curOff+14] = 0.0; */
    /* mats[curOff+15] = 1.0; */
    /* curOff += 16; */
    const RtFloat umin = 0.0;
    const RtFloat vmin = 0.0;
    const RtFloat umax = PI;
    const RtFloat vmax = 2*PI;
    const RtFloat du = (umax - umin) / NUM_U;
    const RtFloat dv = (vmax - vmin) / NUM_V;
    RtFloat u = umin;
    RtFloat v = vmin;

    const RtFloat orad = 0.8;;
    for (size_t i=0; i<NUM_U; ++i) {
        v = vmin;
        for (size_t j=0;j<NUM_V; ++j) {
            mats[curOff+0] = orad;
            mats[curOff+1] = 0.0;
            mats[curOff+2] = 0.0;
            mats[curOff+3] = 0.0;

            mats[curOff+4] = 0.0;
            mats[curOff+5] = orad;
            mats[curOff+6] = 0.0;
            mats[curOff+7] = 0.0;

            mats[curOff+8] = 0.0;
            mats[curOff+9] = 0.0;
            mats[curOff+10] = orad;
            mats[curOff+11] = 0.0;
        
            mats[curOff+12] = xf(u,v);
            mats[curOff+13] = yf(u,v);
            mats[curOff+14] = zf(u,v);
            mats[curOff+15] = 1.0;
            curOff += 16;
            v += dv;
        }
        u += du;
    }
    RtColor css[NUM_SPHERES];
    for (size_t i=0; i< NUM_SPHERES; ++i) {
        randColor(&css[i]);
    }
    size_t numOps = (2*NUM_SPHERES + 2 + NUM_SPHERES);

    RtInt *ops = malloc(sizeof(RtInt)*numOps);
    curOff = 0;
    for (size_t i=0; i<NUM_SPHERES; ++i) {
        ops[curOff+0] = 1001;
        ops[curOff+1] = i*16;
        curOff += 2;
    }
    ops[curOff] = 0;
    curOff += 1;
    ops[curOff] = NUM_SPHERES;
    curOff += 1;
    for (size_t i=0; i<NUM_SPHERES; ++i) {
        ops[curOff] = i;
        curOff += 1;
    }

    for (fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        scene.cam.location[0] = rad*sin(t);
        scene.cam.location[1] = rad;
        scene.cam.location[2] = rad*cos(t);
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
  
        RiFormat(800, 800, 1.0);

        RiProjection((char*)"perspective",RI_NULL);

        PlaceCamera(&scene.cam);
        RiShadingInterpolation("smooth");
        /* RtFloat bound = 0.125; */
        /* char *space = "object"; */
        /* RiAttribute ("displacementbound", "sphere", (RtPointer)&bound, "space", (RtPointer)&space, RI_NULL); */
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
        RtColor opa = {0.5,0.5,0.0};
        RiOpacity(opa);
        /* RtFloat km = 0.125; */
        /* RiDisplacement((char*)"stucco", (RtToken)"Km", (RtPointer)&km, RI_NULL); */
        RiColor(col);
        /* RtColor opa = {0.75,0.75,0.75}; */
        /* RiOpacity(opa); */

        RiTransformBegin();
        RtFloat bsz = t+0.2;
        const RtFloat srad = 2.0 + t*8;
        size_t curOff = 0;
        
        /* for (size_t i=0; i<NUM_SPHERES; ++i) { */
        /*     mats[curOff+0] = srad; */
        /*     mats[curOff+5] = srad; */
        /*     mats[curOff+10] = srad; */
        /*     curOff += 16; */
        /* } */
        /* showDoubleArray(NUM_SPHERES*16, mats); */
        /* showIntArray(3*NUM_SPHERES+2, ops); */

        RiBlobby(NUM_SPHERES,
                 /* Ints */
                 numOps, ops,
                 /* Floats */
                 NUM_SPHERES * 16, mats,
                 /* Strings */
                 0, (RtString*)RI_NULL,
                 "Cs", css,
                 RI_NULL);

        RiTransformEnd();
        RiAttributeEnd();

        RiWorldEnd();
        RiFrameEnd();

    }
    free(ops);
    free(mats);
    RiEnd();

    return 0;
}

