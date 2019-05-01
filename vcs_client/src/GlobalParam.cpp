#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <pthread.h>

using namespace std;

map<string, int> cfParam;
map<string, map<string, double>> rtParam;

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

//********** Property **********
//**  0 = Dynamic Parameter
//**  1 = Static Parameter
//**  2 = Command Parameter
//******************************

void init_rtParam(map<string, double> v, char *listName, int value, int timestamp, int version, int property)
{

    v.insert(pair<string, double>("value", value));
    v.insert(pair<string, double>("timestamp", timestamp));
    v.insert(pair<string, double>("version", version));
    v.insert(pair<string, double>("property", property));

    rtParam[listName] = v;
}

void classify_rtParam()
{
    map<string, map<string, double>>::iterator v;
    map<string, double> v1;
    int p0 = 0, p1 = 0, p2 = 0;
    for (v = rtParam.begin(); v != rtParam.end(); ++v)
    {
        v1 = v->second;
        switch ((int)v1.find("property")->second)
        {
        case 0:
            ++p0;
            break;
        case 1:
            ++p1;
            break;
        case 2:
            ++p2;
            break;
        }
    }
    cfParam.insert(pair<string, double>("dynamic_param.count", p0));
    cfParam.insert(pair<string, double>("static_param.count", p1));
    cfParam.insert(pair<string, double>("command_param.count", p2));
}

int get_rtParam_int(char *name, char *field)
{
    map<string, map<string, double>>::iterator v;
    map<string, double> v1;
    v = rtParam.find(name);
    if (v != rtParam.end())
    {
        pthread_mutex_lock(&mutex_lock);
        v1 = v->second;
        pthread_mutex_unlock(&mutex_lock);
        return v1.find(field)->second;
    }

    printf("No matching rtParam(%s)\n", name);
    return -1;
}
double get_rtParam_double(char *name, char *field)
{
    map<string, map<string, double>>::iterator v;
    map<string, double> v1;
    v = rtParam.find(name);
    if (v != rtParam.end())
    {
        pthread_mutex_lock(&mutex_lock);
        v1 = v->second;
        pthread_mutex_unlock(&mutex_lock);
        return v1.find(field)->second;
    }

    printf("No matching rtParam(%s)\n", name);
    return -1;
}

int set_rtParam(char *name, char *field, double val)
{
    if(strcmp(field, "value")==0)
    {
        int timestamp = get_rtParam_int(name,"timestamp");
        if(timestamp == -1) return -1;
        set_rtParam(name,"timestamp",++timestamp);
    }

    map<string, map<string, double>>::iterator v;
    v = rtParam.find(name);
    if (v == rtParam.end())
        return -1;
    pthread_mutex_lock(&mutex_lock);
    (rtParam.find(name)->second).erase(field);
    (rtParam.find(name)->second).insert(pair<string, double>(field, val));
    pthread_mutex_unlock(&mutex_lock);

    return 1;
}

int get_cfParam(char *name)
{
    map<string, int>::iterator v;
    v = cfParam.find(name);
    if (v != cfParam.end())
    {
            return v->second;
    }
    printf("\nNo matching cfParam(%s)!!\n", name);
    return -1;
}
