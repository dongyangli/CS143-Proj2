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
    if ((rc = pf.open("test.idx", 'w')) < 0){
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
    if ((rc = pf.open("test.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    PageId eid = pf.endPid();
    BTLeafNode leafNode;
    leafNode.read(eid, pf);
    cout<<"current(before insert) keyCount is " << leafNode.getKeyCount() <<endl;
    cout << "Start parsing and inserting...\n";
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




int main(int argc, const char * argv[]) {
    // insert code here...
    cout << "hello world!" << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv "<<i<<" is " << argv[i] <<endl;
    }
    testLeafNode_insert1();
    testLeafNode_insert50();
    //testNonLeafNode_insert1();
    //testNodLeafNode_insert50();
    
    //testLeafNode_insertAndSplit();
    //testNonLeafNode_insertAndSplit();
    return 0;
    
}
