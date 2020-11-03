
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_transform_WithParam__
#define __gnu_xml_transform_WithParam__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace transform
      {
          class Stylesheet;
          class TemplateNode;
          class WithParam;
      }
      namespace xpath
      {
          class Expr;
      }
    }
  }
  namespace javax
  {
    namespace xml
    {
      namespace namespace
      {
          class QName;
      }
    }
  }
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class Node;
      }
    }
  }
}

class gnu::xml::transform::WithParam : public ::java::lang::Object
{

public: // actually package-private
  WithParam(::javax::xml::namespace::QName *, ::gnu::xml::xpath::Expr *);
  WithParam(::javax::xml::namespace::QName *, ::gnu::xml::transform::TemplateNode *);
  ::java::lang::Object * getValue(::gnu::xml::transform::Stylesheet *, ::javax::xml::namespace::QName *, ::org::w3c::dom::Node *, jint, jint);
  ::gnu::xml::transform::WithParam * clone(::gnu::xml::transform::Stylesheet *);
  jboolean references(::javax::xml::namespace::QName *);
  ::javax::xml::namespace::QName * __attribute__((aligned(__alignof__( ::java::lang::Object)))) name;
  ::gnu::xml::xpath::Expr * select;
  ::gnu::xml::transform::TemplateNode * content;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_transform_WithParam__
