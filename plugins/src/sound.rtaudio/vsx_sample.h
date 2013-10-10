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
    position(0),
    state(0)
  {
  }

  vsx_array<int16_t> data;
  size_t position;
  int state;

  inline int16_t consume()
  {
    if (!data.size())
      return 0;

    if (position > data.size())
      return 0; // silence

    return
      data[position++];
  }

  inline void rewind()
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
    data.allocate(file_size / sizeof(int16_t) );
    filesystem->f_read( (void*)data.get_pointer(), file_size, fp);
    filesystem->f_close(fp);
  }

};
