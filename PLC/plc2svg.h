#ifndef _INCLUDE_PLC_2_SVG_H_
#define _INCLUDE_PLC_2_SVG_H_

#include <ostream>
#include <sstream>
#include <unordered_map>

#include "plc2svgbase.h"

class Plc2svg : public Plc2svgBase
{
public:

  Plc2svg(const PlcAst& plcAst, std::ostream& out, const std::initializer_list<SVGOption> options) : Plc2svgBase(plcAst, out, options)
  {
  }

  template<typename AT>
  Plc2svg(const PlcAst& plcAst, std::ostream& out, const AT& options) : Plc2svgBase(plcAst, out, options)
  {
  }

  void convertMultiple(const std::vector<std::string>& names)
  {
    std::unordered_map<std::string, plc::Expression> resolved;
    std::vector<const plc::Expression*> expressions;
    std::unordered_map<std::string,unsigned> toSkip;

    for (auto it = names.begin(); it != names.end(); it++)
    {
      resolved.emplace(*it, plcAst.resolveDependencies(*it, &toSkip));
      expressions.emplace_back(&resolved[*it]);
    }

    for (auto it = toSkip.begin(); it != toSkip.end(); it++)
      if (it->second > 1)
        signalCrossing.emplace(it->first);

    for (auto it = names.begin(); it != names.end(); it++)
      resolved[*it].countInputs(inputCountMap);

    setupParameter(expressions);

    unsigned y = 1;
    for (auto it = names.begin(); it != names.end(); it++)
      y+= 2 + convert(y, 0, resolved[*it], plc::Term::Unary::None, &plcAst.getVariable(*it));

    writeOutput();
  }

  void convert(const plc::Expression& expression, const std::string& name)
  {
    expression.countInputs(inputCountMap);

    std::array<const plc::Expression*, 1> a{ &expression };
    setupParameter(a);

    maxLevel = expression.countLevels();

    const Variable& variable = plcAst.getVariable(name);
    convert(1, 0, expression, plc::Term::Unary::None, &variable);

    writeOutput();
  }

private:

  void countLevelCrossings(unsigned level, const plc::Expression& expression)
  {
    while (crossingsPerLevel.size() <= level)
      crossingsPerLevel.emplace_back(0);

    if (expression.variable())
    {
      auto find = inputCountMap.find(expression.variable()->name());
      if (find != inputCountMap.end())
        ++crossingsPerLevel[level];
    }

    for (auto it = expression.terms().begin(); it != expression.terms().end(); it++)
      if (it->type() == plc::Term::Type::Expression)
        countLevelCrossings(level + 1, *it->expression());
  }

  template <typename T>
  void setupParameter(T v)
  {
    maxLevel = 0;
    for (auto it = v.begin(); it != v.end(); it++)
    {
      countLevelCrossings(0, **it);
      unsigned tmp = (*it)->countLevels();
      if (tmp > maxLevel)
        maxLevel = tmp;
    }

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

    crossingsPerLevel.emplace_back(crossings);

    textWidth = chars * CHAR_WIDTH;
  }

  unsigned convert(unsigned ypos, unsigned level, const plc::Expression& expression, plc::Term::Unary unary, const Variable *variable = nullptr)
  {
    //std::cout << "convert Expression " << expression.signalName() << " " << expression.id() << " ypos: " << ypos << " level: " << level << std::endl;
    
    unsigned size = 0;
    unsigned index = 1;
    unsigned width = textWidth + (maxLevel - level - 1) * (LINE_LENGTH + GATE_WIDTH) + LINE_LENGTH;
    unsigned outy = (index + ypos) * CHAR_HEIGHT;

    for (const plc::Term& term : expression.terms())
    {
      unsigned y = (index + ypos) * CHAR_HEIGHT;
      switch (term.type())
      {
      case plc::Term::Type::Identifier:
        convertInput(term, level, y, width);

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

    unsigned lineX1 = crossingWidthUpToLevel(level) + width;
    unsigned lineX2 = crossingWidthUpToLevel(level - 1) + width + GATE_WIDTH + LINE_LENGTH;

    if (unary != plc::Term::Unary::None)
      lineX2 -= 2 * INVERT_RADIUS;

    if (expression.op() != plc::Expression::Operator::None /*|| expression.terms().size() > 1*/)
    {
      svgOut << svg::Rect(lineX1, ypos * CHAR_HEIGHT, GATE_WIDTH, (size + 1) * CHAR_HEIGHT, { BOX })
        << svg::Text(lineX1 + CHAR_OFFSET, (1 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, operatorSymbol(expression.op()), {});

      if ( hasOption(SVGOption::BoxText))
      {
        std::ostringstream tid;
        tid << 't' << gateCssClass(expression);

        svgOut << svg::Text(lineX1 + CHAR_OFFSET, (2 + ypos) * CHAR_HEIGHT + CHAR_OFFSET_Y, "", {}, tid.str().c_str());
      }

      lineX1 += GATE_WIDTH;
    }
    
    if (variable)
    {
      unsigned len = unsigned(variable->name().length()) * CHAR_WIDTH;
      if (int(len) < lineX2 - lineX1)
        len = lineX2 - lineX1;

      //if(hasOption(SVGOption::NotInteractive))
        svgOut << svg::Line(lineX1, outy, lineX1 + len, outy, { LINK, variableCssClass(*variable) });
      //else
      //  svgOut << svg::Line(lineX1, outy, lineX1 + len, outy, { LINK, gateCssClass(expression) });

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

    if(expression.variable() && signalCrossing.find(expression.variable()->name()) != signalCrossing.end())
      inputPosition.emplace(expression.variable()->name(), COORD{ lineX1 + CROSSING_WIDTH, outy });

    expressionJsEquation(expression);

    return size;
  }

  unsigned crossingWidthUpToLevel(unsigned level)
  {
    unsigned crossingWidth = 0;
    for (int i = int(crossingsPerLevel.size()) - 1; i > int(level); i--)
      crossingWidth += crossingsPerLevel[i];

    return crossingWidth * CHAR_WIDTH;
  }

  void convertInput(const plc::Term& term, unsigned level, unsigned y, unsigned width)
  {
    const Variable& variable = *term.variable();

    unsigned x = XSTART;
    auto input = inputPosition.find(term.variable()->name());

    std::string cssClass(variableCssClass(variable));
    if (input == inputPosition.end() )
    {
      auto inputCount = inputCountMap.find(term.variable()->name());
      if (inputCount != inputCountMap.end() && inputCount->second > 1)
      {
        //std::cout << "crossing " << term.variable()->name() << ": " << crossingCount << std::endl;
        inputPosition.emplace(term.variable()->name(), COORD{ XSTART + textWidth + crossingCount * CROSSING_WIDTH, y });
        ++crossingCount;
      }

      svgOut << svg::Text(x, y + CHAR_OFFSET_Y, term.variable()->name(), { VARIABLE, cssClass.c_str() }, cssClass.c_str());
    }
    else
    {
      x = input->second.x;
      svgOut << svg::Circle(x, input->second.y, JOIN_RADIUS, { JOIN, cssClass.c_str() })
        << svg::Line(x, input->second.y, x, y, { LINK, cssClass.c_str() });

      input->second.y = y;
    }
    
    int lineX1 = crossingWidthUpToLevel(level) + width;

    if (term.unary() == plc::Term::Unary::None)
      svgOut << svg::Line(x, y, lineX1, y, { LINK, cssClass.c_str() });
    else
      svgOut << svg::Line(x, y, lineX1 - 2 * INVERT_RADIUS, y, { LINK, cssClass.c_str() })
      << svg::Circle(lineX1 - INVERT_RADIUS, y, INVERT_RADIUS, { INVERT, cssClass.c_str() });

    if (hasOption(SVGOption::LinkLabels))
      svgOut << svg::Text(lineX1 - 3 * CHAR_WIDTH, y + CHAR_OFFSET_Y, cssClass.c_str(), {});
  }
  
  unsigned jsType(const plc::Expression& expression) const
  {
    if(expression.variable())
    {
      return static_cast<unsigned>(expression.variable()->type());
    }
    else
    {
      return js::Intermediate;
    }
  }

  unsigned jsTypeIndex(const plc::Expression& expression) const
  {
    if (expression.variable())
    {
      return expression.variable()->index();
    }
    else
    {
      return expression.id();
    }
  }

  void expressionJsEquation(const plc::Expression& expression)
  {
    unsigned type = jsType(expression);
    if (!hasOption(SVGOption::NotInteractive) || 
      (type != static_cast<unsigned>(Variable::Type::Monoflop) && (type != static_cast<unsigned>(Variable::Type::Output))))
    {
      jsOut << "data[" << jsType(expression) << "][" << jsTypeIndex(expression) << "]= ";

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
          const Variable& v = plcAst.getVariable(t.variable()->name());
          jsOut << "data[" << static_cast<int>(v.type()) << "][" << v.index() << ']';
        }
        break;

        case plc::Term::Type::Expression:
          jsOut << "data[" << jsType(*t.expression()) << "][" << jsTypeIndex(*t.expression()) << ']';
          break;
        }
      }

      jsOut << ";" << std::endl;
    }
  }

  unsigned maxLevel = 0;
  unsigned crossingCount = 0;
  unsigned textWidth = 0;

  std::vector<unsigned> crossingsPerLevel;
};

#endif // !_INCLUDE_PLC_2_SVG_H_

