extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
}

#include "paskoPlayer.h"
#include <stdio.h>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

player::player(){
  //initialize all members

  isInit = false;

  pFormatCtx = NULL;
  videoStream = -1;
  pCodecCtx = NULL;

  pCodec = NULL;
  pFrame = NULL;

  framesRead = 0;
  pSwsCtx = NULL;
  duration = 0;

  seekToAny = true;
  showIncomplete = false;

  noKeyFrames = false;

  buffer = NULL;
  convertBuffer = NULL;

  changed = false;
}

float player::getDuration(){ //in seconds
  return ((float)duration / AV_TIME_BASE);
}


bool player::openFile(string arg_s) {
  //argument is file name
  filename = arg_s;

  if(isInit){
    //close input & codec
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
  }

  // Open video file
  if(avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL) != 0 ) {
    printf("Couldn't open file\n");
    return false;
  }
  // Retrieve stream information to pFormatCtx->streams
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
    printf("Couldn't find stream information\n");
    return false;
  }
  // Dump information about file onto standard error
  //av_dump_format(pFormatCtx, 0, filename.c_str(), 0);

  duration = pFormatCtx->duration;

  //Find the first video stream
  videoStream = -1;
  for(int i=0; i< pFormatCtx->nb_streams && videoStream == -1; i++){
    if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
      videoStream = i;
  }
  if(videoStream == -1) {
    printf("File has no video streams\n");
    return false;
  }

  //Get a pointer to the codec context for the first video stream
  pCodecCtx = pFormatCtx->streams[videoStream]->codec;

  // Find the decoder for the video stream
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if (pCodec == NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return false;
  }

  // Open the codec
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
    printf("Could not open codec\n");
    return false;
  }

  // Determine required buffer size and allocate buffer.
  numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

  if(buffer != NULL) delete[] buffer;
  buffer = new uint8_t [numBytes * sizeof(uint8_t)];

  if(convertBuffer != NULL) delete[] convertBuffer;
  convertBuffer = new char[pCodecCtx->width * pCodecCtx->height *3];

  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

  //sws scaler context:
  pSwsCtx = sws_getContext( pCodecCtx->width,
			    pCodecCtx->height,
			    pCodecCtx->pix_fmt,
			    pCodecCtx->width,
			    pCodecCtx->height,
			    PIX_FMT_RGB24,
			    SWS_FAST_BILINEAR,
			    NULL,
			    NULL,
			    NULL);
  if (pSwsCtx == NULL) {
    fprintf(stderr, "Cannot initialize the sws context\n");
    return false;
  }

  return true;

}

bool player::init(string arg_s){ //returns true on success, false on error

  if(isInit) return true;

  //register all formats and codecs
  av_register_all();

  // Allocate video frame.
  pFrame = avcodec_alloc_frame();
  // Allocate an AVFrame structure
  pFrameRGB=avcodec_alloc_frame();
  if (pFrame == NULL || pFrameRGB == NULL) {
    printf("Couldn't allocate frame\n");
    return false;
  }

  //Open file
  if (!openFile(arg_s)){
    return false; //error output is inside the function
  }
  /*
  // Make display window, renderer and texture for pixel access
  display = SDL_CreateWindow("PaskoPlayer",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
			  1920, 1080,    //pCodecCtx->width, pCodecCtx->height,
                          0);
  renderer = SDL_CreateRenderer(display, -1, 0); */

/*  renderer = R; //TODO: error checks!

  texture = SDL_CreateTexture(renderer,
			      SDL_PIXELFORMAT_RGB24, // format should be like this: r, g, b, r, g, b, ...
			      SDL_TEXTUREACCESS_STREAMING,
			      pCodecCtx->width, pCodecCtx->height);*/

  //successfully initialized
  isInit = true;
  return true;
}

int player::getNextFrame(){ //returns <0 on error or at the end of file

  changed = false;
  int frameFinished = 0;
  int returnValue = av_read_frame(pFormatCtx, &packet); //TODO: better name for returnValue!

  // Read frame
  if (returnValue >= 0) {
    // Is this a packet from the video stream?
    if (packet.stream_index == videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

      if(!noKeyFrames || pFrame->key_frame == 0) {
	if(frameFinished || showIncomplete) {

	  // Convert the image from its native format to RGB
	  sws_scale(pSwsCtx, (const uint8_t * const *) pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

	  //put pixels into the SDL texture
	  //SDL_UpdateTexture(texture, NULL, pFrameRGB->data[0], pFrameRGB->linesize[0]);

   cout << "read frame" << framesRead << "\n";
   changed = true;
	  framesRead++; //TODO: do we really need this?

	}
      }
    }
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

  return returnValue;
}

const unsigned char* player::getPixelData(){
  return pFrameRGB->data[0];
}

int player::getPixelDataLength(){
  return pFrameRGB->linesize[0] * pCodecCtx->height;
}

int player::getLineWidth(){
  return pFrameRGB->linesize[0];
}

int player::getWidth(){
 return pCodecCtx->width;
}

int player::getHeight(){
 return pCodecCtx->height;
}


void player::rewind(){
  framesRead = 0;
  av_seek_frame(pFormatCtx, videoStream, 0, (seekToAny?AVSEEK_FLAG_ANY:0)|AVSEEK_FLAG_BACKWARD);
}

void player::seek(float pos){ //pos is in seconds
  if(av_seek_frame(pFormatCtx, -1, (pos * AV_TIME_BASE), seekToAny?AVSEEK_FLAG_ANY:0) <0)
    //seek failed
    ;
}

void player::close(){

  delete[] buffer;
  delete[] convertBuffer;
  av_free(pFrame);
  av_free(pFrameRGB);
  avcodec_close(pCodecCtx);
  avformat_close_input(&pFormatCtx);

}

bool player::frameChanged(){
 return changed;
}
