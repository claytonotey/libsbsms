#include <stdlib.h>
#include <string.h>
#include "audio.h"

namespace _sbsmsx_ {

audio *make_audio_buf(long n) {
  return (audio*)calloc(n,sizeof(audio));
}

void free_audio_buf(audio *buf) {
  free(buf);
}

long copy_audio_buf(audio *to, long off1, audio *from, long off2, long n)
{
  memcpy(to+off1,from+off2,n*sizeof(audio));
  return n;
}


long audio_convert_from(float *to, long off1, audio *from, long off2, long n, int channels)
{
  int n2 = n+off2;
  if(channels == 2) {
    int k1 = off1<<1;
    for(int k=off2;k<n2;k++) {
      to[k1++] = (float)from[k][0];
      to[k1++] = (float)from[k][1];
    }
  } else {
    int k1 = off1;
    for(int k=off2;k<n2;k++) {
      to[k1++] = (float)from[k][0];
    }
  }
  return n;
}

long audio_convert_to(audio *to, long off1, float *from, long off2, long n, int channels)
{
  int n1 = n + off1;
  if(channels == 2) {
    int k2 = off2<<1;
    for(int k=off1;k<n1;k++) {
      to[k][0] = (float)from[k2++];
      to[k][1] = (float)from[k2++];
    }
  } else {
    int k2 = off2;
    for(int k=off1;k<n1;k++) {
      to[k][0] = (float)from[k2];
      to[k][1] = (float)from[k2++];
    }
  }
  return n;
}

}
