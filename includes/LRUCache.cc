#include "LRUCache.h"

Node::Node(){
    key="";
    value="";
    prev=nullptr;
    next=nullptr;
    cacheEntryTime=time(nullptr);
    ttl=0;
}

Node::Node(string s1,string s2,time_t timeToLive){
    key=s1;
    value=s2;
    prev=nullptr;
    next=nullptr;
    cacheEntryTime=time(nullptr);
    ttl=timeToLive;
}

LRUCache::LRUCache(int cap){
    cout<<"LRU cache constructor called"<<endl;
            maxCapacity=cap;
            capacity=0;
            //Doubly linked list to store nodes in lru order
            head=new Node();
            tail=new Node();
            head->next=tail;
            tail->prev=head;
}

LRUCache::~LRUCache(){
            cout<<"LRU cache destructor called"<<endl;
            //clear the cache map
            keyMap.clear();
            //clean the DLL
            Node* node=head;
            while(node!=NULL){
                Node* deleteNode=node;
                node=node->next;
                delete deleteNode;
            }
        };

pair<string,time_t> LRUCache::get(string key){
            auto it=keyMap.find(key);
            if(it==keyMap.end()){
                cout<< "\033[91;1m"<<"Cache miss"<< "\033[0m" << endl;
                return {"",0};
            }
            //check whether the ttl expired or not
            Node* node=it->second;
            time_t currentTime=time(nullptr);
            if(currentTime - node->cacheEntryTime >= node->ttl){
                //before cache miss , delete this node 
                Node* nextNode=node->next;
                Node* prevNode=node->prev;
                prevNode->next=nextNode;
                nextNode->prev=prevNode;
                keyMap.erase(key);
                delete node ;
                capacity--;
                cout<< "\033[91;1m"<<"Cache miss - expired "<< "\033[0m" << endl;
                return {"",0};
            }

            //found in cache - move node right after head and return the value 
            if (head->next != node) { 
                Node* frontNode=node->next;
                Node* backNode=node->prev;
                frontNode->prev=backNode;
                backNode->next=frontNode;

                Node* firstNode=head->next;
                head->next=node;
                node->prev=head;
                node->next=firstNode;
                firstNode->prev=node;
            }
            cout<< "\033[92;1m"<<"Cache hit"<< "\033[0m" << endl;
            return {node->value,(node->ttl - (currentTime-node->cacheEntryTime))};
        }

void LRUCache::put(string key ,string val,time_t timeToLive){
            //check if node already exists in cache , if it does then put it in front of head 
            auto it =keyMap.find(key);
            if(it!=keyMap.end()){
                Node* node=it->second;
                time_t currentTime=time(nullptr);
                if(currentTime - node->cacheEntryTime < node->ttl){
                    //move the node after head
                    Node* frontNode=node->next;
                    Node* backNode=node->prev;
                    frontNode->prev=backNode;
                    backNode->next=frontNode;

                    Node* firstNode=head->next;
                    head->next=node;
                    node->prev=head;
                    node->next=firstNode;
                    firstNode->prev=node;

                    return;
                }
            }

            //when capacity is full , erase least recently used node and then insert
            while(capacity>=maxCapacity){
            Node* node=tail->prev;
            Node* prevNode=node->prev;
            node->next=node->prev=nullptr;
            prevNode->next=tail;
            tail->prev=prevNode;
            keyMap.erase(node->key);
            delete node;
            cout<< "\033[91;1m"<<"maximum capacity , deleted least used cache record"<< "\033[0m" << endl;
            capacity--;
            }

            //insert new node after head
            Node* newNode =new Node(key,val,timeToLive);
            Node* front=head->next;
            head->next=newNode;
            newNode->prev=head;
            front->prev=newNode;
            newNode->next=front;
            keyMap[key]=newNode; 
            capacity++;               

        }
