/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 5/28/2008
 */

#ifndef BTNODE_H
#define BTNODE_H

#include "RecordFile.h"
#include "PageFile.h"
/**
 * BTLeafNode: The class representing a B+tree leaf node.
 */
class BTLeafNode {
  public:
	  
  	static const int MAX_KEY_COUNT = 84;  //84 // used 3 for testing index
  	static const int MAX_RECORDID_COUNT = 84; //84 // used 3 for testing index
	  
   /**
    * Insert the (key, rid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param rid[IN] the RecordId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insert(int key, const RecordId& rid);

   /**
    * Insert the (key, rid) pair to the node
    * and split the node half and half with sibling.
    * The first key of the sibling node is returned in siblingKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert.
    * @param rid[IN] the RecordId to insert.
    * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
    * @param siblingKey[OUT] the first key in the sibling node after split.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey);

   /**
    * Find the index entry whose key value is larger than or equal to searchKey
    * and output the eid (entry id) whose key value &gt;= searchKey.
    * Remember that keys inside a B+tree node are sorted.
    * @param searchKey[IN] the key to search for.
    * @param eid[OUT] the entry number that contains a key larger              
    *                 than or equalty to searchKey.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC locate(int searchKey, int& eid);

   /**
    * Read the (key, rid) pair from the eid entry.
    * @param eid[IN] the entry number to read the (key, rid) pair from
    * @param key[OUT] the key from the slot
    * @param rid[OUT] the RecordId from the slot
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC readEntry(int eid, int& key, RecordId& rid);

   /**
    * Return the pid of the current node.
    * @return the PageId of the current node 
    */
    PageId getNodePtr();
	
   /**
    * Return the pid of the next slibling node.
    * @return the PageId of the next sibling node 
    */
    PageId getNextNodePtr();

   /**
    * Set the next slibling node PageId.
    * @param pid[IN] the PageId of the next sibling node 
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC setNextNodePtr(PageId pid);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();
	
   /**
    * Return the first pid. To be used in BTreeIndex
    * @pid the first PageId stored in the node 
    */
	RC getFirstEid( int eid);
 
   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);
	
	// used for tests
	RC print();
	RC getKeysPtr(int** keys);
	RC getRecordIdsPtr(RecordId** recordIds);

  private:
   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */
    char buffer[PageFile::PAGE_SIZE];
	int keyCount;
	int node_key[MAX_KEY_COUNT+1]; //84, the extra one is for splitting
	RecordId node_rid[MAX_RECORDID_COUNT+1]; //84, the extra one is for splitting
	PageFile pf;
	PageId node_curPid;
	PageId node_nextPid; 
}; 


/**
 * BTNonLeafNode: The class representing a B+tree nonleaf node.
 */
class BTNonLeafNode {
  public:
	  
	static const int MAX_KEY_COUNT = 127;  //127 // used 3 for testing index
	static const int MAX_PAGEID_COUNT = 128;  //128 // used 4 for testing index
   /**
    * Insert a (key, pid) pair to the node.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @return 0 if successful. Return an error code if the node is full.
    */
    RC insert(int key, PageId pid);

   /**
    * Insert the (key, pid) pair to the node
    * and split the node half and half with sibling.
    * The sibling node MUST be empty when this function is called.
    * The middle key after the split is returned in midKey.
    * Remember that all keys inside a B+tree node should be kept sorted.
    * @param key[IN] the key to insert
    * @param pid[IN] the PageId to insert
    * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
    * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey);

   /**
    * Given the searchKey, find the child-node pointer to follow and
    * output it in pid.
    * Remember that the keys inside a B+tree node are sorted.
    * @param searchKey[IN] the searchKey that is being looked up.
    * @param pid[OUT] the pointer to the child node to follow.
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC locateChildPtr(int searchKey, PageId& pid);

   /**
    * Initialize the root node with (pid1, key, pid2).
    * @param pid1[IN] the first PageId to insert
    * @param key[IN] the key that should be inserted between the two PageIds
    * @param pid2[IN] the PageId to insert behind the key
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC initializeRoot(PageId pid1, int key, PageId pid2);

   /**
    * Return the number of keys stored in the node.
    * @return the number of keys in the node
    */
    int getKeyCount();
	
   /**
    * Return the pid of the current node.
    * @return the PageId of the current node 
    */
	PageId getNodePtr();
	
   /**
    * Return the first pid. To be used in BTreeIndex
    * @pid the first PageId stored in the node 
    */
	RC getFirstPid( PageId& pid );

   /**
    * Read the content of the node from the page pid in the PageFile pf.
    * @param pid[IN] the PageId to read
    * @param pf[IN] PageFile to read from
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC read(PageId pid, const PageFile& pf);
    
   /**
    * Write the content of the node to the page pid in the PageFile pf.
    * @param pid[IN] the PageId to write to
    * @param pf[IN] PageFile to write to
    * @return 0 if successful. Return an error code if there is an error.
    */
    RC write(PageId pid, PageFile& pf);
	
	// used for tests
	RC print();
	RC getKeysPtr(int** keys);
	RC getPageIdsPtr(PageId** pageIds);

  private:
	  
   /*
    * Given the searchKey, find the proper position for it to insert and
    * output it in pos.
    * @param searchKey[IN] the searchKey that is being looked up.
    * @param pos[OUT] the position of the key to insert.
	* @return 0 if successful. Return an error code if there is an error.
    */
    RC locateKeyPos(int searchKey, int& pos);
	
   /**
    * The main memory buffer for loading the content of the disk page 
    * that contains the node.
    */	
    char buffer[PageFile::PAGE_SIZE];
	int keyCount;
	int node_key[MAX_KEY_COUNT+1]; //127, the extra one is for splitting
	PageId node_pid[MAX_PAGEID_COUNT+1]; //128, the extra one is for splitting
	PageFile pf;
	PageId node_curPid;
	//PageId node_parentPid; 
}; 

#endif /* BTNODE_H */
