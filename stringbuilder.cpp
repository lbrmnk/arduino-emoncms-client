#include <stdio.h>
#include "stringbuilder.h"

    
StringBuilder::StringBuilder(char* buf, int maxlen) {
  _buf = buf;  
  _pos = 0;
  _maxlen = maxlen - 1; // exclude ntc
}

void 
StringBuilder::clear() 
{
  _pos = 0;
  _buf[0] = 0;
}

char *
StringBuilder::c_str() 
{
  return _buf;
}

int 
StringBuilder::append(const char *str){
  int len = 0;
  while (*str && _pos < _maxlen) {
    _buf[_pos] = *str;
    str++;
    _pos++;
    len++;
  }
  _buf[_pos] = 0;
  return (len);
}

int 
StringBuilder::append(char value) {
  char tmp[2] = { value, '\0' };
  return append(tmp);
}

int 
StringBuilder::append(int value) {
  char tmp[11];
  sprintf(tmp, "%d", value);
  return append(tmp);
}

int 
StringBuilder::append(long value) {
  char tmp[21];
  sprintf(tmp, "%ld", value);
  return append(tmp);    
}

int 
StringBuilder::append(unsigned long value) {
  char tmp[21];
  sprintf(tmp, "%lu", value);
  return append(tmp);    
}

int 
StringBuilder::append(float value) {
  char tmp[21];
  sprintf(tmp, "%d.%02d", (int)value, (int)(100 * (value - (int)value)));
  return append(tmp);    
}

int 
StringBuilder::remaining() {
  return _maxlen - _pos;
}

