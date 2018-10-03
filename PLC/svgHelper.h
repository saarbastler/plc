#ifndef _INCLUDE_SVG_HELPER_H_
#define _INCLUDE_SVG_HELPER_H_

#include <iostream>

namespace svg
{
  extern const char X[];
  extern const char Y[];
  extern const char X1[];
  extern const char Y1[];
  extern const char X2[];
  extern const char Y2[];
  extern const char WIDTH[];
  extern const char HEIGHT[];
  extern const char SVG_CLASS[];

  extern const char RECT[];
  extern const char TEXT[];
  extern const char LINE[];

  template<const char *NAME, typename T>
  class Attribute
  {
  public:
    Attribute(T value) : value(value) {}

    void  print(std::ostream& out) const
    {
      out << ' ' << NAME << '=' << '\"' << value << '\"';
    }

  private:

    T value;
  };

  template<const char *NAME, typename T>
  std::ostream& operator << (std::ostream& out, const Attribute<NAME, T>& attribute)
  {
    attribute.print(out);
    return out;
  }

  using AttributeX = Attribute<X, int>;
  using AttributeY = Attribute<Y, int>;
  using AttributeX1 = Attribute<X1, int>;
  using AttributeY1 = Attribute<Y1, int>;
  using AttributeX2 = Attribute<X2, int>;
  using AttributeY2 = Attribute<Y2, int>;
  using AttributeWidth = Attribute<WIDTH, int>;
  using AttributeHeight = Attribute<HEIGHT, int>;
  using AttributeClass = Attribute<SVG_CLASS, const char *>;

  template<const char *NAME>
  class Element 
  {
  public:

    Element(const char *cssClass = nullptr) : cssClass(cssClass) {}

    void start(std::ostream& out) const
    {
      out << '<' << NAME;
      if( cssClass != nullptr)
        out << AttributeClass(cssClass);
    }

    void startClose(std::ostream& out) const
    {
      out << '/' << '>' << std::endl;
    }

    void end(std::ostream& out) const
    {
      out << '<' << '/' << NAME << '>' << std::endl;
    }

  private:
    const char *cssClass;
  };

  template<const char *NAME>
  class ElementXY : public Element<NAME>
  {
  public:
    ElementXY(int x, int y, const char *cssClass = nullptr) : Element<NAME>(cssClass), x(x), y(y) {}

    void printBegin(std::ostream& out) const
    {
      Element<NAME>::start(out);
      out << AttributeX(x) << AttributeY(y);
    }

  private:
    int x;
    int y;
  };

  class Rect : public ElementXY<RECT>
  {
  public:
    Rect(int x, int y, int w, int h, const char *cssClass = nullptr) : ElementXY<RECT>(x, y, cssClass), w(w), h(h) { }

    void print(std::ostream& out) const
    {
      ElementXY<RECT>::printBegin(out);
      out << AttributeWidth(w) << AttributeHeight(h);

      Element<RECT>::startClose(out);
    }

  private:

    int w;
    int h;
  };

  class Line : public Element<LINE>
  {
  public:
    Line(int x1, int y1, int x2, int y2, const char *cssClass = nullptr) : Element<LINE>(cssClass), x1(x1), y1(y1), x2(x2), y2(y2) { }

    void print(std::ostream& out) const
    {
      Element<LINE>::start(out);
      out << AttributeX1(x1) << AttributeY1(y1) << AttributeX2(x2) << AttributeY2(y2);

      Element<LINE>::startClose(out);
    }

  private:

    int x1;
    int y1;
    int x2;
    int y2;
  };
  class Text : public ElementXY<TEXT>
  {
  public:
    Text(int x, int y, const char *text, const char *cssClass = nullptr) : ElementXY<TEXT>(x, y, cssClass), text(text), cssClass(cssClass) { }
    Text(int x, int y, const std::string& text, const char *cssClass = nullptr) : Text(x, y, text.c_str(), cssClass) {}

    void print(std::ostream& out) const
    {
      ElementXY<TEXT>::printBegin(out);
      out << '>' << "<![CDATA[" << text << "]]>";

      Element<TEXT>::end(out);
    }

  private:

    const char *text;
    const char *cssClass;
  };
}

std::ostream& operator << (std::ostream& out, const svg::Rect& elem);
std::ostream& operator << (std::ostream& out, const svg::Text& elem);
std::ostream& operator << (std::ostream& out, const svg::Line& elem);

#endif // !_INCLUDE_SVG_HELPER_H_

