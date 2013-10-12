class vsx_sample_mixer
{
  vsx_array<vsx_sample*> mixing_channels;

public:

  inline int16_t consume_left()
  {
    // optimization
    const float one_div_32768 = 1.0 / 32767.0;

    // accumulator
    float current_value = 0.0f;

    // active channels
    size_t active_channels = 0;

    for (size_t i = 0; i < mixing_channels.size(); i++)
    {
      // ignore deleted samples
      if (!mixing_channels[i])
        continue;

      // is this channel playing?
      if (!mixing_channels[i]->get_state())
        continue;

      // grab channel value
      int16_t cval = mixing_channels[i]->consume_left();

      // increase active channels
      active_channels++;

      // convert cval to float (val)
      float val = (float)cval * one_div_32768;

      // apply gain
      val *= mixing_channels[i]->get_gain();

      // accumulate in current_value
      current_value += val;
    }

    if (active_channels == 0)
      return 0;

    // safeguard
    if (current_value > 1.0)
      current_value = 1.0f;

    if (current_value < -1.0)
      current_value = -1.0f;

    return
      (int16_t) round(current_value * 32767.0);
  }

  inline int16_t consume_right()
  {
    // optimization
    const float one_div_32768 = 1.0 / 32767.0;

    // accumulator
    float current_value = 0.0f;

    // active channels
    size_t active_channels = 0;

    for (size_t i = 0; i < mixing_channels.size(); i++)
    {
      // ignore deleted samples
      if (!mixing_channels[i])
        continue;

      // is this channel playing?
      if (!mixing_channels[i]->get_state())
        continue;

      // grab channel value
      int16_t cval = mixing_channels[i]->consume_right();

      // increase active channels
      active_channels++;

      // convert cval to float (val)
      float val = (float)cval * one_div_32768;

      // apply gain
      val *= mixing_channels[i]->get_gain();

      // accumulate in current_value
      current_value += val;
    }

    if (active_channels == 0)
      return 0;

    // safeguard
    if (current_value > 1.0)
      current_value = 1.0f;

    if (current_value < -1.0)
      current_value = -1.0f;

    return
      (int16_t) round(current_value * 32767.0);
  }

  void register_sample( vsx_sample* ns )
  {
    // recycle unused channels
    for (size_t i = 0; i < mixing_channels.size(); i++)
    {
      if (mixing_channels[i] == 0x0)
      {
        // recycle this channel
        mixing_channels[i] = ns;
        return;
      }
    }
    mixing_channels.push_back( ns );
  }

  void unregister_sample( vsx_sample* us )
  {
    // set channel pointer to zero
    for (size_t i = 0; i < mixing_channels.size(); i++)
    {
      if (mixing_channels[i] == us)
      {
        // disable this channel
        mixing_channels[i] = 0x0;
        return;
      }
    }
  }
};