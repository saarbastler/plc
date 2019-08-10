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

  /// <summary>
  /// SVG Options
  /// </summary>
  enum struct Option
  {
    // Non Interactive, means no Clickable Inputs
    NotInteractive,

    // No Javascript at all, includes NonInteractive
    NoJavascript
  };

  Plc2svg(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<Option> options) : plcAst(plcAst), out(out) 
  {
    setupOptions(options);
  }

  void convert(const plc::Expression& expression)
  {
    expression.countInputs(inputIdMap);

    unsigned crossings = 0;
    unsigned chars = 0;
    for (auto it = inputIdMap.begin(); it != inputIdMap.end(); it++)
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

    out << SVG_HEADER;

    if (!hasOption(Option::NoJavascript))
    {
      out << SVG_FUNCTIONS_JS_START
        << svg::array("i", inputIdMap.size())
        << svg::array("g", plc::Expression::lastId())
        << SVG_FUNCTIONS_A
        << jsOut.str()
        << SVG_FUNCTIONS_B;

      if (!hasOption(Option::NotInteractive))
      {
        out << SVG_FUNCTIONS_TOGGLE_INPUT;
        for (auto it = inputIdMap.begin(); it != inputIdMap.end(); it++)
        {
          unsigned index = plcAst.getVariable(it->first).index();

          out << "document.getElementById('" << index << "').addEventListener('click',toggleInput);" << std::endl;
        }
      }

      out << SVG_FUNCTIONS_JS_END;
    }

    out << SVG_FOOTER
      << svgOut.str()
      << "</svg>" 
      << std::endl;
  }


private:

  bool hasOption(Option option)
  {
    return optionBitvector & (1 << static_cast<unsigned>(option));
  }

  void setupOptions(const std::initializer_list<Option> options)
  {
    unsigned tmp = 0;
    for (auto it = options.begin(); it != options.end(); it++)
      tmp |= 1 << static_cast<unsigned>(*it);

    optionBitvector = tmp;
  }

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

  static const char *SVG_HEADER;
  static const char *SVG_FUNCTIONS_JS_START;
  static const char *SVG_FUNCTIONS_JS_END;
  static const char *SVG_FUNCTIONS_A;
  static const char *SVG_FUNCTIONS_B;
  static const char *SVG_FUNCTIONS_TOGGLE_INPUT;
  static const char *SVG_FOOTER;

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression, plc::Term::Unary unary= plc::Term::Unary::None)
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
        unsigned subSize = 2 + convert(index + ypos, level + 1, *term.expression(), term.unary());

        index += subSize;
        size += subSize;
      }
      break;

      //default:
      }
    }

    svgOut << svg::Rect(crossingWidth + width, ypos * CHAR_HEIGHT, GATE_WIDTH, (size + 1) * CHAR_HEIGHT, { BOX })
      << svg::Text(crossingWidth + width, (1 + ypos) * CHAR_HEIGHT, expression.op() == plc::Expression::Operator::And ? "&" : ">=1", {});

    if (unary == plc::Term::Unary::None)
      svgOut << svg::Line(crossingWidth + width + GATE_WIDTH, outy, crossingWidth + width + GATE_WIDTH + LINE_LENGTH, outy, { LINK, gateCssClass(expression) });
    else
      svgOut << svg::Line(crossingWidth + width + GATE_WIDTH, outy, crossingWidth + width + GATE_WIDTH + LINE_LENGTH - 2 * INVERT_RADIUS, outy, { LINK, gateCssClass(expression) })
      << svg::Circle(crossingWidth + width + GATE_WIDTH + LINE_LENGTH - INVERT_RADIUS, outy, INVERT_RADIUS, { INVERT, gateCssClass(expression) });



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

  std::unordered_map<std::string, unsigned> inputIdMap;
  std::unordered_map<std::string, COORD> inputPosition;

  unsigned optionBitvector;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

