
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_CORBA_CompletionStatusHelper__
#define __org_omg_CORBA_CompletionStatusHelper__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class Any;
          class CompletionStatus;
          class CompletionStatusHelper;
          class TypeCode;
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
    }
  }
}

class org::omg::CORBA::CompletionStatusHelper : public ::java::lang::Object
{

public:
  CompletionStatusHelper();
  static ::org::omg::CORBA::CompletionStatus * extract(::org::omg::CORBA::Any *);
  static ::java::lang::String * id();
  static void insert(::org::omg::CORBA::Any *, ::org::omg::CORBA::CompletionStatus *);
  static ::org::omg::CORBA::CompletionStatus * read(::org::omg::CORBA::portable::InputStream *);
  static void write(::org::omg::CORBA::portable::OutputStream *, ::org::omg::CORBA::CompletionStatus *);
  static ::org::omg::CORBA::TypeCode * type();
  static ::java::lang::Class class$;
};

#endif // __org_omg_CORBA_CompletionStatusHelper__
