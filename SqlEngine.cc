/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  
  BTreeIndex idx;
  IndexCursor cursor;
  
  bool hasIndex = true;
  bool finishScan = false;
  bool hasEqualBoundKey = false;
  bool hasLowerBoundKey = false, hasUpperBoundKey = false;
  int lowerBoundKey = -1, upperBoundKey = -1;
  
  vector<SelCond> keyCond;// to make it simple, we wont consider nq condition as keyCond
  vector<SelCond> otherCond;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  
  // process SelConds first
  // code here ...
  parseSelConds(cond, keyCond, otherCond, hasEqualBoundKey, hasLowerBoundKey, hasUpperBoundKey);
  if(hasLowerBoundKey) getLowerBoundKey(keyCond, lowerBoundKey);//include eqKey
  if(hasUpperBoundKey) getUpperBoundKey(keyCond, upperBoundKey);
  //printf("lowerBoundKey is %d\n", lowerBoundKey);
  //printf("upperBoundKey is %d\n", upperBoundKey);
  
  // check if need index
  // code here ...
  if( keyCond.empty() && attr != 4) { // regardless of the attr, if count(*), use
		hasIndex = false;
  }
  
  // open the table index
  if(hasIndex && (rc = idx.open(table + ".idx", 'r')) < 0) {
	  hasIndex = false;
  }
  
  rid.pid = rid.sid = 0;
  count = 0;
  if ( hasIndex ){ // has index file
	  if( hasLowerBoundKey ){
		  rc = idx.locate(lowerBoundKey, cursor); 
	  } else {
		  rc = idx.locateFirstEntry(cursor);
	  }
	  
	  if(rc < 0 || (rc = idx.readForward(cursor, key, rid)) < 0) {
		  fprintf(stderr, "Error while reading from index for table %s\n", table.c_str());
		  goto exit_select;
	  }
  } 
  
  while(!finishScan ){
	  // match keyConds
	  if(hasIndex && !matchSelCond(keyCond, key, value)) goto next_tuple;
	  	
	  // match valueConds if necessary
	  // read from the table first
	  if(!hasIndex || !otherCond.empty() || (hasIndex && (attr == 2 || attr == 3)) ){
		  if((rc = rf.read(rid, key, value)) < 0){
			  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			  goto exit_select;
		  }
	  }
	  
	  // check valueConds
	  if(hasIndex && !matchSelCond(otherCond, key, value)) goto next_tuple;
	  
	  // for table scan
	  if(!hasIndex && !matchSelCond(cond, key, value)) goto next_tuple;
	  
	  // once the all conditions are met
	  // increasing matching tuple counter
	  count++;
	  switch (attr) {
		  case 1:  // SELECT key
		  fprintf(stdout, "%d\n", key);
		  break;
	      case 2:  // SELECT value
		  fprintf(stdout, "%s\n", value.c_str());
		  break;
	      case 3:  // SELECT *
		  fprintf(stdout, "%d '%s'\n", key, value.c_str());
		  break;
	  }
	  
	  next_tuple:
	  
	  if(hasIndex) {
		  if(hasEqualBoundKey) break;// no need to ckech next if assuming no duplicates
		  if((rc = idx.readForward(cursor, key, rid)) < 0) break;	
		  if(hasUpperBoundKey && key > upperBoundKey) break;
	  } else {
		  ++rid;
		  finishScan = (rid >= rf.endRid());
		  //printf("scan next...\n");
	  }
	  
  } // end while
  
  // print matching tuple count if "select count(*)"
  if(attr == 4){
	  fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  idx.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
    RecordFile rf;   // RecordFile containing the table
    RecordId   rid;  // record cursor for table scanning
	BTreeIndex idx;  // index file for retrieving later

    RC     rc;
    int    key;     
    string value;
	
    if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
		fprintf(stderr, "Error: creating table %s \n", table.c_str());
		return rc;
    }
	
	if (index && (rc = idx.open(table + ".idx", 'w')) < 0){
		fprintf(stderr, "Error: creating table %s index \n", table.c_str());   
		return rc;
	}
	
	std::ifstream inf(loadfile.c_str());
	if (!inf.is_open()){
		rc = RC_FILE_OPEN_FAILED;
		return rc;
	}
	
	string line;
	while(std::getline(inf, line)) {
		// process the line
		parseLoadLine(line, key, value);
		//insert it into the record
		if(rc = rf.append(key, value, rid) < 0) {
			fprintf(stderr, "Error: inserting tuple to table %s \n", table.c_str());
			return rc;
		}
		//insert it into index file, if index is true
		if (index && (rc = idx.insert(key, rid)) < 0){
			fprintf(stderr, "Error: inserting key to table %s index \n", table.c_str());
			return rc;
		}
	}
	
	if(index) idx.close();
	rf.close();
	inf.close();
	return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}

RC SqlEngine::parseSelConds(const vector<SelCond>& cond, vector<SelCond>& keyCond, vector<SelCond>& valueCond, bool& hasEqualBoundKey, bool& hasLowerBoundKey, bool& hasUpperBoundKey){
	
	keyCond.clear();
	valueCond.clear();
	
	for(int i = 0; i < cond.size(); i++) {
		if(cond[i].attr == 1 && cond[i].comp != SelCond::NE){
			switch (cond[i].comp) {
				case SelCond::EQ:
				// assumming no duplicate, can we extract the specific tuple and return directly?
				hasEqualBoundKey = true;
				case SelCond::GE:
				case SelCond::GT:
				hasLowerBoundKey = true;
				break;
				//case SelCond::NE:
				case SelCond::LT:
				case SelCond::LE:
				default:
				hasUpperBoundKey = true;
				break;
			}
			keyCond.push_back(cond[i]);	
		} else{
			valueCond.push_back(cond[i]);
		}
			
	}
	
	return 0;
}

/*find the largest lowerBound*/
RC SqlEngine::getLowerBoundKey(vector<SelCond> cond, int& lowerBoundKey){
	lowerBoundKey = -1;
	for(int i = 0; i < cond.size(); i++) {
		if(cond[i].attr == 1 && cond[i].comp == SelCond::GE || cond[i].comp == SelCond::GT || cond[i].comp == SelCond::EQ){
			lowerBoundKey = max(lowerBoundKey, atoi(cond[i].value));		
		}
	}
	
	return 0;
}

/*find the smallest upperBound*/
RC SqlEngine::getUpperBoundKey(vector<SelCond> cond, int& upperBoundKey){
	
	upperBoundKey = INT_MAX;
	for(int i = 0; i < cond.size(); i++) {
		if(cond[i].attr == 1 && cond[i].comp == SelCond::LT || cond[i].comp == SelCond::LE || cond[i].comp == SelCond::EQ){
			upperBoundKey = min(upperBoundKey, atoi(cond[i].value));		
		}
	}
	
	return 0;
}



bool SqlEngine::matchSelCond(const vector<SelCond>& cond, int key, string value){
	
	int diff;
	// check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      switch (cond[i].attr) {
      case 1:
	diff = key - atoi(cond[i].value);
	break;
      case 2:
	diff = strcmp(value.c_str(), cond[i].value);
	break;
      }

      // skip the tuple if any condition is not met
      switch (cond[i].comp) {
      case SelCond::EQ:
	if (diff != 0) return false;
	break;
      case SelCond::NE:
	if (diff == 0) return false;
	break;
      case SelCond::GT:
	if (diff <= 0) return false;
	break;
      case SelCond::LT:
	if (diff >= 0) return false;
	break;
      case SelCond::GE:
	if (diff < 0) return false;
	break;
      case SelCond::LE:
	if (diff > 0) return false;
	break;
      }
    }
	
	return true;
}




