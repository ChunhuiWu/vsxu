#define VSX_SAMPLE_STATE_STOPPED 0
#define VSX_SAMPLE_STATE_PLAYING 1

class vsx_sample
{
  vsxf* filesystem;
public:
  void set_filesystem(vsxf* n)
  {
    filesystem = n;
  }

  vsx_sample()
    :
    filesystem(0x0),
    position(0.0f),
    state(0),
    pitch_bend(0.0f)
  {
  }

  vsx_array<int16_t> data;
  float position;
  float pitch_bend;
  int state;

  inline int16_t consume()
  {
    if (!data.size())
      return 0;

    position += pitch_bend;

    if (position < 0.0f)
    {
      position = 0.0f;
      return 0;
    }

    if (position > data.size()-1.0)
    {
      position = data.size()-1.0;
      return 0; // silence
    }

    return
      data[round(position)];
  }

  inline void rewind()
  {
    position = 0;
  }

  inline void trigger()
  {
    position = 0;
  }

  void load_filename(vsx_string filename)
  {
    if (!filesystem)
      return;
    vsxf_handle *fp;
    if ((fp = filesystem->f_open(filename.c_str(), "r")) == NULL)
      return;
    size_t file_size = filesystem->f_get_size(fp);
    vsx_printf("file size: %d\n", file_size);
    data.reset_used();
    data.allocate_bytes( file_size );
    vsx_printf("data get used: %d\n", data.get_used());
    filesystem->f_read( (void*)data.get_pointer(), file_size, fp);
    filesystem->f_close(fp);
  }

};
