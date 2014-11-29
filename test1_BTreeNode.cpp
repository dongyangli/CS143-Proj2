//
//  test1_BTreeNode
//
//  Created by Dongyang Li on 11/22/14.
//  Copyright (c) 2014 Dongyang Li. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include "SqlEngine.h"


#include <cassert>

using namespace std;

RC testLeafNode_insert1(){
    cout << "in testLeafNode_insert1 now" << endl;
    
    RC     rc;
    PageFile pf;
    if ((rc = pf.open("testLeafNode_insert1.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    PageId eid = pf.endPid();
    BTLeafNode leafNode;
    leafNode.read(eid, pf);
    cout<<"current(before insert) keyCount is " << leafNode.getKeyCount() <<endl;
    
    RecordId rid;
    int key = -1;
    rid.pid = 0;
    rid.sid = 0;
    leafNode.insert(key, rid);
    assert(leafNode.getKeyCount() == 1);
    std::cout << "Execution continues past the keyCount assert\n";
    // test keys;
    leafNode.print();
    int* keys;
    leafNode.getKeysPtr(&keys);
    assert(keys[0] == key);
    std::cout << "Execution continues past the keys assert\n";
    
}

RC testLeafNode_insert50(){
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf("test_50keys.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    PageFile pf;
    if ((rc = pf.open("testLeafNode_insert50.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    PageId eid = pf.endPid();
    BTLeafNode leafNode;
    leafNode.read(eid, pf);
    //cout<<"current(before insert) keyCount is " << leafNode.getKeyCount() <<endl;
    //cout << "Start parsing and inserting...\n";
    string line;
    while(std::getline(inf, line)) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        RecordId rid;
        rid.pid = 0;
        rid.sid = 0;
        leafNode.insert(key, rid);
    }
    //leafNode.print();
    leafNode.write(eid, pf);
    cout << "Print done...\n";
    
    assert(leafNode.getKeyCount() == 50);
    std::cout << "Execution continues past the keyCount assert\n";
    
    inf.close();
    return 0;
}



RC testNonLeafNode_insert1(){
    cout << "in testNonLeafNode_insert1 now" << endl;
    
    RC     rc;
    PageFile pf;
    if ((rc = pf.open("testNonLeafNode_insert1.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    PageId eid = pf.endPid();
    BTNonLeafNode nonLeafNode;
    nonLeafNode.read(eid, pf);
    //cout<<"current(before insert) keyCount is " << nonLeafNode.getKeyCount() <<endl;
    
    PageId pid1 = 0;
    int key0 = 0;
    PageId pid2 = 1;
    
    nonLeafNode.initializeRoot(pid1, key0, pid2);
    int key = -1;
    PageId pid = 2;
    nonLeafNode.insert(key, pid);
    assert(nonLeafNode.getKeyCount() == 2);
    std::cout << "Execution continues past the keyCount assert\n";
    // test keys;
    nonLeafNode.print();
    int* keys;
    nonLeafNode.getKeysPtr(&keys);
    assert(keys[0] == key && keys[1] == key0);
    std::cout << "Execution continues past the keys assert\n";
    PageId* pids;
    nonLeafNode.getPageIdsPtr(&pids);
    assert(pids[0] == pid1 && pids[1] == pid && pids[2] == pid2);
    std::cout << "Execution continues past the pids assert\n";
    
    
}

RC testNodLeafNode_insert50(){
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf("test_50keys.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    PageFile pf;
    if ((rc = pf.open("testNodLeafNode_insert50.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    PageId eid = pf.endPid();
    BTNonLeafNode nonLeafNode;
    nonLeafNode.read(eid, pf);
    string line;
    while(std::getline(inf, line)) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        PageId pid = 0;
        nonLeafNode.insert(key, pid);
    }
    nonLeafNode.print();
    //cout << "Print done...\n";
    // test write
    nonLeafNode.write(eid, pf);
    nonLeafNode.read(eid, pf);
    nonLeafNode.print();
    
    assert(nonLeafNode.getKeyCount() == 50);
    std::cout << "Execution continues past the keyCount assert\n";
    
    inf.close();
    return 0;
}


RC testLeafNode_insertAndSplit(){
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf("movie.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    PageFile pf;
    if ((rc = pf.open("testLeafNode_insertAndSplit.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    
    PageId eid = pf.endPid();
    BTLeafNode leafNode;
    BTLeafNode siblingLeafNode;
    PageId siblingEid;
    int siblingKey;
    
    leafNode.read(eid, pf);
    string line;
    int count = 0;
    while(std::getline(inf, line) && count < 90) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        RecordId rid;
        rid.pid = 0;
        rid.sid = 0;
        if((rc = leafNode.insert(key, rid)) < 0){
            siblingEid = pf.endPid();
            siblingLeafNode.read(siblingEid, pf);
            leafNode.insertAndSplit(key, rid, siblingLeafNode, siblingKey);
            siblingLeafNode.write(siblingEid, pf);
            assert(leafNode.getKeyCount() == 42);// 84/2
            // test the NodePtr func
            assert(siblingLeafNode.getNodePtr() == siblingEid);
            assert(leafNode.getNextNodePtr() == siblingLeafNode.getNodePtr());
            
        }
        count++;
    }
    //leafNode.print();
    leafNode.write(eid, pf);
    //cout << "Print done...\n";
    
    assert(leafNode.getKeyCount() == 47);// 84/2 + 4
    assert(siblingLeafNode.getKeyCount() == 43);
    std::cout << "Execution continues past the keyCount assert\n";
    
    assert(leafNode.getNextNodePtr() == siblingEid);
    std::cout << "Execution continues past the getNextNodePtr assert\n";
    
    //assert(siblingKey == 2339);
    //std::cout << "Execution continues past the siblingKey assert\n";
    //siblingLeafNode.print();
    //cout << "Print done...\n";
    
    inf.close();
    return 0;
    
    
}


RC testNonLeafNode_insertAndSplit() {
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf("movie.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    PageFile pf;
    if ((rc = pf.open("testNonLeafNode_insertAndSplit.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    
    PageId eid = pf.endPid();
    BTNonLeafNode nonleafNode;
    BTNonLeafNode siblingNonLeafNode;
    PageId siblingEid;
    int midKey;
    
    nonleafNode.read(eid, pf);
    string line;
    int count = 0;
    while(std::getline(inf, line) && count < 130) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        PageId pid = 0;
        if((rc = nonleafNode.insert(key, pid)) < 0){
            siblingEid = pf.endPid();
            siblingNonLeafNode.read(siblingEid, pf);
            nonleafNode.insertAndSplit(key, pid, siblingNonLeafNode, midKey);
            siblingNonLeafNode.write(siblingEid, pf);
            assert(nonleafNode.getKeyCount() == 64);// 128/2
        }
        count++;
    }
    nonleafNode.print();
    nonleafNode.write(eid, pf);
    cout << "Print done...\n";
    
    assert(nonleafNode.getKeyCount() == 66);// 128/2 + 3
    assert(siblingNonLeafNode.getKeyCount() == 63);// 128/2 - 1
    std::cout << "Execution continues past the keyCount assert\n";
    
    //assert(midKey == 2339);
    //std::cout << "Execution continues past the siblingKey assert\n";
    siblingNonLeafNode.print();
    cout << "Print done...\n";
    
    inf.close();
    return 0;
    
    
}


RC testBTreeIndexInsert1(){
    
    RC     rc;
    int    key;
    string value;
    
    BTreeIndex idx;
    if ((rc = idx.open("testBTreeIndexInsert1.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    key = 0;
    RecordId rid;
    
    
    if ((rc = idx.insert(key, rid)) < 0){
        cout << "Failed to insert\n";
        return rc;
    }
    
    int treeHeight;
    PageId rootPid;
    idx.get_rootPid(rootPid);
    idx.get_treeHeight(treeHeight);
    //std::cout << "Current treeHeight is " << treeHeight <<endl;
    //test
    assert(rootPid == 1);
    std::cout << "Execution continues past the getRootPid assert\n";
    assert(treeHeight == 1);
    std::cout << "Execution continues past the TreeHeight assert\n";
    
    idx.close();
    return 0;
}

RC testBTreeIndexInsert50(){
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf("movie.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    BTreeIndex idx;
    if ((rc = idx.open("testBTreeIndexInsert50.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    RecordId rid;
    
    string line;
    int count = 0;
    while(std::getline(inf, line) && count < 50) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        PageId pid = 0;
        if ((rc = idx.insert(key, rid)) < 0){
            cout << "Failed to insert\n";
            return rc;
        }
        count++;
    }
    
    int treeHeight;
    PageId rootPid;
    idx.get_rootPid(rootPid);
    idx.get_treeHeight(treeHeight);
    
    //test
    assert(rootPid == 1);
    std::cout << "Execution continues past the getRootPid assert\n";
    assert(treeHeight == 1);
    std::cout << "Execution continues past the TreeHeight assert\n";
    
    idx.close();
    inf.close();
    return 0;
}

RC testBTreeIndexInsert10(){
    
    // modify the LeafNode keyCount to 3, and NonLeafNode keyCount to 3 as well
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf("movie.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    BTreeIndex idx;
    if ((rc = idx.open("testBTreeIndexInsert10.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    RecordId rid;
    
    string line;
    int count = 0;
    while(std::getline(inf, line) && count < 10) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        PageId pid = 0;
        if ((rc = idx.insert(key, rid)) < 0){
            cout << "Failed to insert\n";
            return rc;
        }
        count++;
    }
    
    int treeHeight;
    PageId rootPid;
    idx.get_rootPid(rootPid);
    idx.get_treeHeight(treeHeight);
    
    //test
    assert(rootPid == 8);
    std::cout << "Execution continues past the getRootPid assert\n";
    assert(treeHeight == 3);
    std::cout << "Execution continues past the TreeHeight assert\n";
    idx.printRootNode();
    idx.close();
    inf.close();
    return 0;
}

RC testBTreeIndexReadForward(){
    RC     rc;
    int    key;
    RecordId rid;
    string value;
    
    std::ifstream inf("movie.del");
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    BTreeIndex idx;
    if ((rc = idx.open("testBTreeIndexReadForward.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    string line;
    int count = 0;
    while(std::getline(inf, line) && count < 10) {
        // process the line
        if((rc = SqlEngine::parseLoadLine(line, key, value))<0) {
            cout << "parse line failed" <<endl;
            return rc;
        }
        PageId pid = 0;
        if ((rc = idx.insert(key, rid)) < 0){
            cout << "Failed to insert\n";
            return rc;
        }
        count++;
    }
    
    
    IndexCursor cursor;
    cursor.pid = 1;
    cursor.eid = 1;
    while(idx.readForward(cursor, key, rid)>=0){
        cout << "the key here is "<< key <<endl;
        idx.printLeafNode(cursor.pid);
    }
    
    cout << "---------------------------------------------------------------------------" <<endl;
    
    //IndexCursor cursor;
    cursor.pid = 1;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 272);
    std::cout << "Execution continues past the readForward Key assert\n";

    cursor.pid = 1;
    cursor.eid = 1;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 1578);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 5;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 2244);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 5;
    cursor.eid = 1;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 2342);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 2;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 2634);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 2;
    cursor.eid = 1;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 2965);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 4;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 3084);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 4;
    cursor.eid = 1;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 3229);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 6;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 3992);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 6;
    cursor.eid = 1;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 4589);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 3;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 2244);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 3;
    cursor.eid = 1;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 2634);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    cursor.pid = 7;
    cursor.eid = 0;
    idx.readForward(cursor, key, rid);
    cout << "In pid: "<<cursor.pid<<" and eid: "<<cursor.eid<<", the key is:" <<key <<endl;
    assert(key == 3992);
    std::cout << "Execution continues past the readForward Key assert\n";
    
    idx.close();
    inf.close();
    return 0;
}

int main(int argc, const char * argv[]) {
    
    cout << "hello world!" << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv "<<i<<" is " << argv[i] <<endl;
    }
//    testLeafNode_insert1();
//    testLeafNode_insert50();
//    testNonLeafNode_insert1();
//    testNodLeafNode_insert50();
//    
//    testLeafNode_insertAndSplit();
//    testNonLeafNode_insertAndSplit();
    
    
    // test index
    //testBTreeIndexInsert1();
    //testBTreeIndexInsert50();
    //testBTreeIndexInsert10();
    testBTreeIndexReadForward();
    //testBTreeIndexLocate();
    
    return 0;
    
}
