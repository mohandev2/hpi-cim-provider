/* Include the required CMPI macros, data types, and API function headers */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

/* Simple logging facility, in case the standard SBLIM _OSBASE_TRACE() isn't available */
#ifndef _OSBASE_TRACE
#include <stdarg.h>
void _OSBASE_TRACE(int LEVEL, char *fmt,...)
{
   va_list ap;
   va_start(ap,fmt);
   vfprintf(stderr,fmt,ap);
   va_end(ap);
   fprintf(stderr,"\n");
}
#endif

/* This is needed by SimpleProcess for kill() */
#include <signal.h>

/* Handle to the CIM broker. This is initialized by the CIMOM when the provider is loaded */
static CMPIBroker * _BROKER;


/* ---------------------------------------------------------------------------
 * CMPI INSTANCE PROVIDER FUNCTIONS
 * --------------------------------------------------------------------------- */

/* EnumInstanceNames() - return a list of all the instances names (i.e. return their object paths only) */
static CMPIStatus EnumInstanceNames(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference)	/* [in] Contains the CIM namespace and classname */
{
   /* Commonly needed vars */
   CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
   CMPIObjectPath * objectpath;			/* CIM object path of each new instance of this class */
   char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
   char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */
   void * instances;				/* Handle to the 'list' of instances */

   /* Custom vars for SimpleProcess */
   char buffer[1024];				/* Text buffer to read in data for each process */

   _OSBASE_TRACE(1,("EnumInstanceNames() called"));

   /* Create a new template object path for returning results */
   objectpath = CMNewObjectPath(_BROKER, namespace, classname, &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("EnumInstanceNames() : Failed to create new object path - %s", CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new object path");
   }

   /* Run the ps command to get the process data and read it in via a pipe */
   instances = popen("ps -e --no-headers", "r");
   if (instances == NULL) {
      _OSBASE_TRACE(1,("EnumInstanceNames() : Failed to get process data"));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get process data");
   }

   /* Go thru the list of processes and return an object path for each */
   while (fgets(buffer, 1024, (FILE *)instances) != NULL) {
      unsigned int pid;
      char tty[9];
      unsigned long hour, min, sec;
      char cmd[1024];

      /* Parse the ps data fields from the buffer. They are: PID TTY TIME CMD */
      sscanf(buffer, "%5d%8s%2d:%2d:%2d%s", &pid, tty, &hour, &min, &sec, cmd);

      /* We only need to set the PID - a key property - to uniquely identify each process's object path */ 
      CMAddKey(objectpath, "PID", (CMPIValue *)&pid, CMPI_uint32);
 
      /* Add the object path for this process to the list of results */
      CMReturnObjectPath(results, objectpath); 
   }

   /* Finished reading all the process data */
   pclose((FILE *) instances);

   /* Finished EnumInstanceNames */
   CMReturnDone(results);
   _OSBASE_TRACE(1,("EnumInstanceNames() %s", (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
   return status;
}


/* EnumInstances() - return a list of all the instances (i.e. return all their instance data) */
static CMPIStatus EnumInstances(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference,	/* [in] Contains the CIM namespace and classname */
		char ** properties)		/* [in] List of desired properties (NULL=all) */
{
   /* Commonly needed vars */
   CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
   CMPIInstance * instance;			/* CIM instance of each new instance of this class */
   char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
   char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */
   void * instances;				/* Handle to the 'list' of instances */

   /* Custom vars for SimpleProcess */
   char buffer[1024];				/* Text buffer to read in data for each process */

   _OSBASE_TRACE(1,("EnumInstances() called"));

   /* Create a new template instance for returning results */
   /* NB - we create a CIM instance from an existing CIM object path */
   instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("EnumInstances() : Failed to create new instance - %s", CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
   }

   /* Run the ps command to get the process data and read it in via a pipe */
   instances = popen("ps -e --no-headers", "r");
   if (instances == NULL) {
      _OSBASE_TRACE(1,("EnumInstances() : Failed to get process data"));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get process data");
   }

   /* Go thru the list of processes and return an instance for each */
   while (fgets(buffer, 1024, (FILE *)instances) != NULL) {
      unsigned int pid;
      char tty[9];
      unsigned long hour, min, sec;
      char cmd[1024];

      /* Parse the ps fields from the line. They are: PID TTY TIME CMD */
      sscanf(buffer, "%5d%8s%2d:%2d:%2d%s", &pid, tty, &hour, &min, &sec, cmd);

      /* Set all the properties of the instance from the process data */
      /* NB - we're being lazy here and ignore the list of desired properties and just return everything! */
      CMSetProperty(instance, "PID", (CMPIValue *)&pid, CMPI_uint32);
      CMSetProperty(instance, "TTY", tty, CMPI_chars);
      sec += hour*3600 + min*60;
      CMSetProperty(instance, "Time", (CMPIValue *)&sec, CMPI_uint64);
      CMSetProperty(instance, "Command", cmd, CMPI_chars);

      /* Add the instance for this process to the list of results */
      CMReturnInstance(results, instance);
   }

   /* Finished reading all the process data */
   pclose((FILE *) instances);

   /* Finished EnumInstances */
   CMReturnDone(results);
   _OSBASE_TRACE(1,("EnumInstances() %s",(status.rc == CMPI_RC_OK)? "succeeded":"failed"));
   return status;
}


/* GetInstance() -  return the instance data for the specified instance only */
static CMPIStatus GetInstance(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference,	/* [in] Contains the CIM namespace, classname and desired object path */
		char ** properties)		/* [in] List of desired properties (NULL=all) */
{
   /* Commonly needed vars */
   CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
   CMPIInstance * instance;			/* CIM instance of each new instance of this class */
   char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
   char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */
   void * instances;				/* Handle to the 'list' of instances */

   /* Custom vars for SimpleProcess */
   char buffer[1024];				/* Text buffer to read in data for each process */
   CMPIData pidData;				/* Desired PID datum from the reference object path */

   _OSBASE_TRACE(1,("GetInstance() called"));

   /* Get the desired process PID from the reference object path */
   /* NB - CMGetKey() returns a CMPIData object which is an encapsulated CMPI data type, not a raw integer */
   pidData = CMGetKey(reference, "PID", &status);
   if (status.rc != CMPI_RC_OK || CMIsNullValue(pidData)) {
      _OSBASE_TRACE(1,("GetInstance() : Cannot determine desired process - %s", CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Cannot determine desired process");
   }

   /* Create a new template instance for returning results */
   /* NB - we create a CIM instance from an existing CIM object path */
   instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("GetInstance() : Failed to create new instance - %s", CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
   }

   /* Run the ps command to get just this process's data and read it in via a pipe */
   sprintf(buffer, "ps -p %d --no-headers", pidData.value.uint32);
   instances = popen(buffer, "r");
   if (instances == NULL) {
      _OSBASE_TRACE(1,("GetInstance() : Failed to get process data"));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get process data");
   }
   
   /* Read in the desired instance */
   if (fgets(buffer, 1024, (FILE *)instances) != NULL) {
      unsigned int pid;
      char tty[9];
      unsigned long hour, min, sec;
      char cmd[1024];

      /* Parse the ps fields from the line. They are: PID TTY TIME CMD */
      sscanf(buffer, "%5d%8s%2d:%2d:%2d%s", &pid, tty, &hour, &min, &sec, cmd);

      /* Set all the properties of the instance from the process data */
      /* NB - we're being lazy here and ignore the list of desired properties and just return everything! */
      CMSetProperty(instance, "PID", (CMPIValue *)&pid, CMPI_uint32);
      CMSetProperty(instance, "TTY", tty, CMPI_chars);
      sec += hour*3600 + min*60;
      CMSetProperty(instance, "Time", (CMPIValue *)&sec, CMPI_uint64);
      CMSetProperty(instance, "Command", cmd, CMPI_chars);

      /* Add the instance for this process to the list of results */
      CMReturnInstance(results, instance);
   }

   /* Finished reading all the process data */
   pclose((FILE *) instances);

   /* Finished */
   CMReturnDone(results);
   _OSBASE_TRACE(1,("GetInstance() %s",(status.rc == CMPI_RC_OK)? "succeeded":"failed"));
   return status;
}


/* SetInstance() - save modified instance data for the specified instance */
static CMPIStatus SetInstance(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference,	/* [in] Contains the CIM namespace, classname and desired object path */
		CMPIInstance * newinstance)	/* [in] Contains all the new instance data */
{
   /* We cannot modify processes programmatically */
   CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}


/* CreateInstance() - create a new instance from the specified instance data */
static CMPIStatus CreateInstance(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference,	/* [in] Contains the CIM namespace, classname and desired object path */
		CMPIInstance * newinstance)	/* [in] Contains all the new instance data */
{
   /* We cannot create new processes programmatically */
   CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}


/* DeleteInstance() - delete/remove the specified instance */
static CMPIStatus DeleteInstance(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference)	/* [in] Contains the CIM namespace, classname and desired object path */
{
   /* Commonly needed vars */
   CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
                                                                                                                            
   /* Custom vars for SimpleProcess */
   CMPIData pidData;				/* Desired PID datum from the reference object path */

   _OSBASE_TRACE(1,("DeleteInstance() called"));

   /* Get the desired process PID from the reference object path */
   /* NB - CMGetKey() returns a CMPIData object which is an encapsulated CMPI data type, not a raw integer */
   pidData = CMGetKey(reference, "PID", &status);
   if (status.rc != CMPI_RC_OK || CMIsNullValue(pidData)) {
      _OSBASE_TRACE(1,("GetInstance() : Cannot determine desired process - %s", CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Cannot determine desired process");
   }

   /* 'Delete' the desired process by kill()'ing it! */
   /* NB - we're being lazy here and not first checking that desired process actually exists! */  
   if (kill(pidData.value.uint32, SIGKILL) != 0) {
      _OSBASE_TRACE(1,("DeleteInstance() : Failed to kill process"));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to kill process");
   }
 
   /* Finished */
   CMReturnDone(results);
   _OSBASE_TRACE(1,("DeleteInstance() %s",(status.rc == CMPI_RC_OK)? "succeeded":"failed"));
   return status;
}


/* ExecQuery() - return a list of all the instances that 'satisfy' the desired query filter */
static CMPIStatus ExecQuery(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference,	/* [in] Contains the CIM namespace and classname */
		char * language,		/* [in] Name of the query language (e.g. "WQL") */ 
		char * query)			/* [in] Text of the query, written in the query language */ 
{
   /* 'nuff said */
   CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}


/* Cleanup() - perform any necessary cleanup immediately before this provider is unloaded */
static CMPIStatus Cleanup(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context)		/* [in] Additional context info, if any */
{
   /* Nothing needs to be done */
   CMReturn(CMPI_RC_OK);
}


/* ---------------------------------------------------------------------------
 * CMPI PROVIDER SETUP
 * --------------------------------------------------------------------------- */

/* Factory method that creates the handle to this provider, specifically setting up the function table */
/* NB - the first param is blank because this provider's instance function names do not need a custom prefix */
CMInstanceMIStub( , SimpleProcess, _BROKER, CMNoHook);

