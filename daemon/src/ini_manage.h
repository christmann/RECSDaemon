/********************************************************************
	created:	2005/10/17
	created:	17:10:2005   18:20
	filename: 	ini_manage.h
	file path:	..\INIManipulate
	file base:	ini_manage
	file ext:	h
	author:		Chanchai Jaiyutitam

	purpose:	Header for manipulate ini file

	updated:	2005/10/19
	updated by: Chanchai
*********************************************************************/

#ifndef INI_MANAGE_H
#define INI_MANAGE_H

#include <stdint.h>
#include <string>

#define COMMENT		'#'
#define INI_ERROR	-1


/************************************************************************/
/* type def                                                             */
/************************************************************************/
typedef enum
{
	REPLACE = 1/*,
	NON_REPLACE = 0*/
} REPLACE_FLAG;

/************************************************************************/
/* for record of ini content                                            */
/************************************************************************/
struct record
{
	char comments[255];
	char key[255];
	char value[255];
	struct record *next;
};

/************************************************************************/
/* for content of file                                                  */
/************************************************************************/
struct section
{
	struct record *data_first;
	struct record *data_last;
	unsigned int data_size;
	char comments[255];
	char name[255];
	struct section *next;
};
/************************************************************************/
/* for content list                                                     */
/************************************************************************/
typedef struct
{
	unsigned int section_size;
	struct section *first;
	struct section *last;
}content;

/************************************************************************/
/* main ini manage function                                             */
/************************************************************************/
/**
 * @brief for start ini manipulate file
 * @param filename
 * @return true if success
 */
bool ini_start(const char* filename);

/**
 * @brief save content to ini file(save as)
 * @return true if success
 */
bool save(void);	// save to load filebool
//static bool save_as(const char *filename);

/**
 *	function:	get_value
 *	parameter:
 *		@
 *	return:		value
 *	purpose:	Return value from INI file saved under section and key
 */
const char *get_value (const char *sec,const char *key);
/**
 *	function:	set_value
 *	parameter:
 *		@
 *	return:		true if success
 *	purpose:
 */
bool set_value  (const char *sec,const char *key,		// will auto replace
				 const char *value);

// for console display use stdio.h stdout
std::string content_string(void);
void print_section(void);						// print all only section
void print_record(char *sec,char *key);		// print selection record
void print_allrecord(char *sec);			// print all record in section

/**
 *	function: ini_end
 *	parameter:
 *		none
 *	return:	void
 *	purpose: for end ini manipulate file
 */
void ini_end(void);

/**
 *	function: get_last_error
 *	parameter:
 *		none
 *	return:	type of error
 *	purpose:	for get error
 */
//static const char *get_last_error(void);

#endif
