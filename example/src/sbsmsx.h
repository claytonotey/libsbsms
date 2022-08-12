// -*- mode: c++ -*-
#ifndef SBSMSX_INCLUDE
#define SBSMSX_INCLUDE

#include <stdio.h>
#include <sbsms.h>
#include <vector>

using namespace _sbsms_;

namespace _sbsmsx_ {

#ifdef HAVE_STDINT_H
#include <stdint.h>
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#else
typedef short int16;
typedef int int32;
typedef long int64;
typedef unsigned short int16;
typedef unsigned int int32;
typedef unsigned long int64;
#endif

class Track;

enum {
  libVersion = 0
};

#define MINSTRETCHMOD 0.0f
#define MAXSTRETCHMOD 4.0f

enum {
  renderLatency = 1,
  numQualityParamsX = 30
};

enum {
  SBSMS_OFFSET_VERSION = 0,
  SBSMS_OFFSET_SAMPLES = 4,
  SBSMS_OFFSET_FRAMES = 12,
  SBSMS_OFFSET_CHANNELS = 16,
  SBSMS_OFFSET_QUALITY = 18,
  SBSMS_OFFSET_NUMTRACKPOINTS = 122,
  SBSMS_OFFSET_NUMTRACKPOINTMODIFIERS = 202,
  SBSMS_OFFSET_DATA = 282
};

long copy_audio_buf(audio *, long off1, audio *, long off2, long n);
long audio_convert_from(float *to, long off1, audio *from, long off2, long n, int channels = 2);
long audio_convert_to(audio *to, long off1, float *from, long off2, long n, int channels = 2);

class SBSMSQualityXParams {
public:
  uint16 minF[maxBands];
  uint16 maxF[maxBands];
  uint16 minM[maxBands];
  uint16 maxM[maxBands];
};

class SBSMSQualityX : public SBSMSQuality {
public:
  SBSMSQualityXParams paramsx;
};

enum SBSMSXError {
  SBSMSXErrorNone = 0,
  SBSMSXErrorFileOpen
};

class FileRenderer : public SBSMSRenderer {
public:
  FileRenderer(const char *fileName, int channels, SBSMSQualityX *quality);
  ~FileRenderer();
  void startFrame(float stretchMod);
  void startTime(int c, const SampleCountType &samplePos, const TimeType &time, int n, float f0, float f1);
  void render(int c, Track *t);
  void endTime(int c);
  void endFrame();
  void end(const SampleCountType &samples);
  
  SBSMSXError getError();

protected:
  SBSMSXError error;
  int channels;
  int nTracks[2];
  long nTrackPoints[2][maxBands];
  long nTrackPointModifiers[2][maxBands];
  FILE *fp;
  int frameBytes;
  std::vector<int> frameBytesV;
  fpos_t start[2];
  TimeType time;
};

class SBSMSynthesizer {
 public:
  virtual ~SBSMSynthesizer() {}
  virtual void synthInit(int c, int n, float srcPitch) {}
  virtual bool synth(int c, int index, float *out, float h2, float offset, int n, bool bStart, bool tailStart, bool bEnd, bool tailEnd, float fScale, long w0, long w1, long m0, long m1, long *ph, long *w, long *m) { return false; }
  virtual void synthStep(int c, int n) {}
  virtual bool isAddNodeRequired() { return false; }
  virtual void addNodeStart(int c) {}
  virtual void addNode(int c, float f, float m) {}
  virtual void addNodeComplete(int c) {}
};

class SBSMSXImp;

class SBSMSX {
 public:
  SBSMSX(const char *filename, SampleCountType *samplesToInput, bool bSynthesize);
  SBSMSX(SBSMSX *source);
  ~SBSMSX();

  long synth(float stretch, float pitch, float srcPitch, audio *buf, long n, SBSMSynthesizer *synthMod);
  void addRenderer(SBSMSRenderer *renderer);
  void removeRenderer(SBSMSRenderer *renderer);
  long renderFrame(SBSMSInterface *iface);
  void reset();
  void seek(const SampleCountType &samplePos);
  void setLeftPos(const SampleCountType &pos);
  void setRightPos(const SampleCountType &pos);
  SampleCountType getSamplePos();
  long getInputFrameSize();
  int getMaxTracks();
  SBSMSXError getError();
  
  bool isReadOpen();
  void closeRead();
  bool isWriteOpen();
  bool openWrite(const char *filename);
  void closeWrite();

  unsigned int getVersion();
  int getChannels();
  SBSMSQuality *getQuality();

  friend class SBSMSXImp;
 protected:
  SBSMSXImp *imp;
};

}

#endif
