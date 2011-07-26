// Copyright (c) 2011, James Hughes
// All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

// suppress 64-bit ready warning about casting on the win32 platform (only)
#pragma warning(disable:4311)
#pragma warning(disable:4312)

// After Sutter in Dr. Dobbs -- http://ddj.com/cpp/210604448?pgno=2
#include <list>
#include <boost/shared_ptr.hpp>
#include "Windows.h"

class DLFrameQueue {

private:
    DLFrameQueue(const DLFrameQueue &);               // Not copyable
    DLFrameQueue & operator= (const DLFrameQueue &); // Not assignable

    struct Node {
        Node( boost::shared_ptr<DLFrame> val ) : value(val), next(NULL) { }
        Node() : next(NULL) { }
        boost::shared_ptr<DLFrame> value;
        Node* next;
    };

    std::list<Node *> freeList;   // for producer only
    Node* first;                  // for producer only
    Node *divider, *last;         // shared -- Use explicit atomic compares only

    // Allocator/Deallocator for nodes -- 
    // only used in the producer thread
    // OR in the destructor.
    Node * Get(boost::shared_ptr<DLFrame> val)
    {
        if(!freeList.empty())
        {
            // Clean because of Release
            Node * next = freeList.front();
            freeList.pop_front();
            next->value = val;
            return next;
        }

        // clean by construction
        return new Node(val);     
    }

    // Avoids costly free() while running
    void Release(Node * node)
    {
        // reset the node to clean before shelving it
		boost::shared_ptr<DLFrame> empty;
        node->value.swap(empty);
        node->next = NULL;
        freeList.push_back(node);
    }


public:
    DLFrameQueue() {
		boost::shared_ptr<DLFrame> empty;
        first = divider = last = Get(empty);                         // add dummy separator
    }

    ~DLFrameQueue() {
        while( first->value.use_count() != 0 ) {               // release the list
            Node* tmp = first;
            first = tmp->next;
            delete tmp;
        }

        // Require -- Producer thread calls this or is dead
        while(!freeList.empty())
        {
			boost::shared_ptr<DLFrame> empty;
            delete Get(empty);
        }
    }

    // Produce is called on the producer thread only:
    void Produce( boost::shared_ptr<DLFrame> t ) {
        last->next = Get(t);                            // add the new item
        InterlockedExchangePointer(&last, last->next);  // publish it

        // Burn the consumed part of the queue
        for( PVOID looper = first;                     // non-null; pointer read is atomic
             InterlockedCompareExchangePointer(&looper, NULL, divider), looper;
             looper = first)
        {
            Node* tmp = first;
            first = first->next;
            Release(tmp);
        }
    }

    // Consume is called on the consumer thread only:
    bool Consume( boost::shared_ptr<DLFrame> & result ) {

        PVOID choice = divider;                                  // non-null; pointer read is atomic
        InterlockedCompareExchangePointer(&choice, NULL, last);

        if(choice)
        {
            result = divider->next->value;                        // C: copy it back
            choice = divider;

            InterlockedExchangePointer(&divider, divider->next);  // D: publish that we took it
            //reinterpret_cast<Node*>(choice)->next = NULL;
            return true;                                          // and report success
        }

        return false;                                             // else report empty
    }
};

#pragma warning(default:4312)
#pragma warning(default:4311)
