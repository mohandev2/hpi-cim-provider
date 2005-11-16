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
 *      Dr. Gareth S. Bestor <bestorga@us.ibm.com>
 *      Renier Morales <renierm@users.sf.net>
 *      David Judkovics <djudkovi@us.ibm.com>
 *
 */

/* Name of this provider */
static char _CLASSNAME[] = "HPI_LogicalDevice";

#define CMPI_VERSION 90
 
/* Include the required CMPI macros, data types, and API function headers */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"
#include <SaHpi.h>
#include <oh_utils.h>

/* NULL terminated list of key property names for this class */
static char * _KEYNAMES[] = {"RID", NULL};

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

static struct hpi_handle {
        SaHpiSessionIdT sid;
        SaHpiDomainInfoT domain_info;
} hpi_hnd;


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
        /* HPI vars */
        SaErrorT error;
        SaHpiRptEntryT entry;
        SaHpiEntryIdT entry_id = SAHPI_FIRST_ENTRY;

        SaHpiEntryIdT rdr_id;
        SaHpiRdrT     rdr;

        oh_big_textbuffer bigbuf;

        char buf[1024];

        /* Commonly needed vars */
        CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
        CMPIObjectPath * objectpath; /* CIM object path of each new instance of this class */
        char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
        char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */

        _OSBASE_TRACE(1,("%s:EnumInstanceNames() called", _CLASSNAME));


        do {

                memset(&entry, 0, sizeof(entry));
                error = saHpiRptEntryGet(hpi_hnd.sid, entry_id, &entry_id, &entry);

                if (error != SA_OK) {
                        _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to get HPI RPT data", _CLASSNAME));
                        CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI RPT data");
                }


                        rdr_id = SAHPI_FIRST_ENTRY;
                        do {
                                memset(&rdr, 0, sizeof(rdr));
                                error = saHpiRdrGet(hpi_hnd.sid, 
                                                    entry.ResourceId, 
                                                    rdr_id, 
                                                    &rdr_id, &rdr);

                                if (error != SA_OK) {
                                        _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to get HPI RDR data", _CLASSNAME));
                                        CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI RDR data");
                                }

                                memset(&bigbuf, 0, sizeof(bigbuf));
                                error = oh_decode_entitypath(&entry.ResourceEntity, &bigbuf);
                                
                                memset(buf, 0, sizeof(buf));
                                sprintf(buf, "%s{%s}{%d}", bigbuf.Data, oh_lookup_rdrtype(rdr.RdrType), rdr.RecordId);


                                /* Create a new template object path for returning results */
                                objectpath = CMNewObjectPath(_BROKER, namespace, classname, &status);
                                if (status.rc != CMPI_RC_OK) {
                                        _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to create new object path - %s",
                                                         _CLASSNAME, CMGetCharPtr(status.msg)));
                                        CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new object path");
                                }

                                CMAddKey(objectpath, "DeviceID", (CMPIValue *)buf, CMPI_chars);

                                CMAddKey(objectpath, "SystemCreationClassName", (CMPIValue *)"Linux_ComputerSystem", CMPI_chars);

                                CMAddKey(objectpath, "SystemName", (CMPIValue *)"Laptop", CMPI_chars);

                                CMAddKey(objectpath, "CreationClassName", (CMPIValue *)"HPI_LogicalDevice", CMPI_chars);

                                /* Add the object path for this resource to the list of results */
                                CMReturnObjectPath(results, objectpath);
                                
                        } while (rdr_id != SAHPI_LAST_ENTRY);

                
        } while (entry_id != SAHPI_LAST_ENTRY);

        /* Finished EnumInstanceNames */
        CMReturnDone(results);
        _OSBASE_TRACE(1,("%s:EnumInstanceNames() %s", _CLASSNAME, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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
        /* HPI vars */
        SaErrorT error;
        SaHpiRptEntryT entry;
        SaHpiEntryIdT entry_id = SAHPI_FIRST_ENTRY;

        SaHpiEntryIdT rdr_id;
        SaHpiRdrT     rdr;

        oh_big_textbuffer bigbuf;

        char buf[1024];

        /* Commonly needed vars */
        CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
        CMPIInstance * instance;			/* CIM instance of each new instance of this class */
        char * namespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /* Our current CIM namespace */
        char * classname = CMGetCharPtr(CMGetClassName(reference, NULL)); /* Registered name of this CIM class */

        _OSBASE_TRACE(1,("%s:EnumInstances() called", _CLASSNAME));

        do {


                error = saHpiRptEntryGet(hpi_hnd.sid, entry_id, &entry_id, &entry);
        
                if (error != SA_OK) {
                        _OSBASE_TRACE(1,("%s:EnumInstanceNames() : Failed to get HPI data", _CLASSNAME));
                        CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI data");
                }

                rdr_id = SAHPI_FIRST_ENTRY;
                do {
                        memset(&rdr, 0, sizeof(rdr));
                        error = saHpiRdrGet(hpi_hnd.sid, 
                                            entry.ResourceId, 
                                            rdr_id, 
                                            &rdr_id, &rdr);

                        /* Create a new template instance for returning results */
                        /* NB - we create a CIM instance from an existing CIM object path */
                        instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
                        if (status.rc != CMPI_RC_OK) {
                                _OSBASE_TRACE(1,("%s:EnumInstances() : Failed to create new instance - %s",
                                                 _CLASSNAME, CMGetCharPtr(status.msg)));
                                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
                        }

                        /* Set all the properties of the instance from the HPI data */
                        /* NB - we're being lazy here and ignore the list of desired properties and just return */
                        /* a predefined set. */

                        CMSetProperty(instance, "RID", (CMPIValue *)&entry.ResourceId, CMPI_uint32);
                        CMSetProperty(instance, "ElementName", (CMPIValue *)entry.ResourceTag.Data, CMPI_chars);


                        memset(&bigbuf, 0, sizeof(bigbuf));
                        error = oh_decode_entitypath(&entry.ResourceEntity, &bigbuf);


                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "%s{%s}{%d}", bigbuf.Data, oh_lookup_rdrtype(rdr.RdrType), rdr.RecordId);

                        printf("*** DeviceID [%s] ***\n", buf);
                        CMSetProperty(instance, "DeviceID", 
                                      (CMPIValue *)buf, CMPI_chars);

                        CMSetProperty(instance, "SystemCreationClassName", (CMPIValue *)"Linux_ComputerSystem", CMPI_chars);
                        CMSetProperty(instance, "SystemName", (CMPIValue *)"Laptop", CMPI_chars);
                        CMSetProperty(instance, "CreationClassName", (CMPIValue *)"HPI_LogicalDevice", CMPI_chars);

                        /* SessionId */
                        printf("*** SId [%d] ***\n", hpi_hnd.sid);
                        CMSetProperty(instance, "SID", (CMPIValue *)&hpi_hnd.sid, CMPI_uint32);

                        /* DomainId */
                        printf("*** DId [%d] ***\n", hpi_hnd.domain_info.DomainId);
                        CMSetProperty(instance, "DID", (CMPIValue *)&hpi_hnd.domain_info.DomainId, CMPI_uint32);

                        /* ResourceId */
                        printf("*** RId [%d] ***\n", entry.ResourceId);
                        CMSetProperty(instance, "RID", 
                                      (CMPIValue *)&entry.ResourceId, CMPI_uint32);

                        /* ResourceRev */
                        printf("*** ResourceRev [%d] ***\n", 
                               entry.ResourceInfo.ResourceRev);
                        CMSetProperty(instance, "ResourceRev", 
                                      (CMPIValue *)&entry.ResourceInfo.ResourceRev, 
                                      CMPI_uint8);

                        /* SpecificVer */
                        printf("*** SpecificVer [%d] ***\n", 
                               entry.ResourceInfo.SpecificVer);
                        CMSetProperty(instance, "SpecificVer", 
                                      (CMPIValue *)&entry.ResourceInfo.SpecificVer, 
                                      CMPI_uint8);

                        /* DeviceSupport */
                        printf("*** DeviceSupport [%d] ***\n", 
                               entry.ResourceInfo.DeviceSupport);
                        CMSetProperty(instance, "DeviceSupport", 
                                      (CMPIValue *)&entry.ResourceInfo.DeviceSupport, 
                                      CMPI_uint8);

                        /* ManufacturerId */
                        printf("*** ManufacturerId [%d] ***\n", 
                               entry.ResourceInfo.ManufacturerId);
                        CMSetProperty(instance, "ManufacturerId", 
                                      (CMPIValue *)&entry.ResourceInfo.ManufacturerId, 
                                      CMPI_uint32);
                        
                        /* ProductId */
                        printf("*** ProductId [%d] ***\n", 
                               entry.ResourceInfo.ProductId);
                        CMSetProperty(instance, "ProductId", 
                                      (CMPIValue *)&entry.ResourceInfo.ProductId, 
                                      CMPI_uint16);

                        /* FirmwareMajorRev */
                        printf("*** FirmwareMajorRev [%d] ***\n", 
                               entry.ResourceInfo.FirmwareMajorRev);
                        CMSetProperty(instance, "FirmwareMajorRev", 
                                      (CMPIValue *)&entry.ResourceInfo.FirmwareMajorRev, 
                                      CMPI_uint8);

                        /* FirmwareMinorRev */
                        printf("*** FirmwareMinorRev [%d] ***\n", 
                               entry.ResourceInfo.FirmwareMinorRev);
                        CMSetProperty(instance, "FirmwareMinorRev", 
                                      (CMPIValue *)&entry.ResourceInfo.FirmwareMinorRev, 
                                      CMPI_uint8);

                        /* AuxFirmwareRev */
                        printf("*** AuxFirmwareRev [%d] ***\n", 
                               entry.ResourceInfo.AuxFirmwareRev);
                        CMSetProperty(instance, "AuxFirmwareRev", 
                                      (CMPIValue *)&entry.ResourceInfo.AuxFirmwareRev, 
                                      CMPI_uint8);

                        /* Guid */
                        printf("*** Guid [%d] ***\n", 
                               entry.ResourceInfo.Guid);
                        CMSetProperty(instance, "Guid", 
                                      (CMPIValue *)&entry.ResourceInfo.Guid,
                                      CMPI_chars);

                        /* EntityPath */
                        //                oh_big_textbuffer bigbuf;
                        //                memset(&bigbuf, 0, sizeof(bigbuf));
                        //                error = oh_decode_entitypath(&entry.ResourceEntity, &bigbuf);
                        printf("*** EntityPath [%s] ***\n", bigbuf.Data);
                        CMSetProperty(instance, "EntityPath", 
                                      (CMPIValue *)bigbuf.Data, CMPI_chars);

                        /* Resource Capabilities */
                        SaHpiTextBufferT buffer;
                        memset(&buffer, 0, sizeof(buffer));
                        printf("*** Capabilities [%s] ***\n", buffer.Data);
                        error = oh_decode_capabilities(entry.ResourceCapabilities, 
                                                       &buffer);
                        CMSetProperty(instance, "Capabilities", 
                                      (CMPIValue *)buffer.Data, CMPI_chars);

                        /* SaHpiHsCapabilitiesT */
                        memset(&buffer, 0, sizeof(buffer));
                        error = oh_decode_hscapabilities(entry.HotSwapCapabilities,
                                                 &buffer);
                        printf("*** HotSwapCapabilities [%s] ***\n", buffer.Data);
                        CMSetProperty(instance, "HotSwapCapabilities", 
                                      (CMPIValue *)buffer.Data, CMPI_chars);

                        /* SaHpiSeverityT */
                        printf("*** ResourceSeverity [%s] ***\n", 
                               oh_lookup_severity(entry.ResourceSeverity));
                        CMSetProperty(instance, "ResourceSeverity", 
                                      (CMPIValue *)oh_lookup_severity(entry.ResourceSeverity), 
                                      CMPI_chars);

                        /* ResourceFailed */
                        printf("*** ResourceSeverity [%s] ***\n", 
                               (entry.ResourceFailed == SAHPI_TRUE) ? "TRUE" : "FALSE");
                        CMSetProperty(instance, "ResourceFailed", 
                                      (CMPIValue *)(entry.ResourceFailed == SAHPI_TRUE) 
                                                ? "TRUE" : "FALSE", 
                                      CMPI_chars);
                        
                        /* ResourceTag */
                        CMSetProperty(instance, "ResourceTag", 
                                      (CMPIValue *)entry.ResourceTag.Data, CMPI_chars);

                        /* Add the instance for this process to the list of results */
                        CMReturnInstance(results, instance);

                } while ( rdr_id != SAHPI_LAST_ENTRY );

        } while (entry_id != SAHPI_LAST_ENTRY);

        /* Finished EnumInstances */
        CMReturnDone(results);
        _OSBASE_TRACE(1,("%s:EnumInstances() %s", _CLASSNAME, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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

        _OSBASE_TRACE(1,("%s:GetInstance() called", _CLASSNAME));

        /* Get the desired process RID from the reference object path */
        /* NB - CMGetKey() returns a CMPIData object which is an encapsulated CMPI data type, not a raw integer */
        ridData = CMGetKey(reference, "RID", &status);
        if (status.rc != CMPI_RC_OK || CMIsNullValue(ridData)) {
                _OSBASE_TRACE(1,("%s:GetInstance() : Cannot determine desired HPI resource - %s",
                              _CLASSNAME, CMGetCharPtr(status.msg)));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Cannot determine desired HPI resource");
        }
        rid = ridData.value.uint32;

        /* Create a new template instance for returning results */
        /* NB - we create a CIM instance from an existing CIM object path */
        instance = CMNewInstance(_BROKER, CMNewObjectPath(_BROKER, namespace, classname, &status), &status);
        if (status.rc != CMPI_RC_OK) {
                _OSBASE_TRACE(1,("%s:GetInstance(): : Failed to create new instance - %s",
                              _CLASSNAME, CMGetCharPtr(status.msg)));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to create new instance");
        }
        
        error = saHpiRptEntryGet(hpi_hnd.sid, rid, &next_rid, &resource);
        if (error) {
                _OSBASE_TRACE(1,("%s:GetInstance() : Failed to get HPI data", _CLASSNAME));
                CMReturnWithChars(_BROKER, CMPI_RC_ERR_FAILED, "Failed to get HPI data");
        }

        CMSetProperty(instance, "RID", (CMPIValue *)&rid, CMPI_uint32);
        CMSetProperty(instance, "ElementName", resource.ResourceTag.Data, CMPI_chars);
        /* Add the instance for this resource to the list of results */
        CMReturnInstance(results, instance);
      
        /* Finished */
        CMReturnDone(results);
        _OSBASE_TRACE(1,("%s:GetInstance() %s", _CLASSNAME, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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
        CMPIStatus status = {CMPI_RC_ERR_NOT_SUPPORTED, NULL};	/* Return status of CIM operations */
   
        _OSBASE_TRACE(1,("%s:SetInstance() called", self->ft->miName));
  
        /* Modifying existing instances is not supported for this class */
 
        /* Finished */
        _OSBASE_TRACE(1,("%s:SetInstance() %s",
                      self->ft->miName, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
        return status;
}


/* CreateInstance() - create a new instance from the specified instance data */
static CMPIStatus CreateInstance(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference,	/* [in] Contains the CIM namespace, classname and desired object path */
		CMPIInstance * newinstance)	/* [in] Contains all the new instance data */
{
        CMPIStatus status = {CMPI_RC_ERR_NOT_SUPPORTED, NULL};	/* Return status of CIM operations */
   
        _OSBASE_TRACE(1,("%s:CreateInstance() called", self->ft->miName));

        /* Creating new instances is not supported for this class */
  
        /* Finished */
        CMReturnDone(results);

        _OSBASE_TRACE(1,("%s:CreateInstance() %s",
                      self->ft->miName, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
        return status;
}


/* DeleteInstance() - delete/remove the specified instance */
static CMPIStatus DeleteInstance(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context,		/* [in] Additional context info, if any */
		CMPIResult * results,		/* [out] Results of this operation */
		CMPIObjectPath * reference)	/* [in] Contains the CIM namespace, classname and desired object path */
{
        CMPIStatus status = {CMPI_RC_ERR_NOT_SUPPORTED, NULL};	/* Return status of CIM operations */
   
        _OSBASE_TRACE(1,("%s:CreateInstance() called", self->ft->miName));

        /* Creating new instances is not supported for this class */
  
        /* Finished */
//        CMReturnDone(results);

        _OSBASE_TRACE(1,("%s:CreateInstance() %s",
                      self->ft->miName, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
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
        CMPIStatus status = {CMPI_RC_ERR_NOT_SUPPORTED, NULL};	/* Return status of CIM operations */
        
        _OSBASE_TRACE(1,("%s:ExecQuery() called", self->ft->miName));
        
        /* Query filtering is not supported for this class */                                                                                                                         
        /* Finished */
        CMReturnDone(results);

        /* Not implemented, yet... */
        _OSBASE_TRACE(1,("%s:ExecQuery() %s",
                      self->ft->miName, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
        return status;
}


/* Cleanup() - perform any necessary cleanup immediately before this provider is unloaded */
static CMPIStatus Cleanup(
		CMPIInstanceMI * self,		/* [in] Handle to this provider (i.e. 'self') */
		CMPIContext * context)		/* [in] Additional context info, if any */
{
        CMPIStatus status = {CMPI_RC_OK, NULL};	/* Return status of CIM operations */
   
        _OSBASE_TRACE(1,("%s:Cleanup() called", self->ft->miName));
   
        /* Nothing needs to be done for cleanup */
   
        /* Finished */

        _OSBASE_TRACE(1,("%s:Cleanup() %s", self->ft->miName, (status.rc == CMPI_RC_OK)? "succeeded":"failed"));
        return status;
}


/* OPTIONAL: Initialize() is *NOT* a predefined CMPI method. See CMInstanceMIStub() below */
static void Initialize(
		CMPIBroker *broker)		/* [in] Handle to the CIMOM */                
{
        SaErrorT error = SA_OK;
   
        _OSBASE_TRACE(1,("%s:Initialize() called", _CLASSNAME)); 

        error = saHpiSessionOpen(SAHPI_UNSPECIFIED_DOMAIN_ID, &(hpi_hnd.sid), 0);
        if (error) {
                hpi_hnd.sid = 0;
                _OSBASE_TRACE(1,("%s:Initialize() failed", _CLASSNAME));
                return;
        }

        error = saHpiDomainInfoGet (hpi_hnd.sid, &(hpi_hnd.domain_info));
        if (error) {
                hpi_hnd.sid = 0;
                _OSBASE_TRACE(1,("%s:saHpiDomainInfoGet() failed", _CLASSNAME));
                return;
        }

        error = saHpiDiscover(hpi_hnd.sid);
        if (error) {
                _OSBASE_TRACE(1,("%s:saHpiDiscover() failed", _CLASSNAME));
                return;
        }
        
        /* Nothing needs to be done */
        _OSBASE_TRACE(1,("%s:Initialize() succeeded", _CLASSNAME));
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
CMInstanceMIStub( , HPI_LogicalDeviceProvider, _BROKER, Initialize(_BROKER));

/* If no special initialization is required then remove the Initialize() function and use:
CMInstanceMIStub( , CWS_ProcessProvider, _BROKER, CMNoHook);
*/

