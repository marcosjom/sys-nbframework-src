//
//  AUApp.h
//  GNavNucleo
//
//  Created by Marcos Ortega on 13/03/14.
//
//

#ifndef NBMngrStoreDefs_h
#define NBMngrStoreDefs_h

typedef enum ENStoreResult_ {
	ENStoreResult_Busy = 0,		//working
	ENStoreResult_NoChanges,	//no changes (no items restores, item already purchased)
	ENStoreResult_Success,		//
	ENStoreResult_Error,		//
	ENStoreResult_Count
} ENStoreResult;

typedef enum ENStoreProdType_ {
	ENStoreProdType_InApp = 0,		//Managed product (consumable or non-consumable)
	ENStoreProdType_Subscription,	//Subscription
	ENStoreProdType_Count
} ENStoreProdType;

typedef enum ENStorePurchaseActionBit_ {
	ENStorePurchaseActionBit_None			= 0,		//No action
	ENStorePurchaseActionBit_Acknowledge	= 1,		//Acknowledge (if necesary)
	ENStorePurchaseActionBit_Consume		= 2,		//Consume
	ENStorePurchaseActionBit_ConsumeIfAcknowledged = 4,	//Consume only if was acknoloege
	ENStorePurchaseActionBits_All			= (ENStorePurchaseActionBit_Acknowledge | ENStorePurchaseActionBit_Consume | ENStorePurchaseActionBit_ConsumeIfAcknowledged)
} ENStorePurchaseActionBit;

typedef struct STAppStoreProdId_ {
	char*			prodId;
	ENStoreProdType	type;
	char*			grpId;
	UI32			actions;			//ENStorePurchaseActionBit
	float			actionsSecsWait;	//Wait before applying actions
	float			actionsSecsRetry;	//Wait before retrying applying actions after failure
} STAppStoreProdId;

#endif
