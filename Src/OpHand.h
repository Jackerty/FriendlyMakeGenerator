/********************************************************************
* This module is general function for option handling.              *
********************************************************************/
#ifndef _OP_HAND_H_
#define _OP_HAND_H_

#include<stdint.h>

/*********************************************************
* Macro for checking does given option flag ask for      *
* argument.                                              *
*********************************************************/
#define HAS_ARGUMENT(A) A.argument
/*********************************************************
* List of types of flags OptionFlag structure member     *
* type can have.                                         *
* OPHAND_VALUE set 32 bit integer to constant given or   *
* to argument.                                           *
* OPHAND_POINTER_VALUE sets a pointer to constant given  *
* or points to argument as a string.                     *
* OPHAND_FUNCTION calls given function to handle the     *
* option.                                                *
* OPHAND_OR performs OR operation to 32 bit variable.    *
* OPHAND_AND performs AND operation to 32 bit variable.  *
* OPHAND_PRINT prints constant message.                  *
*********************************************************/
#define OPHAND_VALUE 0b01
#define OPHAND_POINTER_VALUE 0b10
#define OPHAND_FUNCTION 0b11
#define OPHAND_OR 0b100
#define OPHAND_AND 0b101
#define OPHAND_PRINT 0b110

/*********************************************************
* Return value of the ophand.                            *
* Designed so that zero or one is returned on SUCCESS.   *
* OPHAND_PROCESSING_STOPPED differs from                 *
* OPHAND_PROCESSING_DONE by indicating stop bit was      *
* used to stop the processing.                           *
*********************************************************/
typedef enum OpHandReturn{
	OPHAND_PROCESSING_DONE=0,
	OPHAND_PROCESSING_STOPPED=1,
	OPHAND_UNKNOW_OPTION=2,
	OPHAND_NO_ARGUMENT=3,
}OpHandReturn;
/*********************************************************
* Macros for the option flags.                           *
*                                                        *
* Members:                                               *
*   argument bit tells does option have argument.        *
*   stop     bit tells should parsing to be stoped after *
*            this option.                                *
*   type     bit tells what operation is performed. Look *
*            macros up for operation that can be         *
*            performed.                                  *
*********************************************************/
typedef struct OptionFlag{
  uint8_t argument : 1;
  uint8_t stop : 1;
  uint8_t type : 6;
}OptionFlag;
/*********************************************************
* Type for the function call if argument is hit.         *
* Programmer should send 1 if OptFunction doesn't cause  *
* error and 0 if error happened so that opHand can       *
* stop.                                                  *
*********************************************************/
typedef uint8_t (*OptFunction)(char option,void *restrict coderdata,const char *restrict arg);
/*********************************************************
* Structure declaring option for ophand function.        *
*********************************************************/
typedef struct Option{
	char *longoption;
	union{
		int32_t *p32;
		char **str;
		OptFunction func;
		const char *printstr;
	}variable;
	union{
		int32_t *p32;
		void *coderdata;
		int32_t v32;
		char *str;
	}value;
	char option;
	OptionFlag flags;
}Option;

/*********************************************************
* Function performs the option handling. Returns 1 if    *
* everything went fine. Zero if error occured.           *
* Non-options arguments are put to args (will override!) *
* and null ending tells the end. If "--" is encountered  *
* then opHands execution returns to caller.              *
*                                                        *
* NOTE I: argn and optionslen aren't sanity checked so   *
* better put right size in.                              *
* NOTE II: args or options aren't null check so segment  *
* faults are on you!                                     *
*********************************************************/
OpHandReturn opHand(int argn,char *restrict *restrict args,const Option *restrict options,uint32_t optionslen);
#endif /* _OP_HAND_H_ */
