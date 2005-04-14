// $Id:
// =============================================================================
// (C) Copyright IBM Corp. 2005
//
// THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
//
// You can obtain a current copy of the Common Public License from
// http://www.opensource.org/licenses/cpl1.0.php
//
// Author:       Dr. Gareth S. Bestor, <bestorga@us.ibm.com>
// Contributors:
// Last Updated: April 12, 2005
// Description:  Sample CMPI process provider.
// =============================================================================

/* Include the required CMPI macros, data types, and API function headers */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

/* Simple logging facility, in case the standard SBLIM _OSBASE_TRACE() isn't available */
#ifndef _OSBASE_TRACE
#include <stdarg.h>
#define _OSBASE_TRACE(x,y) _logstderr y
void _logstderr(char *fmt,...)
{
   va_list ap;
   va_start(ap,fmt);
   vfprintf(stderr,fmt,ap);
   va_end(ap);
   fprintf(stderr,"\n");
}
#endif

/* Only needed by this provider for kill() */
#include <signal.h>

/* Handle to the CIM broker. This is initialized by the CIMOM when the provider is loaded */
static CMPIBroker * _BROKER;

/* Name of this provider */
static char * _PROVIDER = "CWS_ProcessProvider";


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
   
   _OSBASE_TRACE(1,("%s:EnumInstanceNames() called", _PROVIDER));

   /* Create a new template object path for returning results */
   objectpath = CMNewObjectPath(_BROKER, namespace, classname, &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to create new object path - %s", _PROVIDER, CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new object path");
   }

   /* Run the ps command to get the process data and read it in via a pipe */
   instances = popen("ps -e --no-headers", "r");
   if (instances == NULL) {
      _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to get process data", _PROVIDER));
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
   _OSBASE_TRACE(1,("%s:EnumInstanceNames() %s", _PROVIDER, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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

   _OSBASE_TRACE(1,("%s:EnumInstances() called", _PROVIDER));

   /* Create a new template instance for returning results */
   /* NB - we create a CIM instance from an existing CIM object path */
   instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("%s:EnumInstances() : Failed to create new instance - %s", _PROVIDER, CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
   }

   /* Run the ps command to get the process data and read it in via a pipe */
   instances = popen("ps -e --no-headers", "r");
   if (instances == NULL) {
      _OSBASE_TRACE(1,("%s:EnumInstances() : Failed to get process data", _PROVIDER));
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
   _OSBASE_TRACE(1,("%s:EnumInstances() %s", _PROVIDER, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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
   unsigned long pid;				/* Desired PID extracted from pidData */

   _OSBASE_TRACE(1,("%s:GetInstance() called", _PROVIDER));

   /* Get the desired process PID from the reference object path */
   /* NB - CMGetKey() returns a CMPIData object which is an encapsulated CMPI data type, not a raw integer */
   pidData = CMGetKey(reference, "PID", &status);
   if (status.rc != CMPI_RC_OK || CMIsNullValue(pidData)) {
      _OSBASE_TRACE(1,("%s:GetInstance() : Cannot determine desired process - %s", _PROVIDER, CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Cannot determine desired process");
   }
   pid = pidData.value.uint32;

   /* Create a new template instance for returning results */
   /* NB - we create a CIM instance from an existing CIM object path */
   instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
   if (status.rc != CMPI_RC_OK) {
      _OSBASE_TRACE(1,("%s:GetInstance(): : Failed to create new instance - %s", _PROVIDER, CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
   }

   /* Run the ps command to get just this process's data and read it in via a pipe */
   sprintf(buffer, "ps -p %u --no-headers", pid);
   instances = popen(buffer, "r");
   if (instances == NULL) {
      _OSBASE_TRACE(1,("%s:GetInstance() : Failed to get process data", _PROVIDER));
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
   } else {
      _OSBASE_TRACE(1,("%s:GetInstance() : Failed to get process data for PID=%u", _PROVIDER, pid));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get process data");
   }

   /* Finished reading all the process data */
   pclose((FILE *) instances);

   /* Finished */
   CMReturnDone(results);
   _OSBASE_TRACE(1,("%s:GetInstance() %s", _PROVIDER, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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
   _OSBASE_TRACE(1,("%s:SetInstance() called", _PROVIDER));

   /* We cannot modify processes programmatically */
   _OSBASE_TRACE(1,("%s:SetInstance() failed", _PROVIDER));
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
   _OSBASE_TRACE(1,("%s:CreateInstance() called", _PROVIDER));

   /* We cannot create new processes programmatically */
   _OSBASE_TRACE(1,("%s:CreateInstance() failed", _PROVIDER));
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
   unsigned long pid;				/* Desired PID extracted from pidData */

   _OSBASE_TRACE(1,("%s:DeleteInstance() called", _PROVIDER));

   /* Get the desired process PID from the reference object path */
   /* NB - CMGetKey() returns a CMPIData object which is an encapsulated CMPI data type, not a raw integer */
   pidData = CMGetKey(reference, "PID", &status);
   if (status.rc != CMPI_RC_OK || CMIsNullValue(pidData)) {
      _OSBASE_TRACE(1,("%s:DeleteInstance() : Cannot determine desired process - %s", _PROVIDER, CMGetCharPtr(status.msg)));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Cannot determine desired process");
   }
   pid = pidData.value.uint32;

   /* 'Delete' the desired process by kill()'ing it! */
   /* NB - we're being lazy here and not first checking that desired process actually exists! */  
   if (kill(pid, SIGKILL) != 0) {
      _OSBASE_TRACE(1,("%s:DeleteInstance() : Failed to kill process PID=%u", _PROVIDER, pid));
      CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to kill process");
   }
 
   /* Finished */
   CMReturnDone(results);
   _OSBASE_TRACE(1,("%s:DeleteInstance() %s", _PROVIDER, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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
   _OSBASE_TRACE(1,("%s:ExecQuery() called", _PROVIDER));

   /* Not implemented, yet... */
   _OSBASE_TRACE(1,("%s:ExecQuery() failed", _PROVIDER));
   CMReturn(CMPI_RC_ERR_NOT_SUPPORTED);
}


/* Cleanup() - perform any necessary cleanup immediately before this provider is unloaded */
static CMPIStatus Cleanup(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context)		/* [in] Additional context info, if any */
{
   _OSBASE_TRACE(1,("%s:Cleanup() called", _PROVIDER));

   /* Nothing needs to be done */
   _OSBASE_TRACE(1,("%s:Cleanup() succeeded", _PROVIDER));
   CMReturn(CMPI_RC_OK);
}


/* OPTIONAL: Initialize() is *NOT* a predefined CMPI method. See CMInstanceMIStub() below */
static void Initialize(
		CMPIBroker *broker)		/* [in] Handle to the CIMOM */
{
   _OSBASE_TRACE(1,("%s:Initialize() called", _PROVIDER));

   /* Nothing needs to be done */
   _OSBASE_TRACE(1,("%s:Initialize() succeeded", _PROVIDER));
}


/* ---------------------------------------------------------------------------
 * CMPI PROVIDER SETUP
 * --------------------------------------------------------------------------- */

/* Factory method that creates the handle to this provider, specifically setting up the function table:
   - 1st param is an optional prefix for method names in the function table. It is blank in this sample
     provider because the instance method function names do not need a unique prefix .
   - 2nd param is the name to call this provider in the CIMOM. It is recommended to call your provider
     "<_CLASSNAME>" or "<_CLASSNAME>Provider" - the name must be unique among all providers.
   - 3rd param is the local variable acting as a handle to the CIMOM. It will be initialized by the CIMOM
     when the provider is loaded. 
   - 4th param specified the the provider's initialization function to be called immediately after
     loading the provider. Specify "CMNoHook" if not required. */
CMInstanceMIStub( , CWS_ProcessProvider, _BROKER, Initialize(_BROKER));

/* If no special initialization is required then remove the Initialize() function and use:
CMInstanceMIStub( , CWS_ProcessProvider, _BROKER, CMNoHook);
*/

