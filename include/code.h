#ifndef CODE_H
#define CODE_H

#define debug_coding 0

unsigned short gb_2_uni[95][95];

int base64_encode(char *gb2312string, char *base64string, int gb2312stringlen);

char getbase64value(char ch);

int base64_decode(char *gb2312string, char *base64string, int base64_len, int force);

int is_bigendian();

int gb2312_to_utf8(unsigned char *gb2312string,int gb2312string_len, unsigned char *utf8string);

#endif
