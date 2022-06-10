#include <common.h>

extern uint64_t g_nr_guest_inst;
FILE *log_fp = NULL;

void init_log(const char *log_file)
{
  log_fp = stdout; //得到的是一个文件名
  if (log_file != NULL)
  {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  //如果文件存在就把 log 写入 log_file,否则输出至标准输出
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable()
{
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) && (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
