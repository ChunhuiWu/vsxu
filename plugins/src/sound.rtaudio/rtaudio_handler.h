/**
* Project: VSXu: Realtime modular visual programming engine.
*
* This file is part of Vovoid VSXu.
*
* @author Jonatan Wallmander, Robert Wenzel, Vovoid Media Technologies AB Copyright (C) 2003-2013
* @see The GNU Lesser General Public License (LGPL)
*
* VSXu Engine is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU Lesser General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#if (PLATFORM == PLATFORM_LINUX)
  #include <sys/prctl.h>
#endif

float fftbuf[1024];
size_t fftbuf_it = 0;

// rt audio instance
RtAudio* padc = 0x0;

// reference counter
size_t rt_refcounter = 0;

FFTReal* fftr = 0x0;

const float one_div_32768 = 1.0f / 32768.0f;
const float one_div_256 = 1.0f / 256.0f;
const float one_div_512 = 1.0f / 512.0f;



int record( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  if ( status )
  {
    printf("Stream overflow detected!\n");
  }

  #if (PLATFORM == PLATFORM_LINUX)
    const char* cal = "sound.rtaudio";
    prctl(PR_SET_NAME,cal);
  #endif

  int16_t* buf = (int16_t*)inputBuffer;

  vsx_paudio_struct* pa_d = &pa_audio_data;

  int j = 0;
  // nab left channel for spectrum data
  for (size_t i = 0; i < 512; i++)
  {
    const float &f = (float)buf[j] * one_div_32768;

    (*(pa_d->wave[0].data))[i] = f * pa_d->l_mul;
    fftbuf[fftbuf_it++] = f;
    j++;
    j++;
  }
  fftbuf_it = fftbuf_it % 1024;

  j = 1;

  for (size_t i = 0; i < 512; i++)
  {
    (*(pa_d->wave[1].data))[i] = (float)buf[j] * one_div_32768 * pa_d->l_mul;
    j++;
    j++;
  }
  // do some FFT's
  float spectrum[1024];
  float spectrum_dest[512];
  fftr->do_fft( (float*)&spectrum, (float*) &fftbuf[0]);
  float re, im;

  for(int ii = 0; ii < 256; ii++)
  {
    re = spectrum[ii];
    im = spectrum[ii + 256];
    spectrum_dest[ii] = (float)sqrt(re * re + im * im) * one_div_256 * pa_d->l_mul;
  }

  // calc vu
  float vu = 0.0f;
  for (int ii = 0; ii < 256; ii++)
  {
    vu += spectrum_dest[ii];
  }
  pa_d->vu[0] = vu;
  pa_d->vu[1] = vu;

  for (size_t ii = 0; ii < 512; ii++)
  {
    (*(pa_d->spectrum[0].data))[ii] =
        spectrum_dest[ii >> 1]
                            *
                            3.0f
                            *
                            pow(
                              log(
                                10.0f
                                +
                                44100.0f * ( (float)ii * one_div_512 )
                                ),
                              1.0f
                              );
  }



  //normalize_fft( (float*)spectrum_dest, pa_d->spectrum[0]);

  /*for (size_t ii = 0; ii < 512; ii++)
  {
    //float f = log( 2.0f + 8.0f * (ii / 512.0f) );
    (*(pa_d->spectrum[0].data))[ii] *= pow(log( 2.0f + 8.0f * (ii / 512.0f)) ,3.0f);
  }*/


  const float one_div_50 = 1.0f / 50.0f;


  #define spec_calc(cur_val, start, offset) \
    cur_val = 0.0f;\
    for (int ii = start * 50 + offset; ii < (start+1)*50; ii++) {\
      cur_val += (*(pa_d->spectrum[0].data))[ii];\
    }\
    cur_val = (cur_val * one_div_50)

  spec_calc(pa_d->octaves[0][0], 0, 10);
  spec_calc(pa_d->octaves[0][1], 1, 0);
  spec_calc(pa_d->octaves[0][2], 2, 0);
  spec_calc(pa_d->octaves[0][3], 3, 0);
  spec_calc(pa_d->octaves[0][4], 4, 0);
  spec_calc(pa_d->octaves[0][5], 5, 0);
  spec_calc(pa_d->octaves[0][6], 6, 0);
  spec_calc(pa_d->octaves[0][7], 7, 0);

  #undef spec_calc
}


const double timeslot = 1.0f / 44100 * 256.0;

double st = 0.0;


vsx_sample_mixer main_mixer;

// Two-channel sawtooth wave generator.
int play_callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  int16_t *buffer = (int16_t *) outputBuffer;
  int16_t *lastValues = (int16_t *) userData;

  if ( status )
    printf("Stream underflow detected!\n");
  // Write interleaved audio data.
  for ( i=0; i<nBufferFrames; i++ )
  {
    //for ( j=0; j<2; j++ )
    {
      int16_t val = (int16_t)main_mixer.consume();
      *buffer = val;
      *buffer++;
      *buffer = val;
      *buffer++;
    }
    st += timeslot;
  }
  return 0;
}

void setup_rtaudio_play()
{
  if (padc)
  {
    rt_refcounter++;
    return;
  }
  else
  {
    padc = new RtAudio((RtAudio::Api)rtaudio_type);
    rt_refcounter++;
    #if (PLATFORM == PLATFORM_WINDOWS)
    rt_refcounter++;
    #endif
  }

  if ( padc->getDeviceCount() < 1 )
  {
    printf("WARNING::::::::      No audio devices found!\n");
    return;
  }


  RtAudio::StreamParameters parameters;
  parameters.deviceId = padc->getDefaultInputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 128;
  double data[2];

  RtAudio::StreamOptions options;
      options.streamName = "vsxu";
  try
  {
    padc->openStream( &parameters, NULL, RTAUDIO_SINT16,
                      sampleRate, &bufferFrames, &play_callback, (void *)&data );
    padc->startStream();
  }
  catch ( RtError& e )
  {
    e.printMessage();
  }


}


void setup_rtaudio_record()
{
  if (padc)
  {
    rt_refcounter++;
    return;
  }
  else
  {
    padc = new RtAudio((RtAudio::Api)rtaudio_type);
    fftr = new FFTReal(512);
    rt_refcounter++;
    #if (PLATFORM == PLATFORM_WINDOWS)
    rt_refcounter++;
    #endif
  }

  if ( padc->getDeviceCount() < 1 )
  {
    printf("WARNING::::::::      No audio devices found!\n");
    return;
  }

  vsx_paudio_struct* pa_d = &pa_audio_data;

  pa_d->wave[0].data = new vsx_array<float>;
  pa_d->wave[1].data = new vsx_array<float>;
  for (int i = 0; i < 512; ++i) pa_d->wave[0].data->push_back(0);
  for (int i = 0; i < 512; ++i) pa_d->wave[1].data->push_back(0);

  pa_d->spectrum[0].data = new vsx_array<float>;
  pa_d->spectrum[1].data = new vsx_array<float>;
  for (int i = 0; i < 512; ++i) pa_d->spectrum[0].data->push_back(0);
  for (int i = 0; i < 512; ++i) pa_d->spectrum[1].data->push_back(0);


  RtAudio::StreamParameters parameters;
  parameters.deviceId = padc->getDefaultInputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = 512;

  RtAudio::StreamOptions options;
      options.streamName = "vsxu";

  try {
    padc->openStream(
      NULL,
      &parameters,
      RTAUDIO_SINT16,
      sampleRate,
      &bufferFrames,
      &record,
      NULL,
      &options
    );
    padc->startStream();
  }
  catch ( RtError& e ) {
    e.printMessage();
  }
}

void shutdown_rtaudio_record()
{
  if (!padc) return;
  if (rt_refcounter == 0) return;
  rt_refcounter--;

  if (rt_refcounter == 0)
  {
    try {
      // Stop the stream
      padc->stopStream();
    }
    catch (RtError& e) {
      e.printMessage();
    }

    if ( padc->isStreamOpen() ) padc->closeStream();
    delete padc;
    delete fftr;
    padc = 0;
  }
}
