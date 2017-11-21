#ifndef JSUTILS_H
#define JSUTILS_H
#include<string>
#include<sstream>
#include<iostream>
#include<chrono>
#include<vector>
#include<regex>
#include<map>
#ifdef linux
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include<iconv.h>
#endif

#ifdef WIN32
#include <io.h>
#include <direct.h>
#include <Windows.h>
#endif

#ifdef linux
#define ACCESS access
#define MKDIR(a) mkdir((a),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

#ifdef WIN32
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#endif

class StrategyTemplate;
class AlgoTradingAPI;
typedef std::map<std::string, std::vector<std::string>> SymbolStrategyNameMap;
typedef StrategyTemplate* (*create_t)(AlgoTradingAPI *);
typedef int (*release_t)();

#ifdef WIN32
typedef HINSTANCE handle;
#define CloseDynamicLibrary(a)  FreeLibrary((a))
#define UseDynamicFunction(a,b) GetProcAddress((a),(b))
#else
#include<dlfcn.h>
typedef void* handle;
#define CloseDynamicLibrary(a)  dlclose((a))
#define UseDynamicFunction(a,b) dlsym((a),(b))
#endif

#define F_OK 0
namespace Utils
{
bool checkExist(const std::string &path);
bool createDirectory(const std::string &path);
std::vector<std::string>dirList(const std::string &path);
void deletedir(const std::string &dir_param);
std::vector<std::string> split(std::string str, std::string pattern);
std::string regexSymbol(const std::string &symbol);
std::string getCurrentDateTime();
std::string getMilliseconds();
std::string getMicroseconds();
std::string getCurrentTime();
std::string getCurrentDate();
std::string TimeTtostring(time_t time);
time_t timetotimeT(int hour, int minute, int second) ;
time_t datetimetotimeT(const std::string &date,const std::string &time);
time_t getcurrenttimeT();
std::string Int2String(int value, int digit);
std::string Bool2String(bool var);
int getWeekDay(const std::string &date);
std::string getPWD();
int codeConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
std::string Utf8ToGbk(const std::string& strUtf8);//传入的strUtf8是UTF-8编码
std::string GbkToUtf8(const std::string& strGbk);//传入的strGbk是GBK编码
std::string clearString(const std::string &str);
handle OpenDynamicLibrary(const char *path);
}
#endif
