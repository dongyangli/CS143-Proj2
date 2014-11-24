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

void testLeafNode_insert1(){
    cout << "in testLeafNode_insert1 now" << endl;
    
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
    std::cout << "Execution continues past the first assert\n";
    // test keys;
    
    
}

void testLeafNode_insert50(){
    
}




int main(int argc, const char * argv[]) {
    // insert code here...
    cout << "hello world!" << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv "<<i<<" is " << argv[i] <<endl;
    }
    
    RC     rc;
    int    key;
    string value;
    
    std::ifstream inf(argv[1]);
    if (!inf.is_open()){
        rc = RC_FILE_OPEN_FAILED;
        cout << "Failed to open file\n";
        return rc;
    }
    
    string line;
    // --- idx
    PageFile pf;
    if ((rc = pf.open("test.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    PageId eid = pf.endPid();
    BTLeafNode leafNode;
    leafNode.read(eid, pf);
    cout<<"current(before insert) keyCount is " << leafNode.getKeyCount() <<endl;
    // --- idx
    cout << "Start parsing and inserting...\n";
    int count = 0;
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
        count++;
        if(count == 1){
            //cout <<"first insert!"<<endl;
            //cout<<"current keyCount is " << leafNode.getKeyCount() <<endl;
            //leafNode.print();
        }
    }
    cout << "Inserting done...\n";
    cout<<"Totally inserted keyCount " << count <<endl;
    cout<<"current keyCount is " << leafNode.getKeyCount() <<endl;
    //leafNode.print();
    
    cout << "Print done...\n";
    
    inf.close();
    return 0;
}
