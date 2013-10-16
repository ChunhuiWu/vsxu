#define VSX_SAMPLE_STATE_STOPPED 0
#define VSX_SAMPLE_STATE_PLAYING 1

#define VSX_SAMPLE_MONO 1
#define VSX_SAMPLE_STEREO 2

#define ONE_DIV_32767 1.0/32767.0

// read-ahead samples, should be the same as buffer size
#define DRIFT 64.0

// no file operations in vorbis loader
#define STB_VORBIS_NO_STDIO 1
#include "ogg_vorbis.c"

#define PB_LENGTH 16384.0f

class vsx_sample
{
  double position;
protected:
  vsx_array<int16_t> data;
  double pitch_bend;
  double gain;
  int16_t prev_left_value;
  int state;
  int stereo_type;
  int rate;
  double play_bit;

public:

  vsx_sample()
  :
    position(0.0f),
    pitch_bend(0.0f),
    gain(1.0f),
    prev_left_value(0),
    state(VSX_SAMPLE_STATE_STOPPED),
    stereo_type(VSX_SAMPLE_MONO),
    rate(44100),
    play_bit(-1.0f)
  {
  }

  vsx_array<int16_t>* get_data()
  {
    return &data;
  }

  inline float get_position()
  {
    return position;
  }

  inline void set_position(float n)
  {
    position = n;
  }

  inline float get_pitch_bend()
  {
    return pitch_bend;
  }

  inline void set_pitch_bend(float n)
  {
    pitch_bend = n;
  }

  inline float get_gain()
  {
    return gain;
  }

  inline void set_gain(float n)
  {
    gain = n;
  }

  inline int get_state()
  {
    if (!state && play_bit > -1.0f)
      return 1;
    return state;
  }

  inline void play()
  {
    pitch_bend = 1.0f;
    state = VSX_SAMPLE_STATE_PLAYING;
  }

  inline void stop()
  {
    state = VSX_SAMPLE_STATE_STOPPED;
  }

  inline void rewind()
  {
    position = 0;
    state = VSX_SAMPLE_STATE_STOPPED;
  }

  inline void trigger()
  {
    position = 0;
    state = VSX_SAMPLE_STATE_PLAYING;
  }

  inline void goto_time(float t)
  {
    //vsx_printf("t: %f, rate: %f, stereo_type: %f\n", t, (float)rate, (float)stereo_type);
    position = t * (float)rate * (float)stereo_type;

    //vsx_printf("playbit in goto time: %f\n", play_bit);
    if (play_bit < 0.0 && state == VSX_SAMPLE_STATE_STOPPED)
    {
      play_bit = PB_LENGTH;
    }

  }

  inline float get_time()
  {
    return position / ((float)rate * (float)stereo_type);
  }


  inline void set_stereo_type(int n)
  {
    stereo_type = n;
  }

  virtual inline int16_t consume_left()
  {
    if (!data.size())
      return 0;

    if (state == VSX_SAMPLE_STATE_STOPPED)
    {
      if (play_bit > -1.0f)
      {
        play_bit -= 1.0f;
        float pp = position + (PB_LENGTH - play_bit) * (float)stereo_type ;
        return data[ round(pp) ];
      }
      return 0;
    }

    // move our sample pointer
    position += pitch_bend * (float)stereo_type;

    // safeguard
    if (position < 0.0f)
    {
      position = 0.0f;
      return 0;
    }

    // safeguard
    if (position > data.size()-(2.0 + DRIFT * stereo_type))
    {
      position = data.size()-(2.0 + (double)DRIFT * (double)stereo_type);
      return 0; // silence
    }

    // set value so that the consume_right
    // method can use it if we're a mono sample


    float i_pos = position + DRIFT * (double)stereo_type * (double)state;
    if (i_pos < 0.0f) i_pos = 0.0;

    float start_val = data[ floor(i_pos) ] * ONE_DIV_32767;
    float end_val = data[ ceil(i_pos) ] * ONE_DIV_32767;
    float factor = i_pos - floor(i_pos);
    float res = (factor) * end_val + (1.0 - factor) * start_val;




    prev_left_value = (int16_t) ( res * 32767.0f );
    return prev_left_value;
  }

  virtual inline int16_t consume_right()
  {
    if (!data.size())
      return 0;

    if (stereo_type == VSX_SAMPLE_MONO)
      return prev_left_value;

    if (state == VSX_SAMPLE_STATE_STOPPED)
      return 0;

    float i_pos = position + DRIFT * (double)stereo_type * (double)state;
    if (i_pos < 0.0f) i_pos = 0.0;

    float start_val = data[ floor(i_pos) + 1.0f ] * ONE_DIV_32767;
    float end_val = data[ ceil(i_pos) + 1.0f ] * ONE_DIV_32767;
    float factor = i_pos - floor(i_pos);
    float res = (factor) * end_val + ( 1.0 - factor) * start_val;


    return
      (int16_t) ( res * 32767.0f );
  }


  virtual void load_filename(vsx_string filename) = 0;

};

class vsx_sample_raw : public vsx_sample
{
  vsxf* filesystem;

public:

  vsx_sample_raw()
  :
  filesystem(0x0)
  {
  }

  void set_filesystem(vsxf* n)
  {
    filesystem = n;
  }

  void load_filename(vsx_string filename)
  {
    if (!filesystem)
      return;

    vsxf_handle *fp;
    if
    (
      (fp = filesystem->f_open(filename.c_str(), "r"))
      ==
      NULL
    )
      return;

    size_t file_size = filesystem->f_get_size(fp);
    data.reset_used();
    data.allocate_bytes( file_size );
    filesystem->f_read( (void*)data.get_pointer(), file_size, fp);
    filesystem->f_close(fp);
  }

};


class vsx_sample_ogg : public vsx_sample
{
  vsxf* filesystem;

public:

  vsx_sample_ogg()
  :
  filesystem(0x0)
  {
  }

  void set_filesystem(vsxf* n)
  {
    filesystem = n;
  }

  void load_filename(vsx_string filename)
  {
    if (!filesystem)
      return;

    vsxf_handle *fp;
    if
    (
      (fp = filesystem->f_open(filename.c_str(), "r"))
      ==
      NULL
    )
      return;

    // file size
    size_t file_size = filesystem->f_get_size(fp);

    // allocate ram to read the file
    void* temp_storage = malloc(file_size);
    if (!temp_storage)
      return;

    // load muffins
    filesystem->f_read( temp_storage, file_size, fp);

    short *result;

    // decode the ogg stream
    // (unsigned char *mem, int len, int *channels, short **output);
    int num_channels;
    int samples_loaded = stb_vorbis_decode_memory
        (
          (unsigned char*)temp_storage,
          file_size,
          &num_channels,
          &result
        );

    free( temp_storage );

    if (-1 == samples_loaded)
    {
      vsx_printf("error loading ogg file, not a vorbis stream or other error...\n");
      return;
    }

    data.set_volatile();
    data.set_data(result,samples_loaded * num_channels);

    filesystem->f_close(fp);
  }

};
