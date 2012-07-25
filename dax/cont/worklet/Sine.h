//=============================================================================
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
#ifndef __dax_cont_worklet_Sine_h
#define __dax_cont_worklet_Sine_h

// TODO: This should be auto-generated.

#include <Worklets/Sine.worklet>

#include <dax/Types.h>
#include <dax/cont/ArrayHandle.h>
#include <dax/cont/DeviceAdapter.h>

namespace dax {
namespace exec {
namespace internal {
namespace kernel {

template<class PortalType1, class PortalType2>
struct Sine
{
  DAX_CONT_EXPORT
  Sine(const dax::worklet::Sine &worklet,
         PortalType1 inValueArray,
         PortalType2 outValueArray)
    : Worklet(worklet),
      InValueArray(inValueArray),
      OutValueArray(outValueArray) {  }

  DAX_EXEC_EXPORT void operator()(
      dax::Id index,
      const dax::exec::internal::ErrorMessageBuffer &errorMessage)
  {
    this->Worklet.SetErrorMessageBuffer(errorMessage);
    const dax::worklet::Sine &constWorklet = this->Worklet;

    const typename PortalType1::ValueType inValue =
        this->InValueArray.Get(index);
    typename PortalType2::ValueType outValue;

    constWorklet(inValue, outValue);

    this->OutValueArray.Set(index, outValue);
  }

private:
  dax::worklet::Sine Worklet;
  const PortalType1 &InValueArray;
  const PortalType2 &OutValueArray;
};

}
}
}
} // dax::exec::internal::kernel

namespace dax {
namespace cont {
namespace worklet {

template<typename ValueType,
         class Container1,
         class Container2,
         class Adapter>
DAX_CONT_EXPORT void Sine(
    const dax::cont::ArrayHandle<ValueType, Container1, Adapter> &inHandle,
    dax::cont::ArrayHandle<ValueType, Container2, Adapter> &outHandle)
{
  dax::Id fieldSize = inHandle.GetNumberOfValues();

  dax::exec::internal::kernel::Sine<
      typename dax::cont::ArrayHandle<ValueType,Container1,Adapter>::PortalConstExecution,
      typename dax::cont::ArrayHandle<ValueType,Container2,Adapter>::PortalExecution>
      kernel(dax::worklet::Sine(),
             inHandle.PrepareForInput(),
             outHandle.PrepareForOutput(fieldSize));

  dax::cont::internal::Schedule(kernel, fieldSize, Adapter());
}

}
}
} //dax::cont::worklet

#endif //__dax_cont_worklet_Sine_h
