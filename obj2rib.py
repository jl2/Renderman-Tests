#!/usr/bin/python

# obj2rib.py

# Copyright (c) 2010, Jeremiah LaRocco jeremiah.larocco@gmail.com

# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.

# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


# This script uses CGKit to translate WaveFront Object files to
# Renderman RIB files.

# To use:
#    obj2rib.py input.obj output.rib

# Or, to write to standard output:
#    obj2rib.py input.obj

# To render immediately:
#    obj2rib.py input.obj | aqsis


from __future__ import print_function
from __future__ import division

from cgkit.ri import *
from cgkit.objmtl import OBJReader

from math import *

import sys

class MyObj(OBJReader):
    def __init__(self):
        self.verts = []
        self.normals = []
        self.faces = []

    def v(self, vert):
        self.verts.append([vert[0], vert[1], vert[2]])

    def vn(self, vert):
        self.normals.append([vert[0], vert[1], vert[2]])

    def f(self, *coords):
        np = ([], [])
        for coord in coords:
            vt, tvt, norm = coord
            np[0].append(vt-1)
            np[1].append(norm-1)
        self.faces.append(np)

    def get_verts(self):
        return self.verts

    def get_normals(self):
        return self.normals

    def get_faces(self):
        return self.faces

    def to_rib(self):
        for fc in self.faces:
            vt, nm = fc
            RiPolygon(P = [ self.verts[i] for i in vt],
                      N = [ self.normals[i] for i in nm])
        
def main(args):
    if len(args)<1:
        print("Not enough arguments given!")
        return
    in_fname = args[0]
    if len(args)>1:
        out_fname=args[1]
    else:
        out_fname = RI_NULL

    RiBegin(out_fname)
    RiFormat(2048,1152, 1)
    obj = MyObj()

    with open(in_fname) as inf:
        obj.read(inf)

    RiObjectBegin(1)
    obj.to_rib()
    RiObjectEnd()


    t = 0
    frames = 1
    tdiff = 1/frames
    for i in range(frames):
        RiFrameBegin(i)
        RiDisplay('output/bg{0:04d}.tif'.format(i), "file", "rgba", RI_NULL)
        RiLightSource("distantlight", RI_NULL)
        RiProjection("perspective", RI_NULL)
        RiTranslate(0.0, 0.0, 6.0)
        RiRotate(45.0, -1.0,1.0,0.0)

        RiRotate(t*360.0, 0.0,1.0,0.0)

        RiWorldBegin()

        RiAttributeBegin()
        RiColor([0,1,0])
        RiOpacity([0.5,0.5,0.5])
        RiSurface("plastic",RI_NULL)

        RiScale([2,2,2])
        RiObjectInstance(1)
        RiAttributeEnd()

        RiWorldEnd()
        RiFrameEnd()
        t += tdiff
        
    RiEnd()
    
if __name__=='__main__':
    main(sys.argv[1:])
