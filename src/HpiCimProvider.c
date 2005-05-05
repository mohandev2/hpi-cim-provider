/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author:
 *      Renier Morales <renierm@users.sf.net>
 *
 */

/* Include the required CMPI macros, data types, and API function headers */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"
#include "SaHpi.h"

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

static struct hpi_handle {
        SaHpiSessionIdT sid;
} hpi_hnd;


/* Handle to the CIM broker. This is initialized by the CIMOM when the provider is loaded */
static CMPIBroker * _BROKER;

/* Name of this provider */
static char * _PROVIDER = "HpiProvider";


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
        CMPIObjectPath * objectpath; /* CIM object path of each new instance of this class */
        char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
        char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */
        /* HPI vars */
        SaErrorT error;
        SaHpiEntryIdT next = SAHPI_FIRST_ENTRY;

        _OSBASE_TRACE(1,("%s:EnumInstanceNames() called", _PROVIDER));

        /* Create a new template object path for returning results */
        objectpath = CMNewObjectPath(_BROKER, namespace, classname, &status);
        if (status.rc != CMPI_RC_OK) {
                _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to create new object path - %s",
                              _PROVIDER, CMGetCharPtr(status.msg)));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new object path");
        }

        do {
                SaHpiRptEntryT entry;
                SaHpiEntryIdT current = next;
                
                error = saHpiRptEntryGet(hpi_hnd.sid, current, &next, &entry);

                if (error) {
                        _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to get HPI data", _PROVIDER));
                        CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI data");
                }
                
                /* Resource ID uniquely identifies HPI Resources. Enough to enumerate instance "name" */
                CMAddKey(objectpath, "RID", (CMPIValue *)&entry.ResourceId, CMPI_uint32);
                /* Add the object path for this resource to the list of results */
                CMReturnObjectPath(results, objectpath);
                
        } while (next != SAHPI_LAST_ENTRY);

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
        /* HPI vars */
        SaErrorT error;
        SaHpiEntryIdT next = SAHPI_FIRST_ENTRY;

        _OSBASE_TRACE(1,("%s:EnumInstances() called", _PROVIDER));

        /* Create a new template instance for returning results */
        /* NB - we create a CIM instance from an existing CIM object path */
        instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
        if (status.rc != CMPI_RC_OK) {
                _OSBASE_TRACE(1,("%s:EnumInstances() : Failed to create new instance - %s",
                              _PROVIDER, CMGetCharPtr(status.msg)));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
        }
   
        do {
                SaHpiRptEntryT entry;
                SaHpiEntryIdT current = next;
                
                error = saHpiRptEntryGet(hpi_hnd.sid, current, &next, &entry);
        
                if (error) {
                        _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to get HPI data", _PROVIDER));
                        CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI data");
                }
                
                /* Set all the properties of the instance from the HPI data */
                /* NB - we're being lazy here and ignore the list of desired properties and just return */
                /* a predefined set. */
                CMSetProperty(instance, "RID", (CMPIValue *)&entry.ResourceId, CMPI_uint32);
                CMSetProperty(instance, "ElementName", (CMPIValue *)entry.ResourceTag.Data, CMPI_chars);
                /* Add the instance for this process to the list of results */
                CMReturnInstance(results, instance);
        } while (next != SAHPI_LAST_ENTRY);

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
        CMPIInstance * instance;		/* CIM instance of each new instance of this class */
        char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
        char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */

        /* Custom vars for HPI */
        SaErrorT error;
        SaHpiRptEntryT resource;
        CMPIData ridData;       /* Desired RID datum from the reference object path */
        SaHpiResourceIdT rid, next_rid;      /* Desired RID extracted from ridData */

        _OSBASE_TRACE(1,("%s:GetInstance() called", _PROVIDER));

        /* Get the desired process RID from the reference object path */
        /* NB - CMGetKey() returns a CMPIData object which is an encapsulated CMPI data type, not a raw integer */
        ridData = CMGetKey(reference, "RID", &status);
        if (status.rc != CMPI_RC_OK || CMIsNullValue(ridData)) {
                _OSBASE_TRACE(1,("%s:GetInstance() : Cannot determine desired HPI resource - %s",
                              _PROVIDER, CMGetCharPtr(status.msg)));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Cannot determine desired HPI resource");
        }
        rid = ridData.value.uint32;

        /* Create a new template instance for returning results */
        /* NB - we create a CIM instance from an existing CIM object path */
        instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
        if (status.rc != CMPI_RC_OK) {
                _OSBASE_TRACE(1,("%s:GetInstance(): : Failed to create new instance - %s",
                              _PROVIDER, CMGetCharPtr(status.msg)));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
        }
        
        error = saHpiRptEntryGet(hpi_hnd.sid, rid, &next_rid, &resource);
        if (error) {
                _OSBASE_TRACE(1,("%s:GetInstance() : Failed to get HPI data", _PROVIDER));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI data");
        }

        CMSetProperty(instance, "RID", (CMPIValue *)&rid, CMPI_uint32);
        CMSetProperty(instance, "ElementName", resource.ResourceTag.Data, CMPI_chars);
        /* Add the instance for this resource to the list of results */
        CMReturnInstance(results, instance);
      
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
#if 0                                                                                                                               
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
#endif 
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

   saHpiSessionClose(hpi_hnd.sid);
   
   /* Nothing needs to be done */
   _OSBASE_TRACE(1,("%s:Cleanup() succeeded", _PROVIDER));
   CMReturn(CMPI_RC_OK);
}


/* OPTIONAL: Initialize() is *NOT* a predefined CMPI method. See CMInstanceMIStub() below */
static void Initialize(
		CMPIBroker *broker)		/* [in] Handle to the CIMOM */                
{
        SaErrorT error = SA_OK;
   
        _OSBASE_TRACE(1,("%s:Initialize() called", _PROVIDER));   
   
        error = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &(hpi_hnd.sid), 0);
        if (error) {
                hpi_hnd.sid = 0;
                _OSBASE_TRACE(1,("%s:Initialize() failed", _PROVIDER));
                return;
        }
        
        error = saHpiDiscover(hpi_hnd.sid);
        if (error) {
                _OSBASE_TRACE(1,("%s:Initialize() failed", _PROVIDER));
                return;
        }
        
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

