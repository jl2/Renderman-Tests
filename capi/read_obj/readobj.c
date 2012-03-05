#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "ri.h"

enum obj_entry_type {vertex, normal, text_coord, face, object, comment, bad};

typedef struct text_coord_s {
    double s;
    double t;
} text_coord_t;

typedef struct face_s {
    size_t size;
    size_t *verts;
    size_t *texts;
    size_t *norms;
} face_t;

typedef struct wave_object_s {
    size_t num_verts;
    size_t num_texts;
    size_t num_norms;
    size_t num_faces;
    size_t largest_face;
    RtPoint *verts;
    text_coord_t *text_coords;
    RtPoint *norms;
    face_t *faces;
    char *name;
} wave_object_t;


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

void doFrame(size_t fNum, scene_info_t *scene, wave_object_t *obj);

void init_object(wave_object_t *obj) {
    obj->num_verts = 0;
    obj->num_norms = 0;
    obj->num_texts = 0;
    obj->num_faces = 0;
    
    obj->verts = NULL;
    obj->text_coords = NULL;
    obj->norms = NULL;
    obj->faces = NULL;
    obj->name = NULL;
    obj->largest_face = 0;
}

void free_object(wave_object_t *obj) {
    if (obj->num_verts>0  &&  obj->verts!=NULL) {
        free(obj->verts);
        obj->verts = NULL;
        obj->num_verts = 0;
    }
    if (obj->num_norms>0  &&  obj->norms!=NULL) {
        free(obj->norms);
        obj->norms = NULL;
        obj->num_norms = 0;
    }
    if (obj->num_texts>0  &&  obj->text_coords!=NULL) {
        free(obj->text_coords);
        obj->text_coords = NULL;
        obj->num_texts = 0;
    }
    if (obj->num_faces>0  &&  obj->faces!= NULL) {
        for (size_t i=0;i<obj->num_faces; ++i) {
            if (obj->faces[i].size>0) {
                if (obj->faces[i].verts!=NULL) {
                    free(obj->faces[i].verts);
                    obj->faces[i].verts = NULL;
                }
                if (obj->faces[i].norms!=NULL) {
                    free(obj->faces[i].norms);
                    obj->faces[i].norms = NULL;
                }
                if (obj->faces[i].texts!=NULL) {
                    free(obj->faces[i].texts);
                    obj->faces[i].texts = NULL;
                }

            }
        }
        free(obj->faces);
        obj->faces = NULL;
        obj->num_texts = 0;
    }
    
}

int is_obj_comment(char *ins) {
    size_t i=0;
    while ((ins[i] != '\0') && isspace(ins[i])) ++i;
    return (ins[i] == '#');
}

const char *type_to_string(enum obj_entry_type ot) {
    switch (ot) {
    case vertex:
        return "vertex";
    case normal:
        return "normal";
    case text_coord:
        return "texture coordinate";
    case face:
        return "face";
    case bad:
        return "bad type";
    case comment:
        return "comment";
    case object:
        return "object";
    }
}

int read_object_type(char *txt, wave_object_t *obj) {
    
    size_t i = 0;

    // Verify it's a 'o'
    // Find the 'o'
    while (isspace(txt[i])) ++i;
    if (txt[i] != 'o') {
        return 0;
    }
    // Skip it
    ++i;
    while (isspace(txt[i])) ++i;

    size_t nlen = strlen(txt+i);
    obj->name = malloc(nlen+1);
    strcpy(obj->name, txt+i);
    return 1;
}

enum obj_entry_type get_entry_type(char *buff) {
    char *space_pos = NULL;
    enum obj_entry_type retVal = bad;
    
    char tmp;
    
    if (is_obj_comment(buff)) {
        return comment;
    }

    if (buff[0] != '\0') {
        space_pos = strchr(buff, ' ');
        if (space_pos == NULL) {
            space_pos = strchr(buff, '\t');
        }
        if (space_pos == NULL) {
            return bad;
        }
        tmp = *space_pos;
        *space_pos = '\0';
        
        if (strcmp(buff, "v") == 0) {
            retVal = vertex;
        } else if (strcmp(buff, "vt") == 0) {
            retVal = text_coord;
        } else if (strcmp(buff, "vn") == 0) {
            retVal = normal;
        } else if (strcmp(buff, "f") == 0) {
            retVal = face;
        } else if (strcmp(buff, "o") == 0) {
            retVal = object;
        } else {
            retVal = bad;
        }
        *space_pos = tmp;
    }
    return retVal;
}



void preprocess(FILE* inf, wave_object_t *obj) {
    /*
      Scan through the file and count the number of vertices, texture
      coordinates, normals and faces
    */
    // Save original position in file
    size_t nv=0;
    size_t nn=0;
    size_t nt=0;
    size_t nf=0;
    size_t no = 0;
    char in_buffer[512] = "";
    size_t blen = 0;
    
    fpos_t original_pos;
    fgetpos(inf, &original_pos);

    // Go to the beginnning
    rewind(inf);

    while (!feof(inf)) {
        char *fgs = fgets(in_buffer, 512, inf);
        if (fgs == NULL) continue;
        blen = strlen(in_buffer);
        in_buffer[blen-1] = '\0';
        --blen;
        if (in_buffer[blen-1] == '\r') {
            in_buffer[blen-1] ='\0';
            --blen;
        }
        if (blen == 0) continue;
        enum obj_entry_type obj_type = get_entry_type(in_buffer);

        switch (obj_type) {
        case vertex:
            ++nv;
            break;
        case normal:
            ++nn;
            break;
        case text_coord:
            ++nt;
            break;
        case face:
            ++nf;
            break;
        case object:
            ++no;
            read_object_type(in_buffer, obj);
            break;
        default:
            break;
        }
    }
    obj->num_verts = nv;
    obj->verts = malloc(sizeof(RtPoint) * nv);
    
    obj->num_norms = nn;
    obj->norms = malloc(sizeof(RtPoint) * nn);
    
    obj->num_texts = nt;
    obj->text_coords = malloc(sizeof(text_coord_t) * nt);
    
    obj->num_faces = nf;
    obj->faces = malloc(sizeof(face_t) * nf);

    // Return to original position
    fsetpos(inf, &original_pos);
}

void read_data(FILE *inf, wave_object_t *obj) {
    double xt, yt, zt;
    double it, jt, kt;
    double st, tt;
    size_t num_pts;

    size_t i;
    size_t j;
    size_t type_pos;
    size_t next_space;
    char tmp;
    size_t cur_vert = 0;
    size_t cur_norm = 0;
    size_t cur_text = 0;
    size_t cur_face = 0;
    char in_buffer[512] = "";
    size_t blen = 0;
    char *fgs = NULL;
    enum obj_entry_type obj_type;
    int end = 0;
    size_t pt_cnt = 0;

    char *end_ptr;
    size_t vert;
    size_t text;
    size_t norm;
            
    // Assume no faces will ever have more than 20 points
    char *pts[20];
    size_t cur_pt = 0;
    int in_word = 0;
    char *num_start;
    char *num_end;
    
    
    // Save original file positon
    fpos_t original_pos;
    fgetpos(inf, &original_pos);

    // Go to the beginnning
    rewind(inf);

    do {
        fgs = fgets(in_buffer, 512, inf);
        if (fgs == NULL) continue;
        blen = strlen(in_buffer);
        in_buffer[blen-1] = '\0';
        --blen;
        if (in_buffer[blen-1] == '\r') {
            in_buffer[blen-1] ='\0';
            --blen;
        }
        obj_type = get_entry_type(in_buffer);

        switch (obj_type) {
        case vertex:
            // Find the 'v' and skip it and the white space after it
            num_start = strchr(in_buffer, 'v')+1;

            xt = strtod(num_start, &num_end);
            yt = strtod(num_end, &num_end);
            zt = strtod(num_end, &num_end);

            obj->verts[cur_vert][0] = (RtFloat)xt;
            obj->verts[cur_vert][1] = (RtFloat)yt;
            obj->verts[cur_vert][2] = (RtFloat)zt;
            
            ++cur_vert;
            break;

        case normal:
            // Find the 'n' and skip it and the white space after it
            num_start = strchr(in_buffer, 'n')+1;

            it = strtod(num_start, &num_end);
            jt = strtod(num_end, &num_end);
            kt = strtod(num_end, &num_end);

            obj->norms[cur_norm][0] = it;
            obj->norms[cur_norm][1] = jt;
            obj->norms[cur_norm][2] = kt;

            ++cur_norm;
            
            break;

        case text_coord:
            // Find the "vt" and skip it and the white space after it
            num_start = strchr(in_buffer, 'v')+2;

            st = strtod(num_start, &num_end);
            tt = strtod(num_end, &num_end);

            obj->text_coords[cur_text].s = st;
            obj->text_coords[cur_text].t = tt;

            ++cur_text;

            break;

        case face:
            // Find the 'f' and skip it and the white space after it
            i = strchr(in_buffer, 'f')  - in_buffer + 1;
            while (isspace(in_buffer[i])) ++i;

            end = 0;
            pt_cnt = 0;
            
            cur_pt = 0;
            in_word = 0;
            while (in_buffer[i] != '\0') {
                if (!isspace(in_buffer[i]) && in_word == 1) {
                    in_word = 1;
                }
                if (!isspace(in_buffer[i]) && in_word == 0) {
                    pts[cur_pt++] = in_buffer+i;
                    in_word = 1;
                    pt_cnt++;
                }
                if (isspace(in_buffer[i]) && in_word == 1) {
                    in_word = 0;
                    in_buffer[i] = '\0';
                }
                if (isspace(in_buffer[i]) && in_word == 0) {
                    in_word = 0;
                    in_buffer[i] = '\0';
                }
                ++i;
            }
            obj->faces[cur_face].size = pt_cnt;
            obj->faces[cur_face].verts = malloc(sizeof(size_t)*pt_cnt);
            obj->faces[cur_face].norms = malloc(sizeof(size_t)*pt_cnt);
            obj->faces[cur_face].texts = malloc(sizeof(size_t)*pt_cnt);

            for (j=0; j<pt_cnt; ++j) {
                vert = strtoul(pts[j], &end_ptr, 10);
                text = strtoul(end_ptr+1, &end_ptr, 10);
                norm = strtoul(end_ptr+1, &end_ptr, 10);
                if (vert) vert -= 1;
                if (norm) norm -= 1;
                if (text) text -= 1;
                obj->faces[cur_face].verts[j] = vert;
                obj->faces[cur_face].norms[j] = norm;
                obj->faces[cur_face].texts[j] = text;
            }
            if (pt_cnt>obj->largest_face) {
                obj->largest_face = pt_cnt;
            }
            ++cur_face;
            
            break;

        case object:
            break;

        default:
            break;
        }
            
    } while (!feof(inf));

    // Return to original position
    fsetpos(inf, &original_pos);
}

void show_object(wave_object_t *obj) {
    RtPoint *verts = malloc(sizeof(RtPoint)*(obj->largest_face));
    RtInt *nverts = malloc(sizeof(RtInt)*(obj->num_faces));
    
    int hasNorms = 1;
    size_t vn;
    size_t nn;
    size_t total_pts = 0;
    size_t i,j;
    face_t f;
    RtInt *polys;
    size_t cur_off;
    
    for (i = 0; i< obj->num_faces; ++i) {
        f = obj->faces[i];
        nverts[i] = f.size;
        total_pts += f.size;
    }
    polys = malloc(sizeof(RtInt)*total_pts);
    cur_off = 0;
    
    for (i = 0; i< obj->num_faces; ++i) {
        f = obj->faces[i];
        for (j = 0; j< f.size; ++j) {
            polys[cur_off++] = f.verts[j];
        }
    }
    printf("cur_off = %zu, obj->num_faces = %zu, total_pts = %zu\n", cur_off, obj->num_faces, total_pts);
    RiPointsPolygons(obj->num_faces,
                     nverts,
                     polys,
                     "P", obj->verts, RI_NULL);
    free(polys);
    free(nverts);
    free(verts);
}

void read_object(FILE *inf, wave_object_t *obj) {
    preprocess(inf, obj);
    read_data(inf, obj);
}

void doFrame(size_t fNum, scene_info_t *scene, wave_object_t *obj) {
    RtInt on = 1;
    char buffer[256];
    RtString on_string = "on";
    RtInt samples = 2;
    RtPoint lightPos = {40,80,40};

    RiFrameBegin(fNum);
    RtColor bgcolor = {0.2,0.8,0.2};
    RiImager("background", "color background", bgcolor, RI_NULL);
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
    show_object(obj);

    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {
    FILE *inf;
    wave_object_t obj;
    const size_t NUM_FRAMES = 360;
    RtInt md = 4;
    scene_info_t scene;
    double rad = 20;
    double t = 0.0;
    double dt = 2.0*PI/(NUM_FRAMES-1);
    size_t fnum;

    if (argc <3) {
        printf("No input and output file names given!\n");
        return 1;
    }
    
    inf = fopen(argv[1], "rt");
    if (inf == NULL) {
        printf("Could not open \"%s\"\n", argv[1]);
        return 1;

    }
    init_object(&obj);
    read_object(inf, &obj);
    
    
    printf("Object file has:\n  %zu vertices\n  %zu normals\n  %zu texture coordinates\n  %zu faces\n  %d objects\n",
           obj.num_verts, obj.num_norms, obj.num_texts, obj.num_faces, 1);

    RiBegin(RI_NULL);
    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(2);


    scene.cam.location[0] = rad;
    scene.cam.location[1] = rad;
    scene.cam.location[2] = rad;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;
    
    scene.fprefix = argv[2];

    for (fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        scene.cam.location[0] = rad * sin(t);
        scene.cam.location[2] = rad * cos(t);
        t += dt;
        printf("Rendering frame %lu\n", fnum);
        doFrame(fnum, &scene, &obj);
    }
    RiEnd();

    free_object(&obj);

    fclose(inf);
    return 0;
}
