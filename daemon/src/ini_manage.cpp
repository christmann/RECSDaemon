/************************************************************************
	created:	2005/10/17
	created:	17:10:2005   18:20
	filename: 	ini_manage.c
	file path:	.\INIManipulate
	file base:	ini_manage
	file ext:	c
	author:		Chanchai Jaiyutitam

	purpose:	ini file management

	updated:	2005/10/19
	updated by: Chanchai
*************************************************************************/

#include "ini_manage.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

// global variable [10/18/2005]
static content *ini_content;
static char gfilename[255];
static const char *error_msg;
static REPLACE_FLAG w_flag;

static void _append(const char *sec,const char *key,			// append data to section
			 const char *value,const char *comment);
static void _ini_end(REPLACE_FLAG flag);
static int  _remove_all(struct section *sec);
static bool _save(const char *filename);
static bool _set_value (const char *sec,const char *key,		// select replace or not replace
				 const char *value,const char *comment,REPLACE_FLAG flag);
static void add_section(const char *sec,const char *comment);	// add section
static void clear(void);								// clear all content
static struct record *get_record(const struct section *sec,const char *key);	// get record
static struct section *get_section(const char *sec);		// search section
static void init_content(void);
static void trim(char *buffer);

/**
 *	@brief for load ini file to content
 *	@param filename
 *	@return	true if success
 */
static bool load(const char *filename);


//////////////////////////////////////////////////////////////////////////
//	ini start
//  [10/18/2005]
bool ini_start(const char* filename)
{
	init_content();

	error_msg = NULL;
	w_flag = REPLACE;
	//memcpy(gfilename,filename,sizeof(gfilename));
	strcpy(gfilename, filename);

	if(!load(gfilename))
	{
		error_msg = "initial parse file error";
		return false;
	};
	return true;
}

//////////////////////////////////////////////////////////////////////////
// ini end
// [10/19/2005]
static void _ini_end(REPLACE_FLAG flag)
{
	if(flag == REPLACE)
		save();
	clear();

	free(ini_content);
}

//////////////////////////////////////////////////////////////////////////
// ini end
void ini_end(void)
{
	_ini_end(REPLACE);
}

//////////////////////////////////////////////////////////////////////////
// load
//  [10/18/2005]
static bool load(const char *filename)
{
	FILE *in_stream;
	char buffer[255];
	char comments[1024];
	char current_section[255];
	char key[255];
	char value[255];
	char *pdest;
	bool sth_loaded;

	strcpy(comments,"");
	strcpy(current_section,"");
	error_msg = NULL;
	sth_loaded = false;

	if( (in_stream = fopen( filename, "r" )) != NULL )
	{
		uint32_t index_pos;
		while(fgets(buffer, (int)sizeof(buffer), in_stream) != NULL)
		{
			trim(buffer);
			switch(buffer[0])
			{
				case '[' : // section;
					pdest = strrchr(buffer,']');
					if (pdest == NULL)
					{
						fclose(in_stream);
						error_msg = "parse ini error";
						return false;
					}
					index_pos = (uint32_t)(pdest - buffer);
					memcpy(current_section,buffer + 1, (size_t)(index_pos - 1));
					current_section[index_pos - 1] = '\0';
					add_section(current_section,comments);
					strcpy(comments,"");
					sth_loaded = true;
					break;
				case '#' : // comment
				case ';' :
					if(strlen(comments) > 0)
						strcat(comments,"\n");
					strcat(comments,buffer);
					sth_loaded = true;
					break;
				case '\0': // Empty line, skip
					break;
				default : // find content
					pdest = strchr(buffer,'=');
					if (pdest == NULL)
					{
						fclose(in_stream);
						error_msg = "parse ini error";
						return false;
					}
					index_pos = (uint32_t)(pdest - buffer);
					memcpy(key,buffer, (size_t)index_pos);
					key[index_pos] = '\0';
					memcpy(value,buffer + index_pos + 1,strlen(buffer)-index_pos);

					if(strcmp(current_section,"") == 0)
					{
						fclose(in_stream);
						error_msg = "parse ini error";
						return false;
					}
					else
					{
						_append(current_section,key,value,comments);
						strcpy(comments,"");
					}
					sth_loaded = true;
					break;
			}
		}
		fclose(in_stream);
	}
	else
	{
		error_msg = "open file error";
		return false;
	}

	return sth_loaded;
}

//////////////////////////////////////////////////////////////////////////
// _save
//  [10/18/2005]
static bool _save(const char *filename)
{
	FILE *stream;
	struct section *sec = ini_content->first;
	struct record *rec;

	error_msg = NULL;

	if( (stream = fopen(filename, "w" )) == NULL )
	{
		error_msg = "open file error";
		return false;
	}

	while (sec != NULL)
	{
		if(strlen(sec->comments) != 0)
		{
			fprintf(stream,"%s\n",sec->comments);
		}
		fprintf(stream,"[%s]\n",sec->name);
		// print section content
		rec = sec->data_first;
		while(rec != NULL)
		{
			if(strlen(rec->comments) != 0)
			{
				fprintf(stream,"%s\n",rec->comments);
			}
			fprintf(stream,"%s=%s\n",rec->key,rec->value);

			rec = rec->next;
		}
		sec = sec->next;
	}
	fclose(stream);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// save to default file
// [10/18/2005]
bool save(void)
{
	return _save(gfilename);
}

//////////////////////////////////////////////////////////////////////////
// get value
// [10/19/2005]
const char *get_value(const char *sec,const char *key)
{
	struct record *result =	get_record(get_section(sec),key);
	if(result != NULL)
		return result->value;
	else
		return "";
}

//////////////////////////////////////////////////////////////////////////
// set value if exitst will be replace
// [10/20/2005]
bool set_value(const char *sec,const char *key,  const char *value)
{
	return _set_value(sec,key,value,"",REPLACE);
}

//////////////////////////////////////////////////////////////////////////
// [10/20/2005]
static bool _set_value(const char *sec,const char *key,  const char *value,const char *comment,REPLACE_FLAG flag)
{
	w_flag = flag;
	error_msg = NULL;
	_append(sec,key,value,comment);
	if(error_msg == NULL)
		return false;
	else
		return true;
}

/*
//////////////////////////////////////////////////////////////////////////
// get last error
// [10/20/2005]
static const char *get_last_error(void)
{
	return error_msg;
}
*/

/************************************************************************/
/* helper function section                                              */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
// init list of ini file
//  [10/18/2005]
static void init_content(void)
{
	ini_content = (content *)malloc(sizeof(content));

	if(ini_content == NULL)
	{
		error_msg = "cannot malloc memory !";
		return;
	}

	ini_content->section_size = 0;
	ini_content->first = NULL;
	ini_content->last = NULL;
}

//////////////////////////////////////////////////////////////////////////
// add section
//  [10/18/2005]
static void add_section(const char *sec,const char *comment)
{
	struct section *temp;
	temp = get_section(sec);

	error_msg = NULL;

	if(temp == NULL)
	{
		temp = (struct section *)malloc(sizeof(struct section));

		if(temp == NULL)
		{
			error_msg = "cannot malloc memory !";
			return;
		}

		// for section name
		strcpy(temp->name,sec);

		if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
			sprintf(temp->comments,"#%s",comment);
		else
			strcpy(temp->comments,comment);

		// for data link
		temp->data_first = NULL;
		temp->data_last = NULL;
		temp->next = NULL;
		temp->data_size = 0;

		// increment section size
		ini_content->section_size++;

		// for content link
		if (ini_content->first == NULL)
		{
			ini_content->first = temp;
			ini_content->last  = temp;
		}
		else
		{
			ini_content->last->next = temp;
			ini_content->last = temp;
		}
	}
	else if(w_flag == REPLACE)
	{


		strcpy(temp->name,sec);
		if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
			sprintf(temp->comments,"#%s",comment);
		else
			strcpy(temp->comments,comment);
	}
}

//////////////////////////////////////////////////////////////////////////
// append list
//  [10/18/2005]
static void _append(const char *sec,const char *key,const char *value,const char *comment)
{
	struct section *tmp_sec;
	struct record *temp;

	// find section
	tmp_sec = get_section(sec);

	if(tmp_sec != NULL)
	{
		temp = get_record(tmp_sec,key);
		if(temp == NULL)
		{
			temp = (struct record *)malloc(sizeof(struct record));
			if(temp == NULL)
			{
				error_msg = "cannot malloc memory !";
				return;
			}
			temp->next = NULL;

			if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
				sprintf(temp->comments,"#%s",comment);
			else
				strcpy(temp->comments,comment);
			strcpy(temp->key,key);
			strcpy(temp->value,value);
			tmp_sec->data_size++;

			if (tmp_sec->data_first == NULL)
			{
				tmp_sec->data_first = temp;
				tmp_sec->data_last  = temp;
			}
			else
			{
				tmp_sec->data_last->next = temp;
				tmp_sec->data_last = temp;
			}
		}
		else if(w_flag == REPLACE)
		{
			if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
				sprintf(temp->comments,"#%s",comment);
			else
				strcpy(temp->comments,comment);

			strcpy(temp->key,key);
			strcpy(temp->value,value);
		}

	}
	else
	{
		add_section(sec,"");
		_append(sec,key,value,comment);
	}
}

//////////////////////////////////////////////////////////////////////////
// search and get section
//  [10/18/2005]
static struct section *get_section(const char *sec)
{
	bool found = false;
	struct section *esection = ini_content->first;
	while (esection != NULL)
	{
		if(strcmp(esection->name,sec) == 0)
		{
			found = true;
			break;
		}
		esection = esection->next;
	}

	if(found == true)
		return esection;
	else
		return NULL;
}

//////////////////////////////////////////////////////////////////////////
// search and get record
static struct record *get_record(const struct section *sec,const char *key)
{
	bool found = false;
	struct record *tmp;

	if (sec == NULL) {
		return NULL;
	}

	tmp = sec->data_first;

	while(tmp != NULL)
	{
		if(strcmp(key,tmp->key) == 0)
		{
			found = true;
			break;
		}
		tmp = tmp->next;
	}

	if(found == true)
	{
		return tmp;
	}
	else
	{
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// remove all record
// [10/18/2005]
static int  _remove_all(struct section *sec)
{
	struct record *tmp;
	int remove_ret = 0;

	if(sec == NULL)
		return 0;

	tmp = sec->data_first;
	while(tmp != NULL)
	{
		sec->data_first = tmp->next;
		sec->data_size--;
		free(tmp);
		remove_ret++;
		tmp = sec->data_first;
	}
	return remove_ret;
}

//////////////////////////////////////////////////////////////////////////
// clear all content
//  [10/18/2005]
static void clear(void)
{
	struct section *tmp;
	if(ini_content == NULL)
		return;

	tmp = ini_content->first;
	while(tmp != NULL)
	{
		ini_content->first = tmp->next;
		ini_content->section_size--;
		_remove_all(tmp);
		free(tmp);
		tmp = ini_content->first;
	}
}

static void _string_allrecord(struct record *sec, ostringstream* out);
string content_string(void) {
	ostringstream reply;

	struct section *sec = ini_content->first;
	while (sec != NULL)
	{
		if(strlen(sec->comments) != 0) {
			reply << sec->comments << endl;
		}
		reply << "[" << sec->name << "]" << endl;
		_string_allrecord(sec->data_first, &reply);
		sec = sec->next;
	}

	string replyp = reply.str();
	return replyp;
}

static void _string_allrecord(struct record *sec, ostringstream* out) {
	struct record *temp = sec;
	while(temp != NULL) {
		if(strlen(temp->comments) != 0) {
			*out << temp->comments << endl;
		}
		*out << temp->key << "=" << temp->value << endl;
		temp = temp->next;
	}
}

//////////////////////////////////////////////////////////////////////////
// trime ' ' \n\t\r
//  [10/19/2005]
static void trim(char *buffer)
{
	if(buffer[strlen(buffer)-1] == '\n')
		buffer[strlen(buffer)-1] = '\0';
	if(buffer[strlen(buffer)-1] == '\r')
		buffer[strlen(buffer)-1] = '\0';
	//char temp[1024];
	//if(buffer[0] )
}
