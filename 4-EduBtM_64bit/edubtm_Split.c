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
 * Module: edubtm_Split.c
 *
 * Description : 
 *  This file has three functions about 'split'.
 *  'edubtm_SplitInternal(...) and edubtm_SplitLeaf(...) insert the given item
 *  after spliting, and return 'ritem' which should be inserted into the
 *  parent page.
 *
 * Exports:
 *  Four edubtm_SplitInternal(ObjectID*, BtreeInternal*, Two, InternalItem*, InternalItem*)
 *  Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 */


#include <string.h>
#include "EduBtM_common.h"
#include "BfM.h"
#include "EduBtM_Internal.h"



/*@================================
 * edubtm_SplitInternal()
 *================================*/
/*
 * Function: Four edubtm_SplitInternal(ObjectID*, BtreeInternal*,Two, InternalItem*, InternalItem*)
 *
 * Description:
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  At first, the function edubtm_SplitInternal(...) allocates a new internal page
 *  and initialize it.  Secondly, all items in the given page and the given
 *  'item' are divided by halves and stored to the two pages.  By spliting,
 *  the new internal item should be inserted into their parent and the item will
 *  be returned by 'ritem'.
 *
 *  A temporary page is used because it is difficult to use the given page
 *  directly and the temporary page will be copied to the given page later.
 *
 * Returns:
 *  error code
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitInternal(
    ObjectID                    *catObjForFile,         /* IN catalog object of B+ tree file */
    BtreeInternal               *fpage,                 /* INOUT the page which will be splitted */
    Two                         high,                   /* IN slot No. for the given 'item' */
    InternalItem                *item,                  /* IN the item which will be inserted */
    InternalItem                *ritem)                 /* OUT the item which will be returned by spliting */
{
    Four                        e;                      /* error number */
    Two                         i;                      /* slot No. in the given page, fpage */
    Two                         j = 0;                  /* slot No. in the splitted pages */
    Two                         k = 0;                  /* slot No. in the new page */
    Two                         maxLoop;                /* # of max loops; # of slots in fpage + 1 */
    Four                        sum = 0;                /* the size of a filled area */
    Boolean                     flag = FALSE;           /* TRUE if 'item' become a member of fpage -> nono~~ */
    PageID                      newPid;                 /* for a New Allocated Page */
    BtreeInternal               *npage;                 /* a page pointer for the new allocated page */
	BtreeInternal				tpage = *fpage;
    Two                         fEntryOffset = 0;       /* starting offset of an entry in fpage */
    Two                         nEntryOffset = 0;       /* starting offset of an entry in npage */
    Two                         entryLen;               /* length of an entry */
    btm_InternalEntry           *fEntry;                /* internal entry in the given page, fpage */
    btm_InternalEntry           *nEntry;                /* internal entry in the new page, npage*/
    Boolean                     isTmp;


	// Allocate new page.
	e = btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
	if (e < 0) ERR(e);

	// Initialize the page to internal page.
	e = edubtm_InitInternal(&newPid, FALSE, FALSE);
	if (e < 0) ERR(e);

	e = BfM_GetTrain(&newPid, &npage, PAGE_BUF);
	if (e < 0) ERR(e);

	// Split and save entries in overflow page(fpage) and new page(npage).
	maxLoop = fpage->hdr.nSlots + 1; 
	for (i=0; i < maxLoop; i++)
	{
		// if half of the fpage is not full, save entry in fpage. 
		if (sum < BI_HALF)
		{
			if ((i+1) < high)
			{
				// copy i-th entry for j-th entry of fpage
				fEntry = tpage.data + tpage.slot[-1*i];
				entryLen = sizeof(ShortPageID) + sizeof(Two) + fEntry->klen;
				memcpy(fpage->data + fEntryOffset, tpage.data + tpage.slot[-1*i], entryLen);
			}
			else if ((i+1) == high)
			{
				// copy item for j-th entry of fpage
				entryLen = sizeof(ShortPageID) + sizeof(Two) + ALIGNED_LENGTH(item->klen);
				memcpy(fpage->data + fEntryOffset, item, entryLen);
			}
			else
			{
				// copy (i-1)-th entry for j-th entry of fpage
				fEntry = tpage.data + tpage.slot[-1*(i-1)];
				entryLen = sizeof(ShortPageID) + sizeof(Two) + fEntry->klen;
				memcpy(fpage->data + fEntryOffset, tpage.data + tpage.slot[-1*(i-1)], entryLen);
			}
			fpage->slot[-1*j] = fEntryOffset;
			fEntryOffset += entryLen;
			fpage->hdr.free = fEntryOffset;
			j++;
		}
		// if half of the fpage is full, save entry in npage.
		else
		{
			if ((i+1) < high)
			{
				// copy i-th entry for k-th entry of npage.
				nEntry = tpage.data + tpage.slot[-1*i];
				entryLen = sizeof(ShortPageID) + sizeof(Two) + nEntry->klen;
				memcpy(npage->data + nEntryOffset, tpage.data + tpage.slot[-1*i], entryLen);
			}
			else if ((i+1) == high)
			{
				// copy item for k-th entry of npage.
				entryLen = sizeof(ShortPageID) + sizeof(Two) + item->klen;
				memcpy(npage->data + nEntryOffset, item, entryLen);
			}
			else
			{
				// copy (i-1)-th entry for k-th entry of npage.
				nEntry = tpage.data + tpage.slot[-1*(i-1)];
				entryLen = sizeof(ShortPageID) + sizeof(Two) + nEntry->klen;
				memcpy(npage->data + nEntryOffset, tpage.data + tpage.slot[-1*(i-1)], entryLen);
			}
			npage->slot[-1*k] = nEntryOffset;
			nEntryOffset += entryLen;
			npage->hdr.free = nEntryOffset;
			k++;
		}
		sum += entryLen + sizeof(Two);
	}
	nEntry = npage->data + npage->slot[0];
	npage->hdr.p0 = nEntry->spid;

	fpage->hdr.nSlots = j;
	npage->hdr.nSlots = k;

	memcpy(ritem, nEntry, sizeof(InternalItem));

	e = BfM_FreeTrain(&newPid, PAGE_BUF);
	if (e < 0) ERR(e);


    return(eNOERROR);
    
} /* edubtm_SplitInternal() */



/*@================================
 * edubtm_SplitLeaf()
 *================================*/
/*
 * Function: Four edubtm_SplitLeaf(ObjectID*, PageID*, BtreeLeaf*, Two, LeafItem*, InternalItem*)
 *
 * Description: 
 * (Following description is for original ODYSSEUS/COSMOS BtM.
 *  For ODYSSEUS/EduCOSMOS EduBtM, refer to the EduBtM project manual.)
 *
 *  The function edubtm_SplitLeaf(...) is similar to edubtm_SplitInternal(...) except
 *  that the entry of a leaf differs from the entry of an internal and the first
 *  key value of a new page is used to make an internal item of their parent.
 *  Internal pages do not maintain the linked list, but leaves do it, so links
 *  are properly updated.
 *
 *  Error code
 *  eDUPLICATEDOBJECTID_BTM
 *    some errors caused by function calls
 *
 * Note:
 *  The caller should call BfM_SetDirty() for 'fpage'.
 */
Four edubtm_SplitLeaf(
    ObjectID                    *catObjForFile, /* IN catalog object of B+ tree file */
    PageID                      *root,          /* IN PageID for the given page, 'fpage' */
    BtreeLeaf                   *fpage,         /* INOUT the page which will be splitted */
    Two                         high,           /* IN slotNo for the given 'item' */
    LeafItem                    *item,          /* IN the item which will be inserted */
    InternalItem                *ritem)         /* OUT the item which will be returned by spliting */
{
    Four                        e;              /* error number */
    Two                         i;              /* slot No. in the given page, fpage */
    Two                         j = 0;          /* slot No. in the splitted pages */
    Two                         k = 0;          /* slot No. in the new page */
    Two                         maxLoop;        /* # of max loops; # of slots in fpage + 1 */
    Four                        sum = 0;        /* the size of a filled area */
    PageID                      newPid;         /* for a New Allocated Page */
    PageID                      nextPid;        /* for maintaining doubly linked list */
    BtreeLeaf                   tpage = *fpage; /* a temporary page for the given page */
    BtreeLeaf                   *npage;         /* a page pointer for the new page */
    BtreeLeaf                   *mpage;         /* for doubly linked list */
    btm_LeafEntry               *itemEntry;     /* entry for the given 'item' */
    btm_LeafEntry               *fEntry;        /* an entry in the given page, 'fpage' */
    btm_LeafEntry               *nEntry;        /* an entry in the new page, 'npage' */
    ObjectID                    *iOidArray;     /* ObjectID array of 'itemEntry' */
    ObjectID                    *fOidArray;     /* ObjectID array of 'fEntry' */
    Two                         fEntryOffset = 0;/* starting offset of 'fEntry' */
    Two                         nEntryOffset = 0;/* starting offset of 'nEntry' */
    Two                         oidArrayNo;     /* element No in an ObjectID array */
    Two                         alignedKlen;    /* aligned length of the key length */
    Two                         itemEntryLen;   /* length of entry for item */
    Two                         entryLen;       /* entry length */
    Boolean                     flag;
    Boolean                     isTmp;
 
    
	// Allocate new page.
	e = btm_AllocPage(catObjForFile, &fpage->hdr.pid, &newPid);
	if (e < 0) ERR(e);

	// Initialize the page to leaf page.
	e = edubtm_InitLeaf(&newPid, FALSE, FALSE);
	if (e < 0) ERR(e);

	e = BfM_GetTrain(&newPid, &npage, PAGE_BUF);
	if (e < 0) ERR(e);

	// Split and save entries in overflow page(fpage) and new page(npage).
	maxLoop = fpage->hdr.nSlots + 1; 
	for (i=0; i < maxLoop; i++)
	{
		// if half of the fpage is not full, save entry in fpage. 
		if (sum < BL_HALF)
		{
			if ((i+1) < high)
			{
				// copy i-th entry for j-th entry of fpage
				fEntry = tpage.data + tpage.slot[-1*i];
				entryLen = 2*sizeof(Two) + ALIGNED_LENGTH(fEntry->klen) + sizeof(ObjectID);
				memcpy(fpage->data + fEntryOffset, tpage.data + tpage.slot[-1*i], entryLen);
			}
			else if ((i+1) == high)
			{
				// copy item for j-th entry of fpage
				fEntry = &item->nObjects;
				entryLen = 2*sizeof(Two) + ALIGNED_LENGTH(item->klen);
				memcpy(fpage->data + fEntryOffset, fEntry, entryLen);
				memcpy(fpage->data + fEntryOffset + entryLen, &item->oid, sizeof(ObjectID));
				entryLen += sizeof(ObjectID);
			}
			else
			{
				// copy (i-1)-th entry for j-th entry of fpage
				fEntry = tpage.data + tpage.slot[-1*(i-1)];
				entryLen = 2*sizeof(Two) + ALIGNED_LENGTH(fEntry->klen) + sizeof(ObjectID);
				memcpy(fpage->data + fEntryOffset, tpage.data + tpage.slot[-1*(i-1)], entryLen);
			}
			fpage->slot[-1*j] = fEntryOffset;
			fEntryOffset += entryLen;
			fpage->hdr.free = fEntryOffset;
			j++;
		}
		// if half of the fpage is full, save entry in npage.
		else
		{
			if ((i+1) < high)
			{
				// copy i-th entry for k-th entry of npage.
				nEntry = tpage.data + tpage.slot[-1*i];
				entryLen = 2*sizeof(Two) + ALIGNED_LENGTH(nEntry->klen) + sizeof(ObjectID);
				memcpy(npage->data + nEntryOffset, tpage.data + tpage.slot[-1*i], entryLen);
			}
			else if ((i+1) == high)
			{
				// copy item for k-th entry of npage.
				fEntry = &item->nObjects;
				entryLen = 2*sizeof(Two) + ALIGNED_LENGTH(item->klen);
				memcpy(npage->data + nEntryOffset, fEntry, entryLen);
				memcpy(npage->data + nEntryOffset + entryLen, &item->oid, sizeof(ObjectID));
				entryLen += sizeof(ObjectID);
			}
			else
			{
				// copy (i-1)-th entry for k-th entry of npage.
				nEntry = tpage.data + tpage.slot[-1*(i-1)];
				entryLen = 2*sizeof(Two) + ALIGNED_LENGTH(nEntry->klen) + sizeof(ObjectID);
				memcpy(npage->data + nEntryOffset, tpage.data + tpage.slot[-1*(i-1)], entryLen);
			}
			npage->slot[-1*k] = nEntryOffset;
			nEntryOffset += entryLen;
			npage->hdr.free = nEntryOffset;
			k++;
		}
		sum += entryLen + sizeof(Two);
	}
	fpage->hdr.nSlots = j;
	npage->hdr.nSlots = k;

	// Insert the allocated page into doubly liked list of leaf pages.
	npage->hdr.prevPage = fpage->hdr.pid.pageNo;
	npage->hdr.nextPage = fpage->hdr.nextPage;
	fpage->hdr.nextPage = npage->hdr.pid.pageNo;

	// Make internal index entry that points the allocated page.
	ritem->spid = npage->hdr.pid.pageNo;
	nEntry = npage->data + npage->slot[0];
	memcpy(&ritem->klen, &nEntry->klen, sizeof(InternalItem) - sizeof(ShortPageID));

	e = BfM_FreeTrain(&newPid, PAGE_BUF);
	if (e < 0) ERR(e);


	return(eNOERROR);
    
} /* edubtm_SplitLeaf() */
