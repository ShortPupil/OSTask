# 说明

## 本代码在第八章的代码基础上修改

## 新增内容

### 新增D、E用户进程

- 在global.c的user_proc_table中新增一项proca
- 在proc.h新增定义的任务堆栈、信号量、并做对应的新增
- 在proto.h中新增心任务执行体
- 在main.c中新增DE进程，其中包含主要的逻辑

### 新增的系统调用

- 在proto.h中新增需要的sleep、p、v
- 在syscall.asm中添加函数体
- 在global.c的sys_call_table的调用列表中添加新的系统调用
- NR_SYS_CALL 要改为5
- 在proto.h中进行函数声明 testC testD testE
- proc.c中sys_process_sleep函数实现
- main.c中sys_tem_p,sys_tem_v函数实现
- global.c中声明数据，main.c中初始化数据
- console.c添加颜色输出
