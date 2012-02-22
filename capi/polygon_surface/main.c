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

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <ri.h>

typedef struct camera_s {
    RtPoint location;
    RtPoint look_at;
    double roll;
} camera_t;

typedef struct scene_info_s {
    camera_t cam;
    char *fprefix;
} scene_info_t;

const double PI = 3.141592654;

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
        yrot = (direction[1] < 0) ? 180 : 0;
    else
        yrot = 180*acos(direction[2]/xzlen)/PI;

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
    RiRotate(-cam->roll, 0.0, 0.0, 1.0);
    RtPoint direction;
    direction[0] = cam->look_at[0]-cam->location[0];
    direction[1] = cam->look_at[1]-cam->location[1];
    direction[2] = cam->look_at[2]-cam->location[2];
    AimZ(direction);
    RiTranslate(-cam->location[0], -cam->location[1], -cam->location[2]);
}

void doFrame(int fNum, scene_info_t *scene);

double x(double u, double v) {
    return u;
}
double y(double u, double v) {
    return v;
}
double z(double u, double v) {
    return 2*sin(u) * cos(v);
}

double r(double u, double v) {
    return 0.0;
}
double g(double u, double v) {
    return 0.8;
}
double b(double u, double v) {
    return 0.2;
}

void doFrame(int fNum, scene_info_t *scene) {

    RiFrameBegin(fNum);

    char buffer[256];
    sprintf(buffer, "images/%s%05d.tif", scene->fprefix, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);
    RiLightSource((char*)"distantlight",RI_NULL);
    RiProjection((char*)"perspective",RI_NULL);
  
    /* RiTranslate(scene->x_trans, scene->y_trans, scene->z_trans); */
    /* RiRotate( scene->x_rotation, 1.0, 0.0, 0.0); */
    /* RiRotate( scene->y_rotation, 0.0, 1.0, 0.0); */
    /* RiRotate( scene->z_rotation, 0.0, 0.0, 1.0); */

    PlaceCamera(&scene->cam);
    RiWorldBegin();
  
    RiSurface((char*)"matte", RI_NULL);

    const size_t NUM_I = 256;
    const size_t NUM_J = 256;

    RtPoint *pts = malloc(sizeof(RtPoint)*NUM_I*NUM_J);
    RtColor *colors = malloc(sizeof(RtPoint)*NUM_I*NUM_J);
    
    double umin  = -4*PI;
    double umax = 4*PI;
    double vmin = -4*PI;
    double vmax = 4*PI;
    double du = (umax - umin)/(NUM_J-1);
    double dv = (vmax - vmin)/(NUM_I-1);

    double u = umin;
    for (size_t i=0; i< NUM_I; ++i) {
        double v = vmin;
        for (size_t j=0; j < NUM_J; ++j) {
            pts[i*NUM_J + j][0] = x(u,v);
            pts[i*NUM_J + j][1] = z(u,v);
            pts[i*NUM_J + j][2] = y(u,v);

            colors[i*NUM_J + j][0] = r(u,v);
            colors[i*NUM_J + j][1] = g(u,v);
            colors[i*NUM_J + j][2] = b(u,v);

            /* RiTransformBegin(); */
            /* RiTranslate(x(u,v), y(u,v), z(u,v)); */
            /* RiSphere(du, -du, du, 360.0, RI_NULL); */
            /* RiTransformEnd(); */
            v += dv;
        }
        u += du;
    }

    
    RtInt npolys = 2*(NUM_J+1)*(NUM_I+1);
    RtInt *nvertices = malloc(sizeof(RtInt) * npolys);
    for (size_t i=0; i<npolys; ++i) {
        nvertices[i] = 3;
    }
    RtInt *vertices = malloc(sizeof(RtInt)*3*npolys);

    size_t curIdx = 0;
    for (size_t i = 0; i<(NUM_I-1); ++i) {
        for (size_t j=0; j<(NUM_J-1); ++j) {
            vertices[curIdx] = j*NUM_I + i;
            vertices[curIdx+1] = (j+1)*NUM_I + i;
            vertices[curIdx+2] = j*NUM_I + i+1;
            vertices[curIdx+3] = (j+1)*NUM_I + i;
            vertices[curIdx+4] = (j+1)*NUM_I + (i+1);
            vertices[curIdx+5] = j*NUM_I + (i+1);
            curIdx += 6;
        }
    }
    
    RiPointsPolygons(curIdx/3, nvertices, vertices, "P", pts, "Cs", colors, RI_NULL);
    /* RiSphere(10.0, -10.0, 10.0, 360.0); */

    free(vertices);
    free(nvertices);
    free(colors);
    free(pts);
                    
    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {
    if (argc<2) {
        printf("No output file name prefixgiven.\n");
        printf("Use:\n\t%s output_prefix\n\n", argv[0]);
        exit(1);
    }

    /* const size_t NUM_FRAMES = 10; */

    RiBegin(RI_NULL);

    scene_info_t scene;

    scene.cam.location[0] = 20;
    scene.cam.location[1] = 20;
    scene.cam.location[2] = 20;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;
    
    scene.fprefix = argv[1];

    size_t cur_frame = 0;
    
    for (size_t fnum = 0; fnum <= 20; ++fnum) {
        doFrame(cur_frame, &scene);
        scene.cam.location[0] -= 2;
        cur_frame += 1;
    }
    for (size_t fnum = 0; fnum <= 20; ++fnum) {
        doFrame(cur_frame, &scene);
        scene.cam.location[1] -= 2;
        cur_frame += 1;
    }
    for (size_t fnum = 0; fnum <= 20; ++fnum) {
        doFrame(cur_frame, &scene);
        scene.cam.location[0] += 2;
        cur_frame += 1;
    }
    for (size_t fnum = 0; fnum <= 20; ++fnum) {
        doFrame(cur_frame, &scene);
        scene.cam.location[1] += 2;
        cur_frame += 1;
    }

    RiEnd();

    return 0;
}
