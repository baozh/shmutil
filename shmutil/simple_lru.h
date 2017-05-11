/*
 * simple_lru.cc
 *
 *      Created on: 2017.5.9
 *      Author: ZengHui Bao (bao_z_h@163.com)
 */

#ifndef SHMUTIL_SIMPLE_LRU_H
#define SHMUTIL_SIMPLE_LRU_H

#include <string>
#include <list>
#include <map>

class SimpleLru{
public:
    SimpleLru() {
        lruMem_.clear();
        keyToIter_.clear();
    };

    ~SimpleLru(){};

    std::string getRemoveKey() {
        if (lruMem_.empty()) {
            return "";
        } else {
            return lruMem_.back();
        }
    };

    std::string removeKey() {
        std::string key = lruMem_.back();
        lruMem_.pop_back();
        if (keyToIter_.find(key) != keyToIter_.end()) {
            keyToIter_.erase(key);
        }
        return key;
    };

    void visitKey(std::string key) {
        if (keyToIter_.find(key) == keyToIter_.end()) {
            lruMem_.push_front(key);
            keyToIter_[key] = lruMem_.begin();
        } else {
            std::list<std::string>::iterator it = keyToIter_[key];
            lruMem_.erase(it);
            lruMem_.push_front(key);
            keyToIter_[key] = lruMem_.begin();
        }
    };

private:
    std::list<std::string> lruMem_;
    std::map<std::string, std::list<std::string>::iterator> keyToIter_;
};

#endif //SHMUTIL_SIMPLE_LRU_H
