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

int main(int argc, const char * argv[]) {
    // insert code here...
    cout << "hello world!" << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv "<<i<<" is " << argv[i] <<endl;
    }
    testLeafNode_insert1();
    testLeafNode_insert50();
    testNonLeafNode_insert1();
    testNodLeafNode_insert50();
    
    testLeafNode_insertAndSplit();
    testNonLeafNode_insertAndSplit();
    return 0;
    
}
