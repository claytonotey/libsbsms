// -*- mode: c++ -*-
#ifndef SBSMS_INCLUDE
#define SBSMS_INCLUDE

#include <stdio.h>

namespace _sbsms_ {

typedef float t_fft[2];
typedef t_fft audio;
typedef long long int SampleCountType;
typedef long long int TimeType;
typedef unsigned char TrackIndexType;

enum {
  maxBands = 10,
  numQualityParams = 52
};

static const SBSMSQualityParams SBSMSQualityStandard = {
  8,3,
  {512,512,384,384,384,384,384,384,0,0},
  {168,144,128,96,64,36,24,14,0,0},
  {384,288,256,168,128,84,52,28,0,0},
  {512,448,360,288,192,128,84,44,0,0},
  {1,1,2,1,1,2,1,1,0,0}
};

struct SBSMSQualityParams {
  int bands;
  int H;
  int N[maxBands];
  int N0[maxBands];
  int N1[maxBands];
  int N2[maxBands];
  int res[maxBands];
};

class SBSMSQuality {
 public:
  SBSMSQuality(const SBSMSQualityParams *params);
  SBSMSQualityParams params;
  long getFrameSize();
  long getMaxPresamples();
};

static const SBSMSQualityParams SBSMSQualityStandard = {
  8,3,
  {512,512,384,384,384,384,384,384,0,0},
  {168,144,128,96,64,36,24,14,0,0},
  {384,288,256,168,128,84,52,28,0,0},
  {512,448,360,288,192,128,84,44,0,0},
  {1,1,2,1,1,2,1,1,0,0}
};

struct SBSMSFrame {
  float ratio0;
  float ratio1;
  audio *buf;
  long size;
};

typedef long (*SBSMSResampleCB)(void *cbData, SBSMSFrame *frame);

class SBSMSInterface /* not final */ {
 public:
  virtual ~SBSMSInterface() {}
  virtual long samples(audio *buf, long n) { return 0; }
  virtual float getStretch(float t)=0;
  virtual float getMeanStretch(float t0, float t1)=0;
  virtual float getPitch(float t)=0;
  virtual long getPresamples()=0;
  virtual SampleCountType getSamplesToInput()=0;
  virtual SampleCountType getSamplesToOutput()=0;
};

class SBSMSTrackPoint {
 public:
  virtual ~SBSMSTrackPoint() {}
  virtual float getF()=0;
  virtual float getM()=0;
  virtual float getPhase()=0;
};

class SBSMSTrack {
 public:
  virtual ~SBSMSTrack() {}
  virtual SBSMSTrackPoint *getSBSMSTrackPoint(const TimeType &time)=0;
  virtual TrackIndexType getIndex()=0;
  virtual bool isFirst(const TimeType &synthtime)=0;
  virtual bool isLast(const TimeType &synthtime)=0;
};

class SBSMSRenderer {
 public:
  virtual ~SBSMSRenderer() {}
  virtual void startFrame() {}
  virtual void startTime(int c, const TimeType &time, int n) {}
  virtual void render(int c, SBSMSTrack *t) {}
  virtual void endTime(int c) {}
  virtual void endFrame() {}
  virtual void end(const SampleCountType &samples) {}
};

enum SBSMSError {
  SBSMSErrorNone = 0,
  SBSMSErrorInvalidRate
};

class SBSMSImp;

class SBSMS {
 public:
  SBSMS(int channels, SBSMSQuality *quality, bool bSynthesize);
  ~SBSMS();

  long read(SBSMSInterface *iface, audio *buf, long n);
  void addRenderer(SBSMSRenderer *renderer);
  void removeRenderer(SBSMSRenderer *renderer);
  long renderFrame(SBSMSInterface *iface);
  long getInputFrameSize();
  SBSMSError getError();
  friend class SBSMSImp;
 protected:
  SBSMSImp *imp;
};

enum SlideType {
  SlideIdentity = 0,
  SlideConstant,
  SlideLinearInputRate,
  SlideLinearOutputRate,
  SlideLinearInputStretch,
  SlideLinearOutputStretch,
  SlideGeometricInput,
  SlideGeometricOutput
};

class SlideImp;

class Slide {
 public:
  Slide(SlideType slideType, float rate0 = 1.0f, float rate1 = 1.0f, const SampleCountType &n = 0);
  ~Slide();
  float getTotalStretch();
  float getStretchedTime(float t);
  float getInverseStretchedTime(float t);
  float getRate(float t);
  float getStretch(float t);
  float getMeanStretch(float t0, float t1);
  float getRate();
  float getStretch();
  void step();
 protected:
  SlideImp *imp;
};
 
class SBSMSInterfaceSlidingImp;

class SBSMSInterfaceSliding /* not final */ : public SBSMSInterface {
public:
  SBSMSInterfaceSliding(Slide *rateSlide, 
                        Slide *pitchSlide, 
                        bool bPitchReferenceInput, 
                        const SampleCountType &samplesToInput, 
                        long preSamples,
                        SBSMSQuality *quality);
  virtual ~SBSMSInterfaceSliding();
  virtual float getStretch(float t);
  virtual float getMeanStretch(float t0, float t1);
  virtual float getPitch(float t);
  virtual long getPresamples();
  virtual SampleCountType getSamplesToInput();
  virtual SampleCountType getSamplesToOutput();

  friend class SBSMSInterfaceSlidingImp;
protected:
  SBSMSInterfaceSlidingImp *imp;
};

class ResamplerImp;

class Resampler {
 public:
  Resampler(SBSMSResampleCB func, void *data, SlideType slideType = SlideConstant);
  ~Resampler();
  long read(audio *audioOut, long frames);
  void reset();
  long samplesInOutput();

 protected:
  ResamplerImp *imp;
};

}

#endif
