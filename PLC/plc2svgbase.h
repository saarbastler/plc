#ifndef _INCLUDE_PLC_2_SVG_BASE_H_
#define _INCLUDE_PLC_2_SVG_BASE_H_

#include "SvgOption.h"
#include "PlcAst.h"
#include "svgHelper.h"


class Plc2svgBase
{
public:

  Plc2svgBase(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<SVGOption> options) : plcAst(plcAst), out(out)
  {
    setupOptions(options.begin(), options.end());
  }

  template<typename AT>
  Plc2svgBase(const PlcAst& plcAst, std::ostream& out, const AT& options) : plcAst(plcAst), out(out)
  {
    setupOptions(options.begin(), options.end());
  }

protected:

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
  static const char *SVG_JS_FOOTER;

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

  void writeOutput()
  {
    out << SVG_HEADER;

    if (!hasOption(SVGOption::NoJavascript))
    {
      out << SVG_FUNCTIONS_JS_START
        << "this.svg = new SVGData(" << 1 + plcAst.maxVariableIndexOfType(Variable::Type::Input) << ", "
        << 1 + plcAst.maxVariableIndexOfType(Variable::Type::Output) << ", "
        << 1 + plcAst.maxVariableIndexOfType(Variable::Type::Monoflop) << ", "
        << 1 + plcAst.maxVariableIndexOfType(Variable::Type::Flag) << ", "
        << 1 + plc::Expression::lastId() << ", function(data) {" << std::endl
        << jsOut.str()
        << "});" << std::endl;

      if (!hasOption(SVGOption::NotInteractive))
      {
        out
          << "var that=this;" << std::endl
          << "this.toggleInput = function(event) { that.svg.toggleById(event.target.id); } " << std::endl;
        for (auto it = inputCountMap.begin(); it != inputCountMap.end(); it++)
          if (signalCrossing.find(it->first) == signalCrossing.end())
          {
            const Variable& variable = plcAst.getVariable(it->first);
            unsigned index = variable.index();

            out << "document.getElementById('" << variableTypeIdentifier(variable.type()) << index << "').addEventListener('click',that.toggleInput);" << std::endl;
          }
      }

      out << SVG_FUNCTIONS_JS_END;
    }

    out << SVG_JS_FOOTER
      << svgOut.str()
      << "</svg>"
      << std::endl;
  }

  static char variableTypeIdentifier(Variable::Type type)
  {
    switch (type)
    {
    case Variable::Type::Input:     return 'i';
    case Variable::Type::Output:    return 'o';
    case Variable::Type::Monoflop:  return 'm';
    case Variable::Type::Flag:      return 'f';
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
    if (expression.variable())
      return variableCssClass(*expression.variable());

    std::ostringstream out;
    out << 'g' << expression.id();
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

  struct COORD
  {
    unsigned x;
    unsigned y;
  };

  std::unordered_map<std::string, unsigned> inputCountMap;
  std::unordered_map<std::string, COORD> inputPosition;
  std::unordered_set<std::string> signalCrossing;

  std::ostringstream svgOut;
  std::ostringstream jsOut;

  const PlcAst& plcAst;
  std::ostream& out;

  std::string tmpCssClass;

private:

  unsigned optionBitvector;
};

#endif // _INCLUDE_PLC_2_SVG_BASE_H_

