#Authors
Wenting Li, UID 004434117
Dongyang Li, UID 404408946

#What is this
This is CS143 project 2. In this project, B+ tree indexes for Bruinbase is implemented and used to handle SELECT queries.

#Solution Summary
Class BTLeafNode and BTNonLeafNode are used to manage the node itself in the B+ tree. And class BTreeIndex is used to make tree operations.
The first page of the index file is used to store the root pid and the height of tree.
Leaf node stored in the disk:
--------------------------------------
| key count | keys | rids | next pid |
--------------------------------------
NonLeaf node stored in the disk:
---------------------------
| key count | keys | pids |
---------------------------
Function load and select in SqlEngine are also modified according to the specification to implement the index funcion.

we decide to use index:
1) whenever there is a constraint applied on key 
2) or the selected attribute is just count(*)





