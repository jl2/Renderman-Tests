#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

enum obj_entry_type {vertex, normal, text_coord, face, object, comment, bad};

typedef struct vertex_s {
    double x;
    double y;
    double z;
} vertex_t;

typedef struct text_coord_s {
    double s;
    double t;
} text_coord_t;

typedef struct normal_s {
    double i;
    double j;
    double k;
} normal_t;

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
    vertex_t *verts;
    text_coord_t *text_coords;
    normal_t *norms;
    face_t *faces;
    char *name;
} wave_object_t;

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
    /* default: */
    /*     return "unknown type"; */
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
            
        /* printf("\"%s\" is a %s.\n", in_buffer, type_to_string(get_entry_type(in_buffer))); */
    }
    obj->num_verts = nv;
    obj->verts = malloc(sizeof(vertex_t) * nv);
    
    obj->num_norms = nn;
    obj->norms = malloc(sizeof(normal_t) * nn);
    
    obj->num_texts = nt;
    obj->text_coords = malloc(sizeof(text_coord_t) * nt);
    
    obj->num_faces = nf;
    obj->faces = malloc(sizeof(face_t) * nf);

    // Return to original position
    fsetpos(inf, &original_pos);
}

void read_data(FILE *inf, wave_object_t *obj) {
    // Go to the beginnning
    rewind(inf);

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

        
    do {
        char *fgs = fgets(in_buffer, 512, inf);
        if (fgs == NULL) continue;
        blen = strlen(in_buffer);
        in_buffer[blen-1] = '\0';
        --blen;
        if (in_buffer[blen-1] == '\r') {
            in_buffer[blen-1] ='\0';
            --blen;
        }
        enum obj_entry_type obj_type = get_entry_type(in_buffer);

        switch (obj_type) {
        case vertex:
            // Find the 'v' and skip it and the white space after it
            i = strchr(in_buffer, 'v')  - in_buffer + 1;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            xt = atof(in_buffer+i);
            in_buffer[j] = tmp;

            i = j;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            yt = atof(in_buffer+i);
            in_buffer[j] = tmp;

            i = j;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            zt = atof(in_buffer+i);
            in_buffer[j] = tmp;

            obj->verts[cur_vert].x = xt;
            obj->verts[cur_vert].y = yt;
            obj->verts[cur_vert].z = zt;
            
            ++cur_vert;
            break;

        case normal:
            // Find the 'n' and skip it and the white space after it
            i = strchr(in_buffer, 'n')  - in_buffer + 1;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            it = atof(in_buffer+i);
            in_buffer[j] = tmp;

            i = j;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            jt = atof(in_buffer+i);
            in_buffer[j] = tmp;

            i = j;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            kt = atof(in_buffer+i);
            in_buffer[j] = tmp;
            obj->norms[cur_norm].i = it;
            obj->norms[cur_norm].j = jt;
            obj->norms[cur_norm].k = kt;

            ++cur_norm;
            
            break;

        case text_coord:
            // Find the "vt" and skip it and the white space after it
            i = strchr(in_buffer, 'v')  - in_buffer + 2;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            st = atof(in_buffer+i);
            in_buffer[j] = tmp;

            i = j;
            while (isspace(in_buffer[i])) ++i;
            j = i;
            while (!isspace(in_buffer[j])) ++j;
            tmp = in_buffer[j];
            in_buffer[j] = '\0';
            tt = atof(in_buffer+i);
            in_buffer[j] = tmp;

            obj->text_coords[cur_text].s = st;
            obj->text_coords[cur_text].t = tt;

            ++cur_text;

            break;

        case face:
            // Find the 'f' and skip it and the white space after it
            i = strchr(in_buffer, 'f')  - in_buffer + 1;
            while (isspace(in_buffer[i])) ++i;

            int end = 0;
            size_t pt_cnt = 0;
            
            // Assume no faces will ever have more than 20 points
            char *pts[20];
            size_t cur_pt = 0;
            int in_word = 0;
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

                char *end_ptr;
                size_t vert = strtoul(pts[j], &end_ptr, 10);
                size_t text = strtoul(end_ptr+1, &end_ptr, 10);
                size_t norm = strtoul(end_ptr+1, &end_ptr, 10);
                obj->faces[cur_face].verts[j] = vert;
                obj->faces[cur_face].norms[j] = norm;
                obj->faces[cur_face].texts[j] = text;
            }
            ++cur_face;
            
            break;

        case object:
            break;

        default:
            break;
        }
            
        /* printf("\"%s\" is a %s.\n", in_buffer, type_to_string(get_entry_type(in_buffer))); */
    } while (!feof(inf));
    
}
void read_object(FILE *inf, wave_object_t *obj) {
    preprocess(inf, obj);
    read_data(inf, obj);
}

int main(int argc, char *argv[]) {
    FILE *inf;
    wave_object_t obj;

    if (argc <2) {
        printf("No input file name given!\n");
        return 1;
    }
    inf = fopen(argv[1], "rt");
    if (inf == NULL) {
        printf("Could not open \"%s\"\n", argv[1]);
        return 1;
    }
    init_object(&obj);
    read_object(inf, &obj);
    
    
    printf("Object file has:\n  %zu vertices\n  %zu normals\n  %zu texture coordinates\n  %zu faces\n  %d objects",
           obj.num_verts, obj.num_norms, obj.num_texts, obj.num_faces, 1);

    free_object(&obj);

    fclose(inf);
    return 0;
}
