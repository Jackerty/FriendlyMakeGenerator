/***************************************************
* Module gives easy to functions for printing more *
* complicated output. Meant to be alternative to   *
* printf and other formatting bullshit             *
***************************************************/
#ifndef _PRINT_TOOLS_H_
#define _PRINT_TOOLS_H_

/**************************************************
* Macro for writing that uses sizeof to detect    *
* length of the string.                           *
**************************************************/
#define printconst(D,S) (void)write(D,S,sizeof(S)-1)
/**************************************************
* Macro for writing that uses strlen to detect    *
* length of the string.                           *
**************************************************/
#define print(D,S) (void)write(D,S,strlen(S))
/**************************************************
* Concatenates two strings and prints the result  *
* to given file descriptor. Returns amount        *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat(const int fd, const char *str1,const char *str2,const int str1len,const int str2len);
/**************************************************
* Concatenates three strings and prints the       *
* result to given file desctiptor. Returns amount *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat3(const int fd,const char *str1,const char *str2,const char *str3,const int str1len,const int str2len,const int str3len);
/**************************************************
* Concatenates four strings and prints the result *
* to given file desctiptor. Returns amount        *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat4(const int fd,const char *str1,const char *str2,const char *str3,const char *str4,const int str1len,const int str2len,const int str3len,const int str4len);
/**************************************************
* Concatenates four strings and prints the result *
* to given file desctiptor. Returns amount        *
* written or -1 on error (errno is set).          *
**************************************************/
int printStrCat5(const int fd,const char *str1,const char *str2,const char *str3,const char *str4,const char *str5,const int str1len,const int str2len,const int str3len,const int str4len,const int str5len);
/**************************************************
* Give length of the string in the given base.    *
**************************************************/
int itostrlen(int number,const int base);
/**************************************************
* If number's string length is known this         *
* function will write the number with given base  *
* to string. For hexadecimal lower letters are    *
* used.                                           *
* NOTE: this function does not check buffer or    *
*       that strlength is correct size.           *
**************************************************/
void itostr(int number,const int base,char *buffer,const int strlength);
/**************************************************
* Formated writev. General function to write      *
* formated output. Advance over printf is that    *
* not every string's character is processed.      *
*                                                 *
* Normal writev will jump over null pointed       *
* io_vec.iov_base although this is undocumented   *
* behavior. However, here null vec.iov_base       *
* indicates a formatting. Then iov_len is         *
* format_type which tells what type is turned     *
* into string. Ignore behavior happens with       *
* format_type equaling IO_VEC_IGNORE.             *
**************************************************/
typedef struct IoVec{
	void *iov_base;
	union{
		size_t iov_len;
		enum{IO_VEC_IGNORE=0,IO_VEC_INT_TO_DEC} format_type;
	};
}IoVec;
int writevf(const int fd,IoVec *vec,int vecslen,...);

#endif /* _PRINT_TOOLS_H_ */
