//111111111111111111111111111111111111111111111111111111111111111111111111
//格式测试:
//邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵邵
//起始日期:
//完成日期:
//************************************************************************
//修改日志:
//	修改程序功能为:
//		删除当前文件夹下所有的‘文件名前缀’一致的文件;
//		删除当前文件夹下所有的‘文件名后缀’一致的文件;
//		删除当前文件夹下所有的‘前缀+后缀都匹配’一致的文件;
//		(但不支持遍历子文件夹)
//, , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , ,, , ,


//编译:
//g++ -o x ./del_all_file.cpp -g3

//生成测试数据: 每次测试都生成新测试数据最好.
//touch a2 a21 a2a2.pug a.22.png a12a2.png x.png x.png.png a2..png.png

//前序删除测试: ./x -i a2
//后序删除测试: ./x -o .png
//前序+后续删除测试: ./x -o -i a2 .png


/* 0 版权/许可证 */



/* 1 包含 */
#include <stdint.h> // c 标准库
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <algorithm>
using namespace std;


#include <libgen.h> // for 命令行解析
#include <getopt.h>

#include <sys/types.h> // linux 系统库

#include <string.h>
#include <fcntl.h> // for 原子操作O_RDONLY 之类的
#include <sys/stat.h>

#include <limits.h>
#include <dirent.h>//for readdir()
#include <stddef.h>



/* 2 定义 */
#define OPTSTR "i:o:h" //命令行开关 -i -o -h
#define USAGE_FMT \
	"usage:\n\
	[i:o:h]\n\
	%s [-i 'file name prefix'] [-o 'file name suffix'] [-h]\n\
	example:\n\
	./x -i 前缀-匹配\n\
	./x -o 后缀文件类型-匹配\n\
	./x -i 前缀-匹配 -o 后缀文件类型-匹配\n"
	//USAGE_FMT = 打印函数usage()显示的program用法字符串
#define HELP_CONTACT "contact us: adan_shaw@qq.com\n\n"//作者联系方式
#define DEF_PROGRAM_NAME "x"//默认程序名
#define DEF_VERSION "%s version: 0.0.1\n"//默认版本号
#define PERROR_QUIT(s) perror(s);exit(EXIT_FAILURE);//EXIT_SUCCESS
//宏声明--#define _printf(args...) assert(printf(args) != -1);

#define OPTARG_BUF_LEN 256



/* 3 外部声明 */
//3.1 getopt() 函数相关家族
extern char *optarg;	//用来保存选项的参数
extern int optind;		//用来记录下一个检索位置
extern int opterr;		//表示的是是否将错误信息输出到stderr
extern int optopt;		//表示不在'选项字符串optstring中'的选项



/* 4 类型定义 */



/* 5 全局变量声明 */
pid_t mainPid = 0;//主进程pid
char buf_prefix[OPTARG_BUF_LEN] = "";//前缀
char buf_suffix[OPTARG_BUF_LEN] = "";//后缀



/* 6 函数声明 */
//选项-h
void opt_h(char *program_name);

//选项-i
void opt_i(char *prefix);
//选项-o
void opt_o(char *suffix);



//1.dir基础测试
void dir_basic_test(void);

//2 遍历某个目录下的所有文件
//	因为内部使用了静态数据,所以readdir()不是线程安全函数
void listFiles(const char *dirpath,\
		const char *prefix,const char *suffix,short execute_on);



/* 7 命令行解析 */
int main(int argc, char *argv[]) {
	short count = 0;
	int opt;
	short execute_on = 0;//执行开关



	while((opt = getopt(argc, argv, OPTSTR)) != EOF){
		switch(opt){
			case 'i':
				opt_i(optarg);
				execute_on += 1;
				break;
			case 'o':
				opt_o(optarg);
				execute_on += 3;
				break;
			case 'h':
			default:
				opt_h(basename(argv[0]));
				/* NOTREACHED */
				break;
		}
		count++;
	}
	if(count == 0){
		printf("./%s: no any option(default option)\n",basename(argv[0]));
		opt_h(basename(argv[0]));
	}

	if(execute_on){
		//1.dir基础测试
		dir_basic_test();
		printf("\n");



		printf("2.2 删除所有匹配文件:\n");
		listFiles(".",buf_prefix,buf_suffix,execute_on);
		printf("\n\n");
	}


	return EXIT_SUCCESS;//0, EXIT_FAILURE = 1

}



/* 8 函数原型 */

//选项-h
void opt_h(char *program_name){
	printf(DEF_VERSION,program_name?program_name:DEF_PROGRAM_NAME);
	printf(USAGE_FMT,program_name?program_name:DEF_PROGRAM_NAME);
	printf(HELP_CONTACT);
	exit(EXIT_SUCCESS);//0, EXIT_FAILURE = 1
	/* NOTREACHED */
}



//选项-i
void opt_i(char *prefix){
	printf("optarg prefix = %s\n",prefix);
	strncpy(buf_prefix,prefix,OPTARG_BUF_LEN);
	return;
}
//选项-o
void opt_o(char *suffix){
	printf("optarg suffix = %s\n",suffix);
	strncpy(buf_suffix,suffix,OPTARG_BUF_LEN);
	return;
}





//1.dir基础测试
void dir_basic_test(void){
	int tmp;
	char cwdbuf[4096];



	printf("1.dir基础测试\n");
	//1.重命名-文件
	tmp = rename("./x","./x_new");
	if(tmp == -1){
		perror("rename() failed");
		return ;
	}


	//2.创建link(目标, ‘生成的link 存放位置’和名称)
	tmp = link("./x_new","./x");
	if(tmp == -1){
		perror("link() failed");
		return;
	}


	//3.移除link
	tmp = unlink("./x");
	if(tmp == -1){
		perror("unlink() failed");
		return;
	}


	//4.创建文件夹
	tmp = mkdir("./.myMkdir",0600);
	if(tmp == -1){
		perror("mkdir() failed");
		return;
	}


	//5.移除文件夹
	tmp = rmdir("./.myMkdir");
	if(tmp == -1){
		perror("rmdir() failed");
		return;
	}


	//6.删除-文件
	tmp = remove("./x_new");
	if(tmp == -1){
		perror("remove() failed");
		return;
	}


	//7.1: 获取进程当前的工作目录
	getcwd(cwdbuf,4096);
	printf("进程当前的工作目录:\n%s\n\n",cwdbuf);

	//7.2: 更改进程当前的工作目录
	chdir(".");
	//fchdir()

	//7.3：更改进程根目录
	chroot(".");

	return ;
}



//2 遍历某个目录下的所有文件
//	因为内部使用了静态数据,所以readdir()不是线程安全函数
void listFiles(const char *dirpath,\
		const char *prefix,const char *suffix,short execute_on){
	DIR *dirp;
	struct dirent *dp;
	char buf_inverted[OPTARG_BUF_LEN];
	string str;
	int tmp;
	bool del_execute_on;



	//排除错误参数
	switch(execute_on){
		case 3:
		case 4:
			//如果需要后缀匹配, 先前置并倒序后缀名, 这样操作更快, 速度更快
			str = suffix;
			std::reverse(str.begin(), str.end());
			memset(buf_inverted,'\0',OPTARG_BUF_LEN);
			strncpy(buf_inverted,str.c_str(),OPTARG_BUF_LEN);
		case 1:
			break;
		default:
			printf("error !! Syntax error, please cheap up -h\n");
			return;
	}


	//获取目录流DIR*
	dirp = opendir(dirpath);
	if(dirp  == NULL){
		perror("opendir() fail");
		return;
	}


	//解析目录流DIR*
	for(;;){
		dp = readdir(dirp);
		if(dp == NULL)
			break;

		//跳过目录自身和父目录??
		if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
			continue;

		str = dp->d_name;//继承文件名

		del_execute_on = false;
		switch(execute_on){

			case 3://后缀匹配删除
				std::reverse(str.begin(), str.end());//先倒序文件名, 再匹配
				tmp = str.find(buf_inverted,0);
				if(tmp == string::npos)
					break;//找不到子串
				if(tmp != 0)
					break;//子串不是最前-前缀
				del_execute_on = true;
				break;

			case 4:
				//先顺序查找-前序匹配
				tmp = str.find(prefix,0);
				if(tmp == string::npos)
					break;//找不到子串
				if(tmp != 0)
					break;//子串不是最前-前缀

				//再倒序查找-后序匹配
				std::reverse(str.begin(), str.end());//先倒序文件名, 再匹配
				tmp = str.find(buf_inverted,0);
				if(tmp == string::npos)
					break;//找不到子串
				if(tmp != 0)
					break;//子串不是最前-前缀

				del_execute_on = true;//都ok, 才打开del 开关
				break;

			case 1://前缀匹配删除
				tmp = str.find(prefix,0);
				if(tmp == string::npos)
					break;//找不到子串
				if(tmp != 0)
					break;//子串不是最前-前缀
				del_execute_on = true;
				break;

			default:
				printf("error !! unknow error, program crash!!\n");
				return;
		}

		if(del_execute_on){//执行删除
			printf("%s\n", dp->d_name);
			tmp = remove(dp->d_name);
			if(tmp == -1){
				perror("remove() failed");
				return;
			}
		}//if end

	}//for end

	if(closedir(dirp) == -1)
		perror("closedir() fail");

	return;
}
