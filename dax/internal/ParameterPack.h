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
//  Copyright 2013 Sandia Corporation.
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//=============================================================================
#ifndef BOOST_PP_IS_ITERATING

#ifndef __dax_internal_ParameterPack_h
#define __dax_internal_ParameterPack_h

#include <dax/Types.h>

#ifdef DAX_USE_VARIADIC_TEMPLATE
#error Parameter Pack currently not implemented for variadic template parameters.
#endif

#define DAX_MAX_PARAMETER_SIZE 10

#define BOOST_FUSION_INVOKE_PROCEDURE_MAX_ARITY DAX_MAX_PARAMETER_SIZE
#define FUSION_MAX_VECTOR_SIZE DAX_MAX_PARAMETER_SIZE

#include <dax/internal/ParameterPackCxx03.h>

#include <boost/static_assert.hpp>

#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/enable_if.hpp>

#include <boost/mpl/if.hpp>
#include <boost/mpl/assert.hpp>

#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/inc.hpp>

#define _dax_pp_T___all                 BOOST_PP_ENUM(DAX_MAX_PARAMETER_SIZE, __dax_pp_T___, notUsed)
#define __dax_pp_name___(x,i)           BOOST_PP_CAT(x,BOOST_PP_INC(i))
#define __dax_pp_T___(z,i,x)            __dax_pp_name___(T___,i)
#define _dax_pp_typename___T_all        BOOST_PP_ENUM(DAX_MAX_PARAMETER_SIZE, __dax_pp_typename___T, notUsed)
#define __dax_pp_typename___T(z,i,x)    typename __dax_pp_T___(z,i,x) = dax::internal::detail::NullParam
#define _dax_pp_params_default___all(x) BOOST_PP_ENUM(DAX_MAX_PARAMETER_SIZE, __dax_pp_param_default, x)
#define __dax_pp_param_default(z,i,x)   __dax_pp_T___(z,i,x) __dax_pp_arg___(z,i,x) = __dax_pp_T___(z,i,x)()
#define _dax_pp_args___all(x)           BOOST_PP_ENUM(DAX_MAX_PARAMETER_SIZE, __dax_pp_arg___, x)
#define __dax_pp_arg___(z,i,x)          __dax_pp_name___(x,i)

namespace dax {
namespace internal {

/// Used to overload the behavior of a constructor to copy values from a given
/// \c ParameterPack.
///
struct ParameterPackCopyTag {  };

/// Used to overload the behavior of a constructor to initialize all values in
/// a \c ParameterPack with a given object.
///
struct ParameterPackInitialArgumentTag {  };


/// Used to overload a \c ParameterPack method that is exported only in the
/// control environment.
///
struct ParameterPackContTag {  };

/// Used to overload a \c ParameterPack method that is exported in both the
/// control and exection environments.
///
struct ParameterPackExecContTag {  };

/// Used as the default transform for ParameterPack::InvokeCont.
///
struct IdentityFunctorCont
{
  template<typename T>
  struct ReturnType {
    typedef T type;
  };

  template<typename T>
  DAX_CONT_EXPORT
  T &operator()(T &x) const { return x; }
};

/// Used as the default transform for ParameterPack::InvokeExec.
struct IdentityFunctorExec
{
  template<typename T>
  struct ReturnType {
    typedef T type;
  };

  template<typename T>
  DAX_EXEC_EXPORT
  T &operator()(T &x) const { return x; }
};

namespace detail {

/// Placeholder for an unused parameter
struct NullParam { };

/// If you encounter this (partial) type, then you have probably given
/// ParameterPack::Parameter<Index> an invalid Index.
template<typename T> struct InvalidParameterPackType;

template<typename ParameterPackImplType>
struct ParameterPackImplAccess;

template<int Index, typename ParameterPackImplType>
struct ParameterPackImplAtIndex;

template<int NumParameters,
         typename Function,
         typename Transform,
         typename ParameterPackImplType,
         _dax_pp_typename___T_all>
struct ParameterPackDoInvokeContImpl;

template<typename Function,
         typename Transform,
         typename ParameterPackImplType>
DAX_CONT_EXPORT
void ParameterPackDoInvokeCont(Function &f,
                                   const Transform &transform,
                                   ParameterPackImplType &params)
{
  ParameterPackDoInvokeContImpl<
      0,
      Function,
      Transform,
      ParameterPackImplType>
    implementation;
  implementation(f, transform, params);
}

template<int NumParameters,
         typename Function,
         typename Transform,
         typename ParameterPackImplType,
         _dax_pp_typename___T_all>
struct ParameterPackDoInvokeExecImpl;

template<typename Function,
         typename Transform,
         typename ParameterPackImplType>
DAX_EXEC_EXPORT
void ParameterPackDoInvokeExec(Function &f,
                               const Transform &transform,
                               ParameterPackImplType &params)
{
  ParameterPackDoInvokeExecImpl<
      0,
      Function,
      Transform,
      ParameterPackImplType>
    implementation;
  implementation(f, transform, params);
}

/// Implementation class of a ParameterPack.  Uses a lisp-like cons construction
/// to build the type list.
///
/// \tparam CarType The first type in the parameter list.
/// \tparam CdrType A ParameterPackImpl containing the remaining list.
template<typename CarT, typename CdrT>
class ParameterPackImpl
{
  // If there is a compile error here, then a parameter was incorrectly set to
  // the NullParam type. The only exception is the terminating
  // ParameterPackImpl<NullParam,NullParam>.
  BOOST_MPL_ASSERT_NOT(( boost::is_same<CarT,NullParam> ));

private:
  typedef CarT CarType;
  typedef CdrT CdrType;

  typedef ParameterPackImpl<CarType, CdrType> ThisType;

  template<typename ParameterPackImplType>
  friend struct ParameterPackImplAccess;

public:
  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl() {  }

  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(CarType car, const CdrType &cdr)
    : Car(car), Cdr(cdr) {  }

  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(const ParameterPackImpl<CarType,CdrType> &src)
    : Car(src.Car), Cdr(src.Cdr) {  }

  DAX_CONT_EXPORT
  ParameterPackImpl(CarType car, const CdrType &cdr, ParameterPackContTag)
    : Car(car), Cdr(cdr) {  }

  template<typename SrcCarType, typename SrcCdrType>
  DAX_CONT_EXPORT
  ParameterPackImpl(const ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                    ParameterPackCopyTag,
                    ParameterPackContTag)
    : Car(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetFirstArgument(src)),
      Cdr(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetCdr(src),
          ParameterPackCopyTag(),
          ParameterPackContTag())
  {  }
  template<typename SrcCarType, typename SrcCdrType>
  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(const ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                    ParameterPackCopyTag,
                    ParameterPackExecContTag)
    : Car(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetFirstArgument(src)),
      Cdr(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetCdr(src),
          ParameterPackCopyTag(),
          ParameterPackExecContTag())
  {  }

  template<typename SrcCarType, typename SrcCdrType>
  DAX_CONT_EXPORT
  ParameterPackImpl(ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                    ParameterPackCopyTag,
                    ParameterPackContTag)
    : Car(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetFirstArgument(src)),
      Cdr(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetCdr(src),
          ParameterPackCopyTag(),
          ParameterPackContTag())
  {  }
  template<typename SrcCarType, typename SrcCdrType>
  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                    ParameterPackCopyTag,
                    ParameterPackExecContTag)
    : Car(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetFirstArgument(src)),
      Cdr(ParameterPackImplAccess<ParameterPackImpl<SrcCarType, SrcCdrType> >
            ::GetCdr(src),
          ParameterPackCopyTag(),
          ParameterPackExecContTag())
  {  }

  template<typename InitialParameter>
  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(InitialParameter &initial,
                    ParameterPackInitialArgumentTag,
                    ParameterPackExecContTag)
    : Car(initial),
      Cdr(initial,ParameterPackInitialArgumentTag(),ParameterPackExecContTag())
  {  }
  template<typename InitialParameter>
  DAX_CONT_EXPORT
  ParameterPackImpl(InitialParameter &initial,
                    ParameterPackInitialArgumentTag,
                    ParameterPackContTag)
    : Car(initial),
      Cdr(initial,ParameterPackInitialArgumentTag(),ParameterPackContTag())
  {  }

  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl<CarType,CdrType> &operator=(
      const ParameterPackImpl<CarType,CdrType> &src) {
    this->Car = src.Car;
    this->Cdr = src.Cdr;
    return *this;
  }


private:
  CarType Car;
  CdrType Cdr;
};

template<>
class ParameterPackImpl<NullParam, NullParam>
{
public:
  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl() {   }

  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(const NullParam &, const NullParam &) {   }

  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(const ParameterPackImpl<NullParam,NullParam> &,
                    ParameterPackCopyTag,
                    ParameterPackExecContTag) {   }
  DAX_CONT_EXPORT
  ParameterPackImpl(const ParameterPackImpl<NullParam,NullParam> &,
                    ParameterPackCopyTag,
                    ParameterPackContTag) {   }

  template<typename T>
  DAX_EXEC_CONT_EXPORT
  ParameterPackImpl(const T &,
                    ParameterPackInitialArgumentTag,
                    ParameterPackExecContTag) {  }
  template<typename T>
  DAX_CONT_EXPORT
  ParameterPackImpl(const T &,
                    ParameterPackInitialArgumentTag,
                    ParameterPackContTag) {  }
};

typedef ParameterPackImpl<NullParam,NullParam> ParameterPackImplNull;

template<typename ParameterPackImplType>
struct ParameterPackImplIsNull
{
  typedef typename
    boost::is_same<
      typename boost::remove_const<ParameterPackImplType>::type,
      ParameterPackImplNull>::type type;
};

template<typename ParameterPackType>
struct FindParameterPackImpl;

/// A cheating friend class that accesses the private members of ParameterPack.
/// It is necessary to access private members outside of ParameterPackImpl to
/// implement some of the features of variable arguments.
template<typename ParameterPackImplType>
struct ParameterPackImplAccess
{
  typedef typename ParameterPackImplType::CarType CarType;
  typedef typename ParameterPackImplType::CdrType CdrType;

  DAX_EXEC_CONT_EXPORT
  static void SetFirstArgument(
      ParameterPackImplType &parameters,
      const typename boost::remove_reference<CarType>::type &arg){
    parameters.Car = arg;
  }

  DAX_EXEC_CONT_EXPORT
  static const typename boost::remove_reference<CarType>::type &
  GetFirstArgument(
      const typename boost::remove_const<ParameterPackImplType>::type &parameters) {
    return parameters.Car;
  }

  DAX_EXEC_CONT_EXPORT
  static typename boost::remove_reference<CarType>::type &
  GetFirstArgument(
      typename boost::remove_const<ParameterPackImplType>::type &parameters) {
    return parameters.Car;
  }

  DAX_EXEC_CONT_EXPORT
  static const CdrType &GetCdr(
      const typename boost::remove_const<ParameterPackImplType>::type &parameters) {
    return parameters.Cdr;
  }
  DAX_EXEC_CONT_EXPORT
  static CdrType &GetCdr(
      typename boost::remove_const<ParameterPackImplType>::type &parameters) {
    return parameters.Cdr;
  }
};

/// This class provides the size (in number of parameters) of a
/// ParameterPackImpl instance.
///
template<typename ParameterPackImplType>
struct ParameterPackImplSize
{
  static const int NUM_PARAMETERS =
      ParameterPackImplSize<
        typename ParameterPackImplAccess<ParameterPackImplType>::CdrType>::
      NUM_PARAMETERS + 1;
};
template<>
struct ParameterPackImplSize<ParameterPackImplNull>
{
  static const int NUM_PARAMETERS = 0;
};

/// This class provides the implementation of all types and methods that
/// rely on dereferencing an index.  It is here because it requires template
/// specialization, which cannot be done as an internal class.
template<int Index, typename ParameterPackImplType>
struct ParameterPackImplAtIndex
{
  // These check for a valid index. If you get a compile error in these lines,
  // it is probably caused by an invalid Index template parameter for one of
  // the ParameterPack functions, methods, or classes. Remember that parameters
  // are indexed starting at 1.
  BOOST_STATIC_ASSERT(Index > 0);
  BOOST_STATIC_ASSERT(
      Index <= ParameterPackImplSize<ParameterPackImplType>::NUM_PARAMETERS);

private:
  typedef ParameterPackImplAccess<ParameterPackImplType> Access;

  typedef ParameterPackImplAtIndex<Index-1, typename Access::CdrType>
    ShiftedParameterPackImplAtIndex;
public:
  typedef typename ShiftedParameterPackImplAtIndex::CarType CarType;
  typedef typename ShiftedParameterPackImplAtIndex::CdrType CdrType;

  DAX_EXEC_CONT_EXPORT
  static void SetArgument(
      ParameterPackImplType &parameters,
      const typename boost::remove_reference<CarType>::type &arg)
  {
    ShiftedParameterPackImplAtIndex::
        SetArgument(Access::GetCdr(parameters), arg);
  }

  DAX_EXEC_CONT_EXPORT
  static const typename boost::remove_reference<CarType>::type &
  GetArgument(const ParameterPackImplType &parameters)
  {
    return ShiftedParameterPackImplAtIndex::
        GetArgument(Access::GetCdr(parameters));
  }
  DAX_EXEC_CONT_EXPORT
  static typename boost::remove_reference<CarType>::type &
  GetArgument(ParameterPackImplType &parameters)
  {
    return ShiftedParameterPackImplAtIndex::
        GetArgument(Access::GetCdr(parameters));
  }
};
template<typename ParameterPackImplType>
struct ParameterPackImplAtIndex<1, ParameterPackImplType>
{
private:
  typedef ParameterPackImplAccess<ParameterPackImplType> Access;
public:
  typedef typename Access::CarType CarType;
  typedef typename Access::CdrType CdrType;

  DAX_EXEC_CONT_EXPORT
  static void SetArgument(
      ParameterPackImplType &parameters,
      const typename boost::remove_reference<CarType>::type &arg) {
    Access::SetFirstArgument(parameters, arg);
  }
  DAX_EXEC_CONT_EXPORT
  static const typename boost::remove_reference<CarType>::type &
  GetArgument(const ParameterPackImplType &parameters) {
    return Access::GetFirstArgument(parameters);
  }
  DAX_EXEC_CONT_EXPORT
  static typename boost::remove_reference<CarType>::type &
  GetArgument(ParameterPackImplType &parameters) {
    return Access::GetFirstArgument(parameters);
  }
};

template<typename ParameterPackType,
         typename AppendType>
struct ParameterPackAppendType;

template<typename NewType, typename ParameterPackType>
struct PPackPrepend;

template<typename NewType, int Index, typename ParameterPackType>
struct PPackReplace;

//template<int Start, int End>
//struct CopyArgumentsImpl
//{
//  template<typename DestType, typename SrcType>
//  DAX_EXEC_CONT_EXPORT
//  void operator()(DestType &dest, const SrcType &src) {
//    CopyArgumentsImpl<Start,End-1>()(dest, src);
//    dest.template SetArgument<End-1>(src.template GetArgument<End-1>());
//  }
//};

//template<int Index>
//struct CopyArgumentsImpl<Index, Index>
//{
//  template<typename DestType, typename SrcType>
//  DAX_EXEC_CONT_EXPORT
//  void operator()(DestType &daxNotUsed(dest), const SrcType &daxNotUsed(src)) {
//    // Nothing left to copy.
//  }
//};

} // namespace detail (in dax::internal)

/// \brief Sets the argument at \c Index of the given \c ParameterPack.
///
/// The statement
/// \code{.cpp}
/// ParameterPackSetArgument<Index>(params, arg);
/// \endcode
/// is equivalent to either
/// \code{.cpp}
/// params.template SetArgument<Index>(arg);
/// \endcode
/// or
/// \code{.cpp}
/// params.SetArgument<Index>(arg);
/// \endcode
/// depending on the context that it is used. Using this function allows
/// you not not worry about whether the \c template keyword is necessary.
///
template<int Index, typename ParameterPackType, typename ParameterType>
DAX_EXEC_CONT_EXPORT
void ParameterPackSetArgument(ParameterPackType &params,
                              const ParameterType &arg)
{
  params.template SetArgument<Index>(arg);
}

/// \brief Returns the argument at \c Index of the given \c ParameterPack.
///
/// The statement
/// \code{.cpp}
/// value = ParameterPackGetArgument<Index>(params);
/// \endcode
/// is equivalent to either
/// \code{.cpp}
/// value = params.template GetArgument<Index>();
/// \endcode
/// or
/// \code{.cpp}
/// value = params.GetArgument<Index>();
/// \endcode
/// depending on the context that it is used. Using this function allows
/// you not not worry about whether the \c template keyword is necessary.
///
template<int Index, typename ParameterPackType>
DAX_EXEC_CONT_EXPORT
typename ParameterPackType::template Parameter<Index>::type
ParameterPackGetArgument(const ParameterPackType &params)
{
  return params.template GetArgument<Index>();
}

/// \brief Holds an arbitrary set of parameters in a single class.
///
/// To make using Dax easier for the end user developer, the
/// dax::cont::Scheduler::Invoke() method takes an arbitrary amount of
/// arguments that get transformed and swizzled into arguments and return value
/// for a worklet operator. In between these two invocations in a complicated
/// series of transformations and operations that can occur.
///
/// Supporting arbitrary function and template arguments is difficult and
/// really requires seperate implementations for ANSI and C++11 versions of
/// compilers. Thus, variatic template arguments are, at this point in time,
/// something to be avoided when possible. The intention of \c ParameterPack is
/// to collect most of the variatic template code into one place. The
/// ParameterPack template class takes a variable number of arguments that are
/// intended to match the parameters of some function. This means that all
/// arguments can be passed around in a single object so that objects and
/// functions dealing with these variadic parameters can be templated on a
/// single type (the type of ParameterPack).
///
/// Note that the indexing of the parameters in a \c ParameterPack starts at 1.
/// Although this is uncommon in C++, it matches better the parameter indexing
/// for other classes that deal with signatures for whole functions that use
/// the 0 index for the return value.
///
/// The \c ParameterPack contains several ways to invoke a functor whose
/// parameters match those of the parameter pack. This allows you to complete
/// the transition of calling an arbitrary function (like a worklet).
///
template<_dax_pp_typename___T_all>
class ParameterPack
    : public detail::FindParameterPackImpl<ParameterPack<_dax_pp_T___all> >::type
{
private:
  typedef typename
    detail::FindParameterPackImpl<ParameterPack<_dax_pp_T___all> >::type
      ImplementationType;
  typedef ImplementationType Superclass;
  typedef ParameterPack<_dax_pp_T___all> ThisType;
public:
  DAX_EXEC_CONT_EXPORT
  ParameterPack() {  }

  DAX_EXEC_CONT_EXPORT
  ParameterPack(const ThisType &src)
    : Superclass(src) {  }

  /// \brief Copy data from another \c ParameterPack
  ///
  /// The first argument is a source \c ParameterPack to copy from. The
  /// parameter types of the \c src \c ParameterPack do not have to be the
  /// exact same type as this object, but must be able to work in a copy
  /// constructor. The second argument is an instance of the \c
  /// ParamterPackCopyTag.
  ///
  template<typename SrcCarType, typename SrcCdrType>
  DAX_EXEC_CONT_EXPORT
  ParameterPack(const detail::ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                ParameterPackCopyTag,
                ParameterPackExecContTag)
    : Superclass(src, ParameterPackCopyTag(), ParameterPackExecContTag()) {  }
  template<typename SrcCarType, typename SrcCdrType>
  DAX_CONT_EXPORT
  ParameterPack(const detail::ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                ParameterPackCopyTag,
                ParameterPackContTag)
    : Superclass(src, ParameterPackCopyTag(), ParameterPackContTag()) {  }

  /// \brief Copy data from another \c ParameterPack
  ///
  /// The first argument is a source \c ParameterPack to copy from. The
  /// parameter types of the \c src \c ParameterPack do not have to be the
  /// exact same type as this object, but must be able to work in a copy
  /// constructor. The second argument is an instance of the \c
  /// ParamterPackCopyTag.
  ///
  template<typename SrcCarType, typename SrcCdrType>
  DAX_EXEC_CONT_EXPORT
  ParameterPack(detail::ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                ParameterPackCopyTag,
                ParameterPackExecContTag)
    : Superclass(src, ParameterPackCopyTag(), ParameterPackExecContTag()) {  }
  template<typename SrcCarType, typename SrcCdrType>
  DAX_CONT_EXPORT
  ParameterPack(detail::ParameterPackImpl<SrcCarType, SrcCdrType> &src,
                ParameterPackCopyTag,
                ParameterPackContTag)
    : Superclass(src, ParameterPackCopyTag(), ParameterPackContTag()) {  }

  /// \brief Initialize all the parameters with the given argument.
  ///
  /// The first argument is past to the constructors of all arguments held in
  /// this \c ParameterPack. The second argument is an instance of the \c
  /// ParameterPackInitialArgumentTag.
  ///
  template<typename InitialArgumentType>
  DAX_EXEC_CONT_EXPORT
  ParameterPack(InitialArgumentType &initial,
                ParameterPackInitialArgumentTag,
                ParameterPackExecContTag)
    : Superclass(initial,
                 ParameterPackInitialArgumentTag(),
                 ParameterPackExecContTag())
  {  }
  template<typename InitialArgumentType>
  DAX_CONT_EXPORT
  ParameterPack(InitialArgumentType &initial,
                ParameterPackInitialArgumentTag,
                ParameterPackContTag)
    : Superclass(initial,
                 ParameterPackInitialArgumentTag(),
                 ParameterPackContTag())
  {  }

  DAX_EXEC_CONT_EXPORT
  ThisType &operator=(const ThisType &src)
  {
    this->Superclass::operator=(src);
    return *this;
  }

  /// \brief Provides type information about a particular parameter.
  ///
  /// The templated \c Parameter subclass provides type information about a
  /// particular parmater specified by the template parameter \c Index. The \c
  /// Parameter subclass contains a typedef named \c type set to the given
  /// parameter type.
  ///
  template<int Index>
  struct Parameter {
    /// \brief Type of the parameter at \c Index.
    ///
    typedef typename
        detail::ParameterPackImplAtIndex<Index, Superclass>::CarType type;
  };

  /// The number of parameters in this \c ParameterPack type.
  ///
  const static int NUM_PARAMETERS =
      detail::ParameterPackImplSize<Superclass>::NUM_PARAMETERS;

  /// Returns the number of parameters (and argument values) held in this
  /// \c ParameterPack. The return value is the same as \c NUM_PARAMETERS.
  ///
  DAX_EXEC_CONT_EXPORT
  dax::Id GetNumberOfParameters() const {
    return ParameterPack::NUM_PARAMETERS;
  }

  /// Sets the argument associated with the template parameter \c Index to the
  /// given \c argument.
  template<int Index>
  DAX_EXEC_CONT_EXPORT
  void SetArgument(const typename boost::remove_reference<
                     typename Parameter<Index>::type>::type &argument)
  {
    detail::ParameterPackImplAtIndex<Index, Superclass>::
        SetArgument(*this, argument);
  }

  /// Returns the argument associated with the template parameter \c Index.
  ///
  template<int Index>
  DAX_EXEC_CONT_EXPORT
  const typename boost::remove_reference<typename Parameter<Index>::type>::type&
  GetArgument() const
  {
    return detail::ParameterPackImplAtIndex<Index, Superclass>::
        GetArgument(*this);
  }
  template<int Index>
  DAX_EXEC_CONT_EXPORT
  typename boost::remove_reference<typename Parameter<Index>::type>::type &
  GetArgument()
  {
    return detail::ParameterPackImplAtIndex<Index, Superclass>::
        GetArgument(*this);
  }

  /// Invoke a function \c f using the arguments stored in this \c
  /// ParameterPack.
  ///
  /// An optional \c parameterTransform functor allows you to transform each
  /// argument before passing it to the function. In addition to an overloaded
  /// parenthesis operator, the \c Transform class must also have a templated
  /// subclass named \c ReturnType containing a typedef named \c type giving
  /// the return type for a given type. For example, the default \c Transform
  /// is an identity function that looks essentially like this.
  ///
  /// \code{.cpp}
  /// struct IdentityTransform
  /// {
  ///   template<typename T> struct ReturnType {
  ///     typedef T typedef;
  ///   };
  ///   template<typename T> T &operator()(T &x) const { return x; }
  /// };
  /// \endcode
  ///
  /// As another example, this \c Transform class converts a reference to an
  /// argument to a pointer to that argument.
  ///
  /// \code{.cpp}
  /// struct GetReferenceTransform
  /// {
  ///   template<typename T> struct ReturnType {
  ///     typedef const typename boost::remove_reference<T>::type *type;
  ///   };
  ///   template<typename T> T *operator()(T &x) const { return &x; }
  /// };
  /// \endcode
  ///
  template<typename Function, typename Transform>
  DAX_CONT_EXPORT
  void InvokeCont(Function &f, const Transform &parameterTransform) const
  {
    detail::ParameterPackDoInvokeCont(f, parameterTransform, *this);
  }
  template<typename Function, typename Transform>
  DAX_CONT_EXPORT
  void InvokeCont(Function &f, const Transform &parameterTransform)
  {
    detail::ParameterPackDoInvokeCont(f, parameterTransform, *this);
  }
  template<typename Function, typename Transform>
  DAX_CONT_EXPORT
  void InvokeCont(const Function &f, const Transform &parameterTransform) const
  {
    detail::ParameterPackDoInvokeCont(f, parameterTransform, *this);
  }
  template<typename Function, typename Transform>
  DAX_CONT_EXPORT
  void InvokeCont(const Function &f, const Transform &parameterTransform)
  {
    detail::ParameterPackDoInvokeCont(f, parameterTransform, *this);
  }
  template<typename Function>
  DAX_CONT_EXPORT
  void InvokeCont(Function &f) const
  {
    this->InvokeCont(f, IdentityFunctorCont());
  }
  template<typename Function>
  DAX_CONT_EXPORT
  void InvokeCont(Function &f)
  {
    this->InvokeCont(f, IdentityFunctorCont());
  }
  template<typename Function>
  DAX_CONT_EXPORT
  void InvokeCont(const Function &f) const
  {
    this->InvokeCont(f, IdentityFunctorCont());
  }
  template<typename Function>
  DAX_CONT_EXPORT
  void InvokeCont(const Function &f)
  {
    this->InvokeCont(f, IdentityFunctorCont());
  }
  template<typename Function, typename Transform>
  DAX_EXEC_EXPORT
  void InvokeExec(Function &f, const Transform &parameterTransform) const
  {
    detail::ParameterPackDoInvokeExec(f, parameterTransform, *this);
  }
  template<typename Function, typename Transform>
  DAX_EXEC_EXPORT
  void InvokeExec(Function &f, const Transform &parameterTransform)
  {
    detail::ParameterPackDoInvokeExec(f, parameterTransform, *this);
  }
  template<typename Function, typename Transform>
  DAX_EXEC_EXPORT
  void InvokeExec(const Function &f, const Transform &parameterTransform) const
  {
    detail::ParameterPackDoInvokeExec(f, parameterTransform, *this);
  }
  template<typename Function, typename Transform>
  DAX_EXEC_EXPORT
  void InvokeExec(const Function &f, const Transform &parameterTransform)
  {
    detail::ParameterPackDoInvokeExec(f, parameterTransform, *this);
  }
  template<typename Function>
  DAX_EXEC_EXPORT
  void InvokeExec(Function &f) const
  {
    this->InvokeExec(f, IdentityFunctorExec());
  }
  template<typename Function>
  DAX_EXEC_EXPORT
  void InvokeExec(Function &f)
  {
    this->InvokeExec(f, IdentityFunctorExec());
  }
  template<typename Function>
  DAX_EXEC_EXPORT
  void InvokeExec(const Function &f) const
  {
    this->InvokeExec(f, IdentityFunctorExec());
  }
  template<typename Function>
  DAX_EXEC_EXPORT
  void InvokeExec(const Function &f)
  {
    this->InvokeExec(f, IdentityFunctorExec());
  }

  /// \brief Append an argument to this \c ParameterPack.
  ///
  /// Returns a new \c ParameterPack where the first arguments are all the same
  /// as the ones in this \c ParameterPack and then \c newArg is added to the
  /// end of the argument list.
  ///
  /// The \c Append method is intended to pass further arguments to a called
  /// function without worrying about manipulation of the type list. \c Append
  /// method invocations can be chained together to specify multiple arguments
  /// to be appended. The following is a simple example of a method that
  /// derives some arrays from a set of keys and then passes this information
  /// to another method that presumably knows how to use them.
  ///
  /// \code{.cpp}
  /// template<typename ParameterPackType>
  /// void DoSomethingWithKeys(const ParameterPackType &arguments)
  /// {
  ///   typedef dax::cont::internal::DeviceAdapterAlgorithm<DeviceAdapterTag>
  ///       Algorithm;
  ///   typedef typename ParameterPackType::template Parameter<1>::type
  ///       KeysType;
  ///
  ///   KeysType keys = arguments::template Get<1>();
  ///
  ///   KeysType sortedKeys;
  ///   Algorithm::Copy(keys, sortedKeys);
  ///   Algorithm::Sort(sortedKeys);
  ///
  ///   KeysType uniqueKeys;
  ///   Algorithm::Copy(sortedKeys, uniqueKeys);
  ///   Algorithm::Unique(uniqueKeys);
  ///
  ///   DoSomethingWithSortedKeys(
  ///       arguments
  ///       .Append(sortedKeys)
  ///       .Append(uniqueKeys));
  /// }
  /// \endcode
  ///
  /// The \c Append method is only supported for the control environment.
  ///
  template<typename NewType>
  DAX_CONT_EXPORT
  typename detail::ParameterPackAppendType<ThisType, NewType>::type
  Append(NewType newArg) const {
    typedef detail::ParameterPackAppendType<ThisType, NewType> PPackAppendType;
    typename PPackAppendType::type appendedPack(
          PPackAppendType::ConstructImpl(*this, newArg),
          ParameterPackCopyTag(),
          ParameterPackContTag());
    return appendedPack;
  }

  /// \brief Replaces an argument in this \c ParameterPack
  ///
  /// Returns a new ParameterPack with all entries replaced except for the
  /// argument at the given index, which is replaced with \c newArg, which can
  /// be a different type.
  ///
  /// \code{.cpp}
  /// template<typename ParameterPackType>
  /// void MangleFirstArgument(const ParameterPackType &arguments)
  /// {
  ///   typedef typename ParameterPackType::template Parameter<1>::type
  ///       FirstArgType;
  ///   FirstArgType firstArg = arguments::template Get<1>();
  ///
  ///   // Derive new argumente mangledArg...
  ///
  ///   DoSomethingElse(arguments.template Replace<1>(mangledArg));
  /// }
  /// \endcode
  ///
  /// The \c Replace method is only supported for the control environment.
  ///
  template<int Index, typename NewType>
  DAX_CONT_EXPORT
  typename detail::PPackReplace<NewType, Index, ThisType>::type
  Replace(NewType newArg) const {
    BOOST_STATIC_ASSERT(Index > 0);
    BOOST_STATIC_ASSERT(Index <= NUM_PARAMETERS);
    typedef detail::PPackReplace<NewType, Index, ThisType> PPackReplaceType;
    typename PPackReplaceType::type replacedPack(
          PPackReplaceType::ConstructImpl(newArg, *this),
          ParameterPackCopyTag(),
          ParameterPackContTag());
    return replacedPack;
  }
};

namespace detail {

template<>
struct FindParameterPackImpl<ParameterPack<> >
{
  typedef ParameterPackImplNull type;

  DAX_EXEC_CONT_EXPORT
  static type Construct() { return type(); }
};

template<typename AppendType>
struct ParameterPackAppendType<ParameterPack<>, AppendType>
{
  typedef ParameterPack<AppendType> type;

private:
  typedef detail::ParameterPackImpl<AppendType,ParameterPackImplNull> _implType;

public:
  DAX_CONT_EXPORT
  static _implType
  ConstructImpl(ParameterPackImplNull, AppendType toAppend)
  {
    return _implType(toAppend, ParameterPackImplNull());
  }
};

} // namespace detail

/// Given an instance of the \c ParameterPack template and an (optional) return
/// type, provides the type for the function that accepts those arguments and
/// returns a value of that type.  The appropriate function type is available
/// as a typedef in this class named \c type.
///
template<typename ParameterPackType, typename ReturnType = void>
struct ParameterPackToSignature
#ifdef DAX_DOXYGEN_ONLY
{
  /// The type signature for a function with the given arguments and return
  /// type.
  ///
  typedef ReturnType type(parameters);
}
#endif
;

/// Given a type for a function, provides the type for a ParameterPack that
/// matches the parameters of that function.  The appropriate \c ParameterPack
/// type is available as a typedef in this class named \c type.
///
template<typename Signature>
struct SignatureToParameterPack
#ifdef DAX_DOXYGEN_ONLY
{
  /// The type signature for a \c ParameterPack that matches the parameters of
  /// the template's function \c Signature type.
  ///
  typedef ParameterPack<parameters> type;
}
#endif
;

/// Invokes the given function with the arguments stored in the given
/// \c ParameterPack.
///
/// An optional \c Transform functor will be called on each argument before
/// being passed on to \c f.  See the ParameterPack::Invoke functions for
/// a description of the requirements for the \c Transform type.
///
template<typename Function, typename ParameterPackType, typename Transform>
DAX_CONT_EXPORT
void ParameterPackInvokeCont(Function &f,
                             ParameterPackType &params,
                             const Transform &parameterTransform)
{
  params.InvokeCont(f, parameterTransform);
}
template<typename Function, typename ParameterPackType, typename Transform>
DAX_CONT_EXPORT
void ParameterPackInvokeCont(const Function &f,
                             ParameterPackType &params,
                             const Transform &parameterTransform)
{
  params.InvokeCont(f, parameterTransform);
}
template<typename Function, typename ParameterPackType>
DAX_CONT_EXPORT
void ParameterPackInvokeCont(Function &f,
                             ParameterPackType &params)
{
  ParameterPackInvokeCont(f, params, IdentityFunctorCont());
}
template<typename Function, typename ParameterPackType>
DAX_CONT_EXPORT
void ParameterPackInvokeCont(const Function &f,
                             ParameterPackType &params)
{
  ParameterPackInvokeCont(f, params, IdentityFunctorCont());
}

template<typename Function, typename ParameterPackType, typename Transform>
DAX_EXEC_EXPORT
void ParameterPackInvokeExec(Function &f,
                             ParameterPackType &params,
                             const Transform &parameterTransform)
{
  params.InvokeExec(f, parameterTransform);
}
template<typename Function, typename ParameterPackType, typename Transform>
DAX_EXEC_EXPORT
void ParameterPackInvokeExec(const Function &f,
                             ParameterPackType &params,
                             const Transform &parameterTransform)
{
  params.InvokeExec(f, parameterTransform);
}
template<typename Function, typename ParameterPackType>
DAX_EXEC_EXPORT
void ParameterPackInvokeExec(Function &f,
                             ParameterPackType &params)
{
  ParameterPackInvokeExec(f, params, IdentityFunctorExec());
}
template<typename Function, typename ParameterPackType>
DAX_EXEC_EXPORT
void ParameterPackInvokeExec(const Function &f,
                             ParameterPackType &params)
{
  ParameterPackInvokeExec(f, params, IdentityFunctorExec());
}

namespace detail {

template<typename ReturnType, typename FunctionType>
class ParameterPackReturnFunctorContBase
{
  typedef ParameterPackReturnFunctorContBase<ReturnType, FunctionType> ThisType;

public:
  DAX_CONT_EXPORT
  ParameterPackReturnFunctorContBase(const FunctionType &f)
    : Function(f)
  {  }

  DAX_CONT_EXPORT
  ReturnType GetReturnValue() const {
    return this->ReturnValue;
  }

protected:
  DAX_CONT_EXPORT
  void RecordReturnValue(const ReturnType &returnValue) {
    this->ReturnValue = returnValue;
  }

private:
  DAX_CONT_EXPORT
  ParameterPackReturnFunctorContBase(const ThisType &); // Not implemented.
  DAX_CONT_EXPORT
  void operator()(const ThisType &); // Not implemented.

  ReturnType ReturnValue;

protected:
  FunctionType Function;
};

// Special implementation to return a reference.
template<typename ReturnType, typename FunctionType>
class ParameterPackReturnFunctorContBase<ReturnType &, FunctionType>
    : protected ParameterPackReturnFunctorContBase<ReturnType *, FunctionType>
{
  typedef ParameterPackReturnFunctorContBase<ReturnType *, FunctionType>
      Superclass;
public:
  DAX_CONT_EXPORT
  ParameterPackReturnFunctorContBase(const FunctionType &f)
    : Superclass(f) {  }

  DAX_CONT_EXPORT
  ReturnType &GetReturnValue() const {
    return *this->Superclass::GetReturnValue();
  }

protected:
  DAX_CONT_EXPORT
  void RecordReturnValue(ReturnType &returnValue) {
    this->Superclass::RecordReturnValue(&returnValue);
  }
};

template<typename ReturnType, typename FunctionType>
class ParameterPackReturnFunctorExecBase
{
  typedef ParameterPackReturnFunctorExecBase<ReturnType, FunctionType> ThisType;

public:
  DAX_EXEC_EXPORT
  ParameterPackReturnFunctorExecBase(const FunctionType &f)
    : Function(f)
  {  }

  DAX_EXEC_EXPORT
  ReturnType GetReturnValue() const {
    return this->ReturnValue;
  }

protected:
  DAX_EXEC_EXPORT
  void RecordReturnValue(const ReturnType &returnValue) {
    this->ReturnValue = returnValue;
  }

private:
  DAX_CONT_EXPORT
  ParameterPackReturnFunctorExecBase(const ThisType &); // Not implemented.
  DAX_CONT_EXPORT
  void operator()(const ThisType &); // Not implemented.

  ReturnType ReturnValue;

protected:
  FunctionType Function;
};

// Special implementation to return a reference.
template<typename ReturnType, typename FunctionType>
class ParameterPackReturnFunctorExecBase<ReturnType &, FunctionType>
    : protected ParameterPackReturnFunctorExecBase<ReturnType *, FunctionType>
{
  typedef ParameterPackReturnFunctorExecBase<ReturnType *, FunctionType>
      Superclass;
public:
  DAX_EXEC_EXPORT
  ParameterPackReturnFunctorExecBase(const FunctionType &f)
    : Superclass(f) {  }

  DAX_EXEC_EXPORT
  ReturnType &GetReturnValue() const {
    return *this->Superclass::GetReturnValue();
  }

protected:
  DAX_EXEC_EXPORT
  void RecordReturnValue(ReturnType &returnValue) {
    this->Superclass::RecordReturnValue(&returnValue);
  }
};

template<typename ReturnType, typename Function, int NumParameters>
class ParameterPackReturnFunctorCont;

template<typename ReturnType, typename Function, int NumParameters>
class ParameterPackReturnFunctorExec;

} // namespace detail (in dax::internal)

/// Invokes the given function with the arguments stored in the given
/// \c ParameterPack.  The functor is assumed to return a value of type
/// \c ReturnType, and this value will be returned from this function.
///
/// An optional \c Transform functor will be called on each argument before
/// being passed on to \c f.  See the ParameterPack::Invoke functions for
/// a description of the requirements for the \c Transform type.
///
template<typename ReturnType,
         typename Function,
         typename ParameterPackType,
         typename Transform>
DAX_CONT_EXPORT
ReturnType ParameterPackInvokeWithReturnCont(
    Function &f,
    ParameterPackType &params,
    const Transform &parameterTransform)
{
  detail::ParameterPackReturnFunctorCont<
      ReturnType,
      Function,
      ParameterPackType::NUM_PARAMETERS> functor(f);
  ParameterPackInvokeCont(functor, params, parameterTransform);
  return functor.GetReturnValue();
}
template<typename ReturnType,
         typename Function,
         typename ParameterPackType,
         typename Transform>
DAX_CONT_EXPORT
ReturnType ParameterPackInvokeWithReturnCont(
    const Function &f,
    ParameterPackType &params,
    const Transform &parameterTransform)
{
  detail::ParameterPackReturnFunctorCont<
      ReturnType,
      const Function,
      ParameterPackType::NUM_PARAMETERS> functor(f);
  ParameterPackInvokeCont(functor, params, parameterTransform);
  return functor.GetReturnValue();
}
template<typename ReturnType,
         typename Function,
         typename ParameterPackType>
DAX_CONT_EXPORT
ReturnType ParameterPackInvokeWithReturnCont(Function &f,
                                             ParameterPackType &params)
{
  return ParameterPackInvokeWithReturnCont<ReturnType>(
        f, params, IdentityFunctorCont());
}
template<typename ReturnType,
         typename Function,
         typename ParameterPackType>
DAX_CONT_EXPORT
ReturnType ParameterPackInvokeWithReturnCont(const Function &f,
                                             ParameterPackType &params)
{
  return ParameterPackInvokeWithReturnCont<ReturnType>(
        f, params, IdentityFunctorCont());
}

template<typename ReturnType,
         typename Function,
         typename ParameterPackType,
         typename Transform>
DAX_EXEC_EXPORT
ReturnType ParameterPackInvokeWithReturnExec(
    Function &f,
    ParameterPackType &params,
    const Transform &parameterTransform)
{
  detail::ParameterPackReturnFunctorExec<
      ReturnType,
      Function,
      ParameterPackType::NUM_PARAMETERS> functor(f);
  ParameterPackInvokeExec(functor, params, parameterTransform);
  return functor.GetReturnValue();
}
template<typename ReturnType,
         typename Function,
         typename ParameterPackType,
         typename Transform>
DAX_EXEC_EXPORT
ReturnType ParameterPackInvokeWithReturnExec(
    const Function &f,
    ParameterPackType &params,
    const Transform &parameterTransform)
{
  detail::ParameterPackReturnFunctorExec<
      ReturnType,
      const Function,
      ParameterPackType::NUM_PARAMETERS> functor(f);
  ParameterPackInvokeExec(functor, params, parameterTransform);
  return functor.GetReturnValue();
}
template<typename ReturnType,
         typename Function,
         typename ParameterPackType>
DAX_EXEC_EXPORT
ReturnType ParameterPackInvokeWithReturnExec(Function &f,
                                             ParameterPackType &params)
{
  return ParameterPackInvokeWithReturnExec<ReturnType>(
        f, params, IdentityFunctorExec());
}
template<typename ReturnType,
         typename Function,
         typename ParameterPackType>
DAX_EXEC_EXPORT
ReturnType ParameterPackInvokeWithReturnExec(const Function &f,
                                             ParameterPackType &params)
{
  return ParameterPackInvokeWithReturnExec<ReturnType>(
        f, params, IdentityFunctorExec());
}

namespace detail {

template<typename ObjectToConstruct, int NumParameters>
struct ParameterPackConstructFunctorCont;

template<typename ObjectToConstruct, int NumParameters>
struct ParameterPackConstructFunctorExec;

} // namespace detail

/// Constructs an object of type \c ObjectToConstruct by passing the
/// arguments in \c params to the object's constructor and returns this
/// newly created object.
///
/// An optional \c Transform functor will be called on each argument before
/// being passed on to \c f.  See the ParameterPack::Invoke functions for
/// a description of the requirements for the \c Transform type.
///
template<typename ObjectToConstruct,
         typename ParameterPackType,
         typename Transform>
DAX_CONT_EXPORT
ObjectToConstruct ParameterPackConstructCont(
    ParameterPackType &params,
    const Transform &parameterTransform)
{
  detail::ParameterPackConstructFunctorCont<
      ObjectToConstruct, ParameterPackType::NUM_PARAMETERS> functor;
  return ParameterPackInvokeWithReturnCont<ObjectToConstruct>(
        functor, params, parameterTransform);
}
template<typename ObjectToConstruct, typename ParameterPackType>
DAX_CONT_EXPORT
ObjectToConstruct ParameterPackConstructCont(ParameterPackType &params)
{
  return ParameterPackConstructCont<ObjectToConstruct>(
        params, IdentityFunctorCont());
}

template<typename ObjectToConstruct,
         typename ParameterPackType,
         typename Transform>
DAX_EXEC_EXPORT
ObjectToConstruct ParameterPackConstructExec(
    ParameterPackType &params,
    const Transform &parameterTransform)
{
  detail::ParameterPackConstructFunctorExec<
      ObjectToConstruct, ParameterPackType::NUM_PARAMETERS> functor;
  return ParameterPackInvokeWithReturnExec<ObjectToConstruct>(
        functor, params, parameterTransform);
}
template<typename ObjectToConstruct, typename ParameterPackType>
DAX_EXEC_EXPORT
ObjectToConstruct ParameterPackConstructExec(ParameterPackType &params)
{
  return ParameterPackConstructExec<ObjectToConstruct>(
        params, IdentityFunctorExec());
}

#ifdef DAX_DOXYGEN_ONLY
/// Convenience function that creates a \c ParameterPack with parameters of the
/// same types as the arguments to this function and initialized to the values
/// passed to this function. Only works in the control environment.
///
template<typename...T>
DAX_CONT_EXPORT
dax::internal::ParameterPack<T...>
make_ParameterPack(T...arguments);
#endif // DAX_DOXYGON_ONLY


}
} // namespace dax::internal

#define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PP_INC(DAX_MAX_PARAMETER_SIZE), <dax/internal/ParameterPack.h>))
#include BOOST_PP_ITERATE()

#undef _dax_pp_T___all
#undef __dax_pp_T___
#undef _dax_pp_typename___T_all
#undef __dax_pp_typename___T

#endif //__dax_internal_ParameterPack_h

#else //BOOST_PP_IS_ITERATING

namespace dax {
namespace internal {
namespace detail {

#if _dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

template<typename FirstType _dax_pp_comma _dax_pp_typename___T>
struct FindParameterPackImpl<
    ParameterPack<FirstType _dax_pp_comma _dax_pp_T___> >
{
  typedef ParameterPackImpl<
      typename boost::remove_const<FirstType>::type,
      typename FindParameterPackImpl<ParameterPack<_dax_pp_T___> >::type> type;

  DAX_EXEC_CONT_EXPORT
  static type Construct(FirstType first _dax_pp_comma
                        _dax_pp_params___(rest))
  {
    return type(first,
                FindParameterPackImpl<ParameterPack<_dax_pp_T___> >::Construct(
                  _dax_pp_args___(rest)));
  }
};

#endif //_dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

#if _dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE - 1

template<typename FirstType,
         _dax_pp_typename___T _dax_pp_comma
         typename AppendType>
struct ParameterPackAppendType<
    ParameterPack<FirstType _dax_pp_comma _dax_pp_T___>,AppendType>
{
  typedef ParameterPack<FirstType _dax_pp_comma _dax_pp_T___, AppendType> type;

private:
  typedef typename FindParameterPackImpl<type>::type _implType;
  typedef typename FindParameterPackImpl<
    ParameterPack<FirstType _dax_pp_comma _dax_pp_T___> >::type _implInputType;

public:
  DAX_CONT_EXPORT
  static _implType ConstructImpl(const _implInputType &originals,
                                 AppendType toAppend)
  {
    typedef ParameterPackImplAccess<_implInputType> Access;
    typedef ParameterPackAppendType<ParameterPack<_dax_pp_T___>, AppendType>
        PPackAppendRemainder;
    return _implType(
          Access::GetFirstArgument(originals),
          PPackAppendRemainder::ConstructImpl(Access::GetCdr(originals),
                                              toAppend),
          ParameterPackContTag());
  }
};

#endif //_dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE - 1

#if _dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

template<typename Function,
         typename Transform,
         typename ParameterPackImplType _dax_pp_comma
         _dax_pp_typename___T>
struct ParameterPackDoInvokeContImpl<
    _dax_pp_sizeof___T,
    Function,
    Transform,
    ParameterPackImplType _dax_pp_comma
    _dax_pp_T___>
{
  DAX_CONT_EXPORT
  void operator()(Function &f,
                  const Transform &transform,
                  ParameterPackImplType &params _dax_pp_comma
                  _dax_pp_params___(arguments))
  {
    typedef ParameterPackImplAccess<ParameterPackImplType> Access;
    typedef typename boost::mpl::if_<
        typename boost::is_const<ParameterPackImplType>::type,
        const typename Access::CdrType,
        typename Access::CdrType>::type RemainingArgumentsType;
    typedef typename boost::mpl::if_<
        typename boost::is_const<ParameterPackImplType>::type,
        const typename boost::remove_reference<typename Access::CarType>::type &,
        typename boost::remove_reference<typename Access::CarType>::type &>::type
          InputParameterType;
    typedef typename Transform::template ReturnType<InputParameterType>::type
        TransformedParameterType;
    // It is important to explicitly specify the template parameters. When
    // automatically determining the types, then reference types get dropped and
    // become pass-by-value instead of pass-by-reference. If the functor has
    // reference arguments that it modifies, it will modify a variable on the
    // call stack rather than the original variable.
    ParameterPackDoInvokeContImpl<
        _dax_pp_sizeof___T + 1,
        Function,
        Transform,
        RemainingArgumentsType,
        _dax_pp_T___ _dax_pp_comma
        TransformedParameterType>()(
          f,
          transform,
          Access::GetCdr(params),
          _dax_pp_args___(arguments) _dax_pp_comma
          transform(Access::GetFirstArgument(params)));
  }
};

#endif //_dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

template<typename Function,
         typename Transform _dax_pp_comma
         _dax_pp_typename___T>
struct ParameterPackDoInvokeContImpl<
    _dax_pp_sizeof___T,
    Function,
    Transform,
    ParameterPackImplNull _dax_pp_comma
    _dax_pp_T___>
{
  DAX_CONT_EXPORT
  void operator()(Function &f,
                  const Transform &daxNotUsed(transform),
                  ParameterPackImplNull &daxNotUsed(params) _dax_pp_comma
                  _dax_pp_params___(arguments))
  {
    f(_dax_pp_args___(arguments));
  }
};

template<typename Function,
         typename Transform _dax_pp_comma
         _dax_pp_typename___T>
struct ParameterPackDoInvokeContImpl<
    _dax_pp_sizeof___T,
    Function,
    Transform,
    const ParameterPackImplNull _dax_pp_comma
    _dax_pp_T___>
{
  DAX_CONT_EXPORT
  void operator()(Function &f,
                  const Transform &daxNotUsed(transform),
                  const ParameterPackImplNull &daxNotUsed(params) _dax_pp_comma
                  _dax_pp_params___(arguments))
  {
    f(_dax_pp_args___(arguments));
  }
};

#if _dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

template<typename Function,
         typename Transform,
         typename ParameterPackImplType _dax_pp_comma
         _dax_pp_typename___T>
struct ParameterPackDoInvokeExecImpl<
    _dax_pp_sizeof___T,
    Function,
    Transform,
    ParameterPackImplType _dax_pp_comma
    _dax_pp_T___>
{
  DAX_EXEC_EXPORT
  void operator()(Function &f,
                  const Transform &transform,
                  ParameterPackImplType &params _dax_pp_comma
                  _dax_pp_params___(arguments))
  {
    typedef ParameterPackImplAccess<ParameterPackImplType> Access;
    typedef typename boost::mpl::if_<
        typename boost::is_const<ParameterPackImplType>::type,
        const typename Access::CdrType,
        typename Access::CdrType>::type RemainingArgumentsType;
    typedef typename boost::mpl::if_<
        typename boost::is_const<ParameterPackImplType>::type,
        const typename boost::remove_reference<typename Access::CarType>::type &,
        typename boost::remove_reference<typename Access::CarType>::type &>::type
          InputParameterType;
    typedef typename Transform::template ReturnType<InputParameterType>::type
        TransformedParameterType;
    // It is important to explicitly specify the template parameters. When
    // automatically determining the types, then reference types get dropped and
    // become pass-by-value instead of pass-by-reference. If the functor has
    // reference arguments that it modifies, it will modify a variable on the
    // call stack rather than the original variable.
    ParameterPackDoInvokeExecImpl<
        _dax_pp_sizeof___T + 1,
        Function,
        Transform,
        RemainingArgumentsType,
        _dax_pp_T___ _dax_pp_comma
        TransformedParameterType>()(
          f,
          transform,
          Access::GetCdr(params),
          _dax_pp_args___(arguments) _dax_pp_comma
          transform(Access::GetFirstArgument(params)));
  }
};

#endif //_dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

template<typename Function,
         typename Transform _dax_pp_comma
         _dax_pp_typename___T>
struct ParameterPackDoInvokeExecImpl<
    _dax_pp_sizeof___T,
    Function,
    Transform,
    ParameterPackImplNull _dax_pp_comma
    _dax_pp_T___>
{
  DAX_EXEC_EXPORT
  void operator()(Function &f,
                  const Transform &daxNotUsed(transform),
                  ParameterPackImplNull &daxNotUsed(params) _dax_pp_comma
                  _dax_pp_params___(arguments))
  {
    f(_dax_pp_args___(arguments));
  }
};

template<typename Function,
         typename Transform _dax_pp_comma
         _dax_pp_typename___T>
struct ParameterPackDoInvokeExecImpl<
    _dax_pp_sizeof___T,
    Function,
    Transform,
    const ParameterPackImplNull _dax_pp_comma
    _dax_pp_T___>
{
  DAX_EXEC_EXPORT
  void operator()(Function &f,
                  const Transform &daxNotUsed(transform),
                  const ParameterPackImplNull &daxNotUsed(params) _dax_pp_comma
                  _dax_pp_params___(arguments))
  {
    f(_dax_pp_args___(arguments));
  }
};

template<typename ReturnType, typename FunctionType>
class ParameterPackReturnFunctorCont<
    ReturnType, FunctionType, _dax_pp_sizeof___T>
    : public ParameterPackReturnFunctorContBase<ReturnType, FunctionType>
{
  typedef ParameterPackReturnFunctorContBase<ReturnType, FunctionType>
      Superclass;
public:
  DAX_CONT_EXPORT
  ParameterPackReturnFunctorCont(const FunctionType &f)
    : Superclass(f)
  {  }

#if _dax_pp_sizeof___T > 0
  template<_dax_pp_typename___T>
#endif
  DAX_CONT_EXPORT
  void operator()(_dax_pp_params___(&arguments)) {
    this->RecordReturnValue(this->Function(_dax_pp_args___(arguments)));
  }
};

template<typename ReturnType, typename FunctionType>
class ParameterPackReturnFunctorExec<
    ReturnType, FunctionType, _dax_pp_sizeof___T>
    : public ParameterPackReturnFunctorExecBase<ReturnType, FunctionType>
{
  typedef ParameterPackReturnFunctorExecBase<ReturnType, FunctionType>
      Superclass;
public:
  DAX_EXEC_EXPORT
  ParameterPackReturnFunctorExec(const FunctionType &f)
    : Superclass(f)
  {  }

#if _dax_pp_sizeof___T > 0
  template<_dax_pp_typename___T>
#endif
  DAX_EXEC_EXPORT
  void operator()(_dax_pp_params___(&arguments)) {
    this->RecordReturnValue(this->Function(_dax_pp_args___(arguments)));
  }
};

template<typename ObjectToConstruct>
struct ParameterPackConstructFunctorCont<ObjectToConstruct, _dax_pp_sizeof___T>
{
public:
#if _dax_pp_sizeof___T > 0
  template<_dax_pp_typename___T>
#endif
  DAX_CONT_EXPORT
  ObjectToConstruct operator()(_dax_pp_params___(arguments)) const {
    return ObjectToConstruct(_dax_pp_args___(arguments));
  }
};

template<typename ObjectToConstruct>
struct ParameterPackConstructFunctorExec<ObjectToConstruct, _dax_pp_sizeof___T>
{
public:
#if _dax_pp_sizeof___T > 0
  template<_dax_pp_typename___T>
#endif
  DAX_EXEC_EXPORT
  ObjectToConstruct operator()(_dax_pp_params___(arguments)) const {
    return ObjectToConstruct(_dax_pp_args___(arguments));
  }
};

#if _dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

template<typename NewType _dax_pp_comma _dax_pp_typename___T>
struct PPackPrepend<NewType, ParameterPack<_dax_pp_T___> >
{
  typedef ParameterPack<NewType _dax_pp_comma _dax_pp_T___> type;
};

template<typename NewType,
         int Index,
         typename FirstType _dax_pp_comma
         _dax_pp_typename___T
         >
struct PPackReplace<NewType,
                    Index,
                    ParameterPack<FirstType _dax_pp_comma _dax_pp_T___> >
{
  typedef ParameterPack<FirstType _dax_pp_comma _dax_pp_T___>
      ParameterPackInputType;
  // These check for a valid index. If you get a compile error in these lines,
  // it is probably caused by an invalid Index template parameter for one of
  // the ParameterPack::Replace. Remember that parameters are index starting at
  // 1.
  BOOST_STATIC_ASSERT(Index > 0);
  BOOST_STATIC_ASSERT(Index <= ParameterPackInputType::NUM_PARAMETERS);

  typedef typename
      PPackPrepend<
          FirstType,
          typename PPackReplace<NewType, Index-1, ParameterPack<_dax_pp_T___> >::type
      >::type type;

private:
   typedef typename FindParameterPackImpl<type>::type _implType;
   typedef typename FindParameterPackImpl<ParameterPackInputType>::type
      _implInputType;

public:
   DAX_CONT_EXPORT
   static _implType ConstructImpl(NewType replacement,
                                  const _implInputType &originals)
   {
     typedef ParameterPackImplAccess<_implInputType> Access;
     typedef PPackReplace<NewType, Index-1, ParameterPack<_dax_pp_T___> >
         PPackReplaceRemainder;
     return _implType(
           Access::GetFirstArgument(originals),
           PPackReplaceRemainder::ConstructImpl(replacement,
                                                Access::GetCdr(originals)),
           ParameterPackContTag());
   }
};

template<typename NewType,
         typename FirstType _dax_pp_comma
         _dax_pp_typename___T
         >
struct PPackReplace<NewType,
                    1,
                    ParameterPack<FirstType _dax_pp_comma _dax_pp_T___> >
{
  typedef ParameterPack<FirstType _dax_pp_comma _dax_pp_T___>
      ParameterPackInputType;

  typedef ParameterPack<NewType _dax_pp_comma _dax_pp_T___> type;
  typedef typename FindParameterPackImpl<ParameterPackInputType>::type
     _implInputType;

  typedef typename FindParameterPackImpl<type>::type _implType;

  DAX_CONT_EXPORT
  static _implType ConstructImpl(NewType replacement,
                                 const _implInputType &originals)
  {
    typedef ParameterPackImplAccess<_implInputType> Access;

    return _implType(replacement,
                     Access::GetCdr(originals),
                     ParameterPackContTag());
  }
};

#endif //_dax_pp_sizeof___T < DAX_MAX_PARAMETER_SIZE

} // namespace detail

#if _dax_pp_sizeof___T > 0

template<_dax_pp_typename___T>
DAX_CONT_EXPORT
dax::internal::ParameterPack<_dax_pp_T___>
make_ParameterPack(_dax_pp_params___(arguments))
{
  typedef dax::internal::ParameterPack<_dax_pp_T___> ParameterPackType;
  return ParameterPackType(
        dax::internal::detail::FindParameterPackImpl<ParameterPackType>::
          Construct(_dax_pp_args___(arguments)),
        dax::internal::ParameterPackCopyTag(),
        dax::internal::ParameterPackContTag());
}

#endif //_dax_pp_sizeof___T > 0

template<_dax_pp_typename___T _dax_pp_comma typename ReturnType>
struct ParameterPackToSignature<ParameterPack<_dax_pp_T___>, ReturnType>
{
  typedef ReturnType type(_dax_pp_T___);
};

template<typename ReturnType _dax_pp_comma _dax_pp_typename___T>
struct SignatureToParameterPack<ReturnType(_dax_pp_T___)>
{
  typedef dax::internal::ParameterPack<_dax_pp_T___> type;
};


}
} // namespace dax::internal

#endif //BOOST_PP_IS_ITERATING
