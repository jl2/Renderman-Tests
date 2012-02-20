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

typedef struct scene_info_s {
    double x_rotation;
    double y_rotation;
    double z_rotation;
    double x_trans;
    double y_trans;
    double z_trans;
    char *fprefix;
} scene_info_t;

void doFrame(int fNum, scene_info_t *scene);

double x(double u, double v) {
    return u;
}
double y(double u, double v) {
    return v;
}
double z(double u, double v) {
    return 2.0 * sin(u) * cos(v);
}

void doFrame(int fNum, scene_info_t *scene) {

    RiFrameBegin(fNum);

    char buffer[256];
    sprintf(buffer, "images/%s%05d.tif", scene->fprefix, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);
    RiLightSource((char*)"distantlight",RI_NULL);
    RiProjection((char*)"perspective",RI_NULL);
  
    RiTranslate(scene->x_trans, scene->y_trans, scene->z_trans);
    RiRotate( scene->x_rotation, 1.0, 0.0, 0.0);
    RiRotate( scene->y_rotation, 0.0, 1.0, 0.0);
    RiRotate( scene->z_rotation, 0.0, 0.0, 1.0);
    
    RiWorldBegin();
  
    RiSurface((char*)"matte", RI_NULL);

    const size_t NUM_I = 256;
    const size_t NUM_J = 256;

    RtPoint *pts = malloc(sizeof(RtPoint)*NUM_I*NUM_J);
    const double PI = 3.141592654;
    
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
            pts[i*NUM_J + j][1] = y(u,v);
            pts[i*NUM_J + j][2] = z(u,v);

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
    
    RiPointsPolygons(curIdx/3, nvertices, vertices, "P", pts, RI_NULL);
    /* RiSphere(10.0, -10.0, 10.0, 360.0); */

    free(vertices);
    free(nvertices);
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

    const size_t NUM_FRAMES = 100;

    RiBegin(RI_NULL);

    scene_info_t scene;
    scene.x_rotation = -120.0;
    scene.z_rotation = 90.0;
    scene.y_rotation = 0.0;

    scene.x_trans = 0.0;
    scene.y_trans = 0.0;
    scene.z_trans = 0.0;

    scene.fprefix = argv[1];

    scene.z_trans = 15.0;

    for (size_t fnum = 0; fnum< NUM_FRAMES; ++fnum) {
        doFrame(fnum, &scene);
        scene.z_rotation += (360.0/(NUM_FRAMES-1));
    }

    RiEnd();

    return 0;
}
