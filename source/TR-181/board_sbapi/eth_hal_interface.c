/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "cosa_ethernet_apis.h"
#include <stdio.h>
#include <string.h>
#include "ansc_string_util.h"
#include "ccsp_hal_ethsw.h"
#include "safec_lib_common.h"
#include "secure_wrapper.h"

#define ARP_CACHE "/tmp/arp.txt"
#ifndef _SR213_PRODUCT_REQ_
#define DNSMASQ_CACHE "/tmp/dns.txt"
#define DNSMASQ_FILE "/nvram/dnsmasq.leases"
#endif /*_SR213_PRODUCT_REQ_*/
#define ETH_POLLING_PERIOD 180
#define ETH_NODE_HASH_SIZE 256

CcspHalExtSw_ethAssociatedDevice_callback AssociatedDevice_callback = NULL;

/*********** Function Prototype Start**************/

void* CcspHalExtSw_AssociatedDeviceMonitorThread( void *arg );
eth_device_t* CcspHalExtSw_FindHost( eth_device_t *pstEthHost, eth_device_t* eth_device_ArrayList[ ], unsigned int* puiHashIndex );
int CcspHalExtSw_AddHost( eth_device_t *pstEthHost, eth_device_t* eth_device_ArrayList[ ], BOOL bIsNeed2Notify ); 
int CcspHalExtSw_DeleteHost( eth_device_t *pstEthHost, eth_device_t* eth_device_ArrayList[ ], BOOL bIsNeed2Notify );
void CcspHalExtSw_DeleteAllHosts( eth_device_t* eth_device_ArrayList[ ], BOOL bIsNeed2Notify );
void CcspHalExtSw_SendNotificationForAllHosts( void );
unsigned int hash( char *str );
unsigned int mac_hash( char *str);

/*********** Function Prototype End**************/

eth_device_t* eth_device_hashArrayList[ ETH_NODE_HASH_SIZE ];
eth_device_t* eth_device_hashArrayTempList[ ETH_NODE_HASH_SIZE ];

int ValidateClient(char *mac)
{
	int ret = 0;
	char buf[200]= {0};
#ifndef _SR213_PRODUCT_REQ_
	char buf1[200];
	FILE *fp2 = NULL;
#endif /*_SR213_PRODUCT_REQ_*/
	FILE *fp1 = NULL;
        errno_t rc = -1;
		//Need to ignore brlan1 - XHS clients when during CB case
        v_secure_system("ip nei show | grep -v brlan1 | grep -i %s | grep -i REACHABLE > " ARP_CACHE, mac);
	if ( (fp1 = fopen(ARP_CACHE, "r")) == NULL )
	{
        	return ret;
	}
	rc  =  memset_s(buf,sizeof(buf),0,sizeof(buf));
        ERR_CHK(rc);
	if(fgets(buf, sizeof(buf), fp1)!= NULL)
	{

			ret = 1;
            		fclose(fp1);
            		unlink(ARP_CACHE);
			return ret;

	}
#ifdef _SR213_PRODUCT_REQ_
	fclose(fp1);
	unlink(ARP_CACHE);
	return ret;
#else
	else
	{
				//Need to ignore brlan1 - XHS clients when during CB case
        		v_secure_system("cat " DNSMASQ_FILE " | grep -v 172.16.12. | grep -i %s > " DNSMASQ_CACHE, mac);
		        if ( (fp2 = fopen(DNSMASQ_CACHE, "r")) == NULL )
		        {
				        printf("not able to open dnsmasq cache file\n");
            			fclose(fp1);
            			unlink(ARP_CACHE);
               			return ret;
       			}
		  rc =	memset_s(buf1,sizeof(buf1),0,sizeof(buf1));
                  ERR_CHK(rc);
			if(fgets(buf1,sizeof(buf1),fp2)!= NULL)
			{
				ret = 1;
			}
	                fclose(fp1);
	                fclose(fp2);
        	        unlink(ARP_CACHE);
        	        unlink(DNSMASQ_CACHE);
			return ret;
	}
#endif /*_SR213_PRODUCT_REQ_*/
}

/* hash() */
unsigned int hash( char *str )
{
    unsigned int hash = 5381;
    int c;

    while ( ( c = *str++ ) ) 
	{
        hash = ( ( hash << 5 ) + hash ) + c; 
    }

    return hash;
}

/* mac_hash() */
unsigned int mac_hash( char *str )
{
    return hash( str ) % ETH_NODE_HASH_SIZE;
}

/* CcspHalExtSw_FindHost() */
eth_device_t* CcspHalExtSw_FindHost( eth_device_t *pstEthHost, eth_device_t* eth_device_ArrayList[ ], unsigned int* puiHashIndex )
{
   char recv_mac_id[ 18 ];
    errno_t                         rc           = -1;
    int                             ind          = -1;

   //Validate received pointer
   if( NULL == pstEthHost )
   {
	   CcspTraceInfo(("%s %d - NULL\n" ,
							  __FUNCTION__,
							  __LINE__ ) );
      return NULL;
   }

   snprintf
    (
        recv_mac_id,
        sizeof( recv_mac_id ),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        pstEthHost->eth_devMacAddress[0],
        pstEthHost->eth_devMacAddress[1],
        pstEthHost->eth_devMacAddress[2],
        pstEthHost->eth_devMacAddress[3],
        pstEthHost->eth_devMacAddress[4],
        pstEthHost->eth_devMacAddress[5]
    );
 
   //get the hash 
   unsigned int hashIndex = mac_hash( recv_mac_id );  
   unsigned int start_index = hashIndex;

//   CcspTraceInfo(("%s %d - RecvMac:%s\n" , __FUNCTION__, __LINE__, recv_mac_id ) );

  //move in array until an empty 
   while( eth_device_ArrayList[hashIndex] != NULL ) 
   {
	 char tmp_mac_id[ 18 ];

		//MAC Conversion
	    snprintf
	    (
			tmp_mac_id,
			sizeof( tmp_mac_id ),
			"%02X:%02X:%02X:%02X:%02X:%02X",
			eth_device_ArrayList[hashIndex]->eth_devMacAddress[0],
			eth_device_ArrayList[hashIndex]->eth_devMacAddress[1],
			eth_device_ArrayList[hashIndex]->eth_devMacAddress[2],
			eth_device_ArrayList[hashIndex]->eth_devMacAddress[3],
			eth_device_ArrayList[hashIndex]->eth_devMacAddress[4],
			eth_device_ArrayList[hashIndex]->eth_devMacAddress[5]
	    );

	   //Compare with received host and current host 
       rc = strcmp_s(tmp_mac_id,sizeof(tmp_mac_id),recv_mac_id,&ind);
       ERR_CHK(rc);
       if((rc == EOK) && (!ind))
	   {
//		     CcspTraceInfo(("%s %d - [Found] RecvMac:%s\n" , __FUNCTION__,__LINE__, recv_mac_id ) );

			//Fill Hash index For Delete Case
			if( NULL != puiHashIndex )
		 	{
		 		*puiHashIndex = hashIndex;
		 	}
			
	        return eth_device_ArrayList[hashIndex]; 
	   }
			
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= ETH_NODE_HASH_SIZE;

      // Dont allow indefinite loop if hash table full.
      if (start_index == hashIndex)
        return NULL;
   }        
	
   return NULL;        
}

/* CcspHalExtSw_AddHost() */
int CcspHalExtSw_AddHost( eth_device_t *pstEthHost, eth_device_t* eth_device_ArrayList[ ], BOOL bIsNeed2Notify )
{
   eth_device_t *pstEthLocalHost;
   char recv_mac_id[ 18 ];
   errno_t rc = -1;

   if (pstEthHost == NULL)
   {
       CcspTraceInfo(("%s %d - pstEthHost Null\n" ,__FUNCTION__,__LINE__ ));
       return -1;
   }

   pstEthLocalHost = malloc(sizeof(eth_device_t));
   if (pstEthLocalHost == NULL)
   {
       CcspTraceInfo(("%s %d - pstEthLocalHost Null\n" ,__FUNCTION__,__LINE__ ));
       return -1;
   }

   //Copy received host details
   rc = memcpy_s(pstEthLocalHost, sizeof(eth_device_t), pstEthHost, sizeof(eth_device_t));
   if(rc != EOK)
   {
      ERR_CHK(rc);
      free(pstEthLocalHost);
      return -1;
   }

   //MAC Conversion
   snprintf
   (
        recv_mac_id,
        sizeof( recv_mac_id ),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        pstEthLocalHost->eth_devMacAddress[0],
        pstEthLocalHost->eth_devMacAddress[1],
        pstEthLocalHost->eth_devMacAddress[2],
        pstEthLocalHost->eth_devMacAddress[3],
        pstEthLocalHost->eth_devMacAddress[4],
        pstEthLocalHost->eth_devMacAddress[5]
   );

   //get the hash 
   unsigned int hashIndex = mac_hash( recv_mac_id );
   unsigned int start_index = hashIndex;

//   CcspTraceInfo(("%s %d - RecvMac:%s\n" , __FUNCTION__, __LINE__, recv_mac_id ) );

   //move in array until an empty or deleted cell
   while( eth_device_ArrayList[hashIndex] != NULL ) 
   {
      //go to next cell
      ++hashIndex;

      //wrap around the table
      hashIndex %= ETH_NODE_HASH_SIZE;
      // Dont allow indefinite loop if hash table full.
      if (start_index == hashIndex)
      {
         free(pstEthLocalHost);
         pstEthLocalHost = NULL;
         return -1;
      }
   }
	
   //Make it online explicitly	
   pstEthLocalHost->eth_Active = 1;
   eth_device_ArrayList[hashIndex] = pstEthLocalHost;

   //Send Notification to consumer
   if( ( AssociatedDevice_callback ) && \
   	   ( TRUE == bIsNeed2Notify )
  	  )
   {
//	 CcspTraceInfo(("%s %d - (+) Ntfn:%s \n" ,__FUNCTION__,__LINE__,recv_mac_id ) );
	  
	 AssociatedDevice_callback( eth_device_ArrayList[ hashIndex ] );
   }

   return 0;
}

/* CcspHalExtSw_DeleteHost() */
int CcspHalExtSw_DeleteHost( eth_device_t *pstEthHost, eth_device_t* eth_device_ArrayList[ ], BOOL bIsNeed2Notify )
{
	unsigned int uiDeleteHashIndex = ETH_NODE_HASH_SIZE;
		
	//Validate received pointer
	if( NULL == pstEthHost )
	{
		 CcspTraceInfo(("%s %d - Null\n" ,
								__FUNCTION__,
								__LINE__ ) );
		 return -1;
	}

	// Find and delete based on hash index
	if( NULL != CcspHalExtSw_FindHost( pstEthHost, eth_device_ArrayList, &uiDeleteHashIndex ) )
	{
		//Send Notification to consumer
		if( ( AssociatedDevice_callback ) && \
			  ( TRUE == bIsNeed2Notify ) 
			)
		 {
//			CcspTraceInfo(("%s %d - (-) Ntfn\n" ,__FUNCTION__,__LINE__ ) );
	
			eth_device_ArrayList[ uiDeleteHashIndex ]->eth_Active = 0;
			AssociatedDevice_callback( eth_device_ArrayList[ uiDeleteHashIndex ] );
		 }
	
		//Free and assign a dummy item at deleted position
		free( eth_device_ArrayList[ uiDeleteHashIndex ] );
		eth_device_ArrayList[ uiDeleteHashIndex ] = NULL; 
	}

	return 0;
}

/* CcspHalExtSw_DeleteAllHosts() */
void CcspHalExtSw_DeleteAllHosts( eth_device_t* eth_device_ArrayList[ ], BOOL bIsNeed2Notify )
{
   int iLoopCount = 0;
	
   for( iLoopCount = 0; iLoopCount< ETH_NODE_HASH_SIZE; iLoopCount++ ) 
   {
      if( eth_device_ArrayList[iLoopCount] != NULL )
      {
		  //Delete and send notification
		  if( ( AssociatedDevice_callback ) && \
				( TRUE == bIsNeed2Notify ) 
			  )
		   {
//	  		   CcspTraceInfo(("%s %d - (-ALL) Ntfn\n" ,__FUNCTION__,__LINE__ ) );
	  
			  eth_device_ArrayList[iLoopCount]->eth_Active = 0;
			  AssociatedDevice_callback( eth_device_ArrayList[ iLoopCount ] );
		   }

		  free( eth_device_ArrayList[iLoopCount] );
		  eth_device_ArrayList[iLoopCount] = NULL;
      }
   }
}

/* CcspHalExtSw_SendNotificationForAllHosts() */
void CcspHalExtSw_SendNotificationForAllHosts( void ) 
{
   int iLoopCount = 0;
	
   for( iLoopCount = 0; iLoopCount< ETH_NODE_HASH_SIZE; iLoopCount++ ) 
   {
      if( eth_device_hashArrayList[iLoopCount] != NULL )
      {
		  //Send notification    
		  if( AssociatedDevice_callback )
		  {
			AssociatedDevice_callback( eth_device_hashArrayList[ iLoopCount ] );
		  }
      }
   }
}

/* CcspHalExtSw_AssociatedDeviceMonitorThread(  ) */
void* CcspHalExtSw_AssociatedDeviceMonitorThread( void *arg )
{
    UNREFERENCED_PARAMETER(arg);
	//Monitor Associated Devices based on periodical time
    while( 1 )
    {
    	eth_device_t *pstRecvEthDevice   	= NULL;
    	ULONG 		  ulTotalEthDeviceCount	= 0;
		INT			  iLoopCount;
		BOOL 		  bProcessFurther		= TRUE;
		static BOOL   isDeleteAllDone	 	= FALSE;


//		CcspTraceInfo(("<EthMonThrd> Iteration Start\n") );

		//Get Associated Device Details from HAL. Do nothing if failure case
    	if( -1 == CcspHalExtSw_getAssociatedDevice( &ulTotalEthDeviceCount, &pstRecvEthDevice ) )
		{
			CcspTraceInfo(("%s %d - Fail to get AssociatedDevice details\n" ,__FUNCTION__,__LINE__ ) );
			bProcessFurther = FALSE;
		}

		if( bProcessFurther )
		{
			/* 
			  * Handle Notification based on Add or Delete cases
			  * -----------------------------------------
			  * 1. Check whether ulTotalEthDeviceCount is greater than 0 or not. 
			  * 1.1 If 0 then we need to delete all hosts and send notification 
			  *
			  * 2. Check whether received client mac is valid or not based on "ip nei show" & "dnsmasq.leases" file
			  * 	
			  * 3. if mac valid then compare with existing list whether this client is available or not
			  * 3.1   if not available then add as a new entry and send connected (+) notification to ethernet 
			  * 3.2   if not available then add as a new entry in temp list only
			  * 
			  * 4. if mac is not valid and compare with existing list,
			  * 4.1  if it is available then delele that entry from list 
			  * 4.2  if it is not available then go to next iteration
			  *
			  * 5. Compare current hash list and temp hash list,
			  * 5.1   if not available then delete this entry in current hash and send disconnected (-) notification to ethernet 
			  * 
			  * 6. Remove all hosts from temp list
			  */
			
			//if 0 then delete all nodes and send disconnected notification to Ethernet	
			if( 0 == ulTotalEthDeviceCount )
			{
				/*
				  * We should not do more than one time when all host disconnected case again
				  * and again
				  */
				if( FALSE == isDeleteAllDone )
				{
					CcspTraceInfo(("<EthMonThrd> - DeleteAllHosts due to count is 0\n") );
					CcspHalExtSw_DeleteAllHosts( eth_device_hashArrayList, TRUE );
					isDeleteAllDone = TRUE;
				}

				bProcessFurther = FALSE;
			}

			if( bProcessFurther )
			{
//				CcspTraceInfo(("<EthMonThrd> - Host(+) Loop Start\n") );

				// Reset isDeleteAllDone variable to proceed further from next iteration
				isDeleteAllDone = FALSE;

				for( iLoopCount = 0; iLoopCount < (int)ulTotalEthDeviceCount; iLoopCount++ )
				{ 
					char tmp_mac_id[ 18 ];
				
					//MAC Conversion
					snprintf
					(
						tmp_mac_id,
						sizeof( tmp_mac_id ),						
						"%02X:%02X:%02X:%02X:%02X:%02X",
						pstRecvEthDevice[ iLoopCount ].eth_devMacAddress[0],
						pstRecvEthDevice[ iLoopCount ].eth_devMacAddress[1],
						pstRecvEthDevice[ iLoopCount ].eth_devMacAddress[2],
						pstRecvEthDevice[ iLoopCount ].eth_devMacAddress[3],
						pstRecvEthDevice[ iLoopCount ].eth_devMacAddress[4],
						pstRecvEthDevice[ iLoopCount ].eth_devMacAddress[5]
					);

					// If valid then it will return 1
					// If invalid then it will return 0
					if( 0 == ValidateClient( tmp_mac_id ) )
					{
                                           //Delete and send notification
                                           CcspHalExtSw_DeleteHost( &pstRecvEthDevice[ iLoopCount ], eth_device_hashArrayList, TRUE );
					   continue;
					}
				
					// If found then it will give host address 
					// If not found then it will give NULL value
					if ( NULL == CcspHalExtSw_FindHost( &pstRecvEthDevice[ iLoopCount ], eth_device_hashArrayList, NULL ) )
					{
						//Add and send notification  
						CcspHalExtSw_AddHost( &pstRecvEthDevice[ iLoopCount ], eth_device_hashArrayList, TRUE );
					}

					//Add in temp hash list and Don't send notification  
					CcspHalExtSw_AddHost( &pstRecvEthDevice[ iLoopCount ], eth_device_hashArrayTempList, FALSE );
				}

//				CcspTraceInfo(("<EthMonThrd> - Host(+) Loop End\n") );

//				CcspTraceInfo(("<EthMonThrd> - Host(-) Loop Start\n") );

				//Disconnection Case
				for( iLoopCount = 0; iLoopCount< ETH_NODE_HASH_SIZE; iLoopCount++ ) 
				{
					// If found then it will give host address 
					// If not found then it will give NULL value
					if ( ( NULL != eth_device_hashArrayList[ iLoopCount ] ) &&\
						 ( NULL == CcspHalExtSw_FindHost( eth_device_hashArrayList[ iLoopCount ], eth_device_hashArrayTempList, NULL ) )
						)
					{
						//Delete and Need to send notification	 
						CcspHalExtSw_DeleteHost( eth_device_hashArrayList[ iLoopCount ], eth_device_hashArrayList, TRUE );
					}
				}

				//Delete all hosts from temp hash list
				CcspHalExtSw_DeleteAllHosts( eth_device_hashArrayTempList, FALSE );

//				CcspTraceInfo(("<EthMonThrd> - Host(-) Loop End\n") );
			}
			//Free if memory is valid case
			if( NULL != pstRecvEthDevice )
			{
				free( pstRecvEthDevice );
				pstRecvEthDevice = NULL;
			}
		}

//		CcspTraceInfo(("<EthMonThrd> Iteration End\n") );
		
		//Sleep
    	sleep( ETH_POLLING_PERIOD );
    }
}
#ifndef _SR213_PRODUCT_REQ_
void CcspHalExtSw_ethAssociatedDevice_callback_register(CcspHalExtSw_ethAssociatedDevice_callback callback_proc)
{
    AssociatedDevice_callback = callback_proc;
    pthread_t GetAssociatedDeviceThread;
    pthread_create(&GetAssociatedDeviceThread, NULL, &CcspHalExtSw_AssociatedDeviceMonitorThread, NULL);  
}
#endif // _SR213_PRODUCT_REQ_
