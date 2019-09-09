/****************************************************************************
    
    ringbuffer.c - a ringbuffer designed to be used in shared memory
    
    Copyright (C) 2005  Lars Luthman <larsl@users.sourceforge.net>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 01222-1307  USA

****************************************************************************/

/* #include <stdio.h> */
#include <string.h>

#include "ringbuffer.h"


void ringbuf_init(ringbuf_t* rb, int atom_size, int size) {
  rb->read_pos = 0;
  rb->write_pos = 0;
  rb->atom_size = atom_size;
  rb->max_pos = size;
}


int ringbuf_read(ringbuf_t* rb, void* dest, int size) {
  
  int n = 0;
  
  if (size == 0)
    return 0;
  
  /*fprintf(stderr, "Request to read %d atoms\n", size);*/
  
  if (rb->read_pos > rb->write_pos) {
    n = rb->max_pos - rb->read_pos;
    n = (n > size ? size : n);
    if (dest)
      memcpy(dest, rb->data + rb->read_pos * rb->atom_size, n * rb->atom_size);
    rb->read_pos = (rb->read_pos + n) % rb->max_pos;
  }
  if (rb->read_pos < rb->write_pos && n < size) {
    int m = rb->write_pos - rb->read_pos;
    m = (m > size - n ? size - n : m);
    if (dest)
      memcpy((char*)dest + n * rb->atom_size, 
	     rb->data + rb->read_pos * rb->atom_size, m * rb->atom_size);
    rb->read_pos = (rb->read_pos + m) % rb->max_pos;
    n += m;
  }
  
  /*  fprintf(stderr, "RB %d atoms read from the ringbuffer\n"
      "\tread_pos = %d write_pos = %d max_pos = %d\n",
      n, rb->read_pos, rb->write_pos, rb->max_pos); */

  /*if (n < size)
    fprintf(stderr, "RB: read collision %d\n", col++);*/

  return n;
}


int ringbuf_write(ringbuf_t* rb, void* src, int size) {
  int n = 0;

  if (size == 0)
    return 0;

  if (rb->write_pos >= rb->read_pos) {
    n = rb->max_pos - rb->write_pos;
    if (rb->read_pos == 0)
      --n;
    n = (n > size ? size : n);
    memcpy(rb->data + rb->write_pos * rb->atom_size, src, n * rb->atom_size);
    rb->write_pos = (rb->write_pos + n) % rb->max_pos;
  }
  if (rb->write_pos + 1 < rb->read_pos && n < size) {
    int m = rb->read_pos - rb->write_pos - 1;
    m = (m > size - n ? size - n : m);
    memcpy(rb->data + rb->write_pos * rb->atom_size, 
	   (char*)src + n * rb->atom_size, m * rb->atom_size);
    rb->write_pos = (rb->write_pos + m) % rb->max_pos;
    n += m;
  }

  /*fprintf(stderr, "RB %d atoms written to the ringbuffer\n"
    "\tread_pos = %d write_pos = %d max_pos = %d\n",
    n, rb->read_pos, rb->write_pos, rb->max_pos);*/
  
  /* if (n < size)
     fprintf(stderr, "RB: write collision %d\n", col++); */
  
  return n;
}


int ringbuf_write_zeros(ringbuf_t* rb, int size) {
  int n = 0;

  if (size == 0)
    return 0;

  if (rb->write_pos >= rb->read_pos) {
    n = rb->max_pos - rb->write_pos;
    if (rb->read_pos == 0)
      --n;
    n = (n > size ? size : n);
    memset(rb->data + rb->write_pos * rb->atom_size, 0, n * rb->atom_size);
    rb->write_pos = (rb->write_pos + n) % rb->max_pos;
  }
  if (rb->write_pos + 1 < rb->read_pos && n < size) {
    int m = rb->read_pos - rb->write_pos - 1;
    m = (m > size - n ? size - n : m);
    memset(rb->data + rb->write_pos * rb->atom_size, 0, m * rb->atom_size);
    rb->write_pos = (rb->write_pos + m) % rb->max_pos;
    n += m;
  }

  /*fprintf(stderr, "RB %d atoms written to the ringbuffer\n"
    "\tread_pos = %d write_pos = %d max_pos = %d\n",
    n, rb->read_pos, rb->write_pos, rb->max_pos);*/
  
  /* if (n < size)
     fprintf(stderr, "RB: write collision %d\n", col++); */
  
  return n;
}


int ringbuf_available(ringbuf_t* rb) {
  if (rb->read_pos <= rb->write_pos)
    return rb->write_pos - rb->read_pos;
  else
    return rb->max_pos - rb->read_pos + rb->write_pos;
}


int ringbuf_available_contiguous(ringbuf_t* rb) {
  if (rb->read_pos <= rb->write_pos)
    return rb->write_pos - rb->read_pos;
  else
    return rb->max_pos - rb->read_pos;
}


int ringbuf_get_size(int atom_size, int size) {
  return sizeof(ringbuf_t) - 1 + size * atom_size;
}


int ringbuf_get_read_pos(ringbuf_t* rb) {
  return rb->read_pos;
}


int ringbuf_get_write_pos(ringbuf_t* rb) {
  return rb->write_pos;
}


char* ringbuf_get_read_ptr(ringbuf_t* rb) {
  return rb->data + rb->read_pos * rb->atom_size;
}
