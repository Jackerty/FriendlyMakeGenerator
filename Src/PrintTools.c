/***************************************************
* See PrintTools.h for module description.         *
***************************************************/
#include<unistd.h>
#include<sys/uio.h>
#include<string.h>
#include<stdarg.h>
#include"PrintTools.h"

	/*******************
	* See PrintTools.h *
	*******************/
	int printStrCat(const int fd,const char *str1,const char *str2,const int str1len,const int str2len){
		// Test that POSIX.1-2001 is in use.
		#if _POSIX_C_SOURCE>=200112L
			// Use system call writev so we don't need to copy to buffer.
			const struct iovec io[2]={{(char*)str1,str1len},{(char*)str2,str2len}};
			return writev(fd,io,sizeof(io)/sizeof(struct iovec));
		#else
			// Just create the full buffer since this would not be used with
			// long files where we would care about optimal buffer size.
			const size_t iobufferlen=str1len+str2len;
			char iobuffer[iobufferlen];
			memcpy(iobuffer,str1,str1len);
			memcpy(iobuffer+str1len,str2,str2len);
			return write(fd,iobuffer,iobufferlen);
		#endif /* _POSIX_C_SOURCE */
	}
	/*******************
	* See PrintTools.h *
	*******************/
	int printStrCat3(const int fd,const char *str1,const char *str2,const char *str3,const int str1len,const int str2len,const int str3len){
		#if _POSIX_C_SOURCE>=200112L
			const struct iovec io[]={{(char*)str1,str1len},{(char*)str2,str2len},{(char*)str3,str3len}};
			return writev(fd,io,sizeof(io)/sizeof(struct iovec));
		#else
			const size_t iobufferlen=str1len+str2len+str3len;
			char iobuffer[iobufferlen];
			memcpy(iobuffer,str1,str1len);
			memcpy(iobuffer+str1len,str2,str2len);
			memcpy(iobuffer+str1len+str2len,str3,str3len);
			return write(fd,iobuffer,iobufferlen);
		#endif /* _POSIX_C_SOURCE */
	}
	/*******************
	* See PrintTools.h *
	*******************/
	int printStrCat4(const int fd,const char *str1,const char *str2,const char *str3,const char *str4,const int str1len,const int str2len,const int str3len,const int str4len){
		#if _POSIX_C_SOURCE>=200112L
			const struct iovec io[]={{(char*)str1,str1len},{(char*)str2,str2len},{(char*)str3,str3len},{(char*)str4,str4len}};
			return writev(fd,io,sizeof(io)/sizeof(struct iovec));
		#else
			const size_t iobufferlen=str1len+str2len+str3len+str4len;
			char iobuffer[iobufferlen];
			memcpy(iobuffer,str1,str1len);
			memcpy(iobuffer+str1len,str2,str2len);
			memcpy(iobuffer+str1len+str2len,str3,str3len);
			memcpy(iobuffer+str1len+str2len+str3len,str4,str4len);
			return write(fd,iobuffer,iobufferlen);
		#endif /* _POSIX_C_SOURCE */
	}
	/*******************
	* See PrintTools.h *
	*******************/
	int printStrCat5(const int fd,const char *str1,const char *str2,const char *str3,const char *str4,const char *str5,const int str1len,const int str2len,const int str3len,const int str4len,const int str5len){
		#if _POSIX_C_SOURCE>=200112L
			const struct iovec io[]={{(char*)str1,str1len},{(char*)str2,str2len},{(char*)str3,str3len},{(char*)str4,str4len},{(char*)str5,str5len}};
			return writev(fd,io,sizeof(io)/sizeof(struct iovec));
		#else
			const size_t iobufferlen=str1len+str2len+str3len+str4len+str5len;
			char iobuffer[iobufferlen];
			memcpy(iobuffer,str1,str1len);
			memcpy(iobuffer+str1len,str2,str2len);
			memcpy(iobuffer+str1len+str2len,str3,str3len);
			memcpy(iobuffer+str1len+str2len+str3len,str4,str4len);
			memcpy(iobuffer+str1len+str2len+str3len+str4len,str5,str5len);
			return write(fd,iobuffer,iobufferlen);
		#endif /* _POSIX_C_SOURCE */
	}
	/*******************
	* See PrintTools.h *
	*******************/
	int itostrlen(int number,const int base){
		// Special handle zero.
		if(number==0) return 1;
		else{
			// Returning length of the string.
			int32_t strlength;
			// Strings length of the number is number of times base divades it.
			for(strlength=0;number!=0;strlength++,number/=base);
			return strlength;
		}
	}
	/*******************
	* See PrintTools.h *
	*******************/
	void itostr(int number,const int base,char *buffer,const int strlength){
		// Special handle zero.
		if(number==0) buffer[0]='0';
		else{
			// Buffer index.
			int i=strlength-1;
			while(number!=0){
				int remainder=number%base;
				buffer[i--]=(remainder>9)?(remainder-10)+'a':remainder+'0';
				number=number/base;
			}
		}
	}
	/*******************
	* See PrintTools.h *
	*******************/
	int writevf(const int fd,IoVec *vec,int vecslen,...){
		// Initlize va_list
		va_list vaargs;
		va_start(vaargs,vecslen);

		// Additional character buffer.
		// NOTE:Hope this is enough.
		char buffer[4096];
		int bufferposition=0;

		// Go through vec and handle formatting points.
		// There is notting to be done if caller gives
		// wrong formatting types or not enough of them.
		for(int32_t i=0;i<vecslen;i++){
			if(vec[i].iov_base==NULL){
				switch(vec[i].format_type){
					case IO_VEC_INT_TO_DEC:
						int var=va_arg(vaargs,int);
						// Interger to dec string exchanges.
						vec[i].iov_len=itostrlen(var,10);
						vec[i].iov_base=buffer+bufferposition;
						itostr(var,10,vec[i].iov_base,vec[i].iov_len);
						// Move buffer position.
						bufferposition+=vec[i].iov_len;
						break;
					default:
						vec[i].iov_len=0;
					case IO_VEC_IGNORE:
						break;
				}
			}
		}
		va_end(vaargs);
		#if _POSIX_C_SOURCE>=200112L
			return writev(fd,(struct iovec*)vec,vecslen);
		#else
			#error Make non writev implementation.
		#endif /* _POSIX_C_SOURCE */
	}
