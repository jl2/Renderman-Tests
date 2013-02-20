/*
  main.cpp
  
  Copyright (c) 2013, Jeremiah LaRocco jeremiah.larocco@gmail.com

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

#include <iostream>
#include <cstdio>
#include <cmath>

#include <chrono>

#include <ri.h>

namespace sc = std::chrono;
sc::time_point<sc::steady_clock> tstamp() {
    return sc::steady_clock::now();
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
    RtInt on = 1;
    RiFrameBegin(fNum);

    char buffer[256];
    std::sprintf(buffer, "images/%s%05d.tif", scene->fprefix, fNum);
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
    RtString on_string = "on";
    RtInt samples = 2;
    RiAttribute( "light", (RtToken)"shadows", (RtPointer)&on_string, (RtToken)"samples", (RtPointer)&samples, RI_NULL );
    RtPoint lightPos = {40,80,40};
    RiAttribute((RtToken)"light", "string shadow", (RtPointer)"on", RI_NULL);
    RiLightSource("distantlight", (RtToken)"from", (RtPointer)lightPos, RI_NULL);
    
    RiWorldBegin();
  
    RiSurface((char*)"plastic", RI_NULL);

    RiAttributeBegin();

    /* RiAttribute("shade", "string transmissionhitmode", "shader", RI_NULL); */


    RtColor colors[] = {{1.0, 0.0, 0.0},
                        {0.0, 1.0, 0.0},
                        {0.0, 0.0, 1.0},
                        {1.0, 0.0, 1.0},
    };
    
    RiTranslate(-10, 10, -10);                        
    RiColor(colors[0]);
    RiSphere(5, -5, 5, 360.0, RI_NULL);

    RiTranslate(20, 0, 0);
    RiColor(colors[1]);
    RiSphere(5, -5, 5, 360.0, RI_NULL);

    RiTranslate(0, 0, 20);
    RiColor(colors[2]);
    RiSphere(5, -5, 5, 360.0, RI_NULL);

    RiTranslate(-20, 0, 0);
    RiColor(colors[3]);
    RiSphere(5, -5, 5, 360.0, RI_NULL);

    RiAttributeEnd();
    
    RtPoint pts[4] = {
        {-40.0, 0.0, -40.0},
        {-40.0, 0.0, 40.0},
        {40.0, 0.0, 40.0},
        {40.0, 0.0, -40.0},
    };

    RiAttributeBegin();
    /* RiAttribute("shade", "string transmissionhitmode", "shader", RI_NULL); */
    RiSurface("matte", RI_NULL);
    RiPolygon(4, "P", pts, RI_NULL);
    RiAttributeEnd();
    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {
    if (argc<2) {
        std::cout << "No output file name prefixgiven.\n";
        std::cout << "Use:\n\t" << argv[0] << " output_prefix\n\n";
        return 1;
    }

    const size_t NUM_FRAMES = 360;
    auto start = tstamp();

    RiBegin(RI_NULL);
    RtInt md = 4;
    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(2);

    scene_info_t scene;

    scene.cam.location[0] = 20;
    scene.cam.location[1] = 20;
    scene.cam.location[2] = 20;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;
    
    scene.fprefix = argv[1];

    /* size_t cur_frame = 0; */

    double rad = 40.0;
    double t = 0.0;
    double dt = 2.0*PI/(NUM_FRAMES-1);
    
    for (size_t fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        scene.cam.location[0] = rad * sin(t);
        scene.cam.location[2] = rad * cos(t);
        t += dt;
        std::cout << "Rendering frame " << fnum << "\n";
        doFrame(fnum, &scene);
    }

    RiEnd();

    auto end = tstamp();
    auto diff = end - start;

    std::cout << "Took " << sc::duration<double, std::milli>(diff).count()
              << " ms to render " << NUM_FRAMES << " frames.\n";

    return 0;
}
