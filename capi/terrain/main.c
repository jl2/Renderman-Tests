/*
  main.c
  
  Copyright (c) 2012, Jeremiah LaRocco jeremiah.larocco@gmail.com

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4244 4267 )
#endif

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <ri.h>

#include "trimesh.h"


typedef struct camera_s {
    RtPoint location;
    RtPoint look_at;
    double roll;
} camera_t;

typedef struct scene_info_s {
    camera_t cam;
    char *fprefix;
} scene_info_t;


double randF(double min, double max);

const double PI = 3.141592654;

double randF(double min, double max) {
    return (max-min) + ((double)rand())/((double)RAND_MAX) + min;
}

/*
 * AimZ(): rotate the world so the direction vector points in
 *  positive z by rotating about the y axis, then x. The cosine
 *  of each rotation is given by components of the normalized
 *  direction vector. Before the y rotation the direction vector
 *  might be in negative z, but not afterward.
 */
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
    RiRotate(-cam->roll, 0.0, 0.0, 1.0);
    direction[0] = cam->look_at[0]-cam->location[0];
    direction[1] = cam->look_at[1]-cam->location[1];
    direction[2] = cam->look_at[2]-cam->location[2];
    AimZ(direction);
    RiTranslate(-cam->location[0], -cam->location[1], -cam->location[2]);
}

void doFrame(size_t fNum, scene_info_t *scene, tri_mesh_t *tmesh);

double x(double u, double v) {
    return u;
}
double y(double u, double v) {
    return v;
}
double z(double u, double v) {
    return 4*sin(u) * cos(v);
}

double r(double u, double v) {
    return 0.0;
}
double g(double u, double v) {
    return 0.8f;
}
double b(double u, double v) {
    return 0.2f;
}

void gen_terrain(tri_mesh_t *tmesh) {
    size_t NUM_I = tmesh->NUM_I;
    size_t NUM_J = tmesh->NUM_J;
    size_t step, i, j;

    double x0,y0,z0;
    double x1,y1,z1;
    double x2,y2,z2;
    double x3,y3,z3;
    double nx,ny,nz;
    size_t hs;
    
    for (step = NUM_I/2; step > 0; step /=2) {
        for (i=step; i< NUM_I; i += step) {
            for (j=step; j < NUM_J; j += step) {
                
                tmesh_get_pt(tmesh, i-step,j-step, &x0,&y0,&z0);
                tmesh_get_pt(tmesh, i+step,j-step, &x1,&y1,&z1);
                tmesh_get_pt(tmesh, i+step,j+step, &x2,&y2,&z2);
                tmesh_get_pt(tmesh, i-step,j+step, &x3,&y3,&z3);
                
                
                tmesh_get_pt(tmesh, i,j, &nx,&ny,&nz);
                ny = (y0+y1+y2+y3)/4.0 + randF(-2.0, 2.0);
                nx = (i-(double)NUM_I/2.0)/2.0;
                nz = (j-(double)NUM_J/2.0)/2.0;
                // printf("Assigning pt %lu %lu to %f %f %f\n", i,j, nx, ny, nz);
                tmesh_set_pt(tmesh, i,j, nx,ny,nz);
            
                tmesh_set_color(tmesh, i,j, 0.0,1.0,0.0);
            }
        }
        hs = step/2;
        for (i=hs; i< NUM_I; i += step) {
            for (j=hs; j < NUM_J; j += step) {
                
                tmesh_get_pt(tmesh, i-hs,j-hs, &x0,&y0,&z0);
                tmesh_get_pt(tmesh, i+hs,j-hs, &x1,&y1,&z1);
                tmesh_get_pt(tmesh, i+hs,j+hs, &x2,&y2,&z2);
                tmesh_get_pt(tmesh, i-hs,j+hs, &x3,&y3,&z3);
                
                tmesh_get_pt(tmesh, i,j, &nx,&ny,&nz);
                ny = (y0+y1+y2+y3)/4.0 + randF(-2.0,2.0);
                nx = (i-(double)NUM_I/2.0)/2.0;
                nz = (j-(double)NUM_J/2.0)/2.0;
                // printf("Assigning pt %lu %lu to %f %f %f\n", i,j, nx, ny, nz);
                tmesh_set_pt(tmesh, i,j, nx,ny,nz);
            
                tmesh_set_color(tmesh, i,j, 0.0,1.0,0.0);
            }
        }

    }
}

void doFrame(size_t fNum, scene_info_t *scene, tri_mesh_t *tmesh) {
    RtInt on = 1;
    char buffer[256];
    RtString on_string = "on";
    RtInt samples = 2;
    RtPoint lightPos = {40,80,40};

    RiFrameBegin(fNum);

    sprintf(buffer, "images/%s%05lu.tif", scene->fprefix, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);


    RiProjection((char*)"perspective",RI_NULL);
    PlaceCamera(&scene->cam);

    /* RiAttribute("visibility", "int trace", &on, RI_NULL); */
    RiAttribute( "visibility",
                 "int camera", (RtPointer)&on,
                 "int transmission", (RtPointer)&on,
                 "int diffuse", (RtPointer)&on,
                 "int specular", (RtPointer)&on,
                 "int photon", (RtPointer)&on,
                 RI_NULL );
    RiAttribute( "light", (RtToken)"shadows", (RtPointer)&on_string, (RtToken)"samples", (RtPointer)&samples, RI_NULL );

    RiAttribute((RtToken)"light", "string shadow", (RtPointer)"on", RI_NULL);
    RiLightSource("distantlight", (RtToken)"from", (RtPointer)lightPos, RI_NULL);
    
    RiWorldBegin();
  
    RiSurface((char*)"matte", RI_NULL);

    tmesh_render(tmesh);

    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {

    const size_t NUM_FRAMES = 20;
    RtInt md = 4;
    scene_info_t scene;
    double rad = 40.0;
    double t = 0.0;
    double dt = 2.0*PI/(NUM_FRAMES-1);
    tri_mesh_t tmesh;
    size_t fnum;

    if (argc<2) {
        printf("No output file name prefixgiven.\n");
        printf("Use:\n\t%s output_prefix\n\n", argv[0]);
        exit(1);
    }



    RiBegin(RI_NULL);

    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(2);


    scene.cam.location[0] = 50;
    scene.cam.location[1] = 50;
    scene.cam.location[2] = 50;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;
    
    scene.fprefix = argv[1];

    /* size_t cur_frame = 0; */


    tmesh_alloc(&tmesh, 256,256);
    gen_terrain(&tmesh);
    
    for (fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        scene.cam.location[0] = rad * sin(t);
        scene.cam.location[2] = rad * cos(t);
        t += dt;
        printf("Rendering frame %lu\n", fnum);
        doFrame(fnum, &scene, &tmesh);
    }
    tmesh_free(&tmesh);
    RiEnd();

    return 0;
}
