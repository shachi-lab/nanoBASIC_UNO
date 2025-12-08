/*
 * NanoBASIC UNO
 * ------------------------------------------
 * A minimal BASIC interpreter for 8-bit AVR.
 *
 * Original concept and implementation:
 *    shachi-lab
 *
 * Based on the first STM8S prototype (2012),
 * reconstructed and refined for Arduino UNO.
 *
 * This interpreter keeps a minimalist design:
 *   - Program lines are stored in input order
 *   - Line numbers act as jump labels
 *   - No automatic renumbering
 *   - Recursive-descent expression evaluator
 *
 * Designed with the philosophy:
 *   "Modern BASIC for small microcontrollers."
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#include <Arduino.h>
#include "nano_basic_uno.h"
#include "nano_basic_uno_conf.h"
#include "nano_basic_defs.h"
#include "bios_uno.h"

#define	inputChar()					(char)bios_SerialGetChar()
#define printChar( ch )			bios_SerialPutChar((uint8_t)ch)

#ifndef FPSTR
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#endif

char RawLine[RAW_LINE_SIZE];
uint8_t InternalcodeLine[CODE_LINE_SIZE];
int16_t globalVariables[VARIABLE_NUM];
int16_t arrayValiables[ARRAY_INDEX_NUM];
stack_t stacks[STACK_NUM];
uint16_t lineNumber;
uint8_t *executionPointer;
uint8_t errorCode;
uint8_t returnRequest;
uint8_t stackPointer;
uint8_t *dataReadPointer;
uint8_t *resumePointer;
uint16_t resumeLineNumber;

#define PROGRAM_AREA_SIZE		512
uint8_t	programArea[PROGRAM_AREA_SIZE];
#define PROGRAM_AREA_TOP 		programArea

void basicMain( void );
char *inputString( void );
uint8_t convertInternalCode( uint8_t *dst, char *src );
void interpreterMain( void );

void proc_print( void );
void proc_input( void );
void proc_goto( void );
void proc_gosub( void );
void proc_return( void );
void proc_for( void );
void proc_next( void );
void proc_do( void );
void proc_loop( void );
void proc_while( void );
void proc_if( void );
void proc_else( void );
void proc_elseif( void );
void proc_endif( void );
void proc_run( void );
void proc_resume( void );
void proc_stop( void );
void proc_end( void );
void proc_new( void );
void proc_list( void );
void proc_prog( void );
void proc_comment( void );
void proc_outp( void );
void proc_delay( void );
void proc_pause( void );
void proc_exit( void );
void proc_continue( void );
void proc_randomize( void );
void proc_data( void );
void proc_read( void );
void proc_restore( void );
void proc_pwm( void );

uint8_t *proc_exit_cont_sub( uint8_t ercode );
void proc_let_valiable( int16_t *pvar );
uint8_t proc_let( int16_t *pvar, uint8_t ope );

int16_t *getArrayReference( void );
int16_t calcValue( void );
int16_t expr4th( void );
int16_t expr3nd( void );
int16_t expr2nd( void );
int16_t expr( void );
void initializeValiables( void );
int16_t calcValueFunc( void );
void printInternalcode( void );
void printError( void );
void executeBreak( void );
uint8_t chkST( uint8_t ch );
uint8_t *findST( uint8_t st1, uint8_t st2, uint8_t st3, uint16_t *lnum );
int8_t checkBreakKey( void );

void printVal( int16_t val );
void printString( char *str );
void printStringFlash( const __FlashStringHelper* ifsh );

char *conv2str( int16_t para, uint8_t ff, int8_t len );

const char newlineStr[] PROGMEM = "\r\n";

typedef void (*PROC)( void );

const PROC procCodeList[] = {
	proc_print		,	// 0x80 : ST_PRINT
	proc_input		,	// 0x81 : ST_INPUT
	proc_goto			,	// 0x82 : ST_GOTO
	proc_gosub		,	// 0x83 : ST_GOSUB
	proc_return		,	// 0x84 : ST_RETURN
	proc_for			,	// 0x85 : ST_FOR
	proc_next			,	// 0x86 : ST_NEXT
	proc_do				,	// 0x87 : ST_DO
	proc_loop			,	// 0x88 : ST_LOOP
	proc_while		,	// 0x89 : ST_WHILE
	proc_if				,	// 0x8a : ST_IF
	proc_run			,	// 0x8b : ST_RUN
	proc_resume		,	// 0x8c : ST_RESUME
	proc_stop			,	// 0x8d : ST_STOP
	proc_end			,	// 0x8e : ST_END
	proc_new			,	// 0x8f : ST_NEW
	proc_list			,	// 0x90 : ST_LIST
	proc_prog			,	// 0x91 : ST_PROG
	proc_delay		,	// 0x92 : ST_DELAY
	proc_pause		,	// 0x93 : ST_PAUSE
	proc_exit			,	// 0x94 : ST_EXIT
	proc_continue	, // 0x95 : ST_CONTINUE
	proc_randomize, // 0x96 : ST_RONDOMIZE
	proc_data			,	// 0x97 : ST_DATA
	proc_read			,	// 0x98 : ST_READ
	proc_restore	,	// 0x99 : ST_RESTORE
	proc_outp			,	// 0x9a : ST_OUTP
	proc_pwm			,	// 0x9b : ST_PWM
	proc_else			,	// 0x9c : ST_ELSE
	proc_elseif		,	// 0x9d : ST_ELSEIF
	proc_endif		,	// 0x9e : ST_ENDIF
};

const char token_st_80[] PROGMEM = "PRINT"		;	// 0x80 : ST_PRINT
const char token_st_81[] PROGMEM = "INPUT"		;	// 0x81 : ST_INPUT
const char token_st_82[] PROGMEM = "GOTO"			;	// 0x82 : ST_GOTO
const char token_st_83[] PROGMEM = "GOSUB"		;	// 0x83 : ST_GOSUB
const char token_st_84[] PROGMEM = "RETURN"		;	// 0x84 : ST_RETURN
const char token_st_85[] PROGMEM = "FOR"			;	// 0x85 : ST_FOR
const char token_st_86[] PROGMEM = "NEXT"			;	// 0x86 : ST_NEXT
const char token_st_87[] PROGMEM = "DO"				;	// 0x87 : ST_DO
const char token_st_88[] PROGMEM = "LOOP"			;	// 0x88 : ST_LOOP
const char token_st_89[] PROGMEM = "WHILE"		;	// 0x89 : ST_WHILE
const char token_st_8a[] PROGMEM = "IF"				;	// 0x8a : ST_IF
const char token_st_8b[] PROGMEM = "RUN"			;	// 0x8b : ST_RUN
const char token_st_8c[] PROGMEM = "RESUME"		;	// 0x8c : ST_RESUME
const char token_st_8d[] PROGMEM = "STOP"			;	// 0x8d : ST_STOP
const char token_st_8e[] PROGMEM = "END"			;	// 0x8e : ST_END
const char token_st_8f[] PROGMEM = "NEW"			;	// 0x8f : ST_NEW
const char token_st_90[] PROGMEM = "LIST"			;	// 0x90 : ST_LIST
const char token_st_91[] PROGMEM = "PROG"			;	// 0x91 : ST_PROG
const char token_st_92[] PROGMEM = "DELAY"		;	// 0x92 : ST_DELAY
const char token_st_93[] PROGMEM = "PAUSE"		;	// 0x93 : ST_PAUSE
const char token_st_94[] PROGMEM = "EXIT"			;	// 0x94 : ST_EXIT
const char token_st_95[] PROGMEM = "CONTINUE"	;	// 0x95 : ST_CONTINUE
const char token_st_96[] PROGMEM = "RANDOMIZE";	// 0x96 : ST_RONDOMIZE
const char token_st_97[] PROGMEM = "DATA"			;	// 0x97 : ST_DATA
const char token_st_98[] PROGMEM = "READ"			;	// 0x98 : ST_READ
const char token_st_99[] PROGMEM = "RESTORE"	;	// 0x99 : ST_RESTORE
const char token_st_9a[] PROGMEM = "OUTP"			;	// 0x9a : ST_OUTP
const char token_st_9b[] PROGMEM = "PWM"			;	// 0x9b : ST_PWM
const char token_st_9c[] PROGMEM = "ELSE"			;	// 0x9c : ST_ELSE
const char token_st_9d[] PROGMEM = "ELSEIF"		;	// 0x9d : ST_ELSEIF
const char token_st_9e[] PROGMEM = "ENDIF"		;	// 0x9e : ST_ENDIF
const char token_st_9f[] PROGMEM = "THEN"			;	// 0x9f : ST_THEN
const char token_st_a0[] PROGMEM = "TO"				;	// 0xa0 : ST_TO
const char token_st_a1[] PROGMEM = "STEP"			;	// 0xa1 : ST_STEP
const char token_fn_a2[] PROGMEM = "RND"			;	// 0xa2 : FUNC_RND
const char token_fn_a3[] PROGMEM = "ABS"			;	// 0xa3 : FUNC_ABS
const char token_fn_a4[] PROGMEM = "INP"  		;	// 0xa4 : FUNC_INP
const char token_fn_a5[] PROGMEM = "ADC"  		;	// 0xa5 : FUNC_ADC
const char token_fn_a6[] PROGMEM = "CHR"			;	// 0xa6 : FUNC_CHR
const char token_va_a7[] PROGMEM = "TICK"			;	// 0xa7 : VAL_TICK
const char token_va_a8[] PROGMEM = "INKEY"		;	// 0xa8 : VAL_INKEY

const char * const keyWordList[] PROGMEM = {
  token_st_80, token_st_81, token_st_82, token_st_83,token_st_84, token_st_85, token_st_86, token_st_87,
  token_st_88, token_st_89, token_st_8a, token_st_8b,token_st_8c, token_st_8d, token_st_8e, token_st_8f,
  token_st_90, token_st_91, token_st_92, token_st_93,token_st_94, token_st_95, token_st_96, token_st_97,
  token_st_98, token_st_99, token_st_9a, token_st_9b,token_st_9c, token_st_9d, token_st_9e, token_st_9f,
  token_st_a0, token_st_a1, 
  token_fn_a2, token_fn_a3, token_fn_a4, token_fn_a5,token_fn_a6,
  token_va_a7, token_va_a8, 
	NULL
};

const char error00[] PROGMEM = "";										// 00 : No error
const char error01[] PROGMEM = "Syntax";							// 01 : ERROR_SYNTAX
const char error02[] PROGMEM = "Division by 0";				// 02 : ERROR_DIVZERO
const char error03[] PROGMEM = "Array index over";		// 03 : ERROR_ARRAY
const char error04[] PROGMEM = "Parameter";						// 04 : ERROR_PARA
const char error05[] PROGMEM = "Stack overflow";			// 05 : ERROR_STACK
const char error06[] PROGMEM = "Can't resume";				// 06 : ERROR_RESUME
const char error07[] PROGMEM = "Label not found";			// 07 : ERROR_LABEL
const char error08[] PROGMEM = "Unless from pg-mode";	// 08 : ERROR_PRGMODE
const char error09[] PROGMEM = "PG area overflow";		// 09 : ERROR_PGOVER
const char error10[] PROGMEM = "Loop nothing";				// 10 : ERROR_NOLOOP
const char error11[] PROGMEM = "Endif not found";			// 11 : ERROR_NOENDIF
const char error12[] PROGMEM = "Next";								// 12 : ERROR_UXNEXT
const char error13[] PROGMEM = "Return";							// 13 : ERROR_UXRETURN
const char error14[] PROGMEM = "Loop";								// 14 : ERROR_UXLOOP
const char error15[] PROGMEM = "Exit";								// 15 : ERROR_UXEXIT
const char error16[] PROGMEM = "Continue";						// 16 : ERROR_UXCONTINUE

const char * const errorSting[] PROGMEM = {
	error00, error01, error02, error03, error04, error05, error06, error07,
	error08, error09, error10, error11, 
	error12, error13, error14, error15, error16
};

#define IS_OPERATOR_CHR( n )	((n)=='+' || (n)=='-' || (n)=='*' || (n)=='/' || (n)=='%' || (n)=='|' || (n)=='&' || (n)=='^')
#define IS_VALID_CHR( n )			((n)<0x3f || (n)=='^' || (n)=='|' || (n)=='~' || (n)=='[' || (n)==']')
#define IS_DIVZERO( val )			(val ? 0 : (errorCode = ERROR_DIVZERO))

//*************************************************
void basicInit( void )
{
	bios_init();
	initializeValiables();
	printStringFlash( F( "\r\n" NAME_STR " " VERSION_STR "\r\n") );
}

//*************************************************
void basicMain( void )
{
	uint8_t	ret;

	lineNumber = 0;
	errorCode = 0;
	returnRequest = 0;
	printStringFlash( F("OK\r\n") );
	while( true ){
		if( inputString() == NULL ){
			printError();
			return;
		}
		ret = convertInternalCode( InternalcodeLine, RawLine );
		if( errorCode ){
			printError();
			return;
		}
		if( ret > 1 ){
			executionPointer = InternalcodeLine;
#if CODE_DEBUG_ENABLE
			printInternalcode();
#endif
			interpreterMain();
			return;
		}
	}
}

#if CODE_DEBUG_ENABLE
//*************************************************
void printInternalcode( void )
{
	uint8_t len, ch, *ptr;

	ptr = executionPointer;
	len = *ptr;
	if( len > 0 ){
		len++;
		while( len-- ){
			ch = *ptr++;
			printString(conv2str( ch, FORM_HEX, -2 ));
			printChar( 0x20 );
		}
		printStringFlash( FPSTR(newlineStr) );
	}
}
#endif

//*************************************************
void initializeValiables( void )
{
	stackPointer = 0;
	resumePointer = NULL;
	resumeLineNumber = 0;
	dataReadPointer = 0;
	memset( globalVariables, 0, sizeof(globalVariables));
	memset( arrayValiables , 0, sizeof(arrayValiables ));
}

//*************************************************
void printError( void )
{
	if( errorCode != 0 ){
		if( errorCode == ERROR_BREAK ){
			printStringFlash( F("\r\nBreak") );
		}else{
			printStringFlash( FPSTR(newlineStr) );
			if( errorCode >= ERROR_UXNEXT ){
				printStringFlash( F("Unexpected ") );
			}
			if(errorCode > ERROR_CODE_MAX) errorCode = ERROR_CODE_MAX;
			PGM_P p = (PGM_P)pgm_read_ptr(&errorSting[errorCode]);
			printStringFlash( FPSTR(p) );
			printStringFlash( F(" error") );
		}
		if( lineNumber ){
			printStringFlash( F(" in ") );
			printVal( lineNumber );
		}
	}
		printStringFlash( FPSTR(newlineStr) );
}

//*************************************************
void executeBreak( void )
{
	if( lineNumber ){
		resumePointer = executionPointer;
		resumeLineNumber = lineNumber;
	}
	errorCode = ERROR_BREAK;
}

//*************************************************
void interpreterMain( void )
{
	uint8_t	ch;
	int16_t *pvar;

	while( true ){
		ch = *executionPointer++;
		if( ch == ST_EOL || returnRequest == REQUEST_EXIT ){
			if( lineNumber ){
				resumePointer = NULL;
			}
			return;
		}
		ch = *executionPointer;
		if( ch == ST_DECVAL && lineNumber ){
			executionPointer += 3;
		}
		while( true ) {
			if( checkBreakKey() < 0 ){
				printError();
				return;
			}
			returnRequest = 0;
			ch = *executionPointer++;
			if( ch == ST_EOL ){
				if( lineNumber == 0 ){
					return;
				}
				lineNumber++;
				break;
			} else
			if( ch == ' ' || ch == '\t' || ch == ':' ) {
				/* nop */
			} else
			if( ch == ST_ARRAY ) {
				pvar = getArrayReference();
				if( pvar == NULL ) {
					printError();
					return;
				}
				proc_let_valiable( pvar );
			} else
			if( isupper( ch ) ){
				ch -= 'A';
				proc_let_valiable( &globalVariables[ch] );
			} else
			if( ch == ST_COMMENT ){
				proc_comment();
			} else
			if( ch >= STCODE_START && ch <= STCODE_END ) {
				ch -= STCODE_START;
				(*procCodeList[ch])();
			} else {
				errorCode = ERROR_SYNTAX;
			}
			if( errorCode ){
				printError();
				return;
			}
			if( returnRequest ){
				break;
			}
		}
	}
}

//*************************************************
char *inputString( void )
{
	char ch, *str;
	uint8_t len = 0;

	str = RawLine;
	while( true ) {
		ch = inputChar();
		switch( ch ){
		case 0x03 :
			executeBreak();
			return NULL;
		case ASCII_CR :
			str[len] = 0;
			printChar( ch );
			printChar( ASCII_LF );
			return RawLine;
		case ASCII_BS :
			if( len > 0 ){
				len--;
				printChar( ch );
			}
			break;
		default :
			if( ch >= 0x20 && len < RAW_LINE_SIZE ){
				str[len++] = ch;
				printChar( ch );
			}
		}
	}
}

//*************************************************
uint8_t hex2byte( char ch )
{
	if( ch >= '0' && ch <= '9' ){
		return( ch - 0x30 );
	}
	if( ch >= 'A' && ch <= 'F' ){
		return( ch - 0x37 );
	}
	if( ch >= 'a' && ch <= 'f' ){
		return( ch - 0x57 );
	}
	return( 0x10 );
}

//*************************************************
int16_t str2int16_t( char *str )
{
	int16_t	val;
	char ch, flag;

	val = 0;
	flag = 0;
	while( (ch = *str) <= 0x20 ){
		if( ch == 0x00 ){
			return val;
		}
		str++;
	}
	if( ch == '-' ){
		flag = ch;
		ch = *++str;
	}
	if( ch == ST_HEXCHR ){
		while( (ch = hex2byte( *++str ) ) < 0x10 ){
			val = (val << 4) + ch;
		}
	}else{
		while( isdigit( ch )){
			val = val * 10 + (ch - '0');
			ch = *++str;
		}
	}
	if( flag ){
		val = -val;
	}
	return val;
}

//*************************************************
void printVal( int16_t val )
{
	printString( conv2str( val, 0, 0 ) );
}

//*************************************************
void printString( char *str )
{
	while( *str != 0 )
		printChar( *str++ );
}

//*************************************************
void printStringFlash(const __FlashStringHelper* ifsh)
{
    PGM_P p = (PGM_P)ifsh;   // Flashアドレスに変換
    char c;
    while ((c = pgm_read_byte(p++)) != 0) {
        printChar( c );
    }
}

//*************************************************
uint8_t convertInternalCode( uint8_t *dst, char *src )
{
	char ch, cx, cc, *s2;
	uint8_t	len, intcode, *topptr;
	uint16_t val;

	topptr = dst++;
	len = 0;
	while( true ){
		while((ch = *src) <= 0x20){
			if( *src == 0 ){
				*dst++ = ST_EOL;
				len++;
				*topptr = len;
				return( len );
			}
			src++;
		}
		ch = toupper( ch );
		cx = toupper( src[1] );
		if( ch == '?' ){
			*dst++ = ST_PRINT;
			src++;
			len++;
		}else
		if( isupper( ch ) ){
			if( !isupper( cx ) ){
				*dst++ = ch;
				src++;
				len++;
			}else{
				uint8_t index = 0;
				intcode = TOKEN_START;
				while( true ){
						PGM_P s1 = (PGM_P)pgm_read_ptr( &keyWordList[index] );
						if( s1 == NULL ){
						errorCode = ERROR_SYNTAX;
						return( 0 );
					}
					s2 = src;
					while( true ){
						ch = toupper( *s2 );
						cc = pgm_read_byte( s1 );
						if( ch != cc || cc == 0 ){
							break;
						}
						s1++;
						s2++;
					}
					if( cc == 0 && !isupper(ch) ){
						src = s2;
						*dst++ = intcode;
						len++;
						break;
					}
					intcode++;
					index++;
				}
			}
		}else
		if( isdigit( ch ) ){
			val = 0;
			do{
				val = val * 10 + (ch - '0');
				ch = *++src;
			}while( isdigit( ch ) );
			if( val == 0 ){
				*dst++ = VAL_ZERO;
				len++;
			}else
			{
				*dst++ = ST_DECVAL;
				*((uint16_t*)dst) = val;
				dst += 2;
				len += 3;
			}
		}else
		if( ch == ST_HEXCHR ){
			if( isxdigit( *++src ) ) {
				val = 0;
				while( (ch = hex2byte( *src ) ) < 0x10 ) {
					val = (val << 4) + ch;
					src++;
				}
				*dst++ = ST_HEXVAL;
				*((uint16_t*)dst) = val;
				dst += 2;
				len += 3;
			}else
			{
				*dst++ = ch;
				len++;
			}
		}else
		if( ch == ST_STRING ){
			do{
				len++;
				*dst++ = ch;
				ch = *++src;
				if( ch < 0x20 ){
					errorCode = ERROR_SYNTAX;
					return( 0 );
				}
			}while( ch != ST_STRING );
			*dst++ = ch;
			src++;
			len++;
		}else
		if( ch == ST_COMMENT ){
			while( ch >= 0x20 ){
				len++;
				*dst++ = ch;
				ch = *++src;
			}
		}else
		if( ch == ST_ARRAY && cx == '[' ){
			*dst++ = ch;
			src++;
			len++;
		}else
		if( IS_VALID_CHR(ch) ){
			*dst++ = ch;
			src++;
			len++;
		}else{
			errorCode = ERROR_SYNTAX;
			return( 0 );
		}
	}
}

//*************************************************
uint8_t *label2exeptr( int16_t val )
{
	uint8_t ch, *ptr;
	int16_t	dec, lnum;

	lnum = 1;
	ptr = (uint8_t*)PROGRAM_AREA_TOP;
	while( true ){
		ch = *ptr++;
		if( ch == ST_EOL ){
			return( NULL );
		}
		ch = *ptr++;
		if( ch == ST_DECVAL ){
			dec = *((int16_t*)ptr);
			if( dec == val ){
				executionPointer = ptr - 2;
				lineNumber = lnum;
				return( ptr );
			}
			ptr += 2;
		}
		while( ch != ST_EOL ){
			if( ch == ST_DECVAL || ch == ST_HEXVAL ){
				ptr += 2;
			}
			ch = *ptr++;
		}
		lnum++;
	}
}

//*************************************************
uint8_t isDelimiter( uint8_t ch )
{
	return( ch == ':' || ch == ST_EOL || ch == ST_ELSE || ch == ST_ENDIF );
}

//*************************************************
uint8_t checkDelimiter( void )
{
	if( !isDelimiter( *executionPointer ) ){
		errorCode = ERROR_SYNTAX;
	}
	return( errorCode );
}

//*************************************************
int16_t *getParameterPointer( void )
{
	uint8_t ch;

	ch = *executionPointer++;
	if( ch == ST_ARRAY ) {
		return( getArrayReference() );
	} else
	if( isupper( ch ) ){
		return( &globalVariables[ch-'A'] );
	}
	errorCode = ERROR_SYNTAX;
	return( NULL );
}

//*************************************************
uint8_t chkST( uint8_t ch )
{
	if( *executionPointer++ != ch ){
		return( errorCode = ERROR_SYNTAX );
	}
	return( 0 );
}

//*************************************************
uint8_t *findST( uint8_t st1, uint8_t st2, uint8_t st3, uint16_t *lnum )
{
	uint8_t ch, count_if, *ptr;
	uint16_t	num;

	count_if = 0;
	ptr = executionPointer;
	num = *lnum;
	while( true ){
		do{
			ch = *ptr++;
			switch( ch ){
			case ST_DECVAL :
			case ST_HEXVAL :
				ptr += 2;
				break;
			case ST_COMMENT :
				while( *ptr++ != ST_EOL );
				ch = ST_EOL;
				break;
			case ST_STRING :
				while( *ptr++ != ST_STRING );
				break;
			case ST_IF :
				count_if++;
				break;
			case ST_ENDIF :
				if( count_if ){
					count_if--;
					break;
				}
			default :
				if( count_if == 0 ){
					if( ch == st1 || ch == st2 || ch == st3 ){
						*lnum = num;
						return( ptr );
					}
				}
			}
		}while( ch != ST_EOL );
		if( num == 0 ){
			break;
		}
		num++;
		if( *ptr++ == ST_EOL ){
			break;
		}
		if( *ptr == ST_DECVAL ){
			ptr += 3;
		}
	}
	return( NULL );
}

//*************************************************
stack_t *pushStack( uint8_t st )
{
	stack_t *prevsp;

	if( stackPointer >= STACK_NUM ) {
		errorCode = ERROR_STACK;
		return( NULL );
	}
	prevsp = &stacks[stackPointer];
	prevsp->type = st;
	prevsp->returnPointer = executionPointer;
	prevsp->returnLineNumber = lineNumber;
	stackPointer++;
	return( prevsp );
}

//*************************************************
stack_t *popStack( uint8_t st )
{
	stack_t *prevsp;

	if( stackPointer == 0 ) {
		return( NULL );
	}
	stackPointer--;
	prevsp = &stacks[stackPointer];
	if( prevsp->type != st ) {
		return( NULL );
	}
	return( prevsp );
}

//*************************************************
int8_t checkBreakKey( void )
{
	switch( inputChar() ){
		case 0x00 :
			return 0;
		case 0x03 :
			executeBreak();
			return -1;
		default :
			return 1;
	}
}

//*************************************************
static char *get_StringPara_Form( uint8_t fm )
{
	uint8_t len = 0;
	int16_t val;

	if ( chkST( '(' ) ) return NULL;
	val = expr();
	if ( errorCode ) return NULL;
	if ( *executionPointer == ',' )
	{
		executionPointer++;
		len = expr();
	}
	if ( errorCode ) return NULL;
	if ( chkST( ')' ) ) return NULL;
	return conv2str( val, fm, len );
}

//*************************************************
void proc_print( void )
{
	uint8_t ch, lastChar, *ptr, len, fm;
	int16_t	val;
	char *p;

	lastChar = 0;
	while( true ) {
		ch = *executionPointer;
		if( isDelimiter( ch )){
			if( lastChar != ';' && lastChar != ',' ){
				printStringFlash( FPSTR(newlineStr) );
			}
			return;
		}
		lastChar = ch = *executionPointer++;
		switch( ch ) {
		case ST_STRING :
			while((ch = *executionPointer++) != ST_STRING ){
				printChar( ch );
			}
			break;

		case ',':
			printChar( '\t' );

		case ';':
			break;

		case FUNC_CHR :
			val = calcValueFunc();
			if( errorCode ) return;
			if( val >= 0x100 ) {
				printChar( val >> 8 );
			}
			printChar( val );
			break;

		case '$' :
			if ( *executionPointer != '(' ) goto print_default;
			p = get_StringPara_Form( FORM_HEX );
			if( !errorCode ) printString( p );
			break;

		case VAL_ZERO :
			if ( *executionPointer != '(' ) goto print_default;
			p = get_StringPara_Form( FORM_DEC );
			if( !errorCode ) printString( p );
			break;

		default:
print_default:
			executionPointer--;
			val = expr();
			if( errorCode ){
				return;
			}
			printVal( val );
			break;
		}
	}
}

//*************************************************
void proc_input( void )
{
	uint8_t flg;
	int16_t *pvar;

	pvar = getParameterPointer();
	if( pvar == NULL )	return;

	flg = *executionPointer;
	if( flg == ST_HEXCHR ){
		executionPointer++;
	}
	if( checkDelimiter() )	return;

	if( inputString() == NULL ){
		return;
	}
	if( flg == ST_HEXCHR ){
		*pvar = RawLine[0];
	}else{
		*pvar = str2int16_t( RawLine );
	}
}

//*************************************************
uint8_t *goto_sub( void )
{
	int16_t	val;
	uint8_t *rptr;

	val = expr();
	rptr = executionPointer;
	if( errorCode ) return NULL;
	if( label2exeptr( val ) == NULL ){
		errorCode = ERROR_LABEL;
		return NULL;
	}
	returnRequest = REQUEST_GOTO;
	return( rptr );
}

//*************************************************
void proc_goto( void )
{
	goto_sub();
}

//*************************************************
void proc_gosub( void )
{
	stack_t *prevsp;

	prevsp = pushStack( ST_GOSUB );
	if( prevsp == NULL )	return;
	prevsp->returnPointer = goto_sub();
	if( errorCode ){
		stackPointer--;
	}
}

//*************************************************
void proc_return( void )
{
	stack_t *prevsp;
	prevsp = &stacks[stackPointer];

	if( checkDelimiter() )	return;
	while( true ) {
		if( stackPointer == 0 ) {
			errorCode = ERROR_UXRETURN;
			return;
		}
		stackPointer--;
		prevsp--;
		if(prevsp->type == ST_GOSUB ) break;
	}
	executionPointer = prevsp->returnPointer;
	lineNumber = prevsp->returnLineNumber;
}

//*************************************************
void proc_for( void )
{
	uint8_t ch;
	int16_t from, to, step, *pvar;
	stack_t *prevsp;

	pvar = getParameterPointer();
	if( pvar == NULL )	return;
	if( chkST( '=' ) ) return;
	from = expr();
	if( errorCode )	return;
	if( chkST( ST_TO ) ) return;
	to = expr();
	if( errorCode )	return;
	ch = *executionPointer++;
	if( ch == ST_STEP ) {
		step = expr();
		if( errorCode )	return;
	} else {
		step = 1;
		executionPointer--;	/* unget it */
	}
	prevsp = pushStack( ST_FOR );
	if( prevsp == NULL )	return;
	*pvar = from;
	prevsp->pvar = pvar;
	prevsp->limit = to;
	prevsp->step = step;
}

//*************************************************
void proc_next( void )
{
	stack_t *prevsp;

	if( checkDelimiter() )	return;
	prevsp = popStack( ST_FOR );
	if( prevsp == NULL ){
		errorCode = ERROR_UXNEXT;
		return;
	}
	if( prevsp->limit == *(prevsp->pvar) ) {
		return;
	}
	*(prevsp->pvar) += prevsp->step;
	if( prevsp->step > 0 ) {
		if( prevsp->limit < *(prevsp->pvar) ) {
			return;
		}
	} else {
		if( prevsp->limit > *(prevsp->pvar) ) {
			return;
		}
	}
	stackPointer++;
	executionPointer = prevsp->returnPointer;
	lineNumber = prevsp->returnLineNumber;
}

//*************************************************
void proc_do( void )
{
	stack_t *prevsp;

	if( checkDelimiter() ) return;
	prevsp = pushStack( ST_DO );
	if( prevsp == NULL )	return;
	prevsp->returnPointer = executionPointer - 1;
}

//*************************************************
void proc_loop( void )
{
	int16_t val;
	stack_t *prevsp;
	uint8_t ch;

	prevsp = popStack( ST_DO );
	if( prevsp == NULL ){
		errorCode = ERROR_UXLOOP;
		return;
	}
	ch = *executionPointer;
	if( ch == ST_WHILE ){
		executionPointer++;
		val = expr();
		if( checkDelimiter() ) return;
		if( !val ){
			return;
		}
	}else
	if( checkDelimiter() ) return;
	executionPointer = prevsp->returnPointer;
	lineNumber = prevsp->returnLineNumber;
}

//*************************************************
void proc_while( void )
{
	int16_t val;
	stack_t *prevsp;
	uint8_t *ptr;

	ptr = executionPointer;
	val = expr();
	if( checkDelimiter() ) return;
	if( val ) {
		prevsp = pushStack( ST_DO );
		if( prevsp == NULL )	return;
		prevsp->returnPointer = ptr - 1;
	}else{
		ptr = findST( ST_LOOP, ST_LOOP, ST_LOOP, &lineNumber );
		if( ptr == NULL ){
			errorCode = ERROR_NOLOOP;
			return;
		}
		while( !isDelimiter( *ptr ) )ptr++;;
		executionPointer = ptr;
	}
}

//*************************************************
void proc_exit( void )
{
	uint8_t *ptr;

	ptr = proc_exit_cont_sub( ERROR_UXEXIT );
	if( ptr == NULL){
		return;
	}
	stackPointer--;
	while( !isDelimiter( *ptr ) )ptr++;
	executionPointer = ptr;
}

//*************************************************
void proc_continue( void )
{
	uint8_t *ptr;

	ptr = proc_exit_cont_sub( ERROR_UXCONTINUE );
	if( ptr == NULL){
		return;
	}
	executionPointer = ptr - 1;
}

//*************************************************
uint8_t *proc_exit_cont_sub( uint8_t ercode )
{
	stack_t *prevsp;
	uint8_t *ptr, tp;

	if( stackPointer == 0 ) {
		errorCode = ercode;
//		return NULL;
	}
	if( checkDelimiter() )	return NULL;
	prevsp = &stacks[stackPointer-1];
	if( prevsp->type == ST_FOR ) tp = ST_NEXT;
	else
	if( prevsp->type == ST_DO  ) tp = ST_LOOP;
	else{
		errorCode = ercode;
		return NULL;
	}
	ptr = findST( tp, 0xff, 0xff, &lineNumber);
	if( ptr == NULL ){
		errorCode = ercode;
		return NULL;
	}
	return( ptr );
}

//*************************************************
void proc_if( void )
{
	int16_t val;
	uint8_t ch, *ptr;

	do{
		val = expr();
		if( errorCode )	return;
		if( chkST( ST_THEN )) return;
		if( val ) {
			if( *executionPointer == ST_DECVAL ) {
				proc_goto();
			}
			return;
		}
		ptr = findST( ST_ENDIF, ST_ELSE, ST_ELSEIF, &lineNumber);
		if( ptr == NULL ){
			errorCode = ERROR_NOENDIF;
			return;
		}
		executionPointer = ptr;
		ch =  *(executionPointer - 1);
	}while( ch == ST_ELSEIF );

	if( ch == ST_ELSE ){
		ch = *executionPointer;
		if( ch == ST_DECVAL ) {
			proc_goto();
		}
	}
}

//*************************************************
void proc_else( void )
{
	uint8_t *ptr;

	ptr = findST( ST_ENDIF, ST_ENDIF, ST_ENDIF, &lineNumber);
	if( ptr == NULL ){
		errorCode = ERROR_NOENDIF;
		return;
	}
	executionPointer = ptr;
}

//*************************************************
void proc_elseif( void )
{
	proc_else();
}

//*************************************************
void proc_endif( void )
{
	checkDelimiter();
}

//*************************************************
void proc_run( void )
{
	if( checkDelimiter() )	return;
	initializeValiables();
	lineNumber = 1;
	executionPointer = (uint8_t*)PROGRAM_AREA_TOP;
	returnRequest = REQUEST_GOTO;
}

//*************************************************
void proc_resume( void )
{
	if( checkDelimiter() )	return;
	if( resumePointer == NULL ) {
		errorCode = ERROR_RESUME;
		return;
	}
	executionPointer = resumePointer;
	lineNumber = resumeLineNumber;
}

//*************************************************
void proc_stop( void )
{
	if( checkDelimiter() )	return;
	executeBreak();
}

//*************************************************
void proc_end( void )
{
	if( checkDelimiter() )	return;
	returnRequest = REQUEST_EXIT;
	lineNumber = 0;
}

//*************************************************
void proc_new( void )
{
	uint8_t *ptr;

	if( checkDelimiter() )	return;
	initializeValiables();
//flash_init();
//flash_unlock();
	ptr = (uint8_t*)PROGRAM_AREA_TOP;
	*ptr = ST_EOL;
//flash_lock();
}

//*************************************************
void proc_list( void )
{
	int16_t	val;
	uint8_t	flag, ch, *ptr;

	if( checkDelimiter() )	return;
	ptr = (uint8_t*)PROGRAM_AREA_TOP;
	while( *ptr++ != ST_EOL ){
		flag = true;
		while( true ) {
			ch = *ptr++;
			if( ch == ST_EOL ) {
				printStringFlash( FPSTR(newlineStr) );
				break;
			} else
			if( ch == ST_DECVAL ) {
				val = *((int16_t*)ptr);
				printVal( val );
				if( flag ){
					printChar( 0x20 );
				}
				ptr += 2;
			} else
			if( ch == ST_HEXVAL ) {
				val = *((int16_t*)ptr);
				printChar( ST_HEXCHR );
				printString(conv2str( val, FORM_HEX, 0 ));
				ptr += 2;
			} else
			if( ch == ST_STRING ) {
				printChar( ST_STRING );
				do{
					ch = *ptr++;
					printChar( ch );
				}while( ch != ST_STRING );
			} else
			if( ch == ST_COMMENT ) {
				printChar( ST_COMMENT );
				while( *ptr != ST_EOL )
					printChar( *ptr++ );
			} else
			if( ch >= TOKEN_START ) {
				if( !flag && (ch >= STSP_START && ch <= STSP_END)) {
					printChar( 0x20 );
				}
			  PGM_P p = (PGM_P)pgm_read_ptr( &keyWordList[ch-TOKEN_START] );
				printStringFlash( FPSTR(p) );
				if( ch <= STSP_END && !isDelimiter( *ptr )){
					printChar( 0x20 );
				}
			} else {
				printChar( ch );
			}
			flag = false;
		}
	}

	int16_t progLength = (int16_t)(ptr - PROGRAM_AREA_TOP);
	if( progLength < 2 ) progLength = 0;
	printStringFlash(F("["));
	printVal(progLength);
	printStringFlash(F(" bytes]\r\n"));
}

//*************************************************
void proc_prog( void )
{
	uint8_t	len, *ptr, *src;
	uint16_t	remain;

	if( checkDelimiter() )	return;
	remain = PROGRAM_AREA_SIZE - 3;
	if( lineNumber ){
		errorCode = ERROR_PRGMODE;
		return;
	}

	ptr = (uint8_t*)PROGRAM_AREA_TOP;
	while( true ){
		errorCode = 0;
		printChar( '>' );
		if( inputString() == NULL ){
			break;
		}
		if( RawLine[0] == PROG_TERM_CHR ){
			returnRequest = REQUEST_EXIT;
			break;
		}
		len = convertInternalCode( InternalcodeLine, RawLine );
		if( remain < len ){
			errorCode = ERROR_PGOVER;
			break;
		}
		if( errorCode ){
			printError();
		} else {
			remain = remain - (len + 1);
			src = InternalcodeLine;
			do{
				*ptr++ = *src++;
			}while( len-- > 0 );
		}
	}
	*ptr++ = ST_EOL;
}

//*************************************************
void proc_comment( void )
{
	while( *executionPointer != ST_EOL )
		executionPointer++;
}

//*************************************************
void proc_outp( void )
{
	int16_t	val_1, val_2;

	val_1 = expr();
	if( errorCode )	return;
	if( chkST( ',' ) ) return;
	val_2 = expr();
	if( checkDelimiter() )	return;

	if( bios_writeGpio( val_1, val_2 ) )	{
			errorCode = ERROR_PARA;
	}
}

//*************************************************
void proc_delay( void )
{
	int16_t	val;

	val = expr();
	if( checkDelimiter() )	return;
	bios_setWaitTick( val );
	while( bios_getWaitTick() > 0 ){
		if( checkBreakKey() < 0 ) return;
	}
}

//*************************************************
void proc_pause( void )
{
	if( checkDelimiter() )	return;
	while( checkBreakKey() == 0 );
}

//*************************************************
uint8_t proc_let( int16_t *pvar, uint8_t ope )
{
	if ( ope == executionPointer[1] )
	{
		executionPointer += 2;
		if ( ope == '+' ) { if ( !checkDelimiter() ) (*pvar)++; return errorCode; }
		if ( ope == '-' ) { if ( !checkDelimiter() ) (*pvar)--; return errorCode; }
		if ( ope !='<' && ope != '>' ) return errorCode = ERROR_SYNTAX;
	} else
	if ( IS_OPERATOR_CHR( ope ) )
	{
		++executionPointer;
	}
	if ( chkST( '=' ) ) return errorCode;
	int16_t val = expr();
	if ( errorCode ) return errorCode;
	switch ( ope )
	{
	case '+' : *pvar += val; break;
	case '-' : *pvar -= val; break;
	case '*' : *pvar *= val; break;
	case '/' : if ( !IS_DIVZERO( val ) ) *pvar /= val; break;
	case '%' : if ( !IS_DIVZERO( val ) ) *pvar %= val; break;
	case '|' : *pvar |= val; break;
	case '&' : *pvar &= val; break;
	case '^' : *pvar ^= val; break;
	case '<' : *pvar <<= val; break;
	case '>' : *pvar >>= val; break;
	default  : *pvar = val; break;
	}
	return errorCode;
}

//*************************************************
void proc_let_valiable( int16_t *pvar )
{
	proc_let( pvar, *executionPointer );
}

//*************************************************
void proc_randomize( void )
{
	int16_t	val;

	val = expr();
	if( checkDelimiter() )	return;
	bios_randomize( val );
}

//*************************************************
void proc_data( void )
{
	uint8_t ch;

	while( true ){
		ch = *executionPointer;
		if( isDelimiter( ch ) ){
			return;
		}
		if( ch == ST_DECVAL || ch == ST_HEXVAL ){
			executionPointer += 2;
		}
		executionPointer++;
	}
}

//*************************************************
void proc_read( void )
{
	int16_t *pvar, val;
	uint8_t ch, *ptr, *ptrsave;

	pvar = getParameterPointer();
//	if( pvar == NULL )	return;
	if( checkDelimiter() )	return;

	ptrsave = executionPointer;
	executionPointer = ( dataReadPointer == 0 ) ? (uint8_t*)PROGRAM_AREA_TOP : dataReadPointer;

	do{
		if( *executionPointer != ',' ){
			val = lineNumber;
			ptr = findST( ST_DATA, ST_DATA, ST_DATA, (uint16_t*)&val );
			if( ptr == NULL ){
				errorCode = ERROR_PARA;
				break;
			}
			executionPointer = ptr;
		}else{
			executionPointer++;
		}
		val = expr();
		if( errorCode ){
			break;
		}
		*pvar = val;
		ch = *executionPointer;
		if( isDelimiter( ch ) || ch == ',' ){
			break;
		}
		errorCode = ERROR_PARA;
	}while( false );
	dataReadPointer = executionPointer;
	executionPointer = ptrsave;
}

//*************************************************
void proc_restore( void )
{
	if( checkDelimiter() )	return;
	dataReadPointer = 0;
}

//*************************************************
void proc_pwm( void )
{
	int16_t	val_1, val_2;

	val_1 = expr();
	if( errorCode )	return;
	if( chkST( ',' ) ) return;
	val_2 = expr();
	if( checkDelimiter() )	return;

	if( bios_setPwm( val_1, val_2 ) )	{
			errorCode = ERROR_PARA;
	}
}

//*************************************************
int16_t *getArrayReference( void )
{
	int16_t index, *pvar;

	if( chkST( '[' ) ) return NULL;
	index = expr();
	if( errorCode ){ return NULL; }
	if( index < 0 || index > ARRAY_INDEX_NUM-1 ){
		errorCode = ERROR_ARRAY;
		return NULL;
	}
	pvar = &arrayValiables[index];
	if( chkST( ']' ) ) return NULL;
	return pvar;
}

//*************************************************
int16_t calcValueFunc( void )
{
	int16_t	val;

	if( chkST( '(' ) ) return -1;
	val = expr();
	if( chkST( ')' ) ) return -1;
	return( val );
}

//*************************************************
int16_t calcValue( void )
{
	uint8_t ch;
	int16_t *pvar, val;

	ch = *executionPointer++;
	if( isupper( ch ) ){
		ch -= 'A';
		return globalVariables[ch];
	}
	if( ch == ST_ARRAY ) {
		pvar = getArrayReference();
		if( pvar == NULL ) return -1;
		return *pvar;
	}
	switch( ch ) {
	case VAL_ZERO :
		return 0;
	case ST_DECVAL :
	case ST_HEXVAL :
		val = *((int16_t *)executionPointer);
		executionPointer += 2;
		return val;
	case '(':
		val = expr();
		if( chkST( ')' ) ) break;
		return( val );
	case '-':
		return -calcValue();
	case '!' :
		return !calcValue();
	case '~' :
		return ~calcValue();
	case FUNC_RND :
		val = calcValueFunc();
		if( errorCode == 0 ){
			return bios_rand( val );
		}
		break;
	case FUNC_ABS :
		val = calcValueFunc();
		if( errorCode == 0 ){
			if( val < 0 ) val = -val;
			return val;
		}
		break;
	case FUNC_INP :
		val = calcValueFunc();
		if( errorCode == 0 ){
			val = bios_readGpio( val );
			if( val < 0 ){
				errorCode = ERROR_PARA;
			}
			return( val );
		}
		break;
	case FUNC_ADC :
		val = calcValueFunc();
		if( errorCode == 0 ){
			val = bios_readAdc( val );
			if( val < 0 ){
				errorCode = ERROR_PARA;
			}
			return( val );
		}
		break;
	case VAL_TICK :
		return bios_getSystemTick();
	case VAL_INKEY :
		return (int16_t)inputChar();
	default :
		errorCode = ERROR_SYNTAX;
	}
	return -1;
}

//*************************************************
int16_t expr4th( void )
{
	int16_t acc, val;
	uint8_t	ch;

	acc = calcValue();
	if( errorCode ){ return -1; }
	while( true ) {
		ch = *executionPointer++;
		switch( ch ) {
		case '*':
			acc = acc * calcValue();
			break;
		case '/':
			val = calcValue();
			if( !IS_DIVZERO( val ) ) {
				acc = acc / val;
			}
			break;
		case '%':
			val = calcValue();
			if( !IS_DIVZERO( val ) ) {
				acc = acc % val;
			}
			break;
		default:
			executionPointer--;
			return acc;
		}
		if( errorCode ){ return -1; }
	}
}

//*************************************************
int16_t expr3nd( void )
{
	int16_t acc;
	uint8_t	ch;

	acc = expr4th();
	if( errorCode ){ return -1; }
	while( true ) {
		ch = *executionPointer++;
		switch( ch ) {
		case '+':
			acc = acc + expr4th();
			break;
		case '-':
			acc = acc - expr4th();
			break;
		default:
			executionPointer--;
			return acc;
		}
		if( errorCode ){ return -1; }
	}
}

//*************************************************
int16_t expr2nd( void )
{
	int16_t acc, tmp;
	uint8_t	ch, ch2;

	acc = expr3nd();
	if( errorCode ){ return -1; }
	while( true ) {
		ch = *executionPointer++;
		switch( ch ) {
		case '>':
			ch2 = *executionPointer++;
			if( ch2 == '=' ) {
				tmp = expr3nd();
				acc = (acc >= tmp);		// >=	
			} else if( ch2 == ch )	{
				tmp = expr3nd();
				acc = (acc >> tmp);		// >>
			} else {
				executionPointer--;
				tmp = expr3nd();
				acc = (acc > tmp);		// >
			}
			break;
		case '<':
			ch2 = *executionPointer++;
			if( ch2 == '=' ) {
				tmp = expr3nd();
				acc = (acc <= tmp);		// <=
			} else if( ch2 == '>' ) {
				tmp = expr3nd();
				acc = (acc != tmp);		// <>
			} else if( ch2 == ch )	{
				tmp = expr3nd();
				acc = (acc << tmp);		// <<
			} else {
				executionPointer--;
				tmp = expr3nd();
				acc = (acc < tmp);		// <
			}
			break;
		case '=':
			if ( *executionPointer == ch ) executionPointer++;
			tmp = expr3nd();
			acc = (acc == tmp);		// =, ==
			break;
		default:
			executionPointer--;
			return acc;
		}
		if( errorCode ){ return -1; }
	}
}

//*************************************************
int16_t expr( void )
{
	int16_t	acc;
	uint8_t	ch;

	acc = expr2nd();
	if( errorCode ){ return -1; }
	while( true ) {
		ch = *executionPointer++;
		switch( ch ) {
		case '&' :
			if( *executionPointer == '&' ){
				executionPointer++;
				acc = acc && expr2nd();		// &&
			}
			else{
				acc = acc & expr2nd();		// &
			}
			break;
		case '|' :
			if( *executionPointer == '|' ){
				executionPointer++;
				acc = acc || expr2nd();		// ||
			}
			else{
				acc = acc | expr2nd();		// |
			}
			break;
		case '^' :
			acc = acc ^ expr2nd();			// ^
			break;
		default:
			executionPointer--;
			return acc;
		}
		if( errorCode ){ return -1; }
	}
}

//*************************************************
char *conv2str( int16_t para, uint8_t ff, int8_t len )
{
	static char str[13];
	char *s, ch , flag, fx;
	uint16_t val;
	int16_t	dot = -1;

	if ( len < 0 ) {
		ff |= FORM_ZERO;
		len = -len;
	}

	if( len > 9 )
	{
		dot = len / 100;	
		len = len % 100;
		if ( len > 10 )	len = 10;
	}

	fx = str[12] = 0;
	if ((para < 0) && (ff & FORM_FHEX) != FORM_HEX ) {
		fx = flag = '-';
		val = (uint16_t)(-para);
	} else {
		flag = ' ';
		if ( ff & FORM_PLUS ) fx = flag = '+';
		val = (uint16_t)para;
	}
	s = &str[10];
	while ( 1 )
	{
		if ( ff & FORM_HEX ) {
			ch = (val & 0x0f) + '0';
			if ( ch > '9' ) {
				ch += 0x07 + (ff & FORM_LOWER);
			}
			val >>= 4;
		} else {
			ch = ( val % 10 ) + '0';
			val /= 10;
		}
		*s-- = ch;
		if ( dot >= 0 && (--dot == 0 ) ) *s-- = '.';
		if ( len > 0 && (--len == 0) ) break;
		if ( dot < 0 && val == 0 ) break;
	}

	if ( ff & FORM_FLAG ) {
		while ( len > 0 ) {
			len--;
			*s-- = 0x20 + (ff & FORM_ZERO);
		}
		*s = flag;
		return ( s );
	}

	if ( ff & FORM_ZERO ) {
		if ( len == 0 && fx ) {
			*s = flag;
			return ( s );
		}
		while ( len > 0 ) {
			len--;
			*s-- = ( len == 0 && fx ) ? flag : '0';
		}
	} else {
		if ( fx ) {
			*s-- = flag;
			if ( len > 0 )
				len--;
		}
		while ( len > 0 ) {
			len--;
			*s-- = ' ';
		}
	}
	return (s + 1);
}
