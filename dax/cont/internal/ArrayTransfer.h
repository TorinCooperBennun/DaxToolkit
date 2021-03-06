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
#ifndef __dax_cont_internal_ArrayTransfer_h
#define __dax_cont_internal_ArrayTransfer_h

#include <dax/cont/ArrayContainerControl.h>
#include <dax/cont/internal/ArrayManagerExecution.h>

namespace dax {
namespace cont {
namespace internal {

/// \brief Class that manages the transfer of data between control and execution.
///
/// This templated class provides a mechanism (used by the ArrayHandle) to
/// transfer data from the control environment to the execution environment and
/// back. The interface for ArrayTransfer is nearly identical to that of
/// ArrayManagerExecution and the default implementation simply delegates all
/// calls to that class.
///
/// The primary motivation for having a separate class is that the
/// ArrayManagerExecution is meant to be specialized for each device adapter
/// whee as the ArrayTransfer is meant to be specialized for each array
/// container (or specific combination of container and device adapter). Thus,
/// transfers for most containers will be delegated through the
/// ArrayManagerExecution, but some containers, like implicit containers, will
/// be specialized to transfer through a different path.
///
template<typename T, class ArrayContainerControlTag, class DeviceAdapterTag>
class ArrayTransfer
{
private:
  typedef dax::cont::internal::ArrayContainerControl<T,ArrayContainerControlTag>
      ContainerType;
  typedef dax::cont::internal::ArrayManagerExecution<
      T,ArrayContainerControlTag,DeviceAdapterTag> ArrayManagerType;

public:
  /// The type of value held in the array (dax::Scalar, dax::Vector3, etc.)
  ///
  typedef T ValueType;

  /// An array portal that can be used in the control environment.
  ///
  typedef typename ContainerType::PortalType PortalControl;
  typedef typename ContainerType::PortalConstType PortalConstControl;

  /// An array portal that can be used in the execution environment.
  ///
  typedef typename ArrayManagerType::PortalType PortalExecution;
  typedef typename ArrayManagerType::PortalConstType PortalConstExecution;


  /// Returns the number of values stored in the array.  Results are undefined
  /// if data has not been loaded or allocated.
  ///
  DAX_CONT_EXPORT dax::Id GetNumberOfValues() const
  {
    return this->ArrayManager.GetNumberOfValues();
  }

  /// Allocates a large enough array in the execution environment and copies
  /// the given data to that array. The allocated array can later be accessed
  /// via the GetPortalConstExecution method. If control and execution share
  /// arrays, then this method may save the iterators to be returned in the \c
  /// GetPortalConst methods.
  ///
  DAX_CONT_EXPORT void LoadDataForInput(PortalConstControl portal)
  {
    this->ArrayManager.LoadDataForInput(portal);
  }

  /// Allocates a large enough array in the execution environment and copies
  /// the given data to that array. The allocated array can later be accessed
  /// via the GetPortalExection method. If control and execution share arrays,
  /// then this method may save the iterators of the container to be returned
  /// in the \c GetPortal* methods.
  ///
  DAX_CONT_EXPORT void LoadDataForInPlace(PortalControl portal)
  {
    this->ArrayManager.LoadDataForInPlace(portal);
  }

  /// Allocates an array in the execution environment of the specified size.
  /// If control and execution share arrays, then this class can allocate
  /// data using the given ArrayContainerExecution and remember its iterators
  /// so that it can be used directly in the execution environment.
  ///
  DAX_CONT_EXPORT void AllocateArrayForOutput(ContainerType &controlArray,
                                              dax::Id numberOfValues)
  {
    this->ArrayManager.AllocateArrayForOutput(controlArray, numberOfValues);
  }

  /// Allocates data in the given ArrayContainerControl and copies data held
  /// in the execution environment (managed by this class) into the control
  /// array. If control and execution share arrays, this can be no operation.
  /// This method should only be called after AllocateArrayForOutput is
  /// called.
  ///
  DAX_CONT_EXPORT void RetrieveOutputData(ContainerType &controlArray) const
  {
    this->ArrayManager.RetrieveOutputData(controlArray);
  }

  /// Similar to RetrieveOutputData except that instead of writing to the
  /// controlArray itself, it writes to the given control environment
  /// iterator. This allows the user to retrieve data without necessarily
  /// allocating an array in the ArrayContainerControl (assuming that control
  /// and exeuction have seperate memory spaces).
  ///
  template <class IteratorTypeControl>
  DAX_CONT_EXPORT void CopyInto(IteratorTypeControl dest) const
  {
    this->ArrayManager.CopyInto(dest);
  }

  /// \brief Reduces the size of the array without changing its values.
  ///
  /// This method allows you to resize the array without reallocating it. The
  /// number of entries in the array is changed to \c numberOfValues. The data
  /// in the array (from indices 0 to \c numberOfValues - 1) are the same, but
  /// \c numberOfValues must be equal or less than the preexisting size
  /// (returned from GetNumberOfValues). That is, this method can only be used
  /// to shorten the array, not lengthen.
  ///
  DAX_CONT_EXPORT void Shrink(dax::Id numberOfValues)
  {
    this->ArrayManager.Shrink(numberOfValues);
  }

  /// Returns an array portal that can be used in the execution environment.
  /// This portal was defined in either LoadDataForInput or
  /// AllocateArrayForOutput. If control and environment share memory space,
  /// this class may return the iterator from the \c controlArray.
  ///
  DAX_CONT_EXPORT PortalExecution GetPortalExecution()
  {
    return this->ArrayManager.GetPortal();
  }

  /// Const version of GetPortal.
  ///
  DAX_CONT_EXPORT PortalConstExecution GetPortalConstExecution() const
  {
    return this->ArrayManager.GetPortalConst();
  }

  /// Frees any resources (i.e. memory) allocated for the exeuction
  /// environment, if any.
  ///
  DAX_CONT_EXPORT void ReleaseResources()
  {
    this->ArrayManager.ReleaseResources();
  }

private:
  ArrayManagerType ArrayManager;
};

}
}
} // namespace dax::cont::internal

#endif //__dax_cont_internal_ArrayTransfer_h
