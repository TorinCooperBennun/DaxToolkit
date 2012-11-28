//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2012 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================

#ifndef __dax_exec_internal_kernel_ScheduleGenerateTopology_h
#define __dax_exec_internal_kernel_ScheduleGenerateTopology_h

#include <dax/Types.h>
#include <dax/exec/WorkletMapField.h>

namespace dax {
namespace exec {
namespace internal {
namespace kernel {

template<class Worklet, typename CSig, typename ESig>
class DerivedWorklet : public Worklet
{
public:
  DerivedWorklet(const Worklet& worklet):
    Worklet(worklet)
    {}
  typedef CSig ControlSignature;
  typedef ESig ExecutionSignature;
};

struct ClearUsedPointsFunctor : public WorkletMapField
{
  typedef void ControlSignature(Field(Out));
  typedef void ExecutionSignature(_1);

  template<typename T>
  DAX_EXEC_EXPORT void operator()(T &t) const
  {
    t = static_cast<T>(0);
  }
};

struct Index : public WorkletMapField
{
  typedef void ControlSignature(Field(Out));
  typedef _1 ExecutionSignature(WorkId);

  DAX_EXEC_EXPORT dax::Id operator()(dax::Id index) const
  {
    return index;
  }
};

struct GetUsedPointsFunctor : public WorkletMapField
{
  typedef void ControlSignature(Field(Out));
  typedef void ExecutionSignature(_1);

  template<typename T>
  DAX_EXEC_EXPORT void operator()(T& t) const
  {
    t = static_cast<T>(1);
  }
};

struct ComputeVisitIndex : public WorkletMapField
{
  typedef void ControlSignature(Field(Out));
  typedef void ExecutionSignature(_1,WorkId);

  DAX_EXEC_EXPORT void operator()(dax::Id& visitIndex, const dax::Id& workId) const
  {
    visitIndex = workId - visitIndex;
  }
};

}
}
}
} //dax::exec::internal::kernel


#endif // SCHEDULEGENERATETOPOLOGY_H
