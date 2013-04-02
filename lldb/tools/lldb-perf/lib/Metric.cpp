//===-- Metric.cpp ----------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Metric.h"
#include "MemoryGauge.h"
#include <cmath>

using namespace lldb_perf;

template <class T>
Metric<T>::Metric () : Metric ("")
{
}

template <class T>
Metric<T>::Metric (const char* n, const char* d) :
    m_name(n ? n : ""),
    m_description(d ? d : ""),
    m_dataset ()
{
}

template <class T>
void
Metric<T>::Append (T v)
{
    m_dataset.push_back(v);
}

template <class T>
size_t
Metric<T>::GetCount () const
{
    return m_dataset.size();
}

template <class T>
T
Metric<T>::GetSum () const
{
    T sum = 0;
    for (auto v : m_dataset)
        sum += v;
    return sum;
}

template <class T>
T
Metric<T>::GetAverage () const
{
    return GetSum()/GetCount();
}

template <class T>
T
Metric<T>::GetStandardDeviation () const
{
    T average = GetAverage();
    T diff_squared = 0;
    size_t count = GetCount();
    for (auto v : m_dataset)
        diff_squared = diff_squared + ( (v-average)*(v-average) );
    diff_squared = diff_squared / count;
    return sqrt(diff_squared);
}

template class lldb_perf::Metric<double>;
template class lldb_perf::Metric<MemoryStats>;
