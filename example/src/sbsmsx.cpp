#include "sbsmsx.h"

namespace _sbsmsx_ {

const SBSMSQualityParams SBSMSQualityStandard = {
  8,4,
  {384,384,512,480,384,288,256,256,0,0},
  {128,96,72,60,48,36,28,16,0,0},
  {192,168,144,120,96,72,56,32,0,0},
  {256,256,192,160,128,96,72,48,0,0},
  {1,2,1,1,2,1,2,1,0,0}
};

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

class SBSMSXImp {
public:
  friend class SBSMSX;

  SBSMSXImp(const char *filename, countType *samplesToInput);
  SBSMSXImp(SBSMSXImp *source);
  ~SBSMSXImp();

  inline long synth(float stretch, float pitch, float srcPitch, audio *buf, long n, SBSMSynthesizer *synthMod);
  inline void addRenderer(SBSMSRenderer *renderer);
  inline void removeRenderer(SBSMSRenderer *renderer);
  inline long renderFrame();
  inline void reset();
  inline void readFooter(long frames);
  inline void seek(const SampleCountType &samplePos);  
  inline bool openWrite(const char *filename);
  inline void closeWrite();
  inline void closeRead();
  inline bool isWriteOpen();
  inline bool isReadOpen();
  inline SampleCountType getSamplePos();

  SubBand *top;  

protected:
  SBSMSXImp *source;
  FILE *fpIn;  
  SBSMSXError error;
  vector<off_t> *frameByteOffset;
  vector<SampleCountType> *frameSampleOffset;
  uint32 version;
  int channels;
  SBSMSQuality *quality;
};

SBSMSX :: SBSMSX(const char *filename, SampleCountType *samplesToInput)
{ imp = new SBSMSXImp(filename, samplesToInput); }
SBSMSImp :: SBSMSXImp(const char *filename, SampleCountType *samplesToInput, bool bSynthesize)
{
  FOPEN(fpIn,filename,"rb");
  if(!fpIn) {
    error = SBSMSXErrorFileOpen;
    return;
  }
  version = fread_32_little_endian(fpIn);
  if(samplesToInput) {
    *samplesToInput = fread_64_little_endian(fpIn);
  } else {
    fread_64_little_endian(fpIn);
  }
  long framesToInput = fread_32_little_endian(fpIn);
  channels = fread_16_little_endian(fpIn);
  SBSMSQualityParams qualityParams;
  int *p = (int*)&qualityParams;
  for(int i=0; i<numQualityParams; i++) {
    p[i] = fread_16_little_endian(fpIn);
  }
  quality = new SBSMSQuality(&qualityParams);
  error = SBSMSErrorNone;
  FSEEK(fpIn,SBSMS_OFFSET_NUMTRACKPOINTS,SEEK_SET);
  long nTrackPoints[2*maxBands];
  for(int i=0; i<2*maxBands; i++) {
    nTrackPoints[i] = fread_32_little_endian(fpIn);
  }
  FSEEK(fpIn,SBSMS_OFFSET_NUMTRACKPOINTMODIFIERS,SEEK_SET);
  long nTrackPointModifiers[2*maxBands];
  for(int i=0; i<2*maxBands; i++) {
    nTrackPointModifiers[i] = fread_32_little_endian(fpIn);
  }
  top = new SubBand(NULL,0,channels,quality,bSynthesize,NULL,nTrackPoints,nTrackPointModifiers);
  top->setFramesInFile(framesToInput);  
  this->frameByteOffset = new vector<off_t>;
  this->frameSampleOffset = new vector<SampleCountType>;
  readFooter(framesToInput);
  FSEEK(fpIn,SBSMS_OFFSET_DATA,SEEK_SET);
  reset();
}

SBSMSX :: SBSMSX(SBSMSX *source) { imp = new SBSMSXImp(source->imp); }
SBSMSXImp :: SBSMSXImp(SBSMSXImp *source)
{
  this->source = source;
  frameByteOffset = source->frameByteOffset;
  frameSampleOffset = source->frameSampleOffset;
  fpIn = NULL;
  version = source->version;
  this->channels = source->channels;
  this->quality = new SBSMSQuality(&source->quality->params);
  error = SBSMSXErrorNone;
  top = new SubBand(NULL,0,channels,quality,true,source->top,NULL,NULL);  
  top->setFramesInFile(source->top->nFramesInFile);
  reset();
}

SBSMSX :: ~SBSMSX() { delete imp; }
SBSMSXImp :: ~SBSMSXImp()
{
  if(top) delete top;
  if(!source && frameByteOffset) delete frameByteOffset;
  if(!source && frameSampleOffset) delete frameSampleOffset;
  if(fpIn) fclose(fpIn);
  if(quality) delete quality;
}

void SBSMSX :: addRenderer(SBSMSRenderer *renderer) { imp->addRenderer(renderer->imp); }
void SBSMSXImp :: addRenderer(SMSRenderer *renderer)
{
  top->addRenderer(renderer);
}

void SBSMSX :: removeRenderer(SBSMSRenderer *renderer) { imp->removeRenderer(renderer->imp); }
void SBSMSXImp :: removeRenderer(SMSRenderer *renderer)
{
  top->removeRenderer(renderer);
}

SBSMSXError SBSMSX :: getError()
{
  return imp->error;
}

long SBSMSX :: synth(float stretch, float pitch, float srcPitch, audio *buf, long n, SBSMSynthesizer *synthMod) { return imp->synthFromMemory(stretch,pitch,srcPitch,buf,n,synthMod); }
long SBSMSXImp :: synth(float stretch, float pitch, float srcPitch, audio *buf, long n, SBSMSynthesizer *synthMod)
{
  long nReadTotal = 0;
  long nRead = -1;
  while(nRead && nReadTotal < n) {
    if(top->nToDrop) {
      audio nullBuf[512];
      nRead = min(512L,top->nToDrop);
      nRead = top->synth(nullBuf,nRead,stretch,pitch,srcPitch,synthMod);
      top->nToDrop -= nRead;
    } else {
      nRead = n - nReadTotal;
      nRead = top->synth(buf+nReadTotal,nRead,stretch,pitch,srcPitch,synthMod);
      nReadTotal += nRead;
    }
  }
  return stretch<0.0f?-nReadTotal:nReadTotal;
}

long SBSMSX :: renderFrame() { return imp->renderFrame(); }
long SBSMSXImp :: renderFrame()
{
  long nRendered = 0;
  while(!nRendered && !top->isRenderDone()) {
    nRendered = top->render();
    if(!nRendered) {
      long nWrite;
      nWrite = top->writeFromMemory();
      if(!nWrite) {
        if(fpIn) {
          nWrite = top->writeFromFile(fpIn);
        } 
      }
      if(!nWrite) {
        top->writingComplete();
      }
    }
  }
  return nRendered;
}

void SBSMSX :: closeRead() { imp->closeRead(); }
void SBSMSXImp :: closeRead()
{
  if(fpIn) {
    fclose(fpIn);
    fpIn = NULL;
  }
}

bool SBSMSX :: isReadOpen() { return imp->isReadOpen(); }
bool SBSMSXImp :: isReadOpen()
{
  return (fpIn != NULL);
}

void SBSMSXImp :: readFooter(long frames)
{
  off_t footerSize = frames*8;
  FSEEK(fpIn,-footerSize,SEEK_END);
  off_t samplesAcc = 0;
  int frameSize = top->getInputFrameSize();
  for(int k=0;k<frames;k++) {
    off_t offset = (off_t)fread_64_little_endian(fpIn);
    frameByteOffset->push_back(offset);
    frameSampleOffset->push_back(samplesAcc);
    samplesAcc += frameSize;
  }
}

void SBSMSX :: setLeftPos(const SampleCountType &pos) { imp->top->setLeftPos(pos); }

void SBSMSX :: setRightPos(const SampleCountType &pos) { imp->top->setRightPos(pos); }

void SBSMSX :: seek(const SampleCountType &samplePos) { imp->seek(samplePos); }
void SBSMSXImp :: seek(const SampleCountType &samplePos)
{
  long i = (long) (samplePos / top->getInputFrameSize());
  if(fpIn) FSEEK(fpIn,frameByteOffset->at(i),SEEK_SET);  
  top->seek(i,samplePos);
}

unsigned int SBSMSX :: getVersion()
{
  return imp->version;
}

int SBSMSX :: getChannels()
{
  return imp->channels;
}

SBSMSXQuality *SBSMSX :: getQuality()
{
  return imp->quality;
}

long SBSMSX :: getInputFrameSize()
{
  return imp->top->getInputFrameSize();
}

SampleCountType SBSMSX :: getSamplePos() { 
  return imp->top->getSamplePos(); 
}

}
