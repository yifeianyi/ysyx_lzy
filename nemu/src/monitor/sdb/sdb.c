#include <isa.h>
#include <cpu/cpu.h>
#include <memory/host.h>
#include <memory/paddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }
  //从stdin读取输入，赋值给line_read
  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

/*-----------------------选项执行函数--------------------------*/
static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args)
{
  return -1;
}

static int cmd_help(char *args);

//自定义环节
static int cmd_si(char *args)
{
  return 0;
}
static int cmd_info(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");

  printf("check into info\n");
  if (arg == NULL)
  {
    /* no argument given */
    printf("Unknown command , you can try 'help'\n");
  }
  else
  {
    if (strcmp(arg, "r") == 0)
    {
      isa_reg_display();
    }
    else if (strcmp(arg, "w") == 0)
    {
      //未完待续
    }
    else
    {
      printf("Unknown command '%s'\n", arg);
    }
  }
  return 0;
}

//输出 以 EXPR 的值为起始地址的连续n个uint32_t
static int cmd_x(char *args)
{

  // char *str_end = args + strlen(args);
  /* extract the first argument */
  char *cmdn = strtok(args, " "); //获取数字
  uint8_t Expr_len;
  paddr_t addr;
  if (cmdn == NULL)
  {
    printf("command error, you can try 'hellp'");
    return 0;
  }
  else
  {
    //从表达式中读取地址
    char *saddr = cmdn + strlen(cmdn) + 1;
    sscanf(saddr, "%x", &addr);

    //读取需要的4字节 长度
    Expr_len = atoi(cmdn);
  }

  for (int i = 0; i < Expr_len; i++)
  {
    printf("0x%08x\n", paddr_read(addr + i, 1));
  }

  return 0;
}

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Execute n step,When N is not given, the default is 1", cmd_si},
    {"info", " r: Print Register state \n f: print monitoring point information", cmd_info},
    {"x", "Evaluate the expression EXPR and use the result as the starting memory Address,output N consecutive 4-bytes in hexadecimal format", cmd_x}
    /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(args, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    // 如果 help 没有带参数，就输出所有指令名称，及其描述
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    //如果有带参数，就遍历选项列表。如果在 选项表中就输出相关信息。
    //否则就输出unknown
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str = NULL; (str = rl_gets()) != NULL;)
  {

    char *str_end = str + strlen(str); //存储输入指令集结尾的地址

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      //如果没有接收到指令，则运行到continue这陷入死循环
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1; // 获取下一个指令的地址
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    //遍历所有选项
    for (i = 0; i < NR_CMD; i++)
    {
      /********************************************
       *
       * 如果输入选项和当前枚举的选项一致，
       * 且是q的话，就退出qemu
       *
       *******************************************/
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        //如果选项是q，就退出nemu
        //如果是c,就进入到sdb的循环中,并模拟cpu执行指令,并返回0
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
