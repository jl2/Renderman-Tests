#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
    vertex_t *verts;
    text_coord_t *text_coords;
    normal_t *norms;
    face_t *faces;
} wave_object_t;

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

void preprocess(FILE* inf, size_t *num_verts, size_t *num_norms, size_t *num_texts, size_t *num_faces, size_t *num_objs) {
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
        fgets(in_buffer, 512, inf);
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
            break;
        default:
            break;
        }
            
        /* printf("\"%s\" is a %s.\n", in_buffer, type_to_string(get_entry_type(in_buffer))); */
    }
    *num_verts = nv;
    *num_norms = nn;
    *num_texts = nt;
    *num_faces = nf;
    *num_objs = no;

    // Return to original position
    fsetpos(inf, &original_pos);
}
int main(int argc, char *argv[]) {
    FILE *inf;
    size_t nv=0;
    size_t nn=0;
    size_t nt=0;
    size_t nf=0;
    size_t no = 0;
    
    if (argc <2) {
        printf("No input file name given!\n");
        return 1;
    }
    inf = fopen(argv[1], "rt");
    if (inf == NULL) {
        printf("Could not open \"%s\"\n", argv[1]);
        return 1;
    }

    preprocess(inf, &nv, &nn, &nt, &nf, &no);
    printf("Object file has:\n  %zu vertices\n  %zu normals\n  %zu texture coordinates\n  %zu faces\n  %zu objects", nv,nn,nt,nf, no);
    fclose(inf);
    return 0;
}
