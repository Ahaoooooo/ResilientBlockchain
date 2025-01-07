
#include "./misc.h"

//该方法处理字符串
vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;//size_t类似于int，可以避免int范围过小或者浪费空间
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();//string:npos是个特殊值，说明查找没有匹配，即pos的值为空
        string token = str.substr(prev, pos-prev);
        /*
        substr有2种用法： 
假设：string s = “0123456789”;

string sub1 = s.substr(5); //只有一个数字5表示从下标为5开始一直到结尾：sub1 = “56789”

string sub2 = s.substr(5, 3); //从下标为5开始截取长度为3位：sub2 = “567”
        */

        //if (!token.empty()) 
        	tokens.push_back(token);
          /*
          push_back()成员函数在向量的最后添加一个新的元素。上面几行代码将"sword"、
          "armor"和"shield"添加至inventory中。因此，inventory[0]等于"sword"，
          inventory[1]等于"armor"，inventory[2]等于"shield"。
          */
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

int safe_stoi( string s, bool &pr)
{
  int v;
  try{ v = stoi(s); }
  catch (...){
    pr = false;
    return 0;
  }
  pr = pr && true;
  return v;
}


unsigned long safe_stoull( string s, bool &pr)
{
  unsigned long v;
  try{ v = stoull(s); }
  catch (...){
    pr = false;
    return 0;
  }
  pr = pr && true;
  return v;
}
