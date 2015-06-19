#include "../include/myheader.h"
#include "../include/code.h"

int base64_encode( char *gb2312string, char *base64string, int gb2312stringlen ) 
{
	static char Base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int base64_len = 0;

	while( gb2312stringlen > 0 ){
		*base64string ++ = Base64Encode[(gb2312string[0] >> 2 ) & 0x3f];
		if( gb2312stringlen > 2 ){
			*base64string ++ = Base64Encode[((gb2312string[0] & 3) << 4) | (gb2312string[1] >> 4)];
			*base64string ++ = Base64Encode[((gb2312string[1] & 0xF) << 2) | (gb2312string[2] >> 6)];
			*base64string ++ = Base64Encode[gb2312string[2] & 0x3F];
		}
		else
		{
			switch( gb2312stringlen )
			{
				case 1:
				*base64string ++ = Base64Encode[(gb2312string[0] & 3) << 4 ];
				*base64string ++ = '=';
				*base64string ++ = '=';
				break;
				case 2:
				*base64string ++ = Base64Encode[((gb2312string[0] & 3) << 4) | (gb2312string[1] >> 4)];
				*base64string ++ = Base64Encode[((gb2312string[1] & 0x0F) << 2) | (gb2312string[2] >> 6)];
				*base64string ++ = '=';
				break;
			}
		}

		gb2312string +=3;
		gb2312stringlen -=3;
		base64_len +=4;
	}
	*base64string = 0;
	return base64_len;
}
char GetBase64Value(char ch) //得到编码值
{
	if ((ch >= 'A') && (ch <= 'Z'))  // A ~ Z
		return ch - 'A';
	if ((ch >= 'a') && (ch <= 'z'))  // a ~ z
		return ch - 'a' + 26;
	if ((ch >= '0') && (ch <= '9'))  // 0 ~ 9
		return ch - '0' + 52;
	switch (ch)       // 其它字符
	{
		case '+':
		return 62;
		case '/':
		return 63;
		case '=':  //Base64 填充字符
		return 0;
		default:
		return 0;
	}
}


int base64_decode( char *gb2312string, char *base64string, int base64_len, int force )  //解码函数
{
// gb2312string 保存解码结果字符串指针
// base64string 待解码字符串指针
// base64_len 待解码字符串长度
// force 当待解码字符串长度错误时,是否强制解码
//     true  强制解码
//     false 不强制解码
     	int 	count = 0;
	int 	index = base64_len-1;
	uint_8  Base64Encode[4];
	int gb2312stringlen=0;
	while(base64string[index] == '=') {
		
		count++;
		index--;
	}
	if( base64_len % 4 && !force )   //如果不是 4 的倍数,则 Base64 编码有问
	{
		gb2312string[0] = '\0';
		return -1;
	}

	while( base64_len > 2 )  //当待解码个数不足3个时,将忽略它(强制解码时有效)
	{
		Base64Encode[0] = GetBase64Value(base64string[0]);
		Base64Encode[1] = GetBase64Value(base64string[1]);
		Base64Encode[2] = GetBase64Value(base64string[2]);
		Base64Encode[3] = GetBase64Value(base64string[3]);

		*gb2312string ++ = (Base64Encode[0] << 2) | (Base64Encode[1] >> 4);
		*gb2312string ++ = (Base64Encode[1] << 4) | (Base64Encode[2] >> 2);
		*gb2312string ++ = (Base64Encode[2] << 6) | (Base64Encode[3]);

		base64string += 4;
		base64_len -= 4;
		gb2312stringlen += 3;
	}
	return gb2312stringlen - count;
}


//判断当前使用的是打断还是小端系统
int is_bigendian(){

	union{
		
		short s;
		char c[sizeof(short)];
	}un;
	un.s = 0x0102;
	
	if(un.c[0] == 1 && un.c[1] == 2) return 1;
	if(un.c[0] == 2 && un.c[1] == 1) return 0;

	return -1;

}


//gb2312 到utf8 的转码函数   当前的函数支持1个字节和两个字节的utf8转换  如有必要可以扩充
int gb2312_to_utf8(unsigned char *gb2312string, int gb2312string_len, unsigned char *utf8string)
{
	int 	i;
	unsigned int 	index1,index2;
	
	wchar	tmp;
	wchar 	*p;
	
	uint_8	*gb2312_char;
	uint_8 *utf8_char;
	int	utf8string_len = 0;

	wchar 	*twochar;

	gb2312_char  =(uint_8*)gb2312string;
	utf8_char = (uint_8 *)utf8string;

	i = 0;
	while(i<gb2312string_len){
		
		if(*gb2312_char >= 0x0 && *gb2312_char <= 0x7f){
		
			if(debug_coding)printf("%x\n\n",*gb2312_char);
			*utf8_char = *gb2312_char;
			utf8_char++;
			utf8string_len++;
			gb2312_char++;
			i++;
		}
		else{
			twochar = (wchar *)gb2312_char;
			if(debug_coding)printf("%x\n",*twochar);
			
			tmp = *twochar - 0xa0a0;
			index1 = tmp >> 8;
			index2 = tmp & 0xff;
			if(debug_coding)printf("index1: %d index2: %d\n",index1,index2);
			
			tmp = gb_2_uni[index1][index2];
			if(debug_coding)printf("unicode: %x\n", gb_2_uni[index1][index2]);
			
			*(utf8_char) = ((tmp >> 12) & 0x0f) | 0xe0;
			if(debug_coding)printf("%x ",*utf8_char);
			
			*(utf8_char+2) = (tmp & 0x3f) | 0x80;
			if(debug_coding)printf("%x ",*(utf8_char+2));
			
			*(utf8_char+1) = ((tmp >> 6) & 0x3f) |0x80;
			if(debug_coding)printf("%x\n\n",*(utf8_char+1));
			
			utf8_char += 3;
			utf8string_len += 3;
			gb2312_char += 2;
			i +=2;
		}
	}
	if(debug_coding)printf("the utf8string: %s\n",utf8string);
	return utf8string_len;
}
