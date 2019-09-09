/****************************************************************************
    
    dssi_shm.h - functions that can handle shared memory segments for
                 DSSI plugins and UIs
    
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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "dssi_shm.h"


void* dssi_shm_allocate(size_t bytes, char** keystring, char** used_flag) {
  int shm_id, i;
  /*key_t key;*/
  void* shared_buffer;
  
  /* create and attach the memory segment */
  shm_id = shmget(IPC_PRIVATE, bytes + 9, IPC_CREAT | IPC_EXCL | S_IRWXU | S_IRWXG);
  if (shm_id == -1) {
    perror("Could not create shared memory segment");
    return NULL;
  }
  shared_buffer = shmat(shm_id, NULL, 0);
  if (!shared_buffer) {
    perror("Could not attach to shared memory segment");
    shmctl(shm_id, IPC_RMID, NULL);
    return NULL;
  }
  
  /* add the unique string */
  *keystring = calloc(100, sizeof(char));
  srand(time(NULL) + getpid() * 1000000);
  for (i = 0; i < 8; ++i)
    sprintf((char*)shared_buffer + bytes + i, "%X", rand() % 16);
  
  /* give the caller a key to send to the plugin */
  sprintf(*keystring, "%X:%s:%" PRIxPTR, shm_id, (char*)shared_buffer + bytes, (uintptr_t)shared_buffer);

  /* set the USED flag to 0 (it should be 0 already, but there's no harm in
     setting it explicitly) */
  ((char*)shared_buffer)[bytes + 8] = 0;
  *used_flag = ((char*)shared_buffer) + bytes + 8;
  
  return shared_buffer;
}


void* dssi_shm_attach(const char* key, void* old_ptr) {
  unsigned int shm_id, bytes;
  int keystart;
  void* ptr;
  struct shmid_ds shminfo;
  int nn;
  if ((nn = sscanf(key, "%X:%n%*X:%X", &shm_id, &keystart, &bytes)) < 2) {
    fprintf(stderr, "Invalid keystring, can not attach "
	    "shared memory segment: '%s' (sscanf returns %d)\n", key, nn);
    return NULL;
  }
  
  /* first, check if this is the same segment as the one we have already */
  if (old_ptr) {
    if (!strncmp(key + keystart, (char*)old_ptr + bytes, 8)) {
      fprintf(stderr, "Trying to attach a memory segment that we already "
	      "have attached\n");
      return old_ptr;
    }
    /* set the USED flag to 0 and detach the segment 
       (will attach it again immediately) */
    ((char*)old_ptr)[bytes + 8] = 0;
    shmdt(old_ptr);
  }

  /* check the segment size */
  /* this does not seem to work, and it's pretty unlikely that
     a memory segment gets the same ID as an old one, so let's
     just assume that the size is correct */
  /* it seems to work now - strange */
  if (shmctl(shm_id, IPC_STAT, &shminfo) == -1) {
    perror("Could not stat the shared memory segment");
    return NULL;
  }
  if (shminfo.shm_segsz < bytes + 9) {
    fprintf(stderr, "The segment is too small: %zu < %d\n",
	    shminfo.shm_segsz, bytes + 9);
    return NULL;
  }
  
  /* try to attach the segment */
  if (!(ptr = shmat(shm_id, NULL, 0)))
    return NULL;
  
  /* check the keystring */
  if (strncmp(key + keystart, (char*)ptr + bytes, 8)) {
    shmdt(ptr);
    fprintf(stderr, "The keystrings do not match, detaching the "
	    "shared memory segment\n");
    return NULL;
  }
  
  /* check that this segment isn't already in use 
     this is potentially dangerous since someone could come along
     and set the use flag to 1 in another thread right after our check */
  if (((char*)ptr)[bytes + 8] == 0)
    ((char*)ptr)[bytes + 8] = 1;
  else {
    shmdt(ptr);
    fprintf(stderr, "The shared memory segment is already in use!\n");
    return NULL;
  }
  
  return ptr;
}


int dssi_shm_free(const char* key) {
  unsigned int shm_id;
  uintptr_t ptr_int;
  if (sscanf(key, "%X:%*X:%" SCNxPTR, &shm_id, &ptr_int) != 2)
    return -1;
  shmdt((void*)ptr_int);
  return shmctl(shm_id, IPC_RMID, NULL);
}


int dssi_shm_detach(void* ptr) {
  return shmdt(ptr);
}
