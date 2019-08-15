#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>
#include <sstream>
#include <unordered_map>

#include "PlcAst.h"
#include "svgHelper.h"
#include "SvgOption.h"

class Plc2svg
{
public:

  Plc2svg(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<SVGOption> options) : plcAst(plcAst), out(out) 
  {
    setupOptions(options.begin(), options.end());
  }

  template<typename AT>
  Plc2svg(const PlcAst& plcAst, std::ostream& out, const AT& options) : plcAst(plcAst), out(out)
  {
    setupOptions(options.begin(), options.end());
  }

  void convert(const plc::Expression& expression, const std::string& name)
  {
    expression.countInputs(inputCountMap);

    unsigned crossings = 0;
    unsigned chars = 0;
    for (auto it = inputCountMap.begin(); it != inputCountMap.end(); it++)
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

    const Variable& variable = plcAst.getVariable(name);
    convert(1, 1, expression, plc::Term::Unary::None, &variable);

    out << SVG_HEADER;

    if (!hasOption(SVGOption::NoJavascript))
    {
      out << SVG_FUNCTIONS_JS_START
        << "this.svg = new SVGData(" << plcAst.countVariableOfType(Variable::Type::Input) << ", "
        << plcAst.countVariableOfType(Variable::Type::Output) << ", "
        << plcAst.countVariableOfType(Variable::Type::Monoflop) << ", "
        << plc::Expression::lastId() << ", function(data) {"
        << jsOut.str()
        << "});" << std::endl;

      if (!hasOption(SVGOption::NotInteractive))
      {
        out
          << "var that=this;" << std::endl
        << "this.toggleInput = function(event) { that.svg.toggleById(event.target.id); } " << std::endl;
        for (auto it = inputCountMap.begin(); it != inputCountMap.end(); it++)
        {
          const Variable& variable = plcAst.getVariable(it->first);
          unsigned index = variable.index();

          out << "document.getElementById('" << variableTypeIdentifier(variable.type()) << index << "').addEventListener('click',that.toggleInput);" << std::endl;
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

  bool hasOption(SVGOption option)
  {
    return optionBitvector & (1 << static_cast<unsigned>(option));
  }

  template <typename Iterator>
  void setupOptions(Iterator begin, Iterator end)
  {
    unsigned tmp = 0;
    for (auto it = begin; it != end; it++)
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
  static constexpr const unsigned CHAR_OFFSET = 5;
  static constexpr const int CHAR_OFFSET_Y = -2;
  static constexpr const unsigned LINE_LENGTH = 50;
  static constexpr const unsigned GATE_WIDTH = 75;
  static constexpr const unsigned INVERT_RADIUS = 4;
  static constexpr const unsigned CROSSING_WIDTH = 10;
  static constexpr const unsigned JOIN_RADIUS = 2;

  static const char *SVG_HEADER;
  static const char *SVG_FUNCTIONS_JS_START;
  static const char *SVG_FUNCTIONS_DOM_CONTENT_LOADED;
  static const char *SVG_FUNCTIONS_JS_END;
  static const char *SVG_FUNCTIONS_A;
  static const char *SVG_FUNCTIONS_B;
  static const char *SVG_FUNCTIONS_TOGGLE_INPUT;
  static const char *SVG_FOOTER;

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression, plc::Term::Unary unary, const Variable *variable = nullptr)
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

    int lineX1 = crossingWidth + width;
    int lineX2 = crossingWidth + width + GATE_WIDTH + LINE_LENGTH;

    if (unary != plc::Term::Unary::None)
      lineX2 -= 2 * INVERT_RADIUS;

    if (expression.op() != plc::Expression::Operator::None || expression.terms().size() > 1)
    {
      lineX1 += GATE_WIDTH;
      svgOut << svg::Rect(crossingWidth + width, ypos * CHAR_HEIGHT, GATE_WIDTH, (size + 1) * CHAR_HEIGHT, { BOX })
        << svg::Text(crossingWidth + width + CHAR_OFFSET, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, operatorSymbol(expression.op()), {});

      if ( hasOption(SVGOption::BoxText))
      {
        std::ostringstream tid;
        tid << 't' << gateCssClass(expression);

        svgOut << svg::Text(crossingWidth + width + CHAR_OFFSET, (2 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, "", {}, tid.str().c_str());
      }
    }
    
    if (variable)
    {
      unsigned len = unsigned(variable->name().length()) * CHAR_WIDTH;
      if (int(len) < lineX2 - lineX1)
        len = lineX2 - lineX1;

      if(hasOption(SVGOption::NotInteractive))
        svgOut << svg::Line(lineX1, outy, lineX1 + len, outy, { LINK, variableCssClass(*variable) });
      else
        svgOut << svg::Line(lineX1, outy, lineX1 + len, outy, { LINK, gateCssClass(expression) });

      svgOut << svg::Text(lineX1 + CHAR_OFFSET, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, variable->name().c_str(), {});
    }
    else
    {
      svgOut << svg::Line(lineX1, outy, lineX2, outy, { LINK, gateCssClass(expression) });
      if (unary != plc::Term::Unary::None)
        svgOut << svg::Circle(lineX2 + INVERT_RADIUS, outy, INVERT_RADIUS, { INVERT, gateCssClass(expression) });

      if(hasOption(SVGOption::LinkLabels))
        svgOut << svg::Text(lineX2 - 3 * CHAR_WIDTH, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, gateCssClass(expression), {});
    }

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
      id << variableTypeIdentifier(variable.type()) << variable.index();
      svgOut << svg::Text(x, y + CHAR_OFFSET_Y, term.identifier(), { VARIABLE, variableCssClass(variable) }, id.str().c_str());
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

    if (hasOption(SVGOption::LinkLabels))
      svgOut << svg::Text(crossingWidth + width - 3 * CHAR_WIDTH, y + CHAR_OFFSET_Y, variableCssClass(variable), {});
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
    out << (expression.op() == plc::Expression::Operator::Timer ? 'm' : 'g') << expression.id();
    tmpCssClass = out.str();

    return tmpCssClass.c_str();
  }

  const char *operatorSymbol(plc::Expression::Operator op)
  {
    switch (op)
    {
    case plc::Expression::Operator::And:
      return "&";
    case plc::Expression::Operator::Or:
      return "\xE2\x89\xA5 1";
    case plc::Expression::Operator::Timer:
      return "\xE2\xAD\xB2 t";
    default:
      return "?";
    }
  }
  std::string tmpCssClass;

  void expressionJsEquation(const plc::Expression& expression)
  {
    if (!hasOption(SVGOption::NotInteractive) || expression.op() != plc::Expression::Operator::Timer)
    {
      jsOut << "data[" << expression.jsTypeIndex() << "][" << expression.id() << "]= ";

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
          jsOut << "data[" << static_cast<int>(v.type()) << "][" << v.index() << ']';
        }
        break;

        case plc::Term::Type::Expression:
          jsOut << "data[" << t.expression()->jsTypeIndex() << "][" << t.expression()->id() << ']';
          break;
        }
      }

      jsOut << ";" << std::endl;
    }
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

  std::unordered_map<std::string, unsigned> inputCountMap;
  std::unordered_map<std::string, COORD> inputPosition;

  unsigned optionBitvector;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

