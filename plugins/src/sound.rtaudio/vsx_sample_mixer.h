class vsx_sample_mixer
{
  vsx_array<vsx_sample*> channels;

public:

  int16_t consume()
  {
    const float one_div_32768 = 1.0 / 32768.0;
    float current_value = 0.0f;
    size_t active_channels = 0;
    for (size_t i = 0; i < channels.size(); i++)
    {
      if (!channels[i])
        continue;

      current_value += (float)channels[i]->consume() * one_div_32768;
      active_channels++;
    }
    current_value /= (float)active_channels;

    return (int16_t) (current_value * 32768.0);
  }

  void register_sample( vsx_sample* ns )
  {
    channels.push_back( ns );
  }
};
