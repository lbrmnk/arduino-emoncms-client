#ifndef __UTILS_H__
#define __UTILS_H__

class Utils {
  public:
    static char nibbleToChar(byte nibble, bool upper) 
    {
      nibble = upper ? (nibble >> 4) : (nibble & 0x0f);
      return ((nibble < 0x0A) ? + (nibble + '0') : ((nibble - 0x0A) + 'A')); 
    }
  
    static void bytesToString(char *dst, uint8_t* addr, byte len) 
    {
      for(int i = 0; i < len; i++) {
        *dst++ = nibbleToChar(addr[i], true);
        *dst++ = nibbleToChar(addr[i], false);
      }
      *dst++ = '\0';
      //return (2*len + 1);
    }

    static void printAddress(byte *addr) {
      char tmp[17];
      bytesToString(tmp, addr, 8);
      Serial.print(tmp);
    }

};

#endif
