#include"utils.h"
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
bool Utils::checkExist(const std::string &path)
{
    int result = ACCESS(path.c_str(), F_OK);

    if (result == -1)
    {
        return false;
    }
    else if (result == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Utils::createDirectory(const std::string &path)
{
    int result = MKDIR(path.c_str());
    if(result == -1)
    {
        return false;
    }
    else if(result == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::vector<std::string> Utils::dirList(const std::string &path)
{
#ifdef WIN32
    std::vector<std::string>result;
    _finddata_t FileInfo;
    std::string strfind = path + "\\*";
    long Handle = _findfirst(strfind.c_str(), &FileInfo);


    if (Handle == -1L)
    {
        std::cerr << "can not match the folder path" << std::endl;
    }
    do{
        //判断是否有子目录
        if (FileInfo.attrib & _A_SUBDIR)
        {
            //这个语句很重要
            if( (strcmp(FileInfo.name,".") != 0 ) &&(strcmp(FileInfo.name,"..") != 0))
            {
                std::string newPath = path + "\\" + FileInfo.name;
                dirList(newPath);
            }
        }
        else
        {
            //string filename = (path + "\\" + FileInfo.name);
            std::string name = FileInfo.name;
            if(name.find(".csv")!=std::string::npos)
            {
                result.push_back(name);
            }
        }
    }while (_findnext(Handle, &FileInfo) == 0);
    _findclose(Handle);
    return result;
#else
    DIR    *dir;
    struct    dirent    *ptr;
    std::vector<std::string>result;
    dir = opendir(path.c_str());
    while((ptr = readdir(dir)) != NULL)
    {
        std::string name=std::string(ptr->d_name);
        if(name.find(".csv")!=std::string::npos)
        {
            result.push_back(name);
        }
    }
    closedir(dir);
    return result;


#endif

}

void Utils::deletedir(const std::string &dir_param)
{
#ifdef WIN32
    system(("rmdir /S "+dir_param).c_str());
#else
    //清空文件夹
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if ((dp = opendir(dir_param.c_str())) == NULL)
    {
        fprintf(stderr, "cannot open directory: %s\n", dir_param);
    }
    chdir (dir_param.c_str());
    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISREG(statbuf.st_mode))
        {
            remove(entry->d_name);
        }
    }
#endif
}

std::vector<std::string> Utils::split(std::string str, std::string pattern)
{
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;
    std::string::size_type size = str.size();
    for (std::string::size_type i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

std::string Utils::regexSymbol(const std::string &symbol)
{
    std::string s = symbol;
    std::regex regex("\\d");
    std::string trans_symbol = std::regex_replace(s, regex, "");
    return trans_symbol;
}

std::string Utils::getCurrentDateTime()
{
    time_t now = time(NULL);
    auto milliseconds = getMilliseconds();
    std::string lastmilliseconds = milliseconds.substr(milliseconds.size() - 3);
    tm local = *localtime(&now);//将UNIX时间戳转TM结构
    std::string datetime = Int2String((local.tm_year + 1900), 4) + "-" + Int2String((local.tm_mon + 1), 2) + "-" + Int2String((local.tm_mday), 2)
            + " " + Int2String((local.tm_hour), 2) + ":" + Int2String((local.tm_min), 2) + ":" + Int2String((local.tm_sec), 2);
    return datetime + "." + lastmilliseconds;
}

std::string Utils::getMilliseconds()
{
    auto milliseconds = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>
                                       (std::chrono::system_clock::now().time_since_epoch()).count());
    return milliseconds;
}

std::string Utils::getMicroseconds()
{
    auto microseconds = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>
                                       (std::chrono::system_clock::now().time_since_epoch()).count());
    return microseconds;
}

std::string Utils::getCurrentTime()
{
    time_t now = time(NULL);
    auto milliseconds = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>
                                       (std::chrono::system_clock::now().time_since_epoch()).count());
    std::string lastmilliseconds = milliseconds.substr(milliseconds.size() - 3);
    tm local = *localtime(&now);//将UNIX时间戳转TM结构
    std::string datetime = Int2String((local.tm_hour), 2) + ":" + Int2String((local.tm_min), 2) + ":" + Int2String((local.tm_sec), 2);
    return datetime + "." + lastmilliseconds;
}

std::string Utils::getCurrentDate()
{
    time_t now = time(NULL);
    tm local = *localtime(&now);//将UNIX时间戳转TM结构
    std::string datetime = Int2String((local.tm_year + 1900), 4) + "-" + Int2String((local.tm_mon + 1), 2) + "-" + Int2String((local.tm_mday), 2);
    return datetime;
}

std::string Utils::TimeTtostring(time_t time)
{
    auto tt = time;
    struct tm* ptm = localtime(&tt);
    char date[60] = { 0 };
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}

time_t Utils::timetotimeT(int hour, int minute, int second)
{
    //时分秒转换成unix时间戳
    time_t now = time(NULL);
    tm local = *localtime(&now);//将UNIX时间戳转TM结构
    local.tm_hour = hour;
    local.tm_min = minute;
    local.tm_sec = second;
    time_t ft = mktime(&local);
    return ft;
}

time_t Utils::datetimetotimeT(const std::string &date, const std::string &time)
{
    std::vector<std::string>datelist = Utils::split(date, "-");
    std::vector<std::string>timelist = Utils::split(time, ":");
    std::vector<std::string>seconds;
    if (timelist.at(2).find(".") != std::string::npos)
    {
        seconds = Utils::split(timelist.at(2), ".");
    }
    else
    {
        seconds.push_back(timelist.at(2));
    }

    if (datelist.size() == 3 && timelist.size() == 3)
    {
        tm l;
        l.tm_year = std::stoi(datelist.at(0)) - 1900;
        l.tm_mon = std::stoi(datelist.at(1)) - 1;
        l.tm_mday = std::stoi(datelist.at(2));
        l.tm_hour = std::stoi(timelist.at(0));
        l.tm_min = std::stoi(timelist.at(1));
        l.tm_sec = std::stoi(seconds.at(0));
        time_t ft = mktime(&l);
        return ft;
    }
    else
    {
        return 0;
    }
}

std::string Utils::Int2String(int value, int digit)
{
    char charstr[100];
    std::string str = std::string("%0" + std::to_string(digit) + "d");
    const char * format = str.c_str();
    sprintf(charstr, format, value);
    return std::string(charstr);
}

std::string Utils::Bool2String(bool var)
{
    if (var == true)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

int Utils::getWeekDay(const std::string &date)
{
    std::vector<std::string>dateVec = Utils::split(date, "-");
    int y = 0, c = 0, m = 0, d = 0;
    int month, year,centrary,day;
    if (dateVec.size()>2)
    {
        centrary = std::stod(dateVec[0].substr(0, 2)) + 1;
        year = std::stod(dateVec[0].substr(2, 2));
        month = std::stod(dateVec[1]);
        day = std::stod(dateVec[2]);
    }
    else
    {

        std::cout << "getweekday日期出错";
        return -1;

    }
    if (month == 1 || month == 2)
    {
        c = (year - 1) / 100;
        y = (year - 1) % 100;
        m = month + 12;
        d = day;
    }
    else
    {
        c = year / 100;
        y = year % 100;
        m = month;
        d = day;
    }

    int w = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + d - 1;
    w = w >= 0 ? (w % 7) : (w % 7 + 7);
    if (w == 0)
    {
        w = 7;
    }
    return w;
}

time_t Utils::getcurrenttimeT()
{
    time_t nowtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());//获取当前的系统时间
    return nowtime;
}


std::string Utils::getPWD()
{
#ifdef WIN32
    char   buffer[MAX_PATH];
    _getcwd(buffer, MAX_PATH);
    std::string s=buffer;
    return s+"/";
#else
    static char buf[PATH_MAX];
    int i;
    int rslt = readlink("/proc/self/exe", buf, PATH_MAX);
    if (rslt < 0 || rslt >= PATH_MAX)
    {
        return NULL;
    }
    buf[rslt] = '\0';
    for (i = rslt; i >= 0; i--)
    {
        if (buf[i] == '/')
        {
            buf[i + 1] = '\0';
            break;
        }
    }
    std::string s=buf;
    return s;
#endif
}

int Utils::codeConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
#ifdef linux
    iconv_t cd;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';
#endif
    return 0;
}

int Utils::u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
#ifdef WIN32
    return 0;
#else
    return codeConvert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
#endif
}

int Utils::g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
#ifdef WIN32
    return 0;
#else
    return codeConvert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
#endif
}

//gbk转UTF-8
std::string Utils::GbkToUtf8(const std::string& strGbk)//传入的strGbk是GBK编码
{
    //gbk转unicode
    int len = MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, NULL, 0);
    wchar_t *strUnicode = new wchar_t[len];
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_ACP, 0, strGbk.c_str(), -1, strUnicode, len);

    //unicode转UTF-8
    len = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    char * strUtf8 = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, strUtf8, len, NULL, NULL);

    std::string strTemp(strUtf8);//此时的strTemp是UTF-8编码
    delete[]strUnicode;
    delete[]strUtf8;
    strUnicode = NULL;
    strUtf8 = NULL;
    return strTemp;
}

//UTF-8转gbk
std::string Utils::Utf8ToGbk(const std::string& strUtf8)//传入的strUtf8是UTF-8编码
{
    //UTF-8转unicode
    int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
    wchar_t * strUnicode = new wchar_t[len];//len = 2
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode, len);

    //unicode转gbk
    len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    char *strGbk = new char[len];//len=3 本来为2，但是char*后面自动加上了\0
    memset(strGbk, 0, len);
    WideCharToMultiByte(CP_ACP,0, strUnicode, -1, strGbk, len, NULL, NULL);

    std::string strTemp(strGbk);//此时的strTemp是GBK编码
    delete[]strUnicode;
    delete[]strGbk;
    strUnicode = NULL;
    strGbk = NULL;
    return strTemp;
}


std::string Utils::clearString(const std::string &str)
{
    std::string s=str;
    size_t n = s.find_last_not_of( " \r\n\t" );
    if( n != std::string::npos )
    {
        s.erase( n + 1 , s.size() - n );
    }

    n = s.find_first_not_of ( " \r\n\t" );
    if( n != std::string::npos )
    {
        s.erase( 0 , n );
    }
    return s;
}

handle Utils::OpenDynamicLibrary(const char *path)
{
#ifdef WIN32
    return LoadLibraryA(path);
#else
    return dlopen(path,RTLD_NOW);
#endif
}
