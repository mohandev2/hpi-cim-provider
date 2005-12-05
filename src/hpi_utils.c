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
 *      David Judkovics <djudkovi@us.ibm.com>
 *
 */

#include <SaHpi.h>

int management_instrument_id(SaHpiRdrT  *rdr)
{

        switch (rdr->RdrType) {
                case SAHPI_ANNUNCIATOR_RDR:
                        return(rdr->RdrTypeUnion.AnnunciatorRec.AnnunciatorNum);
                        break;
                case SAHPI_CTRL_RDR:     
                        return(rdr->RdrTypeUnion.CtrlRec.Num);
                        break;
                case SAHPI_INVENTORY_RDR:
                        return(rdr->RdrTypeUnion.InventoryRec.IdrId);
                        break;
                case SAHPI_NO_RECORD:
                        return(rdr->RecordId);
                        break;
                case SAHPI_SENSOR_RDR:
                        return(rdr->RdrTypeUnion.SensorRec.Num);
                        break;
                case SAHPI_WATCHDOG_RDR:
                        return(rdr->RdrTypeUnion.WatchdogRec.WatchdogNum);
                        break;
                default:
                        return(-1);
                        break;
                }
        return (-1);
}


