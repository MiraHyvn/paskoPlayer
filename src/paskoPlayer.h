#ifndef PLAYER_H
#define PLAYER_H

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
}
#include <string>

using namespace std;

class player{
public:
  player();

  bool openFile(string);

  bool init(string);
  void close();

  int getNextFrame();
  void rewind();
  void seek(float);

  float getDuration();
  string getFilename(){return filename;}

  /*Settings */
  bool seekToAny;
  bool showIncomplete;
  int frameDelay;
  bool noKeyFrames; //disallow showing key frames

  const unsigned char* getPixelData();
  int getPixelDataLength();
  int getLineWidth();
  int getWidth();
  int getHeight();

  bool frameChanged();

protected:

  string filename;
  bool isInit;

  AVFormatContext *pFormatCtx;
  int videoStream; //TODO get a better name!
  AVCodecContext *pCodecCtx;

  AVCodec *pCodec;
  AVFrame *pFrame;
  AVFrame* pFrameRGB;

  unsigned char* buffer;
  char* convertBuffer;
  int numBytes; //TODO: get a better name!

  int framesRead;
  long long int duration;

  struct SwsContext * pSwsCtx;

  AVPacket packet;

  bool changed;
};

#endif
