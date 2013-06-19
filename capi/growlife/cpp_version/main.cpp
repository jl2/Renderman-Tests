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

class GameOfLife {
    GameOfLife(size_t w, size_t h) : _width(w), _height(h) {

        std::vector<bool> tmp;
        tmp.reserve(_height);
        tmp.insert(tmp.begin(), false, _height);
        _board.reserve(_width);
        _board.insert(board.begin(), tmp, _width);
    }
    size_t GetWidth() const {
        return _width;
    }
    size_t GetHeight() const {
        return _height;
    }
    size_t GetNumOn() const {
        return _num_on;
    }
    void DebugPrint() const {
        for (size_t j=0; j<_height; ++j) {
            for (size_t i=0; i<_width; ++i) {
                std::cout << (_board[i][j]?"X":" ");
            }
            std::cout << "\n";
        }
    }
    void Randomize(double prob) {
        size_t numFilled = prob*_width*_height;
        for (int i=0;i<numFilled; ++i) {
            size_t ri = randUInt(0, _height);
            size_t rj = randUInt(0, _width);
            if (! _board[ri][rj]) {
                _board[ri][rj] = true;
                _num_on += 1;
            }
        }

    }

    size_t CountNeighbors(size_t i, size_t j) {
        int w = _width;
        int h = _height;

        int num = 0;
        int up = i-1>0 ? i-1 : h-1;
        int down = i+1 <h-1 ? i+1 : 0;
        int left = j-1>0 ? j-1 : w-1;
        int right = j+1 < w-1 ? j+1 : 0;
    
        num += _board[up][j];
        num += _board[down][j];
        num += _board[i][left];
        num += _board[i][right];
        num += _board[up][left];
        num += _board[up][right];
        num += _board[down][left];
        num += _board[down][right];

        return num;
    }
private:
    size_t _width;
    size_t _height;
    size_t _num_on;
    std::vector<std::vector<bool> > board;
};
typedef struct game_of_life_s {
    size_t width;
    size_t height;
    size_t num_on;
    bool **board;
} game_of_life_t;

size_t gol_count_neighbors(game_of_life_t *board, int i, int j) {
    int w = _width;
    int h = _height;

    int num = 0;
    int up = i-1>0 ? i-1 : h-1;
    int down = i+1 <h-1 ? i+1 : 0;
    int left = j-1>0 ? j-1 : w-1;
    int right = j+1 < w-1 ? j+1 : 0;
    
    num += board->board[up][j];
    num += board->board[down][j];
    num += board->board[i][left];
    num += board->board[i][right];
    num += board->board[up][left];
    num += board->board[up][right];
    num += board->board[down][left];
    num += board->board[down][right];

    return num;
}

game_of_life_t *gol_evolve(game_of_life_t *board) {
    int w = board->width;
    int h = board->height;

    game_of_life_t *goes_to = malloc(sizeof(game_of_life_t));
    goes_to->width = w;
    goes_to->height = h;
    goes_to->num_on = 0;
    goes_to->board = malloc(sizeof(bool*)*goes_to->width);

    for (size_t i=0; i<goes_to->width; ++i) {
        goes_to->board[i] = malloc(sizeof(bool)*goes_to->height);
        for (size_t j=0; j<goes_to->height; ++j) {
            int num = gol_count_neighbors(board, i,j);

            if (board->board[i][j]) {
                if ((num < 2) || (num > 3)) {
                    goes_to->board[i][j] = false;
                } else {
                    goes_to->board[i][j] = true;
                    goes_to->num_on += 1;
                }
            } else {
                if (num == 3) {
                    goes_to->board[i][j] = true;
                    goes_to->num_on += 1;
                } else {
                    goes_to->board[i][j] = false;
                }
            }
        }
    }
    return goes_to;
}

void gol_show_renderman(game_of_life_t *board) {
    RiTransformBegin();
    RiTranslate(-(board->width/2.0), 0.0, -(board->height/2.0));
    for (size_t j=0; j<board->height; ++j) {
        RiTransformBegin();
        for (size_t i=0; i<board->width; ++i) {
            if (board->board[i][j]) {
                RiSphere(0.5, -0.5,0.5, 360.0, RI_NULL);
            }
            RiTranslate(1.0, 0.0, 0.0);
        }
        RiTransformEnd();
        RiTranslate(0.0, 0.0, 1.0);
    }
    RiTransformEnd();
}

void gol_show_renderman_blobby(game_of_life_t *boards[], size_t num) {
    size_t totalOn = 0;
    for (size_t i=0; i<num; ++i) {
        totalOn += boards[i]->num_on;
    }
    RtFloat *mats = malloc(sizeof(RtFloat)*16*totalOn);
    RtInt *ops = malloc(sizeof(RtInt)*(2*totalOn + 1*totalOn + 2));
    size_t curOff = 0;
    RiTransformBegin();
    RiTranslate(-(boards[0]->width/2.0), 0.0, -(boards[0]->height/2.0));
    for (size_t k=0; k<num; ++k) {
        game_of_life_t *board = boards[k];
        for (size_t j=0; j<board->height; ++j) {
            for (size_t i=0; i<board->width; ++i) {
                if (board->board[i][j]) {
                    mats[curOff+0] =1.2;
                    mats[curOff+1] =0.0;
                    mats[curOff+2] =0.0;
                    mats[curOff+3] =0.0;
                    mats[curOff+4] =0.0;
                    mats[curOff+5] =1.2;
                    mats[curOff+6] =0.0;
                    mats[curOff+7] =0.0;
                    mats[curOff+8] =0.0;
                    mats[curOff+9] =0.0;
                    mats[curOff+10] =1.2;
                    mats[curOff+11] =0.0;
                    mats[curOff+12] =(RtFloat)i;
                    mats[curOff+13] =(RtFloat)k;
                    mats[curOff+14] =(RtFloat)j;
                    mats[curOff+15] =1.0;
                    curOff += 16;
                }
            }
        }
    }
    curOff = 0;
    for (size_t i=0;i<totalOn; ++i) {
        ops[curOff] = 1001;
        ops[curOff+1] = i*16;
        curOff+=2;
    }
    ops[curOff] = 0;
    curOff += 1;
    ops[curOff] = totalOn;
    curOff += 1;
    for (size_t i=0;i<totalOn; ++i) {
        ops[curOff] = i;
        curOff += 1;
    }
    RiBlobby(totalOn,
             /* Ints */
             totalOn*3 + 2, ops,
             /* Floats */
             totalOn * 16, mats,
             /* Strings */
             0, (RtString*)RI_NULL, RI_NULL);
    RiTransformEnd();
}

/* #define DEBUG_LIFE 1 */

#ifdef DEBUG_LIFE
int main(int argc, char *argv[]) {
    game_of_life_t *cur= gol_create_board(50,50);

    gol_random_init(cur, 0.025);
    
    for (int i=0;i<50; ++i) {
        
        gol_debug_show_life(cur);
        game_of_life_t *next = gol_evolve(cur);
        gol_destroy_board(&cur);
        cur = next;
    }
}
#else
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
    double rad = 80.0;
    double t = 0.0;
    double dt = 2.0*PI/(NUM_FRAMES-1);
    size_t fnum;

    RiBegin(RI_NULL);
    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(1);

    scene.cam.location[0] = 0;
    scene.cam.location[1] = 0;
    scene.cam.location[2] = 0;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;

    scene.fprefix = fprefix;

    game_of_life_t *boards[NUM_FRAMES+1];
    size_t curBoard = 0;
    boards[curBoard] = gol_create_board(80,80);
    gol_random_init(boards[curBoard], 0.125);
    
    for (fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        scene.cam.location[0] = rad*sin(t);
        scene.cam.location[1] = (double)fnum+(NUM_FRAMES/4.0);
        scene.cam.location[2] = rad*cos(t);
        /* scene.cam.look_at[1] = rad; */
        t += dt;
        printf("Rendering frame %lu\n", fnum);
        RtInt on = 1;
        char buffer[256];
        RtString on_string = "on";
        RtInt samples = 2;
        RtPoint light1Pos = {80,80,80};
        RtPoint light2Pos = {0,120,0};
        RtPoint light3Pos = {0,40,0};
RiImager("background", RI_NULL);
        RiFrameBegin(fnum);

        sprintf(buffer, "images/%s%05zd.jpg", scene.fprefix, fnum);
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
        /* for (size_t i=0;i<(curBoard+1); ++i) { */
        /*     gol_show_renderman(boards[i]); */
        /*     RiTranslate(0,1.0,0); */
        /* } */
        gol_show_renderman_blobby(boards, curBoard);
        RiTransformEnd();
        RiAttributeEnd();

        boards[curBoard+1] = gol_evolve(boards[curBoard]);
        curBoard+=1;
        
        RiWorldEnd();
        RiFrameEnd();

    }
    RiEnd();

    for (size_t i=0; i<curBoard; ++i) {
        gol_destroy_board(&boards[i]);
    }

    return 0;
}
#endif
