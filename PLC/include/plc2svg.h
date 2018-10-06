#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>
#include <sstream>
#include <unordered_map>

#include "PlcAst.h"
#include "svgHelper.h"

class Plc2svg
{
public:

  Plc2svg(const PlcAst& plcAst, std::ostream& out) : plcAst(plcAst), out(out) {}

  void convert(const plc::Expression& expression)
  {
    std::unordered_map<std::string, unsigned> inputs;
    expression.countInputs(inputs);

    unsigned crossings = 0;
    unsigned chars = 0;
    for (auto it = inputs.begin(); it != inputs.end(); it++)
    {
      unsigned width = unsigned(it->first.length());
      if (width > chars)
        chars = width;

      if (it->second > 1)
        crossings++;
    }

    textWidth = chars * CHAR_WIDTH;
    crossingWidth = crossings * CROSSING_WIDTH;
    maxLevel = expression.countLevels();

    convert(1, 1, expression);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl
      << "<?xml-stylesheet type=\"text/css\" href=\"plc.css\"?>" << std::endl
      << "<svg xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns=\"http://www.w3.org/2000/svg\" " //xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
      "width=\"800\" height=\"800\">" << std::endl
      << "<defs>" << std::endl;

    //out << "<script type = \"text/javascript\" xlink:href = \"plc.js\" xlink:actuate=\"onLoad\" xlink:show=\"other\" xlink:type=\"simple\"/>" << std::endl

    /*out << "<style type=\"text/css\">" << std::endl
      << "<![CDATA[" << std::endl
      << "rect.box { stroke:#000; fill:none; }" << std::endl
      << "circle.invert { stroke:#000; fill:none; }" << std::endl
      << "circle.join { stroke:#000; fill:#000; }" << std::endl
      << "line.link { stroke:#000; stroke-width:1px; }" << std::endl
      << "line.on { stroke:#0f0; }" << std::endl

      << "line.test { stroke:#f00; }" << std::endl

      << "]]>" << std::endl
      << "</style>" << std::endl;*/

    out << "<script type=\"text/javascript\"><![CDATA[" << std::endl
      << "document.addEventListener(\"DOMContentLoaded\", function ()" << std::endl
      << "{" << std::endl
      << "var i=new Array(";

    for (auto it = inputs.begin(); it != inputs.end(); it++)
      out << "false,";
    out << ");" << std::endl
      << "var g=new Array(";
    for (unsigned i = 0; i <= plc::Expression::lastId(); i++)
      out << "false,";
    out << ");" << std::endl;

    out << "function updateElement(name,value)" << std::endl
      << "{" << std::endl
      << "  document.querySelectorAll('.' + name).forEach( function(obj)" << std::endl
      << "  {" << std::endl
      << "    if(value)" << std::endl
      << "      obj.classList.add('on');" << std::endl
      << "    else" << std::endl
      << "      obj.classList.remove('on');" << std::endl
      << "  });" << std::endl
      << "}" << std::endl;

    out << "function logic()" << std::endl
      << "{" << std::endl
      << jsOut.str()
      << "i.forEach(function(v,i) { updateElement('i' + i, v);} );" << std::endl
      << "g.forEach(function(v,i) { updateElement('g' + i, v);} );" << std::endl
      << "}" << std::endl;


    out << "function toggleInput(event)" << std::endl
      << "{" << std::endl
      << "  let index = event.target.id;" << std::endl
      << "  i[index] = !i[index];" << std::endl
      << "  logic();" << std::endl
      << "  console.log('i[' + index + ']=' + i[index]);" << std::endl
      << "}" << std::endl;

    for (auto it = inputs.begin(); it != inputs.end(); it++)
    {
      unsigned index = plcAst.getVariable(it->first).index();

      out << "document.getElementById('" << index << "').addEventListener('click',toggleInput);" << std::endl;
    }

    out << "});" << std::endl
      << "]]></script>" << std::endl
      << "</defs>" << std::endl
      << svgOut.str()
      << "</svg>" << std::endl;
  }


private:

  static constexpr const char *BOX = "box";
  static constexpr const char *VARIABLE = "variable";
  static constexpr const char *LINK = "link";
  static constexpr const char *INVERT = "invert";
  static constexpr const char *JOIN = "join";

  static constexpr const unsigned XSTART = 10;
  static constexpr const unsigned CHAR_HEIGHT = 20;
  static constexpr const unsigned CHAR_WIDTH = 10;
  static constexpr const unsigned LINE_LENGTH = 50;
  static constexpr const unsigned GATE_WIDTH = 75;
  static constexpr const unsigned INVERT_RADIUS = 4;
  static constexpr const unsigned CROSSING_WIDTH = 10;
  static constexpr const unsigned JOIN_RADIUS = 2;

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression)
  {
    unsigned size = 0;
    unsigned index = 1;
    unsigned width = textWidth + (maxLevel - level) * (LINE_LENGTH + GATE_WIDTH) + LINE_LENGTH;
    unsigned outy = (index + ypos) * CHAR_HEIGHT;

    for (const plc::Term& term : expression.terms())
    {
      unsigned y = (index + ypos) * CHAR_HEIGHT;

      switch (term.type())
      {
      case plc::Term::Type::Identifier:
        convertInput(term, y, width);

        index++;
        size++;
        break;

      case plc::Term::Type::Expression:
      {
        unsigned subSize = 2 + convert(index + ypos, level + 1, *term.expression());

        index += subSize;
        size += subSize;
      }
      break;

      //default:
      }
    }

    svgOut << svg::Rect(crossingWidth + width, ypos * CHAR_HEIGHT, GATE_WIDTH, (size + 1) * CHAR_HEIGHT, { BOX })
      << svg::Text(crossingWidth + width, (1 + ypos) * CHAR_HEIGHT, expression.op() == plc::Expression::Operator::And ? "&" : ">=1", {})

      << svg::Line(crossingWidth + width + GATE_WIDTH, outy, crossingWidth + width + GATE_WIDTH + LINE_LENGTH, outy, { LINK, gateCssClass(expression) });

    expressionJsEquation(expression);

    return size;
  }

  void convertInput(const plc::Term& term, unsigned y, unsigned width)
  {
    const Variable& variable = plcAst.getVariable(term.identifier());

    unsigned x = XSTART;
    auto input = inputPosition.find(term.identifier());
    if (input == inputPosition.end())
    {
      crossingCount++;
      inputPosition.emplace(term.identifier(), COORD{ XSTART + textWidth + crossingCount * CROSSING_WIDTH, y });

      std::ostringstream id;
      id << variable.index();
      svgOut << svg::Text(x, y, term.identifier(), { VARIABLE, variableCssClass(variable) }, id.str().c_str());
    }
    else
    {
      x = input->second.x;
      svgOut << svg::Circle(x, input->second.y, JOIN_RADIUS, { JOIN, variableCssClass(variable) })
        << svg::Line(x, input->second.y, x, y, { LINK, variableCssClass(variable) });

      input->second.y = y;
    }

    if (term.unary() == plc::Term::Unary::None)
      svgOut << svg::Line(x, y, crossingWidth + width, y, { LINK, variableCssClass(variable) });
    else
      svgOut << svg::Line(x, y, crossingWidth + width - 2 * INVERT_RADIUS, y, { LINK, variableCssClass(variable) })
      << svg::Circle(crossingWidth + width - INVERT_RADIUS, y, INVERT_RADIUS, { INVERT, variableCssClass(variable) });
  }
  
  static char variableTypeIdentifier(Variable::Type type)
  {
    switch (type)
    {
    case Variable::Type::Input:     return 'i';
    case Variable::Type::Output:    return 'o';
    case Variable::Type::Monoflop:  return 'm';
    default:
      throw PlcAstException("undefined Variable Type: %d", int(type));
    }
  }

  const char *variableCssClass(const Variable& variable)
  {
    std::ostringstream out;

    out << variableTypeIdentifier(variable.type()) << variable.index();
    tmpCssClass = out.str();

    return tmpCssClass.c_str();
  }

  const char *gateCssClass(const plc::Expression& expression)
  {
    std::ostringstream out;
    out << 'g' << expression.id();
    tmpCssClass = out.str();

    return tmpCssClass.c_str();
  }

  std::string tmpCssClass;

  void expressionJsEquation(const plc::Expression& expression)
  {
    jsOut << "g[" << expression.id() << "]= ";

    bool first = true;
    char opChar = (expression.op() == plc::Expression::Operator::And) ? '&' : '|';
    for (const plc::Term& t : expression.terms())
    {
      if (first)
        first = false;
      else
        jsOut << ' ' << opChar << ' ';

      if (t.unary() == plc::Term::Unary::Not)
        jsOut << '!';

      switch (t.type())
      {
      case plc::Term::Type::Identifier:
      {
        const Variable& v = plcAst.getVariable(t.identifier());
        jsOut << variableTypeIdentifier(v.type()) << '[' << v.index() << ']';
      }
      break;

      case plc::Term::Type::Expression:
        jsOut << "g[" << t.expression()->id() << ']';
        break;
      }
    }

    jsOut << ";" << std::endl;
  }

  unsigned maxLevel = 0;
  unsigned crossingWidth = 0;
  unsigned crossingCount = 0;
  unsigned textWidth = 0;

  const PlcAst& plcAst;
  std::ostream& out;

  std::ostringstream svgOut;
  std::ostringstream jsOut;

  struct COORD
  {
    unsigned x;
    unsigned y;
  };

  std::unordered_map<std::string, COORD> inputPosition;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

