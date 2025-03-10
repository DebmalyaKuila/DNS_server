#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <bits/stdc++.h>
#include <ctime>
using namespace std;

class LRUCache;

class Node{
    private:
        string key;
        string value;
        Node* prev;
        Node* next;
        time_t cacheEntryTime; //the time when a cache entry was made (seconds since epoch)
        time_t ttl; //time for cache expire in seconds
    public:
    //constructors
        Node();
        Node(string s1,string s2,time_t timeToLive);

        friend class LRUCache ;
};


class LRUCache {
    private:
        int maxCapacity;
        int capacity;
        Node *head, *tail;
        unordered_map<string,Node*>keyMap;// key-nodeAddress pair map
    public:
    //constructor
        LRUCache(int cap);
    //destructor
        ~LRUCache();
    //methoods
        pair<string,time_t> get(string key);
        void put(string key ,string val,time_t timeToLive);

};

#endif