#ifndef __STRINGBUILDER_H__
#define __STRINGBUILDER_H__

class StringBuilder {
  public:
    char *_buf;
    int _pos;
    int _maxlen;
    
    StringBuilder(char* buf, int maxlen);

    void clear();
    char *c_str();

    int append(const char *str);
    inline int append(char *str) { return append((const char*)str); }

    int append(char value);
    int append(int value);
    int append(long value);
    int append(unsigned long value);
    int append(float value);
    int remaining();
};

#endif
