#include "ParseLog.h"
#include <unistd.h>
// Executable Command Format : 
// ./DLULogProcessor FILEDIR FILENAME1 CORE1 FILENAME2 CORE2 ... LOGTYPE TIME1 TIME2 ARGS
// 
//Filenames for conversions between column number and corresponding string for ATP & ATO style files
static const char headerConversions_ATO_Filename[] = "NumToHeaderString_ATO.txt";
static const char headerConversions_ATP_Filename[] = "NumToHeaderString_ATP.txt";

void DetermineSpecificArgs(char* paramStr,struct Parameters* inputParams,char* headerConversionsFilename);

struct Parameters* initializeParams(struct Parameters* inputParams,int argc,char** argv);

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

//Takes integer based parameters which refer to a specific header string
//Conversion occurs here, through a .txt file which determines the string from the number of the argument
//The argument number refers to a line number of the headerConversionsFilename .txt file
//strtok_r is used because strtok does not save context of calls between different strings
void DetermineSpecificArgs(char* paramStr,struct Parameters* inputParams,char* headerConversionsFilename){	
	FILE *file = fopen(headerConversionsFilename,"r");
	char *savePtr, *tempParam = strtok_r(paramStr,"\t",&savePtr);

	int argCount = 0, lineCount = 1;
	if( NULL != file )
	{
		char line[MAX_STRING_SIZE];
		while(fgets(line, sizeof(line), file) != NULL){
			if( NULL == tempParam )
			{
				fclose(file);
				return;
			}
			else if((atoi(tempParam)) == lineCount)
			{
				strcpy(inputParams->Args[argCount],strtok(line,"\t"));
				inputParams->argN[argCount] = lineCount;
				strcpy(inputParams->Args[argCount+1],"\0");
				inputParams->argN[argCount+1] = -1;

				tempParam = strtok_r(NULL,"\t",&savePtr);
				argCount++;

				inputParams->argC = argCount;
			}
			else
			{
				//do nothing
			}
			lineCount++;
		}
		fclose(file);
	}
	else 
	{
		printf("Error: NumToHeaderString.txt doesn't exist in the /exe folder\n");
		exit(EXIT_FAILURE);
	}
}

//Dynamically grows the fileArray, which encompasses necessary information for each file
//This includes filenames for input, filepaths for output, FILE* pointers, & Core info
//Dynamically growing because the number of max possible files is unknown
struct fileInfo* determineFileArray(struct Parameters* inputParams,char* fileStr){
	struct fileInfo* fileArray = (struct fileInfo*)malloc(sizeof(struct fileInfo));
	char* tempFile = strtok(fileStr,"\t");
	char tempPath[MAX_STRING_SIZE];
	int i, arraySize = 1;

	while( NULL != tempFile ){

		//reallocates more memory to the block
		if(i == arraySize)
		{
			arraySize *= 2;
			fileArray = realloc(fileArray,arraySize * sizeof(struct fileInfo));
		}
		else
		{
			//do nothing
		}

		//File Info
		strcpy(fileArray[i].fileName,tempFile);
		sprintf(tempPath,"%s%s",inputParams->dirPath,tempFile);
		fileArray[i].fileInput = fopen(tempPath,"r");
		inputParams->fileC = i+1;
		//Core Info
		fileArray[i].core = atoi(strtok(NULL,"\t"));

		i++;
		tempFile = strtok(NULL,"\t");

	}
	return fileArray;
}

//Initializes the input parameters struct object
//Determines start, end times, as well as all other input parameters
//Determines necessary arguments to parse as well as a fileArray for the needed files
struct Parameters* initializeParams(struct Parameters* inputParams,int argc,char** argv){
	//Dynamically allocate space for struct
	inputParams = (struct Parameters*)malloc(sizeof(struct Parameters));
	
	//Determine directory path
	strcpy(inputParams->dirPath,argv[1]);

	//Determine fileArray
	inputParams->fileArray = determineFileArray(inputParams,argv[2]);
	//Determine whether ATP or ATO file
	inputParams->AT_DEF = atoi(argv[3]);

	//Determine Start & End Times 
	//inputParams->startTime = determineEpochTime(argv[2],argv[3]); 
	inputParams->startTime = determineEpochTime(strtok(argv[4],"\t"),"");
	inputParams->endTime = determineEpochTime(strtok(NULL,"\t"),""); 
	//Determine Timezone
	inputParams->UTC = determineESTorEDT(inputParams->endTime);

	//Update Times Based on Timezone And Normalize
	struct tm* startTMstruct = localtime(&(inputParams->startTime));
	startTMstruct->tm_hour += inputParams->UTC;
	inputParams->startTime = mktime(startTMstruct);

	struct tm* endTMstruct = localtime(&(inputParams->endTime));
	endTMstruct->tm_hour += inputParams->UTC;
	inputParams->endTime = mktime(endTMstruct);

	//Determine Parameter Information
	char* headerConversionsFilepath[MAX_STRING_SIZE];
	sprintf((char*) headerConversionsFilepath,"%s%s",inputParams->dirPath, (inputParams->AT_DEF) ? headerConversions_ATP_Filename : headerConversions_ATO_Filename);
	DetermineSpecificArgs(argv[5],inputParams,(char*) headerConversionsFilepath);

	return inputParams;
}

//Loops through the fileArray & determines an 'open' output filename for each input file
//Necessary to avoid overwriting files -- if a file with that name exists already, a numeric suffix
//will be added. This is incremented until 100 is reached or a non utilized filename is found
bool checkOutputFilenames(struct Parameters* inputParams){
	for(int i = 0; i < inputParams->fileC;i++){
		char outputFileName[MAX_STRING_SIZE];
		inputParams->fileArray[i].fileName[strlen(inputParams->fileArray[i].fileName)-4] = '\0';
		sprintf(outputFileName,"%s%s.%s",inputParams->dirPath,inputParams->fileArray[i].fileName,CSV_SUFFIX);

		char* checkForNull = nonRecursiveNameCheck(outputFileName);
		if ( NULL != checkForNull ) 
		{
			strcpy(outputFileName,checkForNull);
			strcpy(inputParams->fileArray[i].filePath,outputFileName);
			inputParams->fileArray[i].fileOutput = fopen(outputFileName,"w+");
		}
		else{
			return false;
		}
	}
	return true;
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
		sprintf(numSuffix,"-%02d.%s",fileIncrement,CSV_SUFFIX);
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

int main(int argc,char** argv){

	struct Parameters* inputParams;

	inputParams = initializeParams(inputParams,argc,argv);

	printParams(inputParams);
	
	if(!checkOutputFilenames(inputParams))
	{
		return 0;
	}

	parseLogFile(inputParams);

	//If there were multiple files, then they will be concatenated to one Output file
	//Name is determined by whether a specific name was specified by the user
	//An available name will be found with a numeric suffix
	if(inputParams->fileC > 1)
	{
		char* outputFileName[MAX_STRING_SIZE];
		if(!strlen(argv[6]))
		{
			sprintf((char*) outputFileName,"%s%s",inputParams->dirPath,GENERAL_OUTPUT_NAME);
		}
		else
		{
			sprintf((char*) outputFileName,"%s%s.%s",inputParams->dirPath,argv[6],CSV_SUFFIX);
		}
		//Check for pre-existing 
		char *checkForNull = nonRecursiveNameCheck((char*) outputFileName);
		if ( NULL != checkForNull ) 
		{
			strcpy((char*) outputFileName,checkForNull);
			makeCombinedOutput(inputParams,(char*) outputFileName);
			printf("File %s Has Been Created\n",outputFileName);
		}
		else
		{
			printf("Error: The directory is full -- Program will be terminated\n");
		}
	}
	else
	{
		printf("File %s Has Been Created\n",strstr(inputParams->fileArray[0].filePath,inputParams->fileArray[0].fileName));
	}

	free(inputParams);

	return 0;
}
