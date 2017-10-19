/******************************************************************************/
/*                                                                            */
/*    ODYSSEUS/EduCOSMOS Educational-Purpose Object Storage System            */
/*                                                                            */
/*    Developed by Professor Kyu-Young Whang et al.                           */
/*                                                                            */
/*    Database and Multimedia Laboratory                                      */
/*                                                                            */
/*    Computer Science Department and                                         */
/*    Advanced Information Technology Research Center (AITrc)                 */
/*    Korea Advanced Institute of Science and Technology (KAIST)              */
/*                                                                            */
/*    e-mail: kywhang@cs.kaist.ac.kr                                          */
/*    phone: +82-42-350-7722                                                  */
/*    fax: +82-42-350-8380                                                    */
/*                                                                            */
/*    Copyright (c) 1995-2013 by Kyu-Young Whang                              */
/*                                                                            */
/*    All rights reserved. No part of this software may be reproduced,        */
/*    stored in a retrieval system, or transmitted, in any form or by any     */
/*    means, electronic, mechanical, photocopying, recording, or otherwise,   */
/*    without prior written permission of the copyright owner.                */
/*                                                                            */
/******************************************************************************/
/*
 * Module: EduBtM_Fetch.c
 *
 * Description :
 *  Find the first object satisfying the given condition.
 *  If there is no such object, then return with 'flag' field of cursor set
 *  to CURSOR_EOS. If there is an object satisfying the condition, then cursor
 *  points to the object position in the B+ tree and the object identifier
 *  is returned via 'cursor' parameter.
 *  The condition is given with a key value and a comparison operator;
 *  the comparison operator is one among SM_BOF, SM_EOF, SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Exports:
 *  Four EduBtM_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"


/*@ Internal Function Prototypes */
Four edubtm_Fetch(PageID*, KeyDesc*, KeyValue*, Four, KeyValue*, Four, BtreeCursor*);



/*@================================
 * EduBtM_Fetch()
 *================================*/
/*
 * Function: Four EduBtM_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition. See above for detail.
 *
 * Returns:
 *  error code
 *    eBADPARAMETER_BTM
 *    some errors caused by function calls
 *
 * Side effects:
 *  cursor  : The found ObjectID and its position in the Btree Leaf
 *            (it may indicate a ObjectID in an  overflow page).
 */
Four EduBtM_Fetch(
    PageID   *root,		/* IN The current root of the subtree */
    KeyDesc  *kdesc,		/* IN Btree key descriptor */
    KeyValue *startKval,	/* IN key value of start condition */
    Four     startCompOp,	/* IN comparison operator of start condition */
    KeyValue *stopKval,		/* IN key value of stop condition */
    Four     stopCompOp,	/* IN comparison operator of stop condition */
    BtreeCursor *cursor)	/* OUT Btree Cursor */
{
    int i;
    Four e;		   /* error number */

    
    if (root == NULL) ERR(eBADPARAMETER_BTM);

    /* Error check whether using not supported functionality by EduBtM */
    for(i=0; i<kdesc->nparts; i++)
    {
        if(kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

	if (startCompOp == SM_BOF)
		e = edubtm_FirstObject(root, kdesc, stopKval, stopCompOp, cursor);
	else if (startCompOp == SM_EOF)
		e = edubtm_LastObject(root, kdesc, stopKval, stopCompOp, cursor);
	else
		e = edubtm_Fetch(root, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
    if (e < 0) ERR(e);


    return(eNOERROR);

} /* EduBtM_Fetch() */



/*@================================
 * edubtm_Fetch()
 *================================*/
/*
 * Function: Four edubtm_Fetch(PageID*, KeyDesc*, KeyVlaue*, Four, KeyValue*, Four, BtreeCursor*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  Find the first object satisfying the given condition.
 *  This function handles only the following conditions:
 *  SM_EQ, SM_LT, SM_LE, SM_GT, SM_GE.
 *
 * Returns:
 *  Error code *   
 *    eBADCOMPOP_BTM
 *    eBADBTREEPAGE_BTM
 *    some errors caused by function calls
 */
Four edubtm_Fetch(
    PageID              *root,          /* IN The current root of the subtree */
    KeyDesc             *kdesc,         /* IN Btree key descriptor */
    KeyValue            *startKval,     /* IN key value of start condition */
    Four                startCompOp,    /* IN comparison operator of start condition */
    KeyValue            *stopKval,      /* IN key value of stop condition */
    Four                stopCompOp,     /* IN comparison operator of stop condition */
    BtreeCursor         *cursor)        /* OUT Btree Cursor */
{
    Four                e;              /* error number */
    Four                cmp;            /* result of comparison */
    Two                 idx;            /* index */
    PageID              child;          /* child page when the root is an internla page */
    Two                 alignedKlen;    /* aligned size of the key length */
    BtreePage           *apage;         /* a Page Pointer to the given root */
    BtreeOverflow       *opage;         /* a page pointer if it necessary to access an overflow page */
    Boolean             found;          /* search result */
    PageID              *leafPid;       /* leaf page pointed by the cursor */
    Two                 slotNo;         /* slot pointed by the slot */
    PageID              ovPid;          /* PageID of the overflow page */
    PageNo              ovPageNo;       /* PageNo of the overflow page */
    PageID              prevPid;        /* PageID of the previous page */
    PageID              nextPid;        /* PageID of the next page */
    ObjectID            *oidArray;      /* array of the ObjectIDs */
    Two                 iEntryOffset;   /* starting offset of an internal entry */
    btm_InternalEntry   *iEntry;        /* an internal entry */
    Two                 lEntryOffset;   /* starting offset of a leaf entry */
    btm_LeafEntry       *lEntry;        /* a leaf entry */


    /* Error check whether using not supported functionality by EduBtM */
    int i;
    for (i=0; i<kdesc->nparts; i++)
    {
        if (kdesc->kpart[i].type!=SM_INT && kdesc->kpart[i].type!=SM_VARSTRING)
            ERR(eNOTSUPPORTED_EDUBTM);
    }

	e = BfM_GetTrain(root, &apage, PAGE_BUF);
	if (e < 0) ERR(e);

	/* If root page is internal page */
	if (apage->any.hdr.type & INTERNAL) 
	{
		// Decide a child page to find a leaf page to fetch.
		edubtm_BinarySearchInternal(&apage->bi, kdesc, startKval, &idx);

		// Make the child page's PageID and BtreePage.
		if (idx >= 0) 
		{
			iEntry = apage->bi.data + apage->bi.slot[-1*idx];
			MAKE_PAGEID(child, root->volNo, iEntry->spid);
		}
		else 
		{
			MAKE_PAGEID(child, root->volNo, apage->bi.hdr.p0);
		}
		
		// Fetch a object from the child page.
		e = edubtm_Fetch(&child, kdesc, startKval, startCompOp, stopKval, stopCompOp, cursor);
		if (e < 0) ERR(e);
		
		e = BfM_FreeTrain(root, PAGE_BUF);
		if (e < 0) ERR(e);
	}

	/* If root page is leaf page */
	else if (apage->any.hdr.type & LEAF)
	{
		leafPid = root;
		cursor->flag = CURSOR_ON;

		// Filter inappropriate conditions.
		cmp = edubtm_KeyCompare(kdesc, startKval, stopKval);
		if (startCompOp == SM_EQ && 
				((stopCompOp == SM_LT && !(cmp == LESS)) ||
				 (stopCompOp == SM_LE && !(cmp == LESS || cmp == EQUAL)) ||
				 (stopCompOp == SM_GT && !(cmp == GREATER)) ||
				 (stopCompOp == SM_GE && !(cmp == GREATER || cmp == EQUAL))))
			cursor->flag = CURSOR_EOS;
	
		// Search slotNo in the leaf.
		found = edubtm_BinarySearchLeaf(&apage->bl, kdesc, startKval, &slotNo);
		idx = slotNo;
		if (cursor->flag == CURSOR_EOS);
		else if (found == TRUE)
		{
			if (startCompOp == SM_EQ) idx = slotNo;
			else if (startCompOp == SM_LT) idx = slotNo-1;
			else if (startCompOp == SM_LE) idx = slotNo;
			else if (startCompOp == SM_GT) idx = slotNo+1;
			else if (startCompOp == SM_GE) idx = slotNo;
		}
		else
		{
			if (startCompOp == SM_EQ) cursor->flag = CURSOR_EOS;
			else if (startCompOp == SM_LT) idx = slotNo;
			else if (startCompOp == SM_LE) idx = slotNo;
			else if (startCompOp == SM_GT) idx = slotNo+1;
			else if (startCompOp == SM_GE) idx = slotNo+1;
		}

		if (cursor->flag == CURSOR_EOS);
		else if (idx < 0)
		{
			if (apage->bl.hdr.prevPage != NIL)
			{
				MAKE_PAGEID(prevPid, root->volNo, apage->bl.hdr.prevPage);
				leafPid = &prevPid;
				idx = apage->bl.hdr.nSlots-1;
			}
			else
			{
				cursor->flag = CURSOR_EOS;
			}
		}
		else if (idx >= apage->bl.hdr.nSlots)
		{
			if (apage->bl.hdr.nextPage != NIL)
			{
				MAKE_PAGEID(nextPid, root->volNo, apage->bl.hdr.nextPage);
				leafPid = &nextPid;
				idx = 0;
			}
			else
			{
				cursor->flag = CURSOR_EOS;
			}
		}
		e = BfM_FreeTrain(root, PAGE_BUF);
		if (e < 0) ERR(e);
		e = BfM_GetTrain(leafPid, &apage, PAGE_BUF);
		if (e < 0) ERR(e);

		if (cursor->flag != CURSOR_EOS)
		{
			lEntry = apage->bl.data + apage->bl.slot[-1*idx];
			alignedKlen = ALIGNED_LENGTH(lEntry->klen);
			memcpy(&cursor->oid, &lEntry->kval + alignedKlen, sizeof(ObjectID));
			memcpy(&cursor->key, &lEntry->klen, sizeof(KeyValue));
			cursor->leaf = *leafPid;
			cursor->slotNo = idx;

			cmp = edubtm_KeyCompare(kdesc, &cursor->key, stopKval);
			if ((stopCompOp == SM_LT && !(cmp == LESS)) ||
					(stopCompOp == SM_LE && !(cmp == LESS || cmp == EQUAL)) ||
					(stopCompOp == SM_GT && !(cmp == GREATER)) ||
					(stopCompOp == SM_GE && !(cmp == GREATER || cmp == EQUAL)))
				cursor->flag = CURSOR_EOS;	
		}

		e = BfM_FreeTrain(leafPid, PAGE_BUF);
		if (e < 0) ERR(e);
	}
    
	
	return(eNOERROR);
    
} /* edubtm_Fetch() */

