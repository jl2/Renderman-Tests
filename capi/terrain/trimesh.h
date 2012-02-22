/*
  trimesh.h
  
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

#ifndef TRI_MESH_H
#define TRI_MESH_H

#include <ri.h>

#include <stdlib.h>

typedef struct tri_mesh_s {
    size_t NUM_I;
    size_t NUM_J;
    RtInt npolys;
    RtInt *nvertices;
    RtInt *vertices;
    RtPoint *pts;
    RtColor *colors;
} tri_mesh_t;

/* size_t idx(size_t i, size_t j, size_t NUM_I, size_t NUM_J); */
void tmesh_alloc(tri_mesh_t *tmesh, size_t ni, size_t nj);
void tmesh_free(tri_mesh_t *tmesh);
void tmesh_render(tri_mesh_t *tmesh);
void tmesh_set_pt(tri_mesh_t *tmesh, size_t i, size_t j, double x, double y, double z);
void tmesh_set_color(tri_mesh_t *tmesh, size_t i, size_t j, double r, double g, double b);

#endif
