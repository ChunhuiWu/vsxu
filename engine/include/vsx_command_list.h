#ifndef VSX_COMMAND_LIST_H
#define VSX_COMMAND_LIST_H

#include "vsx_command.h"


// thread safety notice:
//  an instance of this class shouldn't be shared among more than 2 threads hence it's a simple mutex
//  combined with provider/consumer FIFO or LIFO buffer (pop/push, pop_front/push_front)
class vsx_command_list
{
  pthread_mutex_t mutex1;

  void get_lock() {
    pthread_mutex_lock( &mutex1 );
  }
  void release_lock() {
    pthread_mutex_unlock( &mutex1 );
  }

public:
#ifdef VSX_ENG_DLL
  vsxf* filesystem;
#else
  void* filesystem;
#endif
  int accept_commands;  // 1 accepts, 0 won't accept
  std::list <vsx_command_s*> commands; // results of commands
  std::list <vsx_command_s*>::const_iterator iter;

  // add copy of command at the end of the list
  vsx_command_s* addc(vsx_command_s *cmd)
  {
    if (!accept_commands) return 0;
    if (cmd->iterations < VSX_COMMAND_MAX_ITERATIONS) {
      ++cmd->iterations;
      vsx_command_s *t = new vsx_command_s;
      t->copy(cmd);
      get_lock();
        commands.push_back(t);
      release_lock();
      return t;
    }
    return 0;
  }

  // add copy of command at the beginning of the list
  bool addc_front(vsx_command_s *cmd)
  {
    if (!accept_commands) return 0;
    if (cmd->iterations < VSX_COMMAND_MAX_ITERATIONS) {
      ++cmd->iterations;
      vsx_command_s *t = new vsx_command_s;
      t->copy(cmd);
      add_front(t);
    }
    return true;
  }
  // add & parse a command to the end of the list
  vsx_command_s* add_raw(vsx_string r) {
    if (!accept_commands) return 0;
    return add(vsx_command_parse(r));
  }
  // add & parse a command to the beginning of the list
  vsx_command_s* add_raw_front(vsx_string r) {
    if (!accept_commands) return 0;
    return add_front(vsx_command_parse(r));
  }
  // add a command by pointer to the end of the list
  vsx_command_s* add(vsx_command_s* cmd_) {
    if (!accept_commands) return 0;
    if (cmd_) {
      if (cmd_->iterations < VSX_COMMAND_MAX_ITERATIONS) {
        ++cmd_->iterations;
        get_lock();
          commands.push_back(cmd_);
        release_lock();
        return cmd_;
      }
    } else return 0;
    return 0;
  }
  // add a command by pointer to the beginning of the list
  vsx_command_s* add_front(vsx_command_s* cmd_) {
    if (!accept_commands) return 0;
    if (cmd_) {
      if (cmd_->iterations < VSX_COMMAND_MAX_ITERATIONS) {
        ++cmd_->iterations;
        get_lock();
          commands.push_front(cmd_);
        release_lock();
        return cmd_;
      }
    } else return 0;
    return 0;
  }
  // add a command by specifying command and command data
  VSX_COMMAND_DLLIMPORT void add(vsx_string cmd, vsx_string cmd_data);

  // adds a command
  // Thread safety: YES

  void add(vsx_string cmd, int cmd_data) {
    if (!accept_commands) return;
    vsx_command_s* t = new vsx_command_s;
    t->cmd = cmd;
    t->cmd_data = i2s(cmd_data);//f.str();
    get_lock();
      commands.push_back(t);
    release_lock();
  }

  VSX_COMMAND_DLLIMPORT void adds(int tp, vsx_string titl,vsx_string cmd, vsx_string cmd_data);

  VSX_COMMAND_DLLIMPORT void clear(bool del = false);

  vsx_command_s* reset() {
    //printf("reset command list %p\n", this);
    get_lock();
      iter = commands.begin();
    release_lock();
    return *iter;
  }

  // gets the current command from internal iterator
  // Thread safety: NO
  vsx_command_s* get_cur() {
    if (iter != commands.end()) {
      return *iter;
    }
    return 0;
  }

  // gets the current command from internal iterator and advancing iterator
  // Thread safety: NO
  vsx_command_s* get()
  {
    if (iter != commands.end())
    {
      vsx_command_s* h = *iter;
      ++iter;
      return h;
    }
    return 0;
  }

  // returns and removes the command first in the list
  // Thread safety: YES
  bool pop(vsx_command_s **t)
  {
    get_lock();
    if (commands.size())
    {
      *t = commands.front();
      commands.pop_front();
      release_lock();
      return true;
    }
    release_lock();
    return false;
  }


  // returns and removes the command first in the list
  // Thread safety: YES
  vsx_command_s *pop()
  {
    get_lock();
    if (commands.size())
    {
      vsx_command_s *t = commands.front();
      commands.pop_front();
      release_lock();
      return t;
    }
    release_lock();
    return 0;
  }

  // returns and removes the command last in the list
  // Thread safety: YES
  vsx_command_s *pop_back()
  {
    get_lock();
    if (commands.size())
    {
      vsx_command_s *t = commands.back();
      commands.pop_back();
      release_lock();
      return t;
    }
    release_lock();
    return 0;
  }

  // loads from file and puts the lines in vsx_command_s::raw.
  // The default is not to parse.
  // Thread safety: NO
  VSX_COMMAND_DLLIMPORT void load_from_file(vsx_string filename, bool parse = false,int type = 0);
  // Thread safety: NO
  VSX_COMMAND_DLLIMPORT void save_to_file(vsx_string filename);

  // Thread safety: NO
  VSX_COMMAND_DLLIMPORT void token_replace(vsx_string search, vsx_string replace);
  // Thread safety: NO
  VSX_COMMAND_DLLIMPORT void parse();

  void set_type(int new_type);

  // Thread safety: YES
  int count()
  {
    get_lock();
    int j = commands.size();
    release_lock();
    return j;
  }
  VSX_COMMAND_DLLIMPORT vsx_command_list();
  ~vsx_command_list()
  {}
};



#endif // VSX_COMMAND_LIST_H
