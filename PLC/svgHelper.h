#ifndef _INCLUDE_SVG_HELPER_H_
#define _INCLUDE_SVG_HELPER_H_

#include <cstring>
#include <iostream>
#include <initializer_list>

namespace svg
{
  extern const char X[];
  extern const char Y[];
  extern const char X1[];
  extern const char Y1[];
  extern const char X2[];
  extern const char Y2[];
  extern const char CX[];
  extern const char CY[];
  extern const char R[];
  extern const char WIDTH[];
  extern const char HEIGHT[];
  extern const char SVG_CLASS[];
  extern const char ID[];

  extern const char RECT[];
  extern const char TEXT[];
  extern const char LINE[];
  extern const char CIRCLE[];

  constexpr const unsigned CHAR_CELL_WIDTH = 10;
  constexpr const unsigned CHAR_CELL_HEIGHT = 20;

  constexpr const char *FALSE = "false";
  constexpr const char *TRUE = "true";
  
  class AreaSize
  {
  public:

    static void setMaxXY(int x, int y)
    {
      if( x > xMax())
        xMax() = x;

      if(y > yMax())
        yMax() = y;
    }

    static int x()
    {
      return xMax();
    }

    static int y()
    {
      return yMax();
    }

    static void clear()
    {
      xMax() = 0;
      yMax() = 0;
    }

  private:

    static int& xMax()
    {
      static int x = 0;

      return x;
    }

    static int& yMax()
    {
      static int y = 0;

      return y;
    }
  };

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
  using AttributeCX = Attribute<CX, int>;
  using AttributeCY = Attribute<CY, int>;
  using AttributeR = Attribute<R, int>;
  using AttributeWidth = Attribute<WIDTH, int>;
  using AttributeHeight = Attribute<HEIGHT, int>;
  using AttributeClass = Attribute<SVG_CLASS, const char *>;
  using AttributeId = Attribute<ID, const char *>;

  template<const char *NAME>
  class Element 
  {
  public:

    Element(const std::initializer_list<const char *>& cssClass, const char *id= nullptr) : cssClass(cssClass), id(id) {}

    void start(std::ostream& out) const
    {
      out << '<' << NAME;

      if (id != nullptr)
        out << AttributeId(id);

      if (cssClass.size() > 0)
      {
        out << ' ' << SVG_CLASS << '=' << '\"';

        bool first = true;
        for (auto it : cssClass)
        {
          if (first)
            first = false;
          else
            out << ' ';

          out << it;
        }

        out << '\"';
      }
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
    const std::initializer_list<const char *>& cssClass;
    const char *id = nullptr;
  };

  template<const char *NAME>
  class ElementXY : public Element<NAME>
  {
  public:
    ElementXY(int x, int y, const std::initializer_list<const char *>& cssClass, const char *id = nullptr) 
      : Element<NAME>(cssClass, id), x(x), y(y) {}

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
    Rect(int x, int y, int w, int h, const std::initializer_list<const char *>& cssClass, const char *id = nullptr) 
      : ElementXY<RECT>(x, y, cssClass, id), w(w), h(h) 
    { 
      AreaSize::setMaxXY(x + w, y + h);
    }

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
    Line(int x1, int y1, int x2, int y2, const std::initializer_list<const char *>& cssClass, const char *id = nullptr) 
      : Element<LINE>(cssClass, id), x1(x1), y1(y1), x2(x2), y2(y2) 
    {
      AreaSize::setMaxXY(x2, y2);
    }

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

  class Circle : public Element<CIRCLE>
  {
  public:
    Circle(int cx, int cy, int r, const std::initializer_list<const char *>& cssClass, const char *id = nullptr) 
      : Element<CIRCLE>(cssClass, id), cx(cx), cy(cy), r(r) 
    {
      AreaSize::setMaxXY(cx + r, cy + r);
    }

    void print(std::ostream& out) const
    {
      Element<CIRCLE>::start(out);
      out << AttributeCX(cx) << AttributeCY(cy) << AttributeR(r);

      Element<CIRCLE>::startClose(out);
    }

  private:

    int cx;
    int cy;
    int r;
  };

  class Text : public ElementXY<TEXT>
  {
  public:
    Text(int x, int y, const char *text, const std::initializer_list<const char *>& cssClass, const char *id = nullptr) 
      : ElementXY<TEXT>(x, y, cssClass, id), text(text) 
    {
      AreaSize::setMaxXY(x + int(strlen(text)) * CHAR_CELL_WIDTH, y + CHAR_CELL_HEIGHT);
    }

    Text(int x, int y, const std::string& text, const std::initializer_list<const char *>& cssClass, const char *id = nullptr) 
      : Text(x, y, text.c_str(), cssClass, id) {}

    void print(std::ostream& out) const
    {
      ElementXY<TEXT>::printBegin(out);
      out << '>' << "<![CDATA[" << text << "]]>";

      Element<TEXT>::end(out);
    }

  private:

    const char *text;
  };

  struct repeat
  {
    repeat(size_t count, const char *text) : count(count), text(text) {}

    friend std::ostream& operator<<(std::ostream& out, const repeat& r)
    {
      for (unsigned i = 0; i < r.count; i++)
        out << r.text;

      return out;
    }

    size_t count;
    const char *text;
  };

  struct array
  {
    array(const char *name, size_t size, const char *defaultValue = FALSE) : name(name), size(size), defaultValue(defaultValue) {}

    friend std::ostream& operator<<(std::ostream& out, const array& a)
    {
      out << "var " << a.name << "= Array(" << a.size << ").fill(" << a.defaultValue<< ");" << std::endl;

      return out;
    }

    const char *name;
    size_t size;
    const char *defaultValue;
  };
}

std::ostream& operator << (std::ostream& out, const svg::Rect& elem);
std::ostream& operator << (std::ostream& out, const svg::Text& elem);
std::ostream& operator << (std::ostream& out, const svg::Line& elem);
std::ostream& operator << (std::ostream& out, const svg::Circle& elem);

#endif // !_INCLUDE_SVG_HELPER_H_

