/* Required C library headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

static char * _CLASSNAME = "SimpleProcess";

/* ---------------------------------------------------------------------------
 * CUSTOMIZED INSTANCE ENUMERATION FUNCTIONS
 * --------------------------------------------------------------------------- */

/*
 * startReadingInstances
 */
static void * _startReadingInstances()
{
   /* Run the 'ps' system command to obtain the desired process data */
   FILE * handle = popen("ps -e --no-headers", "r");

   /* Return the handle to the list of instances, in this case the file pipe */
   return handle;
}


/*
 * readNextInstance
 */
static int _readNextInstance( void * instances, CMPIInstance ** instance, char * namespace )
{
   CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
   CMPIObjectPath * objectpath;			/* CIM object path of the current instance */
   char buffer[1024];				/* Buffer to read in process info text */

   /* Create a new CIM object path for this instance */
   objectpath = CMNewObjectPath(_BROKER, namespace, _CLASSNAME, &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("readNextInstance() : Failed to create new object path - %s", CMGetCharPtr(status.msg)));
      *instance = NULL;
      return EOF;
   }

   /* Create a new CIM instance for the new object path */
   *instance = CMNewInstance(_BROKER, objectpath, &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("readNextInstance() : Failed to create new instance - %s", CMGetCharPtr(status.msg)));
      *instance = NULL;
      return EOF;
   }

   /* Read the next instance data from the process data pipe and set the instance properties accordingly */
   if (fgets(buffer, 1024, (FILE *)instances) != NULL) {
      unsigned pid;
      char tty[9];
      char time[9];
      char cmd[1024];
      CMPIValue val;				/* Stores the time string converted to CMPI_dateTime data type */

      /* The format of 'ps -e' output is: "  PID TTY          TIME CMD" */
      sscanf(buffer, "%5d%8s%8s%s", &pid, tty, time, cmd );
      CMSetProperty( *instance, "PID", (CMPIValue *)&pid, CMPI_uint32 );
      CMSetProperty( *instance, "TTY", tty, CMPI_chars );
//      val.dateTime = CMNewDateTimeFromChars(_BROKER, time, NULL);
//      CMSetProperty( *instance, "Time", &val, CMPI_dateTime );
      CMSetProperty( *instance, "Command", cmd, CMPI_chars );
   } else {
      _OSBASE_TRACE(1,("readNextInstance() : End of instance data"));
      *instance = NULL;
      return EOF;
   }
 
   return 1;
}


/*
 * endReadingInstances
 */
static void _endReadingInstances( void * instances )
{
   /* Cleanup the process info data pipe */
   if (instances != NULL) pclose((FILE *) instances);
}

