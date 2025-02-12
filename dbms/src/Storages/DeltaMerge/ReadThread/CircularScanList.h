// Copyright 2022 PingCAP, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <list>
#include <memory>

namespace DB::DM
{
// CircularScanList is a special circular list.
// It remembers the location of the last iteration and will check whether the object is expired.
template <typename T>
class CircularScanList
{
public:
    using Value = std::shared_ptr<T>;

    CircularScanList()
        : last_itr(l.end())
    {}

    void add(const Value & ptr)
    {
        l.push_back(ptr);
    }

    Value next()
    {
        last_itr = nextItr(last_itr);
        while (!l.empty())
        {
            auto ptr = *last_itr;
            if (ptr->valid())
            {
                return ptr;
            }
            else
            {
                last_itr = l.erase(last_itr);
                if (last_itr == l.end())
                {
                    last_itr = l.begin();
                }
            }
        }
        return nullptr;
    }

    std::pair<int64_t, int64_t> count(int64_t table_id) const
    {
        int64_t valid = 0;
        int64_t invalid = 0;
        for (const auto & p : l)
        {
            if (table_id == 0 || p->tableId() == table_id)
            {
                p->valid() ? valid++ : invalid++;
            }
        }
        return {valid, invalid};
    }

    Value get(uint64_t pool_id) const
    {
        for (const auto & p : l)
        {
            if (p->poolId() == pool_id)
            {
                return p;
            }
        }
        return nullptr;
    }

private:
    using Iter = typename std::list<Value>::iterator;
    Iter nextItr(Iter itr)
    {
        if (itr == l.end() || std::next(itr) == l.end())
        {
            return l.begin();
        }
        else
        {
            return std::next(itr);
        }
    }

    std::list<Value> l;
    Iter last_itr;
};

} // namespace DB::DM