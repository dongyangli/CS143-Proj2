//
//  test1_BTreeNode
//
//  Created by Dongyang Li on 11/22/14.
//  Copyright (c) 2014 Dongyang Li. All rights reserved.
//

#include <iostream>
#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

int main(int argc, const char * argv[]) {
    // insert code here...
    
    PageFile pf;
    if ((rc = pf.open("text.idx", 'w')) < 0){
        cout << "Failed to open file\n";
        return rc;
    }
    
    PageId eid = pf.endPid();
    
    BTLeafNode leafNode;
    leafNode.read(eid, pf);
    
    RecordId rid;
    for (int key = 0; key < 50; key++) {
        BTLeafNode.insert(key, rid);
    }
    
    
    
    return 0;
}
