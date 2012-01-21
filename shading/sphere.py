#!/usr/bin/env python2.7

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

from math import *
import sys

def main(args):
    if len(args)>0:
        out_fname=args[0]
    else:
        out_fname = RI_NULL

    HEIGHT = 400
    WIDTH = 400

    N_FRAMES = 4

    tdiff = 1.0/N_FRAMES
    t = 0
    RiBegin(out_fname)
    RiFormat(WIDTH, HEIGHT, WIDTH/HEIGHT)

    for i in range(0,N_FRAMES):
        
        RiFrameBegin(i)

        RiDisplay('sphere{0:04d}.tif'.format(i), "file", "rgba", RI_NULL)

        RiLightSource("distantlight", RI_NULL)

        RiProjection("perspective", RI_NULL)    

        RiIdentity()

        RiTranslate(0.0, 0.0, 6.0)

        RiRotate(45.0, -1.0,1.0,0.0)

        RiWorldBegin()
    
        RiAttributeBegin()
        RiIdentity()
        RiRotate(360.0*t, 0.0,1.0,0.0)
        RiColor([0,0,1])
        RiOpacity([0.5,0.5,0.5])

        RiSurface("test_surface",RI_NULL)

        RiSphere(4.0, -4.0, 4.0, 360.0)

        RiAttributeEnd()

        RiWorldEnd()

        RiFrameEnd()
        t += tdiff

    RiEnd()
    
if __name__=='__main__':
    main(sys.argv[1:])
