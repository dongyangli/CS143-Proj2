/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

static int getTreeHeight(const char* page);

static void setTreeHeight(char* page, int treeHeight);

static PageId getRootId(const char* page);

static void setRootId(char* page, PageId rootId);

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
	ePid = 0;
    rootPid = -1;
	treeHeight = 0;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	RC rc;
	char page[PageFile::PAGE_SIZE];
	
	///open the page file
	if((rc = pf.open(indexname, mode)) < 0)	return rc;
	
	ePid = pf.endPid();
	rootPid = -1;
	treeHeight = 0;
	if(ePid == 0){
		//printf("current ePid is %d\n", ePid);
		ePid++;
		setRootId(page, rootPid);
		setTreeHeight(page, treeHeight);
		if((rc = pf.write(0, page)) < 0)	return rc;
	}
	else{
		if((rc = pf.read(0, page)) < 0){
			ePid = 0;
			pf.close();
			return rc;
		}
		rootPid = getRootId(page);
		treeHeight = getTreeHeight(page);
	}

    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC rc;
	int newKey;
	PageId newPid;
	char page[PageFile::PAGE_SIZE];
	
	///B+ tree is empty
	if(rootPid == -1){
		BTLeafNode newRoot;
		newRoot.read(ePid++, pf);
		newRoot.insert(key, rid);
		rootPid = ePid - 1;
		newRoot.write(rootPid, pf);
		
		treeHeight++;
		setRootId(page, rootPid);
		setTreeHeight(page, treeHeight);
		if((rc = pf.write(0, page)) < 0)	return rc;
		return 0;
	}
	
	//printf("current treeHeight is %d\n", treeHeight);
	if(treeHeight == 1){
		//printf("begin insert in LeafNode index\n");
		//printf("current rootPid is %d\n", rootPid);
		insert(key, rid, rootPid, newKey, newPid);
		//printf("current newPid is %d\n", newPid);
	}
	else{
		//printf("begin insert in NonLeafNode index\n");
		insert(key, rid, rootPid, 1, newKey, newPid);
	}
	
	///new root
	if(newPid != -1){
		//printf("New Root generated...\n");
		BTNonLeafNode newRoot;
		newRoot.read(ePid++, pf);
		newRoot.initializeRoot(rootPid, newKey, newPid);
		rootPid = ePid - 1;
		newRoot.write(rootPid, pf);
		treeHeight++;
		setRootId(page, rootPid);
		setTreeHeight(page, treeHeight);
		if((rc = pf.write(0, page)) < 0)	return rc;
	}
	return 0;
}

/*
 * Find the leaf-node index entry whose key value is larger than or 
 * equal to searchKey, and output the location of the entry in IndexCursor.
 * IndexCursor is a "pointer" to a B+tree leaf-node entry consisting of
 * the PageId of the node and the SlotID of the index entry.
 * Note that, for range queries, we need to scan the B+tree leaf nodes.
 * For example, if the query is "key > 1000", we should scan the leaf
 * nodes starting with the key value 1000. For this reason,
 * it is better to return the location of the leaf node entry 
 * for a given searchKey, instead of returning the RecordId
 * associated with the searchKey directly.
 * Once the location of the index entry is identified and returned 
 * from this function, you should call readForward() to retrieve the
 * actual (key, rid) pair from the index.
 * @param key[IN] the key to find.
 * @param cursor[OUT] the cursor pointing to the first index entry
 *                    with the key value.
 * @return error code. 0 if no error.
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	RC rc;
	int level = 1;
	PageId curPid = rootPid;
	while(level < treeHeight){
		BTNonLeafNode curNode;
		curNode.read(curPid, pf);
		curNode.locateChildPtr(searchKey, curPid);
		level++;
	}
	BTLeafNode curLeafNode;
	curLeafNode.read(curPid, pf);
	curLeafNode.locate(searchKey, cursor.eid);
	cursor.pid = curPid;
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	RC rc;
	BTLeafNode curNode;
	if((rc = curNode.read(cursor.pid, pf) ) < 0) return rc;
	if((rc = curNode.readEntry(cursor.eid, key, rid) ) < 0) return rc;
	if(cursor.eid == curNode.getKeyCount()-1){
		cursor.pid = curNode.getNextNodePtr();
		//printf("I am node %d, and my next node is %d \n", curNode.getNodePtr(), curNode.getNextNodePtr());
		cursor.eid = 0;
	}
	else{
		cursor.eid++;
	}
    return 0;
}

/*
 * Nonleaf Node
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @param pid[IN] the PageId of the current node
 * @param level[IN] the level of current node in the B+ tree
 * @param newPid[OUT] the new pid of the new slibing node
 * @param newKey[OUT] the new key of the last level
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid, PageId pid, int level, int &newKey, int &newPid)
{
	RC rc;
	BTNonLeafNode curNode, sibling;
	PageId nextPid, siblingPid;
	int midKey;
	
	curNode.read(pid, pf);
	curNode.locateChildPtr(key, nextPid);
	//printf("nextPid is %d \n", nextPid);
	
	//printf("level is %d\n", level);
	if(level + 1 == treeHeight){
		//printf("NonLeafNode insert into LeafNode starts\n");
		insert(key, rid, nextPid, midKey, siblingPid);
	}
	else{
		//printf("NonLeafNode insert into NonLeafNode starts\n");
		insert(key, rid, nextPid, level + 1, midKey, siblingPid);
	}
	
	//printf("insert from children to curNode\n");
	newPid = -1;
	if(siblingPid != -1){
		if(curNode.getKeyCount() < BTNonLeafNode::MAX_KEY_COUNT){
			curNode.insert(midKey, siblingPid);
			if((rc = curNode.write(pid, pf)) < 0){
				printf("NonLeafNode insert write failed\n");
				return rc;
			}
		}
		else{
			sibling.read(ePid++, pf);
			curNode.insertAndSplit(midKey, siblingPid, sibling, newKey);
			newPid = sibling.getNodePtr();
			if((rc = curNode.write(pid, pf)) < 0 || (rc = sibling.write(newPid, pf)) < 0){
				printf("NonLeafNode insertAndSplit write failed\n");
				return rc;
			}
		}
	}
	return 0;
}
/*
 * Leaf Node
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @param pid[IN] the PageId of the current node
 * @param newPid[OUT] the new pid of the new slibing node
 * @param newKey[OUT] the new key of the last level
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid, PageId pid, int &newKey, int &newPid)
{
	RC rc;
	BTLeafNode curNode, sibling;
	if((rc = curNode.read(pid, pf)) < 0){
		printf("LeafNode read failed!, pid to read here is %d, and ePid of pf is %d\n", pid, pf.endPid());
	}
	newPid = -1;
	if(curNode.getKeyCount() < BTLeafNode::MAX_KEY_COUNT){
		//printf("LeafNode insert now\n");
		curNode.insert(key, rid);
		if((rc = curNode.write(pid, pf)) < 0){
			printf("LeafNode insert write failed\n");
			return rc;
		}
	}
	else{
		//printf("LeafNode insertAndSplit now\n");
		newPid = ePid++;
		sibling.read(newPid, pf);
		curNode.insertAndSplit(key, rid, sibling, newKey);
		if((rc = curNode.write(pid, pf)) < 0 || (rc = sibling.write(newPid, pf)) < 0){
			printf("LeafNode insertAndSplit write failed\n");
			return rc;
		}
	}
	return 0;
}


RC BTreeIndex::get_rootPid(PageId& pid){
	pid = rootPid;
	return 0;
}
RC BTreeIndex::get_treeHeight(int& treeH){
	treeH = treeHeight;
	return 0;
}

RC BTreeIndex::printRootNode(){
	if(rootPid == -1){
		printf("The tree is empty now!\n");
		return 0;
	}
	if(treeHeight == 1){
		BTLeafNode rootNode;
		rootNode.read(rootPid, pf);
		rootNode.print();
		
	} else {
		BTNonLeafNode rootNode;
		rootNode.read(rootPid, pf);
		rootNode.print();
	}
	return 0;
}

RC BTreeIndex::printLeafNode(PageId pid){
 	
	if(rootPid == -1){
		printf("The tree is empty now!\n");
		return 0;
	}
	BTLeafNode rootNode;
	rootNode.read(pid, pf);
	rootNode.print();
	
	return 0;
}

static int getTreeHeight(const char* page){
	int treeHeight;
	memcpy(&treeHeight, page, sizeof(int));
	return treeHeight;
}

static void setTreeHeight(char* page, int treeHeight){
	memcpy(page, &treeHeight, sizeof(int));
}

static PageId getRootId(const char* page){
	PageId rootId;
	memcpy(&rootId, page + sizeof(int), sizeof(PageId));
	return rootId;
}

static void setRootId(char* page, PageId rootId){
	memcpy(page + sizeof(int), &rootId, sizeof(PageId));
}


