#include "BTreeNode.h"

using namespace std;

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
	RC rc = 0;
	if(pid < 0)	return RC_INVALID_PID;
	if((rc = pf.read(pid, buffer)) < 0)		return rc;
	node_curPid = pid;
	this->pf = pf; 
	memcpy(node_key, buffer, sizeof(int) * 85);
	memcpy(node_rid, buffer + sizeof(int) * 85, sizeof(RecordId) * 85);
	memcpy(&node_nextPid, buffer + (sizeof(int) + sizeof(RecordId) * 85), sizeof(int));
	return 0; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
	RC rc = 0;
	if(pid < 0)	return RC_INVALID_PID;
	memcpy(buffer, node_key, sizeof(int) * 85);
	memcpy(buffer + sizeof(int) * 85, node_rid, sizeof(RecordId) * 85);
	memcpy(buffer + (sizeof(int) + sizeof(RecordId) * 85), &node_nextPid, sizeof(int));
	if((rc = pf.write(pid, buffer)) < 0)	return rc;
	return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
	return keyCount;  
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{	
	if(keyCount >= 85){
		return RC_NODE_FULL;
	}
	int eid;
	locate(key, eid);
	keyCount++; 
	for(int i = keyCount - 1; i > eid; i--){
		node_rid[i] = node_rid[i - 1];
		node_key[i] = node_key[i - 1];
	}
	node_rid[eid] = rid;
	node_key[eid] = key;
	write(node_curPid, pf);
	return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
	int eid;
	locate(key, eid);
	keyCount++; 
	for(int i = keyCount - 1; i > eid; i--){
		node_rid[i] = node_rid[i - 1];
		node_key[i] = node_key[i - 1];
	}
	node_rid[eid] = rid;
	node_key[eid] = key;
	
	for(int i = keyCount / 2; i < keyCount; i++){
		sibling.insert(node_key[i], node_rid[i]);
	}
	sibling.setNextNodePtr(node_nextPid);
	node_nextPid = sibling.getNodePtr();
	keyCount = keyCount / 2;
	write(node_curPid, pf);
	write(sibling.getNodePtr(), pf);
	return 0; 
}

/*
 * Find the entry whose key value is larger than or equal to searchKey
 * and output the eid (entry number) whose key value >= searchKey.
 * Remeber that all keys inside a B+tree node should be kept sorted.
 * @param searchKey[IN] the key to search for
 * @param eid[OUT] the entry number that contains a key larger than or equalty to searchKey
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	int start = 0, end = keyCount - 1;
	while(start < keyCount){
		int mid = (start + keyCount) / 2;
		if(node_key[mid] == searchKey){
			eid = mid;
			return 0;
		}
		else if(node_key[mid] < searchKey){
			start = mid + 1;
		}
		else{
			end = mid;
		}
	}
	eid = end;
	return 0; 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
	if(eid <= 0 || eid > keyCount){
		return RC_INVALID_CURSOR;
	} 
	key = node_key[eid];
	rid = node_rid[eid];
	return 0; 
}

/*
 * Return the pid of the node.
 * @return the PageId of the node 
 */
PageId BTLeafNode::getNodePtr()
{ 
	return node_curPid; 
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return node_nextPid; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{ 
	if(pid <= 0){
		return RC_INVALID_PID;
	}
	node_nextPid = pid;
	return 0; 
}








// ------------------------------------------------------------------------------------------------------

//		BTNonLeafNode

// ------------------------------------------------------------------------------------------------------


/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
	RC rc = 0;
	if(pid < 0)	return RC_INVALID_PID;
	if((rc = pf.read(pid, buffer)) < 0)		return rc;
	node_curPid = pid;
	this->pf = pf;
	memcpy(node_key, buffer, sizeof(int) * MAX_KEY_COUNT);
	memcpy(node_pid, buffer + sizeof(int) * MAX_KEY_COUNT, sizeof(PageId) * MAX_PAGEID_COUNT);
	return 0; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
	RC rc = 0;
	if(pid < 0)	return RC_INVALID_PID;
	memcpy(buffer, node_key, sizeof(int) * MAX_KEY_COUNT);
	memcpy(buffer + sizeof(int) * MAX_KEY_COUNT, node_pid, sizeof(PageId) * MAX_PAGEID_COUNT);
	if((rc = pf.write(pid, buffer)) < 0)	return rc;
	return 0;
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ 
	return keyCount; 
}

/*
 * Return the pid of the node.
 * @return the PageId of the node 
 */
PageId BTNonLeafNode::getNodePtr()
{ 
	return node_curPid; 
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{
	if(keyCount >= MAX_KEY_COUNT)
		return RC_NODE_FULL;
	int pos;
	locateKeyPos(key, pos);
	keyCount++;
	for(int i = keyCount - 1; i > pos; i--) {
		node_pid[i+1] = node_pid[i];
		node_key[i] = node_key[i-1];
	}
	node_pid[pos+1] = pid;
	node_key[pos] = key;
	
	this->write(node_curPid, this->pf);
	return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
	//insert
	int pos;
	locateKeyPos(key, pos);
	keyCount++;
	for(int i = keyCount - 1; i > pos; i--) {
		node_pid[i+1] = node_pid[i];
		node_key[i] = node_key[i-1];
	}
	node_pid[pos+1] = pid;
	node_key[pos] = key;
	
	// split, delete the middle key
	midKey = node_key[keyCount / 2 ];
	PageId headPageId = node_pid[keyCount / 2];
	
	sibling.initializeRoot(node_pid[keyCount/2+1], node_key[keyCount/2+1], node_pid[keyCount/2+2]);
	for(int i = keyCount / 2 + 2; i < keyCount; i++){
		sibling.insert(node_key[i], node_pid[i+1]);
	}
	keyCount = keyCount / 2;
	write(node_curPid, this->pf);
	write(sibling.getNodePtr(), this->pf);
	return 0; 
}


/*
 * Given the searchKey, find the proper position for it to insert and
 * output it in pos.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pos[OUT] the position of the key to insert.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateKeyPos(int searchKey, int& pos)
{
	//leftmost binary search
	int start = 0, end = keyCount - 1;
	while(start <= end) {
		int mid = (end + start)/2;
		if(node_key[mid] >= searchKey){
			end = mid - 1;
		} else {
			start = mid + 1;
		}
	}
	pos = start;
	
	return 0; 
}


/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{
	int pos = -1;
	locateKeyPos(searchKey, pos);
	if(searchKey < node_key[pos]) pos--;
	pid = node_pid[pos+1];
	
	return 0; 
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
	// initialize this node as root
	// reset node_pid and node_key;
	memset(node_pid, -1, MAX_PAGEID_COUNT+1);
	memset(node_key, -1, MAX_KEY_COUNT+1);
	
	node_pid[0] = pid1;
	node_pid[1] = pid2;
	node_key[0] = key;
	keyCount = 1;
	this->write(node_curPid, this->pf);
	
	return 0; 
}
