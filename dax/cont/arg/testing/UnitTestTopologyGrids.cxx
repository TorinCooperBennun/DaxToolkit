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

#include <dax/cont/arg/TopologyUniformGrid.h>
#include <dax/cont/arg/TopologyUnstructuredGrid.h>

#include <dax/cont/testing/Testing.h>
#include <dax/cont/testing/TestingGridGenerator.h>

#include <dax/cont/internal/Bindings.h>
#include <dax/cont/sig/Tag.h>
#include <dax/exec/WorkletMapField.h>

namespace{
using dax::cont::arg::Topology;


struct Worklet1: public dax::exec::WorkletMapField
{
  typedef void ControlSignature(Topology(In),Topology(Out));
};

template<typename T, typename U>
void verifyBindingExists(T t, U u)
{
  typedef Worklet1 Invocation1(T,U);
  dax::cont::internal::Bindings<Invocation1> binded(t,u);
  (void)binded;
}

template<typename T, typename U>
void verifyConstBindingExists(const T& t, const U& u)
{
  typedef Worklet1 Invocation1(T,U);
  dax::cont::internal::Bindings<Invocation1> binded(t,u);
  (void)binded;
}


//-----------------------------------------------------------------------------
struct BindTopoGrids
{
  template<typename GridType>
  void operator()(const GridType&) const
    {
    dax::cont::testing::TestGrid<GridType> grid(4);
    verifyBindingExists<GridType,GridType>( grid.GetRealGrid(), grid.GetRealGrid() );
    verifyBindingExists<GridType,GridType>( grid.GetRealGrid(), grid.GetRealGrid() );
    }
};

void TopoGrids()
  {
  dax::cont::testing::GridTesting::TryAllGridTypes(BindTopoGrids());
  }
}

int UnitTestTopologyGrids(int, char *[])
{
  return dax::cont::testing::Testing::Run(TopoGrids);
}
