#include "globals.h"
#include "helperFunctions.h"
#include "logDecoderClass.h" 

struct tm tm_buf;

 //Each ATO & ATP Parameter file is formatted like this:
 //     Parameter ID    Parameter Label    Unsigned/Signed_Integer    FirstBitPosition    BitCount    FirstByte   LastByte    Quantum    Offset    DisplayType    EnumeratedLabel    DecimalCount    Unit  
 //Sorted by FirstBitPosition
void parseParameterConfigFile(std::vector<class parameterInfo*> &parameterInfoVec,int logType,char* directoryPath,char (*enumeratedLabels)[MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE],char (*stringLabels)[MAX_STRING_SIZE]){
	char filePath[MAX_STRING_SIZE];
	char line[MAX_STRING_SIZE];

	sprintf(filePath,"%s%s",directoryPath, (logType == ATO_NUM) ? ATO_PARAMETERS_FILENAME : ATP_PARAMETERS_FILENAME);
	FILE* file = fopen(filePath,"r");

	while(fgets(line, sizeof(line),file) != NULL){
		//Make object dynamically and push back
		class parameterInfo* parameterInfoObj = new parameterInfo(line,enumeratedLabels,stringLabels);
		parameterInfoVec.push_back(parameterInfoObj);
	}

	fclose(file);
}

//Each ATO & ATP Label file is formatted like this:
 //     Enumerated ID   Enumerated Label    0    Label0    1    Label1     2    Label2 ... N    LabelN
 //Sorted by Enumerated ID
void parseLabelsConfigFile(char (*enumeratedLabels)[MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE],int logType,char* directoryPath){
    char filePath[MAX_STRING_SIZE];
    char line[MAX_CONFIG_LINE_SIZE];

    sprintf(filePath,"%s%s",directoryPath, (logType == ATO_NUM) ? ATO_ENUMERATED_LABELS_FILENAME : ATP_ENUMERATED_LABELS_FILENAME);
    FILE* file = fopen(filePath,"r");

    while(fgets(line, sizeof(line),file) != NULL){
        //First field is Enumerated ID
        char* field = strtok(line,"\t");
        int enumID = fast_atoi(field);

        //Skip over Label Name
        strtok(NULL,"\t");
        field = strtok(NULL,"\t");

        //First token is Numeric Value
        //Second token is String Value
        while(*field != '\n' && field != NULL){
            int valueID = fast_atoi(field);
            char* valueStr = strtok(NULL,"\t");

            strcpy(enumeratedLabels[enumID][valueID],valueStr);
            field = strtok(NULL,"\t");
        }
    }

    fclose(file);
}

//Outperforms atoi() by 4x
int fast_atoi(const char* str){
	if(!(*str - '-')) return fast_atoi(++str) * (-1);
	int val = 0;
	while(*str){
		val = val*10 + (*str++ - '0');
	}
	return val;
}

//Linear concat algorithm VS O(N^2) strcat()
char* fast_strcat(char* dest, const char* src)
{
	while (*dest) dest++;
	while (*dest++ = *src++);
	return --dest;
}

int skipSeq(int curChar,unsigned int& skipSeqNum){
	if(!skipSeqNum && curChar == 0x1){
		skipSeqNum = 0x1;
		return 0;
	}
	else if(!skipSeqNum){
		return 0;
	}
	else if(skipSeqNum && curChar == 0x1){
		skipSeqNum = 0x1;
		return 0;
	}
	else{
		skipSeqNum = (skipSeqNum << 8) | curChar;
		if(skipSeqNum >= MIN_SET){
			return 1;
		}
		else{
			return 0;
		}
	}
}

char* epochTimeToDate(unsigned long long value, char* str,const char* fmt){

	long long diff = (long long)(value - (long long) TIME_SINCE_1900_01_01);
	struct tm* tm;
	if(diff < 0){
		tm = negativeEpochTimeToDate(value);
	}
	else{
		time_t time = diff;
		tm = gmtime((time_t*) &time);
	}
	strftime(str,MAX_SHORT_STRING_SIZE,fmt,tm);
	return str;
}

struct tm* negativeEpochTimeToDate(unsigned long long time){
	struct tm* tm = &tm_buf;
	long long days, rem, y;
	const unsigned short int *ip;

	days = time / SECS_PER_DAY;
	rem = time % SECS_PER_DAY;

	tm->tm_hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tm->tm_min = rem / 60;
	tm->tm_sec = rem % 60;

	y=1900;

	#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
	#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

	while(days >= (isLeapYear(y) ? 366 : 365)){
		long long yg = y + days / 365 - (days % 365 < 0);

		days -= ((yg - y) * 365 + LEAPS_THRU_END_OF(yg - 1) + LEAPS_THRU_END_OF(y - 1));

		y = yg;
	}
	tm->tm_year = y - 1900;

	ip = __mon_yday[isLeapYear(y)];
	for(y = 11; days < (long int) ip[y] && y >= 0; --y){
		continue;
	}
	days -= ip[y];
	tm->tm_mon = y;
	tm->tm_mday = days + 1;
	return tm;
}

//Returns Epoch Time from date strings of format %Y/%m/%d %H:%M:%S
time_t determineEpochTime(char* yearDate,char* hourDate){
	char stringDate[MAX_TIME_STRING_SIZE];	
	sprintf(stringDate,"%s %.*s",yearDate,8,hourDate);
	struct tm* tm = (struct tm*)malloc(sizeof(struct tm));

	if(strptime_I(stringDate,"%d/%d/%d %d:%d:%d",tm) || strptime_I(stringDate,"%d-%d-%d %d:%d:%d",tm))
	{
		time_t time = mktime(tm);
		free(tm);
		return time;
	}
	else
	{
		printf("Error: Arguments do not contain the correct time. The current time will be returned\n");
		return time(NULL);
	}
}

//Takes dates and configures to 'struct tm' from 'time.h'
//Necessary for converting string dates to epoch times
const char *strptime_I(const char *buf, const char *fmt, struct tm *tm){
	//Strip the string to get the numeric value
	int Y,m,d,H,M,S;
	sscanf(buf,fmt,&Y,&m,&d,&H,&M,&S);
	
	//check for YYYY-MM-DD or YYYY/MM/DD
	if(!m || !d)
	{
		return NULL;
	}
	else
	{
		//do nothing
	}

	//Configure based on struct tm format
	tm->tm_isdst = -1;
	tm->tm_year= (Y-1900);
	tm->tm_mon = (m-1);
	tm->tm_mday = d;
	tm->tm_hour = H;
	tm->tm_min = M;
	tm->tm_sec = S;
	if(mktime(tm) < 1)
	{
		return NULL;
	}
	else
	{
		return buf;
	}
}

int isLeapYear(unsigned long long y){
	return ((y) % 4 == 0 && ((y) % 100 != 0 || (y) % 400 == 0));
}

char* convertToMillisecond(unsigned long long ms, char* str){
	char decimalStr[MAX_SHORT_STRING_SIZE];

	sprintf(decimalStr,".%d", ms / 100);

	return strcat(str,decimalStr);
}

//A reentrant version of strtok()
//char **save_ptr saves the context between successive calls that parse different strings
//POSIX systems have strtok_r -- Windows systems do not
char* strtok_r (char *s, const char *delim, char **save_ptr){
	char *end;
	if ( NULL == s )
	{
		s = *save_ptr;
	}
	if ( '\0' == *s )
	{
		*save_ptr = s;
		return NULL;
	}
	/* Scan leading delimiters.  */
	s += strspn (s, delim);
	if ( '\0' == *s )
	{
		*save_ptr = s;
		return NULL;
	}
	/* Find the end of the token.  */
	end = s + strcspn (s, delim);
	if ( '\0' == *end )
	{
		*save_ptr = end;
		return s;
	}
	/* Terminate the token and make *SAVE_PTR point past it.  */
	*end = '\0';
	*save_ptr = end + 1;
	return s;
}

int determineATPorATO(int directoryPathLength, char* fileName){
	  //Eliminate directory in ATO/ATP search 
	  char* tempFileName = fileName + directoryPathLength;
      if(strstr(tempFileName,ATP_STR) != NULL){
	      return ATP_NUM;
      }
      else if(strstr(tempFileName,ATO_STR) != NULL){
	      return ATO_NUM;
      }
      else{
	      printf("Error: Passed filename was named incorrectly, and 8001c0 or 800180 OR 8002c0 or 800280 was unfound\n");
	      return AT_UNDEFINED_NUM;
      }
}

//Necessary for determining whether the current timezone is EST or EDT
int determineESTorEDT(time_t tm){
	char timezoneString[MAX_STRING_SIZE];
	if(strftime(timezoneString,MAX_STRING_SIZE,"%Z",localtime(&tm)) != 0)
	{
		if(!strcmp(timezoneString,UTC5_TIME))
		{
			return UTC5_TIME_NUM;
		}
		else
		{
			return UTC4_TIME_NUM;
		}
	}
	else {
		printf("Error: Converting to EST or EDT was Unsuccessful\n");
		exit(EXIT_FAILURE);
	}
}

//Input is a filePath of the output file based on the input file
//Adds numeric suffix to a name until an available filename is found
//Returns the overwritten char array
char* nonRecursiveNameCheck(char* outputFileName){
	if ( 0 != access(outputFileName, F_OK) ) 
	{
		return outputFileName;	
	}
	else
	{
		//do nothing
	}
	//memset is necessary to truncate the ending to get rid of .txt or -03.csv for a new filename to test
	memset(outputFileName + strlen(outputFileName) - 4, 0, 4);

	for(int fileIncrement = 0;fileIncrement < 100;fileIncrement++){
		char numSuffix[MAX_STRING_SIZE];
		sprintf(numSuffix,"-%02d%s",fileIncrement,CSV_SUFFIX);
		strcat(outputFileName,numSuffix);

		if ( 0 != access(outputFileName, F_OK) ) 
		{
			return outputFileName;	
		}
		else
		{
			memset(outputFileName + strlen(outputFileName) - 7, 0, 7);
		}
	}
	
	printf("Error: The directory is full -- Program will be terminated\n");
	return NULL;
}

