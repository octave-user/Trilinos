// $Id$
// $Source$
// @HEADER
// *****************************************************************************
//        Phalanx: A Partial Differential Equation Field Evaluation 
//       Kernel for Flexible Management of Complex Dependency Chains
//
// Copyright 2008 NTESS and the Phalanx contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef PHX_TEMPLATEMANAGER_HPP
#define PHX_TEMPLATEMANAGER_HPP

#include <typeinfo>
#include <vector>

#include "Teuchos_RCP.hpp"

#include "Sacado_mpl_size.hpp"
#include "Sacado_mpl_find.hpp"
#include "Sacado_mpl_for_each.hpp"
#include "Sacado_mpl_apply.hpp"

#include "Phalanx_config.hpp"

namespace PHX {

  // Forward declaration
  template <typename TypeSeq, typename BaseT, typename ObjectT>
  class TemplateIterator;
  template <typename TypeSeq, typename BaseT, typename ObjectT>
  class ConstTemplateIterator;

  //! Container class to manager template instantiations of a template class
  /*!
   * This class evaluatedFields a generic container class for managing multiple
   * instantiations of another class ObjectT.  It assumes each class
   * ObjectT<ScalarT> is derived from a non-template base class BaseT.  It
   * stores a vector of reference counted pointers to objects of type
   * BaseT corresponding to each instantiation of ObjectT.  The instantiations
   * ObjectT for each ScalarT are provided by a builder class, passed through
   * the buildObjects() method (see DefaultBuilderOp for an example builder).
   * An iterator is provided for accessing each template instantiation, and
   * non-templated virtual methods of BaseT can be called by dereferencing
   * the iterator.  Finally, template methods are provided to access the
   * stored objects as either objects of type BaseT (getAsBase()) or objects
   * of type ObjectT<ScalarT> (getAsObject()).  A map using RTTI is used to
   * map a typename to an index into the vector corresponding to the object
   * of that type.
   *
   * Template managers for specific types should derive from this class
   * instantiated on those types.  In most cases, a builder class will also
   * need to be created to instantiate the objects for each scalar type.
   */
  template <typename TypeSeq, typename BaseT, typename ObjectT>
  class TemplateManager {

    //! Implementation of < for type_info objects
    struct type_info_less {
      bool operator() (const std::type_info* a, const std::type_info* b) {
	return a->before(*b);
      }
    };

    template <typename BuilderOpT>
    struct BuildObject {
      std::vector< Teuchos::RCP<BaseT> >& objects;
      std::vector< bool >& disabled;
      const BuilderOpT& builder;
      BuildObject(std::vector< Teuchos::RCP<BaseT> >& objects_,
                  std::vector<bool>& disabled_,
		  const BuilderOpT& builder_) :
	objects(objects_),disabled(disabled_),builder(builder_) {}
      template <typename T> void operator()(T) const {
	int idx = Sacado::mpl::find<TypeSeq,T>::value;
        if (!disabled[idx])
          objects[idx] = builder.template build<T>();
      }
    };

    //! Declare TemplateIterator as a friend class
    friend class TemplateIterator<TypeSeq,BaseT,ObjectT>;

  public:

    //! Typedef for iterator
    typedef TemplateIterator<TypeSeq,BaseT,ObjectT> iterator;

    //! Typedef for const_iterator
    typedef ConstTemplateIterator<TypeSeq,BaseT,ObjectT> const_iterator;

    //! The default builder class for building objects for each ScalarT
    struct DefaultBuilderOp {

      //! Returns a new rcp for an object of type ObjectT<ScalarT>
      template<class ScalarT>
      Teuchos::RCP<BaseT> build() const {
	typedef typename Sacado::mpl::apply<ObjectT,ScalarT>::type type;
	return Teuchos::rcp( dynamic_cast<BaseT*>( new type ) );
      }

    };

    //! Default constructor
    TemplateManager();

    //! Destructor
    ~TemplateManager();

    //! Build objects for each ScalarT
    template <typename BuilderOpT>
    void buildObjects(const BuilderOpT& builder);

    //! Build objects for each ScalarT using default builder
    void buildObjects();

    //! Get RCP to object corrensponding to ScalarT as BaseT
    template<typename ScalarT>
    Teuchos::RCP<BaseT> getAsBase();

    //! Get RCP to object corrensponding to ScalarT as BaseT
    template<typename ScalarT>
    Teuchos::RCP<const BaseT> getAsBase() const;

    //! Get RCP to object corrensponding to ScalarT as ObjectT<ScalarT>
    template<typename ScalarT>
    Teuchos::RCP< typename Sacado::mpl::apply<ObjectT,ScalarT>::type > getAsObject();

    //! Get RCP to object corrensponding to ScalarT as ObjectT<ScalarT>
    template<typename ScalarT>
    Teuchos::RCP< const typename Sacado::mpl::apply<ObjectT,ScalarT>::type > getAsObject() const;

    //! Return an iterator that points to the first type object
    typename PHX::TemplateManager<TypeSeq,BaseT,ObjectT>::iterator begin();

    //! Return an iterator that points to the first type object
    typename PHX::TemplateManager<TypeSeq,BaseT,ObjectT>::const_iterator
    begin() const;

    //! Return an iterator that points one past the last type object
    typename PHX::TemplateManager<TypeSeq,BaseT,ObjectT>::iterator end();

    //! Return an iterator that points one past the last type object
    typename PHX::TemplateManager<TypeSeq,BaseT,ObjectT>::const_iterator
    end() const;

    //! Delete the underlying type. Used to clean out unused types.
    template <typename BuilderOpT>
    void deleteType();

    //! Disable the type so that it is not allocated.
    template <typename BuilderOpT>
    void disableType();

  private:

    //! Stores array of rcp's to objects of each type
    std::vector< Teuchos::RCP<BaseT> > objects;

    //! Set to true if the type should not be allocated
    std::vector<bool> disabled;
  };

}

// Include template definitions
#include "Phalanx_TemplateManager_Def.hpp"

// Include template iterator definition
#include "Phalanx_TemplateIterator.hpp"

#endif
